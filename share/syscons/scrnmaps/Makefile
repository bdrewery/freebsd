# $FreeBSD$

SCRMAPS = armscii8-2haik8.scm \
	  iso-8859-1_to_cp437.scm iso-8859-4_for_vga9.scm \
	  iso-8859-7_to_cp437.scm \
	  koi8-r2cp866.scm koi8-u2cp866u.scm \
	  us-ascii_to_cp437.scm

SCRMAPS_MK = ${SCRMAPS:R:S/$/.mk/g}
CLEANFILES+= ${SCRMAPS}

FILES=	${SCRMAPS}
FILESDIR= ${SHAREDIR}/syscons/scrnmaps

${SCRMAPS}: ${BTOOLSPATH:U.}/${SCRMAPS_MK}
	${BTOOLSPATH:U.}/${SCRMAPS_MK} ${.TARGET:R}.tmp
	uuencode ${.TARGET:R}.tmp ${.TARGET:R} > ${.TARGET}
	rm -f ${.TARGET:R}.tmp

build-tools: ${SCRMAPS_MK}
.if make(build-tools) || empty(BTOOLSPATH)
CLEANFILES+=	${SCRMAPS_MK}
${SCRMAPS_MK}: ${.TARGET:R} mkscrfil.c
	${CC} ${CFLAGS} -I${.CURDIR} -DFIL=\"${.TARGET:R}\" ${LDFLAGS} \
	    -o ${.TARGET} ${.CURDIR}/mkscrfil.c
.endif

.include <bsd.prog.mk>
