# Makefile for syscall tables
#
# $FreeBSD$

all:
	@echo "make sysent only"

sysent:  freebsd32_sysent.c freebsd32_syscall.h freebsd32_proto.h freebsd32_systrace_args.c

freebsd32_sysent.c freebsd32_syscalls.c freebsd32_syscall.h freebsd32_proto.h freebsd32_systrace_args.c : \
	    ${.CURDIR}/../../kern/makesyscalls.sh ${.CURDIR}/syscalls.master ${.CURDIR}/syscalls.conf ${.CURDIR}/../../kern/capabilities.conf
	sh ${.CURDIR}/../../kern/makesyscalls.sh ${.CURDIR}/syscalls.master ${.CURDIR}/syscalls.conf

clean:
	rm -f freebsd32_sysent.c freebsd32_syscalls.c freebsd32_syscall.h freebsd32_proto.h
	rm -f freebsd32_systrace_args.c 
