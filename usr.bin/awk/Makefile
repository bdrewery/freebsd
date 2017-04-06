# $FreeBSD$

AWKSRC=	${SRCTOP}/contrib/one-true-awk
.PATH: ${AWKSRC}

PROG=	awk
SRCS=	awkgram.y b.c lex.c lib.c main.c parse.c proctab.c run.c tran.c ytab.h

CFLAGS+= -DHAS_ISBLANK -I. -I${AWKSRC} -DFOPEN_MAX=64

WARNS?=	1

LIBADD=	m

LINKS=	${BINDIR}/awk ${BINDIR}/nawk
MLINKS=	awk.1 nawk.1

CLEANFILES= proctab.c ytab.h

# XXX: awkgram.c isn't really needed here but it is added to keep
# awkgram.h: .NOMETA respected.
ytab.h: awkgram.c awkgram.h .NOMETA
	ln -sf ${.ALLSRC:M*.h} ${.TARGET}

proctab.c: ${BTOOLSPATH:U.}/maketab ytab.h
	${BTOOLSPATH:U.}/maketab > proctab.c

build-tools: maketab
.if make(build-tools) || empty(BTOOLSPATH)
CLEANFILES+=	maketab
maketab: ytab.h maketab.c
.endif

.include <bsd.prog.mk>
