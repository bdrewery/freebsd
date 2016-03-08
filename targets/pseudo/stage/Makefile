# $FreeBSD$

# this makefile gets hooked into everyone's dependencies so that
# distrib-dirs can be done in STAGE_OBJTOP
all:

DISTRIB_ENV=	INSTALL="sh ${SRCTOP}/tools/install.sh" NO_FSCHG=1 MK_TESTS=no
stage-distrib-dirs: .META
	${_+_}${DISTRIB_ENV} ${.MAKE} -C ${SRCTOP}/etc distrib-dirs \
	    DESTDIR=${STAGE_OBJTOP}
	@touch $@

.include <bsd.prog.mk>

.if ${.MAKE.LEVEL} > 0 && ${MK_STAGING} == "yes"
all: stage-distrib-dirs
.endif
