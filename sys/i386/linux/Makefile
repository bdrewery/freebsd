# Makefile for syscall tables
#
# $FreeBSD$

all:
	@echo "make sysent only"

sysent:  linux_sysent.c linux_syscall.h linux_proto.h linux_syscalls.c linux_systrace_args.c

linux_sysent.c linux_syscall.h linux_proto.h linux_syscalls.c linux_systrace_args.c: \
		${.CURDIR}/../../kern/makesyscalls.sh ${.CURDIR}/syscalls.master ${.CURDIR}/syscalls.conf
	sh ${.CURDIR}/../../kern/makesyscalls.sh \
	    -s ${.CURDIR}/syscalls.conf \
	    ${.CURDIR}/syscalls.master
