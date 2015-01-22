/*-
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 *
 *	from: FreeBSD: src/sys/i386/include/db_machdep.h,v 1.16 1999/10/04
 * $FreeBSD$
 */

#ifndef	_MACHINE_DB_MACHDEP_H_
#define	_MACHINE_DB_MACHDEP_H_

#include <machine/armreg.h>
#include <machine/frame.h>
#include <machine/trap.h>

#define T_BREAKPOINT	(EXCP_BRK)
#define T_WATCHPOINT	(EXCP_WATCHPT_EL1)

typedef vm_offset_t	db_addr_t;
typedef long		db_expr_t;

#define	PC_REGS()	((db_addr_t)kdb_thrctx->pcb_pc)

#define	BKPT_INST	(0xd4200000)
#define	BKPT_SIZE	(4)
#define	BKPT_SET(inst)	(BKPT_INST)

#define	BKPT_SKIP do {							\
	kdb_frame->tf_elr += BKPT_SIZE; \
} while (0)

#define SOFTWARE_SSTEP	1

#define	IS_BREAKPOINT_TRAP(type, code)	(type == T_BREAKPOINT)
#define	IS_WATCHPOINT_TRAP(type, code)	(type == T_WATCHPOINT)

#define	inst_trap_return(ins)	(0)
/* ret */
#define	inst_return(ins)	(((ins) & 0xfffffc1fu) == 0xd65f0000)
#define	inst_call(ins)		(0)
/* b, b.cond, br. TODO: b.cond & br */
#define	inst_branch(ins)	(((ins) & 0xfc000000u) == 0x14000000u)
#define	inst_load(ins)		(0)
#define	inst_store(ins)		(0)

#define next_instr_address(pc, bd)	((bd) ? (pc) : ((pc) + 4))

#define	DB_SMALL_VALUE_MAX	(0x7fffffff)
#define	DB_SMALL_VALUE_MIN	(-0x40001)

#define	DB_ELFSIZE		64

u_int branch_taken (u_int insn, u_int pc);

#endif /* !_MACHINE_DB_MACHDEP_H_ */
