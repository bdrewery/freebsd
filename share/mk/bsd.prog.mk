#	from: @(#)bsd.prog.mk	5.26 (Berkeley) 6/25/91
# $FreeBSD$

.include <bsd.init.mk>
.include <bsd.compiler.mk>

.SUFFIXES: .out .o .c .cc .cpp .cxx .C .m .y .l .ln .s .S .asm

# XXX The use of COPTS in modern makefiles is discouraged.
.if defined(COPTS)
CFLAGS+=${COPTS}
.endif

.if ${MK_ASSERT_DEBUG} == "no"
CFLAGS+= -DNDEBUG
NO_WERROR=
.endif

.if defined(DEBUG_FLAGS)
CFLAGS+=${DEBUG_FLAGS}
CXXFLAGS+=${DEBUG_FLAGS}

.if ${MK_CTF} != "no" && ${DEBUG_FLAGS:M-g} != ""
CTFFLAGS+= -g
.endif
.endif

.if defined(PROG_CXX)
PROG=	${PROG_CXX}
.endif

.if !empty(LDFLAGS:M-Wl,*--oformat,*) || !empty(LDFLAGS:M-static)
MK_DEBUG_FILES=	no
.endif

.if defined(CRUNCH_CFLAGS)
CFLAGS+=${CRUNCH_CFLAGS}
.else
.if ${MK_DEBUG_FILES} != "no" && empty(DEBUG_FLAGS:M-g) && \
    empty(DEBUG_FLAGS:M-gdwarf-*)
CFLAGS+= -g
CTFFLAGS+= -g
.endif
.endif

.if !defined(DEBUG_FLAGS)
STRIP?=	-s
.endif

.if defined(NO_SHARED) && (${NO_SHARED} != "no" && ${NO_SHARED} != "NO")
LDFLAGS+= -static
.endif

.if ${MK_DEBUG_FILES} != "no"
PROG_FULL=${PROG}.full
# Use ${DEBUGDIR} for base system debug files, else .debug subdirectory
.if defined(BINDIR) && (\
    ${BINDIR} == "/bin" ||\
    ${BINDIR:C%/libexec(/.*)?%/libexec%} == "/libexec" ||\
    ${BINDIR} == "/sbin" ||\
    ${BINDIR:C%/usr/(bin|bsdinstall|libexec|lpr|sendmail|sm.bin|sbin|tests)(/.*)?%/usr/bin%} == "/usr/bin"\
     )
DEBUGFILEDIR=	${DEBUGDIR}${BINDIR}
.else
DEBUGFILEDIR?=	${BINDIR}/.debug
DEBUGMKDIR=
.endif
.else
PROG_FULL=	${PROG}
.endif

.if defined(PROG)
PROGNAME?=	${PROG}

.if defined(SRCS)

OBJS+=  ${SRCS:N*.h:R:S/$/.o/g}

.if target(beforelinking)
beforelinking: ${OBJS}
${PROG_FULL}: beforelinking
.endif
${PROG_FULL}: ${OBJS}
.if defined(PROG_CXX)
	${CXX:N${CCACHE_BIN}} ${CXXFLAGS:N-M*} ${LDFLAGS} -o ${.TARGET} \
	    ${OBJS} ${LDADD}
.else
	${CC:N${CCACHE_BIN}} ${CFLAGS:N-M*} ${LDFLAGS} -o ${.TARGET} ${OBJS} \
	    ${LDADD}
.endif
.if ${MK_CTF} != "no"
	${CTFMERGE} ${CTFFLAGS} -o ${.TARGET} ${OBJS}
.endif

.else	# !defined(SRCS)

.if !target(${PROG})
.if defined(PROG_CXX)
SRCS=	${PROG}.cc
.else
SRCS=	${PROG}.c
.endif

# Always make an intermediate object file because:
# - it saves time rebuilding when only the library has changed
# - the name of the object gets put into the executable symbol table instead of
#   the name of a variable temporary object.
# - it's useful to keep objects around for crunching.
OBJS+=	${PROG}.o

.if target(beforelinking)
beforelinking: ${OBJS}
${PROG_FULL}: beforelinking
.endif
${PROG_FULL}: ${OBJS}
.if defined(PROG_CXX)
	${CXX:N${CCACHE_BIN}} ${CXXFLAGS:N-M*} ${LDFLAGS} -o ${.TARGET} \
	    ${OBJS} ${LDADD}
