.if defined(PROG_FULL)
_LT_NAME_FULL=	${PROG_FULL}
_LT_NAME=	${PROG}
.elif defined(SHLIB_NAME_FULL)
_LT_NAME_FULL=	${SHLIB_NAME_FULL}
_LT_NAME=	${SHLIB_NAME}
.endif

.if empty(_LT_NAME_FULL)
check-links:
	@true
recheck-links:
	@true
.else

# Run check-links.sh against the target to build.
#
# LD_LIBRARY_PATH is needed as the check is done against libraries that are not
# yet installed. Need to search the OBJDIRS. A cd && trimming of dirs is done
# to make the environment for LD_LIBRARY_PATH smaller and less likely to
# overflow. The dir '../tmp/lib' is WORLDTMP
# where libraries are installed during the build. It is to find /usr/lib
# libraries, avoiding a 'not found' warning from ldd(1).
_CHECK_LINKS_LDPATH=	${LDADD:M-L*:C/-L//:C/^${OBJTOP:tA}\///}
_CHECK_LINKS_LDPATH+=	tmp/lib
_CHECK_LINKS_LDPATH+=	tmp/usr/lib

# _CHECK_LINKS is not executed in the var. It is done like this to share
# between check-links and .overlinktest without needing a submake which
# seems to run into ordering/rebuild issues (Bug 152339).
# A () is used just to make it simpler to copy/paste for testing
# without forcing changing dir.
_CHECK_LINKS=	( \
		  cd ${OBJTOP:tA} && env \
		  sh ${SRCTOP}/tools/build/check-links.sh \
		  -L ${_CHECK_LINKS_LDPATH:ts:} \
		  ${CHECK_LINKS} ${.OBJDIR}/${_LT_NAME_FULL} \
		)

check-links: ${_LT_NAME_FULL} .WAIT .PHONY
	${_CHECK_LINKS}

recheck-links: .PHONY
	rm -f ${_LT_NAME_FULL}
	@${MAKE} -C ${.CURDIR} check-links

# Check for overlinking by comparing what LIBADDS used in the Makefile
# to the actual libraries used according to tools/build/check-links.sh
.if defined(WITH_OVERLINKTEST) && defined(LIBADD) && \
    !empty(_LT_NAME)
CLEANFILES+=	.overlinktest
# Rather than an explicit .ORDER just add the dependencies to serialize
# .overlinktest between the .full and final result. This is intended so that
# if the link test fails then all versions of the output are removed to be
# relinked once fixed.
${_LT_NAME}: .overlinktest
.overlinktest: ${_LT_NAME_FULL}
.if defined(NO_OVERLINKTEST)
	@echo "(overlinktest) [${_LT_NAME}] ==> Skipping test: ${NO_OVERLINKTEST}"
.else
	@echo "(overlinktest) [${_LT_NAME}] ==> Checking for unneeded LIBADDS..."; \
	    needed_libs=$$( \
	      set +e; \
	      ${_CHECK_LINKS} | tee /dev/fd/2 | \
	      grep -v ' no$$' | \
	      awk '{sub(/^lib/,"",$$3);sub(/\.so\..*/,"",$$3);print $$3}' | \
	      tr '\n' ' ' \
	    ); \
	    bad_libs=; \
	    for lib in ${_LINKTEST_LIBADD}; do \
	        case "$${lib}" in \
	            curses|termcap|termlib|tinfo)         lib=ncurses ;; \
	            cursesw|termcapw|termlibw|tinfow)     lib=ncursesw ;; \
	            pthread)                              lib=thr ;; \
	        esac; \
	        case " $${needed_libs} " in \
	            *\ $${lib}\ *) ;; \
	            *) bad_libs="$${bad_libs}$${bad_libs:+ }$${lib}" ;; \
	        esac; \
	    done; \
	    if [ -n "$${bad_libs}" ]; then \
	        echo "(overlinktest) [${_LT_NAME}] ==> FAIL. Unneeded LIBADDS: $${bad_libs}"; \
	        echo rm -f ${_LT_NAME_FULL}; \
	        exit 0; \
	    else \
	        echo "(overlinktest) [${_LT_NAME}] ==> PASS"; \
	    fi
.endif
	@touch ${.TARGET}
.endif
.endif

# XXX: This is not going to fly upstream as too much breaks.  It is easy to
# work around things like libc with the -nodefaultlibs or MK_LINKTEST=no
# but lib/libdpv legitimately uses 'environ' which is definitely provided
# by the binary using it.  Due to that this probably needs to go back to
# linking a real program to verify it works as this section has.
.if 0
.if defined(SHLIB) && defined(WITH_LINKTEST)
.PHONY:        linktest
all: linktest
.if ${SRCS:M*.cc} || ${SRCS:M*.cpp}
_LINKTEST_CC=	${CXX}
_LINKTEST_TYPE=	c++
.else
_LINKTEST_CC=	${CC}
_LINKTEST_TYPE=	c
.endif
# XXX
_LINKTEST_CC=  ${CXX}
_LINKTEST_TYPE=        c++
linktest: ${_LIBS}
.if defined(NO_LINKTEST)
	@echo "==> Skipping link test for ${LIB}: ${NO_LINKTEST}"
.else
	@echo "==> Checking if ${LIB} has linked all dependencies..."
	# Need to -rpath every nested lib/ dir since they are not installed
	# into a comon dir yet.
	@lib_dirs=$$( \
	    cd ${.OBJDIR}; \
	    until [ "$${PWD##*/}" = "lib" ]; do \
	      cd ..; \
	    done; \
	    { find . -type d; echo ${LDFLAGS:C/-L//:C/-rpath//}; }  | \
	      xargs realpath -q | tr ' ' '\n' | sort -u | \
	      sed -e 's,^,-rpath ,'; \
	  ); \
	echo 'int main(){return 0;}' | ${_LINKTEST_CC} -x ${_LINKTEST_TYPE} \
	  -o /dev/null \
	  $${lib_dirs} -L${.OBJDIR} -l:${SHLIB_NAME} - || \
	  (\
	    echo "==> Failed to link. LDADDS missing." >&2; \
	    rm -f ${SHLIB_NAME} ${SHLIB_LINK} ${SHLIB_NAME_FULL} ${_LIBS} \
	          ${SHLIB_NAME}.debug; \
	    exit 1; \
	  )
.endif
.endif
.endif

# Check for underlinking in libraries only. This behavior works by default
# via ld for binaries.
#
# WITH_LINKTEST will build with '-z defs' (aka --no-undefined) to ensure that
# all symbols are resolved by direct links.
.if defined(WITH_LINKTEST) && \
    empty(LDFLAGS:M-nodefaultlibs)
.if !defined(NO_LINKTEST) && defined(WITH_LINKTEST)
LT_LDFLAGS=	-Wl,--no-undefined -Wl,--warn-once
SOLINKOPTS+=	${LT_LDFLAGS}
.endif
CLEANFILES+=	.linktest
# All this is doing is showing a message. Ensure it shows before the real link.
${SHLIB_NAME_FULL}:	.linktest
# This gives a notice that link tests won't be done. When they are there is
# no notice since it's just a --no-defined link flag. No good way to show
# a message in that case. Depend on the objects to recheck for missing symbols
# from new source files.
.linktest: ${SOBJS}
.if defined(NO_LINKTEST)
	@echo "(linktest) [${SHLIB_NAME}] ==> Skipping test: ${NO_LINKTEST}"
.else
	@echo "(linktest) [${SHLIB_NAME}] ==> Linking with --no-undefined to require all symbols to be resolved."
.endif
	@touch ${.TARGET}
.endif
