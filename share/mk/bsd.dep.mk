# $FreeBSD$
#
# The include file <bsd.dep.mk> handles Makefile dependencies.
#
#
# +++ variables +++
#
# CTAGS		A tags file generation program [gtags]
#
# CTAGSFLAGS	Options for ctags(1) [not set]
#
# DEPENDFILE	dependencies file [.depend]
#
# GTAGSFLAGS	Options for gtags(1) [-o]
#
# HTAGSFLAGS	Options for htags(1) [not set]
#
# MKDEP		Options for ${MKDEPCMD} [not set]
#
# MKDEPCMD	Makefile dependency list program [mkdep]
#
# SRCS          List of source files (c, c++, assembler)
#
# DPSRCS	List of source files which are needed for generating
#		dependencies, ${SRCS} are always part of it.
#
# +++ targets +++
#
#	cleandepend:
#		Remove depend and tags file
#
#	depend:
#		Make the dependencies for the source files, and store
#		them in the file ${DEPENDFILE}.
#
#	tags:
#		In "ctags" mode, create a tags file for the source files.
#		In "gtags" mode, create a (GLOBAL) gtags file for the
#		source files.  If HTML is defined, htags(1) is also run
#		after gtags(1).

.if !target(__<bsd.init.mk>__)
.error bsd.dep.mk cannot be included directly.
.endif

CTAGS?=		gtags
CTAGSFLAGS?=
GTAGSFLAGS?=	-o
HTAGSFLAGS?=

_MKDEPCC=	${CC}
.if ${MK_CCACHE_BUILD} == "yes"
# Avoid overhead of calling ccache for mkdep.
_MKDEPCC:=	${_MKDEPCC:N${CCACHE_PATH}}
.endif
# XXX: DEPFLAGS can come out once Makefile.inc1 properly passes down
# CXXFLAGS.
.if !empty(DEPFLAGS)
_MKDEPCC+=	${DEPFLAGS}
.endif
MKDEPCMD?=	CC='${_MKDEPCC}' mkdep
DEPENDFILE?=	.depend
DEPENDFILES=	${DEPENDFILE}

# Keep `tags' here, before SRCS are mangled below for `depend'.
.if !target(tags) && defined(SRCS) && !defined(NO_TAGS)
tags: ${SRCS}
.if ${CTAGS:T} == "gtags"
	@cd ${.CURDIR} && ${CTAGS} ${GTAGSFLAGS} ${.OBJDIR}
.if defined(HTML)
	@cd ${.CURDIR} && htags ${HTAGSFLAGS} -d ${.OBJDIR} ${.OBJDIR}
.endif
.else
	@${CTAGS} ${CTAGSFLAGS} -f /dev/stdout \
	    ${.ALLSRC:N*.h} | sed "s;${.CURDIR}/;;" > ${.TARGET}
.endif
.endif

.if defined(SRCS)
CLEANFILES?=

.if !exists(${.OBJDIR}/${DEPENDFILE})
.for _S in ${SRCS:N*.[dhly]}
${_S:R}.o: ${_S}
.endfor
.endif

# Lexical analyzers
.for _LSRC in ${SRCS:M*.l:N*/*}
.for _LC in ${_LSRC:R}.c
${_LC}: ${_LSRC}
	${LEX} ${LFLAGS} -o${.TARGET} ${.ALLSRC}
.if !exists(${.OBJDIR}/${DEPENDFILE})
${_LC:R}.o: ${_LC}
.endif
SRCS:=	${SRCS:S/${_LSRC}/${_LC}/}
CLEANFILES+= ${_LC}
.endfor
.endfor

# Yacc grammars
.for _YSRC in ${SRCS:M*.y:N*/*}
.for _YC in ${_YSRC:R}.c
SRCS:=	${SRCS:S/${_YSRC}/${_YC}/}
CLEANFILES+= ${_YC}
.if !empty(YFLAGS:M-d) && !empty(SRCS:My.tab.h)
.ORDER: ${_YC} y.tab.h
${_YC} y.tab.h: ${_YSRC}
	${YACC} ${YFLAGS} ${.ALLSRC}
	cp y.tab.c ${_YC}
CLEANFILES+= y.tab.c y.tab.h
.elif !empty(YFLAGS:M-d)
.for _YH in ${_YC:R}.h
${_YH}: ${_YC}
${_YC}: ${_YSRC}
	${YACC} ${YFLAGS} -o ${_YC} ${.ALLSRC}
SRCS+=	${_YH}
CLEANFILES+= ${_YH}
.endfor
.else
${_YC}: ${_YSRC}
	${YACC} ${YFLAGS} -o ${_YC} ${.ALLSRC}
.endif
.if !exists(${.OBJDIR}/${DEPENDFILE})
${_YC:R}.o: ${_YC}
.endif
.endfor
.endfor

