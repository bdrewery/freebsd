/*
 *		arch/parisc/kernel/pdc_chassis.c
 *
 * 		Copyright (C) 2002 Laurent Canet <canetl@esiee.fr>
 *		Copyright (C) 2002 Thibaut Varene <varenet@esiee.fr>
 *
 *
 *		This program is free software; you can redistribute it and/or modify
 *		it under the terms of the GNU General Public License as published by
 *		the Free Software Foundation; either version 2, or (at your option)
 *		any later version.
 *      
 *		This program is distributed in the hope that it will be useful,
 *		but WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *		GNU General Public License for more details.
 *      
 *		You should have received a copy of the GNU General Public License
 *		along with this program; if not, write to the Free Software
 *		Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#undef PDC_CHASSIS_DEBUG
#ifdef PDC_CHASSIS_DEBUG
#define DPRINTK(fmt, args...)	printk(fmt, ## args)
#else
#define DPRINTK(fmt, args...)
#endif

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/notifier.h>

#include <asm/pdc_chassis.h>
#include <asm/processor.h>

static int pdc_chassis_old = 0;	


/** 
 * pdc_chassis_checkold() - Checks for old PDC_CHASSIS compatibility
 * @pdc_chassis_old: 1 if old pdc chassis style
 * 
 * Currently, only E class and A180 are known to work with this.
 * Inspired by Christoph Plattner
 */

static void __init pdc_chassis_checkold(void)
{
	switch(CPU_HVERSION) {
		case 0x480:		/* E25 */
		case 0x481:		/* E35 */
		case 0x482:		/* E45 */
		case 0x483:		/* E55 */
		case 0x516:		/* A180 */
			pdc_chassis_old = 1;
			break;

		default:
			break;
	}
	DPRINTK(KERN_DEBUG "%s: pdc_chassis_checkold(); pdc_chassis_old = %d\n", __FILE__, pdc_chassis_old);
}


/**
 * pdc_chassis_panic_event() - Called by the panic handler.
 *
 * As soon as a panic occurs, we should inform the PDC.
 */

static int pdc_chassis_panic_event(struct notifier_block *this,
		        unsigned long event, void *ptr)
{
	pdc_chassis_send_status(PDC_CHASSIS_DIRECT_PANIC);
		return NOTIFY_DONE;
}   


static struct notifier_block pdc_chassis_panic_block = {
	notifier_call: pdc_chassis_panic_event,
	priority: INT_MAX,
};


/**
 * parisc_reboot_event() - Called by the reboot handler.
 *
 * As soon as a reboot occurs, we should inform the PDC.
 */

static int pdc_chassis_reboot_event(struct notifier_block *this,
		        unsigned long event, void *ptr)
{
	pdc_chassis_send_status(PDC_CHASSIS_DIRECT_SHUTDOWN);
		return NOTIFY_DONE;
}   


static struct notifier_block pdc_chassis_reboot_block = {
	notifier_call: pdc_chassis_reboot_event,
	priority: INT_MAX,
};


/**
 * parisc_pdc_chassis_init() - Called at boot time.
 */

void __init parisc_pdc_chassis_init(void)
{
	DPRINTK(KERN_DEBUG "%s: parisc_pdc_chassis_init()\n", __FILE__);

	/* initialize panic notifier chain */
	notifier_chain_register(&panic_notifier_list, &pdc_chassis_panic_block);

	/* initialize reboot notifier chain */
	register_reboot_notifier(&pdc_chassis_reboot_block);

	/* Check for old LED Panel */
	pdc_chassis_checkold();
}


/** 
 * pdc_chassis_send_status() - Sends a predefined message to the chassis,
 * and changes the front panel LEDs according to the new system state
 * @retval: PDC call return value.
 *
 * Only machines with 64 bits PDC PAT and E-class are supported atm.
 * 
 * returns 0 if no error, -1 if no supported PDC is present or invalid message,
 * else returns the appropriate PDC error code.
 * 
 * For a list of predefined messages, see asm-parisc/pdc_chassis.h
 */

int pdc_chassis_send_status(int message)
{
	/* Maybe we should do that in an other way ? */
	int retval = 0;

	DPRINTK(KERN_DEBUG "%s: pdc_chassis_send_status(%d)\n", __FILE__, message);

#ifdef __LP64__	/* pdc_pat_chassis_send_log is defined only when #ifdef __LP64__ */
	if (is_pdc_pat()) {
		switch(message) {
			case PDC_CHASSIS_DIRECT_BSTART:
				retval = pdc_pat_chassis_send_log(PDC_CHASSIS_PMSG_BSTART, PDC_CHASSIS_LSTATE_RUN_NORMAL);
				break;
			
			case PDC_CHASSIS_DIRECT_BCOMPLETE:
				retval = pdc_pat_chassis_send_log(PDC_CHASSIS_PMSG_BCOMPLETE, PDC_CHASSIS_LSTATE_RUN_NORMAL);
				break;
			
			case PDC_CHASSIS_DIRECT_SHUTDOWN:
				retval = pdc_pat_chassis_send_log(PDC_CHASSIS_PMSG_SHUTDOWN, PDC_CHASSIS_LSTATE_NONOS);
				break;
			
			case PDC_CHASSIS_DIRECT_PANIC:
				retval = pdc_pat_chassis_send_log(PDC_CHASSIS_PMSG_PANIC, PDC_CHASSIS_LSTATE_RUN_CRASHREC);
				break;
		
			case PDC_CHASSIS_DIRECT_LPMC:
				retval = pdc_pat_chassis_send_log(PDC_CHASSIS_PMSG_LPMC, PDC_CHASSIS_LSTATE_RUN_SYSINT);
				break;

			case PDC_CHASSIS_DIRECT_HPMC:
				retval = pdc_pat_chassis_send_log(PDC_CHASSIS_PMSG_HPMC, PDC_CHASSIS_LSTATE_RUN_NCRIT);
				break;

			default:
				retval = -1;
		}
	} else retval = -1;
#else
	if (pdc_chassis_old) {
		switch (message) {
			case PDC_CHASSIS_DIRECT_BSTART:
			case PDC_CHASSIS_DIRECT_BCOMPLETE:
				retval = pdc_chassis_disp(PDC_CHASSIS_DISP_DATA(OSTAT_RUN));
				break;
							
			case PDC_CHASSIS_DIRECT_SHUTDOWN:
				retval = pdc_chassis_disp(PDC_CHASSIS_DISP_DATA(OSTAT_SHUT));
				break;
			
			case PDC_CHASSIS_DIRECT_HPMC:
			case PDC_CHASSIS_DIRECT_PANIC:
				retval = pdc_chassis_disp(PDC_CHASSIS_DISP_DATA(OSTAT_FLT));
				break;
		
			case PDC_CHASSIS_DIRECT_LPMC:
				retval = pdc_chassis_disp(PDC_CHASSIS_DISP_DATA(OSTAT_WARN));
				break;

			default:
				retval = -1;
		}
	} else retval = -1;
#endif
		
	return retval;
}
