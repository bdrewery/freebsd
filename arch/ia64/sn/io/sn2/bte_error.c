/*
 *
 *
 * Copyright (c) 2000-2003 Silicon Graphics, Inc.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of version 2 of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it would be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * 
 * Further, this software is distributed without any warranty that it is 
 * free of the rightful claim of any third person regarding infringement 
 * or the like.  Any license provided herein, whether implied or 
 * otherwise, applies only to this software file.  Patent licenses, if 
 * any, provided herein do not apply to combinations of this program with 
 * other software, or any other product whatsoever.
 * 
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 * 
 * Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pkwy, 
 * Mountain View, CA  94043, or:
 * 
 * http://www.sgi.com 
 * 
 * For further information regarding this notice, see: 
 * 
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan
 */


#include <linux/types.h>
#include <linux/slab.h>
#include <asm/smp.h>
#include <asm/sn/sgi.h>
#include <asm/sn/io.h>
#include <asm/sn/iograph.h>
#include <asm/sn/invent.h>
#include <asm/sn/hcl.h>
#include <asm/sn/labelcl.h>
#include <asm/sn/sn_private.h>
#include <asm/sn/klconfig.h>
#include <asm/sn/sn_cpuid.h>
#include <asm/sn/pci/pciio.h>
#include <asm/sn/pci/pcibr.h>
#include <asm/sn/xtalk/xtalk.h>
#include <asm/sn/pci/pcibr_private.h>
#include <asm/sn/intr.h>
#include <asm/sn/ioerror.h>
#include <asm/sn/sn2/shubio.h>
#include <asm/sn/bte.h>


/*
 * Bte error handling is done in two parts.  The first captures
 * any crb related errors.  Since there can be multiple crbs per
 * interface and multiple interfaces active, we need to wait until
 * all active crbs are completed.  This is the first job of the
 * second part error handler.  When all bte related CRBs are cleanly
 * completed, it resets the interfaces and gets them ready for new
 * transfers to be queued.
 */


void bte_error_handler(unsigned long);


/*
 * First part error handler.  This is called whenever any error CRB interrupt
 * is generated by the II.
 */
void
bte_crb_error_handler(vertex_hdl_t hub_v, int btenum,
		      int crbnum, ioerror_t * ioe, int bteop)
{
	hubinfo_t hinfo;
	struct bteinfo_s *bte;


	hubinfo_get(hub_v, &hinfo);
	bte = &hinfo->h_nodepda->bte_if[btenum];

	/*
	 * The caller has already figured out the error type, we save that
	 * in the bte handle structure for the thread excercising the
	 * interface to consume.
	 */
	switch (ioe->ie_errortype) {
	case IIO_ICRB_ECODE_PERR:
		bte->bh_error = BTEFAIL_POISON;
		break;
	case IIO_ICRB_ECODE_WERR:
		bte->bh_error = BTEFAIL_PROT;
		break;
	case IIO_ICRB_ECODE_AERR:
		bte->bh_error = BTEFAIL_ACCESS;
		break;
	case IIO_ICRB_ECODE_TOUT:
		bte->bh_error = BTEFAIL_TOUT;
		break;
	case IIO_ICRB_ECODE_XTERR:
		bte->bh_error = BTEFAIL_XTERR;
		break;
	case IIO_ICRB_ECODE_DERR:
		bte->bh_error = BTEFAIL_DIR;
		break;
	case IIO_ICRB_ECODE_PWERR:
	case IIO_ICRB_ECODE_PRERR:
		/* NO BREAK */
	default:
		bte->bh_error = BTEFAIL_ERROR;
	}

	bte->bte_error_count++;

	BTE_PRINTK(("Got an error on cnode %d bte %d\n",
		    bte->bte_cnode, bte->bte_num));
	bte_error_handler((unsigned long) hinfo->h_nodepda);
}


/*
 * Second part error handler.  Wait until all BTE related CRBs are completed
 * and then reset the interfaces.
 */
