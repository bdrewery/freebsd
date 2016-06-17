/*
 * Support for o32 Linux/MIPS ELF binaries.
 *
 * Copyright (C) 1999, 2001 Ralf Baechle
 * Copyright (C) 1999, 2001 Silicon Graphics, Inc.
 *
 * Heavily inspired by the 32-bit Sparc compat code which is
 * Copyright (C) 1995, 1996, 1997, 1998 David S. Miller (davem@redhat.com)
 * Copyright (C) 1995, 1996, 1997, 1998 Jakub Jelinek   (jj@ultra.linux.cz)
 */

#define ELF_ARCH		EM_MIPS
#define ELF_CLASS		ELFCLASS32
#ifdef __MIPSEB__
#define ELF_DATA		ELFDATA2MSB;
#else /* __MIPSEL__ */
#define ELF_DATA		ELFDATA2LSB;
#endif

/* ELF register definitions */
#define ELF_NGREG	45
#define ELF_NFPREG	33

typedef unsigned int elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef double elf_fpreg_t;
typedef elf_fpreg_t elf_fpregset_t[ELF_NFPREG];

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(hdr)						\
({									\
	int __res = 1;							\
	struct elfhdr *__h = (hdr);					\
									\
	if (__h->e_machine != EM_MIPS)					\
		__res = 0;						\
	if (__h->e_ident[EI_CLASS] != ELFCLASS32)			\
		__res = 0;						\
	if ((__h->e_flags & EF_MIPS_ABI2) != 0)				\
		__res = 0;						\
	if (((__h->e_flags & EF_MIPS_ABI) != 0) &&			\
	    ((__h->e_flags & EF_MIPS_ABI) != EF_MIPS_ABI_O32))		\
		__res = 0;						\
									\
	__res;								\
})

#define TASK32_SIZE		0x7fff8000UL
#undef ELF_ET_DYN_BASE
#define ELF_ET_DYN_BASE         (TASK32_SIZE / 3 * 2)

#include <asm/processor.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/elfcore.h>

struct timeval32
{
	int tv_sec, tv_usec;
};

#define elf_prstatus elf_prstatus32
struct elf_prstatus32
{
	struct elf_siginfo pr_info;	/* Info associated with signal */
	short	pr_cursig;		/* Current signal */
	unsigned int pr_sigpend;	/* Set of pending signals */
	unsigned int pr_sighold;	/* Set of held signals */
	pid_t	pr_pid;
	pid_t	pr_ppid;
	pid_t	pr_pgrp;
	pid_t	pr_sid;
	struct timeval32 pr_utime;	/* User time */
	struct timeval32 pr_stime;	/* System time */
	struct timeval32 pr_cutime;	/* Cumulative user time */
	struct timeval32 pr_cstime;	/* Cumulative system time */
	elf_gregset_t pr_reg;	/* GP registers */
	int pr_fpvalid;		/* True if math co-processor being used.  */
};

#define elf_prpsinfo elf_prpsinfo32
struct elf_prpsinfo32
{
	char	pr_state;	/* numeric process state */
	char	pr_sname;	/* char for pr_state */
	char	pr_zomb;	/* zombie */
	char	pr_nice;	/* nice val */
	unsigned int pr_flag;	/* flags */
	__kernel_uid_t	pr_uid;
	__kernel_gid_t	pr_gid;
	pid_t	pr_pid, pr_ppid, pr_pgrp, pr_sid;
	/* Lots missing */
	char	pr_fname[16];	/* filename of executable */
	char	pr_psargs[ELF_PRARGSZ];	/* initial part of arg list */
};

#define elf_addr_t	u32
#define elf_caddr_t	u32
#define init_elf_binfmt init_elf32_binfmt


#undef ELF_CORE_COPY_REGS
#define ELF_CORE_COPY_REGS(_dest,_regs) elf32_core_copy_regs(_dest,_regs);

void elf32_core_copy_regs(elf_gregset_t _dest, struct pt_regs *_regs)
{
	int i;

	memset(_dest, 0, sizeof(elf_gregset_t));

	/* XXXKW the 6 is from EF_REG0 in gdb/gdb/mips-linux-tdep.c, include/asm-mips/reg.h */
	for (i=6; i<38; i++)
		_dest[i] = (elf_greg_t) _regs->regs[i-6];
	_dest[i++] = (elf_greg_t) _regs->lo;
	_dest[i++] = (elf_greg_t) _regs->hi;
	_dest[i++] = (elf_greg_t) _regs->cp0_epc;
	_dest[i++] = (elf_greg_t) _regs->cp0_badvaddr;
	_dest[i++] = (elf_greg_t) _regs->cp0_status;
	_dest[i++] = (elf_greg_t) _regs->cp0_cause;
}

#undef CONFIG_BINFMT_ELF
#ifdef CONFIG_BINFMT_ELF32
#define CONFIG_BINFMT_ELF CONFIG_BINFMT_ELF32
#endif
#undef CONFIG_BINFMT_ELF_MODULE
#ifdef CONFIG_BINFMT_ELF32_MODULE
#define CONFIG_BINFMT_ELF_MODULE CONFIG_BINFMT_ELF32_MODULE
#endif

MODULE_DESCRIPTION("Binary format loader for compatibility with o32 Linux/MIPS binaries");
MODULE_AUTHOR("Ralf Baechle (ralf@linux-mips.org)");

#undef MODULE_DESCRIPTION
#undef MODULE_AUTHOR

#include "../../../fs/binfmt_elf.c"
