# $FreeBSD$

all:
	@echo "make sysent only"

sysent: cloudabi32_sysent.c cloudabi32_syscall.h cloudabi32_proto.h \
    cloudabi32_syscalls.c cloudabi32_systrace_args.c

cloudabi32_sysent.c cloudabi32_syscall.h cloudabi32_proto.h \
    cloudabi32_syscalls.c cloudabi32_systrace_args.c: \
    ${.CURDIR}/../../kern/makesyscalls.sh ${.CURDIR}/../../contrib/cloudabi/syscalls32.master \
    ${.CURDIR}/syscalls.conf
	sh ${.CURDIR}/../../kern/makesyscalls.sh \
	    -s ${.CURDIR}/syscalls.conf \
	    ${.CURDIR}/../../contrib/cloudabi/syscalls32.master