.else
	${CC:N${CCACHE_BIN}} ${CFLAGS:N-M*} ${LDFLAGS} -o ${.TARGET} ${OBJS} \
	    ${LDADD}
.endif
.if ${MK_CTF} != "no"
	${CTFMERGE} ${CTFFLAGS} -o ${.TARGET} ${OBJS}
.endif
.endif # !target(${PROG})

.endif # !defined(SRCS)

.if ${MK_DEBUG_FILES} != "no"
${PROG}: ${PROG_FULL} ${PROGNAME}.debug
	${OBJCOPY} --strip-debug --add-gnu-debuglink=${PROGNAME}.debug \
	    ${PROG_FULL} ${.TARGET}

${PROGNAME}.debug: ${PROG_FULL}
	${OBJCOPY} --only-keep-debug ${PROG_FULL} ${.TARGET}
.endif

.if	${MK_MAN} != "no" && !defined(MAN) && \
	!defined(MAN1) && !defined(MAN2) && !defined(MAN3) && \
	!defined(MAN4) && !defined(MAN5) && !defined(MAN6) && \
	!defined(MAN7) && !defined(MAN8) && !defined(MAN9)
MAN=	${PROG}.1
MAN1=	${MAN}
.endif
.endif # defined(PROG)

.if defined(_SKIP_BUILD)
all:
.else
all: beforebuild .WAIT ${PROG} ${SCRIPTS}
beforebuild: objwarn
.if ${MK_MAN} != "no"
all: _manpages
.endif
.endif

.if defined(PROG)
CLEANFILES+= ${PROG}
.if ${MK_DEBUG_FILES} != "no"
CLEANFILES+=	${PROG_FULL} ${PROGNAME}.debug
.endif
.endif

.if defined(OBJS)
CLEANFILES+= ${OBJS}
.endif

.include <bsd.libnames.mk>

.if defined(PROG) && !defined(NO_EXTRADEPEND)
.if 0 && empty(LDFLAGS:M-nostdlib)
_is_static=	0
_is_shared_lib=	0
_is_profiled=	0
.if !empty(LDFLAGS:M-shared) || !empty(SOLINKOPTS:M-shared) || \
    !empty(LDFLAGS:M-pie)
# This is be used for both PIE and shared.
_is_shared_lib=	1
.elif !empty(LDFLAGS:M-static)
_is_static=	1
.endif
# crt1
.if ${_is_shared_lib} == 1
_STD_DEPS+=	${LIBSCRT1}
.elif ${_is_profiled} == 1
_STD_DEPS+=	${LIBGCRT1}
.else
_STD_DEPS+=	${LIBCRT1}
.endif
# crtbegin
.if ${_is_static} == 1
_STD_DEPS+=	${LIBCRTBEGINT}
.elif ${_is_shared_lib} == 1
_STD_DEPS+=	${LIBCRTBEGINS}
.else
_STD_DEPS+=	${LIBCRTBEGIN}
.endif
# libgcc
.if ${_is_static} == 1
_STD_DEPS+=	${LIBGCC_EH}
.elif ${_is_profiled} == 1
_STD_DEPS+=	${LIBGCC_EH_P}
.else
_STD_DEPS+=	${LIBGCC_S}
.endif
# libcompiler_rt
.if ${_is_profiled} == 1
_STD_DEPS+=	${LIBCOMPILER_RT_P}
.else
_STD_DEPS+=	${LIBCOMPILER_RT}
.endif
# crtend
.if ${_is_shared_lib}
_STD_DEPS+=	${LIBCRTENDS}
.else
_STD_DEPS+=	${LIBCRTEND}
.endif
# Common crt files.
_STD_DEPS+=	${LIBCRTI} ${LIBCRTN}
.endif	# empty(LDFLAGS:M-nostdlib)

_DPADD=	${DPADD}
.if empty(LDFLAGS:M-nostdlib)
_DPADD+= ${LIBC}
.if !empty(LDFLAGS:M-static)
_DPADD+= ${_STD_DEPS_STATIC}
.else
_DPADD+= ${_STD_DEPS_PROG}
.endif
.if defined(PROG_CXX)
.if ${COMPILER_TYPE} == "clang" && empty(CXXFLAGS:M-stdlib=libstdc++)
_DPADD+= ${LIBCPLUSPLUS}
.else
_DPADD+= ${LIBSTDCPLUSPLUS}
.endif
.endif	# defined(PROG_CXX)
.endif	# empty(LDFLAGS:M-nostdlib)
_EXTRADEPEND:
.if !empty(_DPADD)
	echo ${PROG_FULL}: ${_DPADD} >> ${DEPENDFILE}
