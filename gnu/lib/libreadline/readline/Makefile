# $FreeBSD$

#SUBDIR= doc

LIB=    readline
NO_MAN=	yes
SHLIBDIR?=	/lib
PRIVATELIB=	true

TILDESRC= tilde.c
SRCS=     readline.c vi_mode.c funmap.c keymaps.c parens.c search.c \
	  rltty.c complete.c bind.c isearch.c display.c signals.c \
	  util.c kill.c undo.c macro.c input.c callback.c terminal.c \
	  text.c nls.c misc.c compat.c xmalloc.c $(HISTSRC) $(TILDESRC)

DPADD=	${LIBTERMCAP}
LDADD=	-ltermcap

HEADERS= readline.h chardefs.h keymaps.h history.h tilde.h \
	   rlstdc.h rlconf.h rltypedefs.h
CLEANFILES+=	${HEADERS}
beforedepend: ${HEADERS}
${HEADERS}:
	@echo ln -fs ${SRCDIR} ${.OBJDIR}/${.TARGET}


.include <bsd.lib.mk>