# DTrace probe definitions
.if ${SRCS:M*.d}
CFLAGS+=	-I${.OBJDIR}
.endif
.for _DSRC in ${SRCS:M*.d:N*/*}
.for _D in ${_DSRC:R}
DHDRS+=	${_D}.h
${_D}.h: ${_DSRC}
	${DTRACE} ${DTRACEFLAGS} -h -s ${.ALLSRC}
SRCS:=	${SRCS:S/^${_DSRC}$//}
OBJS+=	${_D}.o
CLEANFILES+= ${_D}.h ${_D}.o
${_D}.o: ${_DSRC} ${OBJS:S/^${_D}.o$//}
	${DTRACE} ${DTRACEFLAGS} -G -o ${.TARGET} -s ${.ALLSRC}
.if defined(LIB)
CLEANFILES+= ${_D}.So ${_D}.po
${_D}.So: ${_DSRC} ${SOBJS:S/^${_D}.So$//}
	${DTRACE} ${DTRACEFLAGS} -G -o ${.TARGET} -s ${.ALLSRC}
${_D}.po: ${_DSRC} ${POBJS:S/^${_D}.po$//}
	${DTRACE} ${DTRACEFLAGS} -G -o ${.TARGET} -s ${.ALLSRC}
.endif
.endfor
.endfor
beforedepend: ${DHDRS}
beforebuild: ${DHDRS}
.endif

.if ${MK_META_MODE} == "yes"
.include <meta.autodep.mk>
# this depend: bypasses that below
# the dependency helps when bootstrapping
depend: beforedepend ${DPSRCS} ${SRCS} afterdepend
beforedepend:
afterdepend: beforedepend
.endif

.if !target(depend)
.if defined(SRCS)
depend: beforedepend ${DEPENDFILE} afterdepend

# Different types of sources are compiled with slightly different flags.
# Split up the sources, and filter out headers and non-applicable flags.
MKDEP_CFLAGS=	${CFLAGS:M-nostdinc*} ${CFLAGS:M-[BIDU]*} ${CFLAGS:M-std=*} \
		${CFLAGS:M-ansi}
MKDEP_CXXFLAGS=	${CXXFLAGS:M-nostdinc*} ${CXXFLAGS:M-[BIDU]*} \
		${CXXFLAGS:M-std=*} ${CXXFLAGS:M-ansi} ${CXXFLAGS:M-stdlib=*}

DPSRCS+= ${SRCS}
.if !empty(DPSRCS)
.for __dpsrc in ${DPSRCS:O:u}
# Need _dpsrc for .if checks on iteration variable.
_dpsrc= ${__dpsrc}
.if !empty(_dpsrc:M*.[cS])
_mkdep_flags=	${MKDEP_CFLAGS}
.elif !empty(_dpsrc:M*.cc) || !empty(_dpsrc:M*.C) || !empty(_dpsrc:M*.cpp) || \
    !empty(_dpsrc:M*.cxx)
_mkdep_flags=	${MKDEP_CXXFLAGS}
.else
_mkdep_flags=
.endif
.if !empty(_mkdep_flags)
# Using iteration variable allows +=.  If _dpsrc were used then it would
# require :=.
DPDEPS+=	${DEPENDFILE}.${__dpsrc}
_mkdep_flags.${__dpsrc}:=	${_mkdep_flags}
${DEPENDFILE}.${_dpsrc}: ${_dpsrc}
	rm -f ${.TARGET}
	${MKDEPCMD} -f ${.TARGET} -a ${MKDEP} ${_mkdep_flags.${__dpsrc}} \
	    ${.ALLSRC:[1]}
.endif
.endfor
.endif	# !empty(DPSRCS)

.if !empty(DPDEPS)
# The DPDEPS are included via .depend_srcs so that .depend has less to
# regenerate if one of its extra dependencies needs to rebuild .depend.
DEPENDFILES+=	${DEPENDFILE}_srcs ${DPDEPS}
${DEPENDFILE}_srcs: ${DPSRCS} ${DPDEPS}
	rm -f ${.TARGET}
	for f in ${DPDEPS}; do \
		echo ".sinclude \"$${f}\""; \
	done > ${.TARGET}.tmp && \
	mv ${.TARGET}.tmp ${.TARGET}
${DEPENDFILE}: ${DEPENDFILE}_srcs
.endif

${DEPENDFILE}: ${DPSRCS}
	rm -f ${DEPENDFILE}
.if !empty(DPDEPS)
	echo ".sinclude \"${DEPENDFILE}_srcs\"" > ${.TARGET}
.else
	touch ${.TARGET}
.endif
.if target(_EXTRADEPEND)
_EXTRADEPEND: .USE
${DEPENDFILE}: _EXTRADEPEND
.endif

# Tell bmake not to look for generated files via .PATH
.NOPATH: ${DEPENDFILES}

.ORDER: ${DEPENDFILE} afterdepend
.else
depend: beforedepend afterdepend
.endif
.if !target(beforedepend)
beforedepend:
.else
.ORDER: beforedepend ${DEPENDFILE}
.ORDER: beforedepend afterdepend
.endif
.if !target(afterdepend)
afterdepend:
.endif
.endif

.if !target(cleandepend)
cleandepend:
.if defined(SRCS)
.if ${CTAGS:T} == "gtags"
	rm -f ${DEPENDFILES} GPATH GRTAGS GSYMS GTAGS
.if defined(HTML)
	rm -rf HTML
.endif
.else
	rm -f ${DEPENDFILES} tags
.endif
.endif
.endif

.if !target(checkdpadd) && (defined(DPADD) || defined(LDADD))
_LDADD_FROM_DPADD=	${DPADD:R:T:C;^lib(.*)$;-l\1;g}
# Ignore -Wl,--start-group/-Wl,--end-group as it might be required in the
# LDADD list due to unresolved symbols
_LDADD_CANONICALIZED=	${LDADD:N:R:T:C;^lib(.*)$;-l\1;g:N-Wl,--[es]*-group}
checkdpadd:
.if ${_LDADD_FROM_DPADD} != ${_LDADD_CANONICALIZED}
	@echo ${.CURDIR}
	@echo "DPADD -> ${_LDADD_FROM_DPADD}"
	@echo "LDADD -> ${_LDADD_CANONICALIZED}"
.endif
.endif