void
bte_error_handler(unsigned long _nodepda)
{
	struct nodepda_s *err_nodepda = (struct nodepda_s *) _nodepda;
	spinlock_t *recovery_lock = &err_nodepda->bte_recovery_lock;
	struct timer_list *recovery_timer = &err_nodepda->bte_recovery_timer;
	nasid_t nasid;
	int i;
	int valid_crbs;
	unsigned long irq_flags;
	volatile u64 *notify;
	bte_result_t bh_error;
	ii_imem_u_t imem;	/* II IMEM Register */
	ii_icrb0_d_u_t icrbd;	/* II CRB Register D */
	ii_ibcr_u_t ibcr;
	ii_icmr_u_t icmr;


	BTE_PRINTK(("bte_error_handler(%p) - %d\n", err_nodepda,
		    smp_processor_id()));

	spin_lock_irqsave(recovery_lock, irq_flags);

	if ((err_nodepda->bte_if[0].bh_error == BTE_SUCCESS) &&
	    (err_nodepda->bte_if[1].bh_error == BTE_SUCCESS)) {
		BTE_PRINTK(("eh:%p:%d Nothing to do.\n", err_nodepda,
			    smp_processor_id()));
		spin_unlock_irqrestore(recovery_lock, irq_flags);
		return;
	}
	/*
	 * Lock all interfaces on this node to prevent new transfers
	 * from being queued.
	 */
	for (i = 0; i < BTES_PER_NODE; i++) {
		if (err_nodepda->bte_if[i].cleanup_active) {
			continue;
		}
		spin_lock(&err_nodepda->bte_if[i].spinlock);
		BTE_PRINTK(("eh:%p:%d locked %d\n", err_nodepda,
			    smp_processor_id(), i));
		err_nodepda->bte_if[i].cleanup_active = 1;
	}

	/* Determine information about our hub */
	nasid = cnodeid_to_nasid(err_nodepda->bte_if[0].bte_cnode);


	/*
	 * A BTE transfer can use multiple CRBs.  We need to make sure
	 * that all the BTE CRBs are complete (or timed out) before
	 * attempting to clean up the error.  Resetting the BTE while
	 * there are still BTE CRBs active will hang the BTE.
	 * We should look at all the CRBs to see if they are allocated
	 * to the BTE and see if they are still active.  When none
	 * are active, we can continue with the cleanup.
	 *
	 * We also want to make sure that the local NI port is up.
	 * When a router resets the NI port can go down, while it
	 * goes through the LLP handshake, but then comes back up.
	 */
	icmr.ii_icmr_regval = REMOTE_HUB_L(nasid, IIO_ICMR);
	if (icmr.ii_icmr_fld_s.i_crb_mark != 0) {
		/*
		 * There are errors which still need to be cleaned up by
		 * hubiio_crb_error_handler
		 */
		mod_timer(recovery_timer, HZ * 5);
		BTE_PRINTK(("eh:%p:%d Marked Giving up\n", err_nodepda,
			    smp_processor_id()));
		spin_unlock_irqrestore(recovery_lock, irq_flags);
		return;
	}
	if (icmr.ii_icmr_fld_s.i_crb_vld != 0) {

		valid_crbs = icmr.ii_icmr_fld_s.i_crb_vld;

		for (i = 0; i < IIO_NUM_CRBS; i++) {
			if (!((1 << i) & valid_crbs)) {
				/* This crb was not marked as valid, ignore */
				continue;
			}
			icrbd.ii_icrb0_d_regval =
			    REMOTE_HUB_L(nasid, IIO_ICRB_D(i));
			if (icrbd.d_bteop) {
				mod_timer(recovery_timer, HZ * 5);
				BTE_PRINTK(("eh:%p:%d Valid %d, Giving up\n",
					 err_nodepda, smp_processor_id(), i));
				spin_unlock_irqrestore(recovery_lock,
						       irq_flags);
				return;
			}
		}
	}


	BTE_PRINTK(("eh:%p:%d Cleaning up\n", err_nodepda,
		    smp_processor_id()));
	/* Reenable both bte interfaces */
	imem.ii_imem_regval = REMOTE_HUB_L(nasid, IIO_IMEM);
	imem.ii_imem_fld_s.i_b0_esd = imem.ii_imem_fld_s.i_b1_esd = 1;
	REMOTE_HUB_S(nasid, IIO_IMEM, imem.ii_imem_regval);

	/* Reinitialize both BTE state machines. */
	ibcr.ii_ibcr_regval = REMOTE_HUB_L(nasid, IIO_IBCR);
	ibcr.ii_ibcr_fld_s.i_soft_reset = 1;
	REMOTE_HUB_S(nasid, IIO_IBCR, ibcr.ii_ibcr_regval);


	for (i = 0; i < BTES_PER_NODE; i++) {
		bh_error = err_nodepda->bte_if[i].bh_error;
		if (bh_error != BTE_SUCCESS) {
			/* There is an error which needs to be notified */
			notify = err_nodepda->bte_if[i].most_rcnt_na;
			BTE_PRINTK(("cnode %d bte %d error=0x%lx\n",
				    err_nodepda->bte_if[i].bte_cnode,
				    err_nodepda->bte_if[i].bte_num,
				    IBLS_ERROR | (u64) bh_error));
			*notify = IBLS_ERROR | bh_error;
			err_nodepda->bte_if[i].bh_error = BTE_SUCCESS;
		}

		err_nodepda->bte_if[i].cleanup_active = 0;
		BTE_PRINTK(("eh:%p:%d Unlocked %d\n", err_nodepda,
			    smp_processor_id(), i));
		spin_unlock(&pda.cpu_bte_if[i]->spinlock);
	}

	del_timer(recovery_timer);

	spin_unlock_irqrestore(recovery_lock, irq_flags);
}