.endif
.endif	# defined(PROG) && !defined(NO_EXTRADEPEND)

.if !target(install)

.if defined(PRECIOUSPROG)
.if !defined(NO_FSCHG)
INSTALLFLAGS+= -fschg
.endif
INSTALLFLAGS+= -S
.endif

_INSTALLFLAGS:=	${INSTALLFLAGS}
.for ie in ${INSTALLFLAGS_EDIT}
_INSTALLFLAGS:=	${_INSTALLFLAGS${ie}}
.endfor

.if !target(realinstall) && !defined(INTERNALPROG)
realinstall: _proginstall
.ORDER: beforeinstall _proginstall
_proginstall:
.if defined(PROG)
	${INSTALL} ${STRIP} -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    ${_INSTALLFLAGS} ${PROG} ${DESTDIR}${BINDIR}/${PROGNAME}
.if ${MK_DEBUG_FILES} != "no"
.if defined(DEBUGMKDIR)
	${INSTALL} -T debug -d ${DESTDIR}${DEBUGFILEDIR}/
.endif
	${INSTALL} -T debug -o ${BINOWN} -g ${BINGRP} -m ${DEBUGMODE} \
	    ${PROGNAME}.debug ${DESTDIR}${DEBUGFILEDIR}/${PROGNAME}.debug
.endif
.endif
.endif	# !target(realinstall)

.if defined(SCRIPTS) && !empty(SCRIPTS)
realinstall: _scriptsinstall
.ORDER: beforeinstall _scriptsinstall

SCRIPTSDIR?=	${BINDIR}
SCRIPTSOWN?=	${BINOWN}
SCRIPTSGRP?=	${BINGRP}
SCRIPTSMODE?=	${BINMODE}

STAGE_AS_SETS+= scripts
stage_as.scripts: ${SCRIPTS}
FLAGS.stage_as.scripts= -m ${SCRIPTSMODE}
STAGE_FILES_DIR.scripts= ${STAGE_OBJTOP}
.for script in ${SCRIPTS}
.if defined(SCRIPTSNAME)
SCRIPTSNAME_${script:T}?=	${SCRIPTSNAME}
.else
SCRIPTSNAME_${script:T}?=	${script:T:R}
.endif
SCRIPTSDIR_${script:T}?=	${SCRIPTSDIR}
SCRIPTSOWN_${script:T}?=	${SCRIPTSOWN}
SCRIPTSGRP_${script:T}?=	${SCRIPTSGRP}
SCRIPTSMODE_${script:T}?=	${SCRIPTSMODE}
STAGE_AS_${script:T}=		${SCRIPTSDIR_${script:T}}/${SCRIPTSNAME_${script:T}}
_scriptsinstall: _SCRIPTSINS_${script:T}
_SCRIPTSINS_${script:T}: ${script}
	${INSTALL} -o ${SCRIPTSOWN_${.ALLSRC:T}} \
	    -g ${SCRIPTSGRP_${.ALLSRC:T}} -m ${SCRIPTSMODE_${.ALLSRC:T}} \
	    ${.ALLSRC} \
	    ${DESTDIR}${SCRIPTSDIR_${.ALLSRC:T}}/${SCRIPTSNAME_${.ALLSRC:T}}
.endfor
.endif

NLSNAME?=	${PROG}
.include <bsd.nls.mk>

.include <bsd.confs.mk>
.include <bsd.files.mk>
.include <bsd.incs.mk>
.include <bsd.links.mk>

.if ${MK_MAN} != "no"
realinstall: _maninstall
.ORDER: beforeinstall _maninstall
.endif

.endif	# !target(install)

.if !target(lint)
lint: ${SRCS:M*.c}
.if defined(PROG)
	${LINT} ${LINTFLAGS} ${CFLAGS:M-[DIU]*} ${.ALLSRC}
.endif
.endif

.if ${MK_MAN} != "no"
.include <bsd.man.mk>
.endif

.include <bsd.dep.mk>

.if defined(PROG) && !exists(${.OBJDIR}/${DEPENDFILE})
${OBJS}: ${SRCS:M*.h}
.endif

.include <bsd.obj.mk>

.include <bsd.sys.mk>
