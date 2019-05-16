# $FreeBSD$

all:
	@echo "make sysent only"

sysent: cloudabi64_sysent.c cloudabi64_syscall.h cloudabi64_proto.h \
    cloudabi64_syscalls.c cloudabi64_systrace_args.c

cloudabi64_sysent.c cloudabi64_syscall.h cloudabi64_proto.h \
    cloudabi64_syscalls.c cloudabi64_systrace_args.c: \
    ${.CURDIR}/../../kern/makesyscalls.sh ${.CURDIR}/../../contrib/cloudabi/syscalls64.master \
    ${.CURDIR}/syscalls.conf
	sh ${.CURDIR}/../../kern/makesyscalls.sh \
	    -s ${.CURDIR}/syscalls.conf \
	    ${.CURDIR}/../../contrib/cloudabi/syscalls64.master
