# $FreeBSD$

.include <src.opts.mk>

.SUFFIXES: .h .h.in

.if ${MK_CASPER} != "no"
CFLAGS+=-DWITH_CASPER

.h.in.h:
	unifdef -x1 -DWITH_CASPER -o ${.TARGET} ${.IMPSRC}
.else
.h.in.h:
	unifdef -x1 -UWITH_CASPER -o ${.TARGET} ${.IMPSRC}
.endif

.include "../Makefile.inc"
