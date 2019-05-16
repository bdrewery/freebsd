# Makefile for syscall tables
#
# $FreeBSD$

all:
	@echo "make sysent only"

sysent:  linux32_sysent.c linux32_syscall.h linux32_proto.h linux32_syscalls.c linux32_systrace_args.c

linux32_sysent.c linux32_syscall.h linux32_proto.h linux32_syscalls.c linux32_systrace_args.c: ${.CURDIR}/../../kern/makesyscalls.sh \
		${.CURDIR}/syscalls.master ${.CURDIR}/syscalls.conf
	sh ${.CURDIR}/../../kern/makesyscalls.sh ${.CURDIR}/syscalls.master ${.CURDIR}/syscalls.conf
