# $FreeBSD$
#
# The include file <bsd.obj.mk> handles creating the 'obj' directory
# and cleaning up object files, etc.
#
# +++ variables +++
#
# CLEANDIRS	Additional directories to remove for the clean target.
#
# CLEANFILES	Additional files to remove for the clean target.
#
# MAKEOBJDIR 	A pathname for the directory where the targets
#		are built.  Note: MAKEOBJDIR is an *environment* variable
#		and works properly only if set as an environment variable,
#		not as a global or command line variable!
#
#		E.g. use `env MAKEOBJDIR=temp-obj make'
#
# MAKEOBJDIRPREFIX  Specifies somewhere other than /usr/obj to root the object
#		tree.  Note: MAKEOBJDIRPREFIX is an *environment* variable
#		and works properly only if set as an environment variable,
#		not as a global or command line variable!
#
#		E.g. use `env MAKEOBJDIRPREFIX=/somewhere/obj make'
#
# NO_OBJ	Do not create object directories.  This should not be set
#		if anything is built.
#
# +++ targets +++
#
#	clean:
#		remove ${CLEANFILES}; remove ${CLEANDIRS} and all contents.
#
#	cleandir:
#		remove the build directory (and all its contents) created by obj
#
#	obj:
#		create build directory.
#

.if !target(__<bsd.obj.mk>__)
__<bsd.obj.mk>__:
.include <bsd.own.mk>

# Handle special case where SRCS is full-pathed and requires nested objdirs.
# We need to handle all relative files before applying OBJS_SRCS_FILTER to
# deal with generated files that must reside in a nested .OBJDIR.
_nested_srcs=	${SRCS:M*/*} ${DPSRCS:M*/*}
.if !empty(_nested_srcs) && \
    (${MK_AUTO_OBJ} == "yes" || make(obj)) && \
    (${.TARGETS} == "" || ${.TARGETS:Nclean*:N*clean:Ndestroy*} != "")
# First find all generated srcs that need a nested .OBJDIR.
.for _src in ${_nested_srcs}
.if !exists(${_src})
# This file needs to be generated to a nested .OBJDIR.
_nested_dpsrcs+=	${_src}
# Remove from the nested src list
_nested_srcs:=		${_nested_srcs:N${_src}}
.endif
.endfor
.if !empty(_nested_dpsrcs)
_nested_objdirs=	${_nested_dpsrcs:H}
.endif
# Now find objs that need a nested .OBJDIR.
_nested_objs=		${_nested_srcs:${OBJS_SRCS_FILTER:ts:}:M*/*:H:N.}
.if !empty(_nested_objs)
_nested_objdirs+=	${_nested_objs}
.endif
.if !empty(_nested_objdirs)
_nested_objdirs:=	${_nested_objdirs:O:u}
.endif	# !empty(_nested_objdirs)
.endif	# !empty(_nested_srcs)

.if ${MK_AUTO_OBJ} == "yes"
# it is done by now
objwarn:
obj:
CANONICALOBJDIR= ${.OBJDIR}
# This is also done in bsd.init.mk
.if defined(NO_OBJ)
# but this makefile does not want it!
.OBJDIR: ${.CURDIR}
.endif
# This duplicates some auto.obj.mk logic.
_needed_objdirs=
.for _dir in ${_nested_objdirs}
.if !exists(${.OBJDIR}/${_dir}/)
_needed_objdirs+=	${_dir}
.endif
.endfor
.if !empty(_needed_objdirs)
__objdir_made != umask ${OBJDIR_UMASK:U002}; ${Mkdirs}; \
	for dir in ${_needed_objdirs}; do \
	  dir=${.OBJDIR}/$${dir}; \
	  ${ECHO_TRACE} "[Creating nested objdir $${dir}...]" >&2; \
          Mkdirs $${dir}; \
	done
.endif
.elif !empty(MAKEOBJDIRPREFIX)
CANONICALOBJDIR:=${MAKEOBJDIRPREFIX}${.CURDIR}
.elif defined(MAKEOBJDIR) && ${MAKEOBJDIR:M/*} != ""
CANONICALOBJDIR:=${MAKEOBJDIR}
OBJTOP?= ${MAKEOBJDIR}
.else
CANONICALOBJDIR:=/usr/obj${.CURDIR}
.endif

.if defined(SRCTOP) && defined(RELDIR) && \
    (${CANONICALOBJDIR} == /${RELDIR} || ${.OBJDIR} == /${RELDIR})
.error .OBJDIR incorrectly set to /${RELDIR}
.endif

OBJTOP?= ${.OBJDIR:S,${.CURDIR},,}${SRCTOP}

#
# Warn of unorthodox object directory.
#
# The following directories are tried in order for ${.OBJDIR}:
#
# 1.  ${MAKEOBJDIRPREFIX}/`pwd`
# 2.  ${MAKEOBJDIR}
# 3.  obj.${MACHINE}
# 4.  obj
# 5.  /usr/obj/`pwd`
# 6.  ${.CURDIR}
#
# If ${.OBJDIR} is constructed using canonical cases 1 or 5, or
# case 2 (using MAKEOBJDIR), don't issue a warning.  Otherwise,
# issue a warning differentiating between cases 6 and (3 or 4).
#
objwarn: .PHONY
.if !defined(NO_OBJ) && ${.OBJDIR} != ${CANONICALOBJDIR} && \
    !(defined(MAKEOBJDIRPREFIX) && exists(${CANONICALOBJDIR}/)) && \
    !(defined(MAKEOBJDIR) && exists(${MAKEOBJDIR}/))
.if ${.OBJDIR} == ${.CURDIR}
	@${ECHO} "Warning: Object directory not changed from original ${.CURDIR}"
.elif exists(${.CURDIR}/obj.${MACHINE}/) || exists(${.CURDIR}/obj/)
	@${ECHO} "Warning: Using ${.OBJDIR} as object directory instead of\
		canonical ${CANONICALOBJDIR}"
.endif
.endif
beforebuild: objwarn

.if !defined(NO_OBJ)
.if !target(obj)
obj: .PHONY
	@if ! test -d ${CANONICALOBJDIR}/; then \
		mkdir -p ${CANONICALOBJDIR}; \
		if ! test -d ${CANONICALOBJDIR}/; then \
			${ECHO} "Unable to create ${CANONICALOBJDIR}."; \
			exit 1; \
		fi; \
		${ECHO} "${CANONICALOBJDIR} created for ${.CURDIR}"; \
	fi
.for dir in ${_nested_objdirs}
	@if ! test -d ${CANONICALOBJDIR}/${dir}/; then \
		mkdir -p ${CANONICALOBJDIR}/${dir}; \
		if ! test -d ${CANONICALOBJDIR}/${dir}/; then \
			${ECHO} "Unable to create ${CANONICALOBJDIR}/${dir}."; \
			exit 1; \
		fi; \
		${ECHO} "${CANONICALOBJDIR}/${dir} created for ${.CURDIR}"; \
	fi
.endfor
.endif

.if !target(objlink)
objlink:
	@if test -d ${CANONICALOBJDIR}/; then \
		rm -f ${.CURDIR}/obj; \
		ln -s ${CANONICALOBJDIR} ${.CURDIR}/obj; \
	else \
		echo "No ${CANONICALOBJDIR} to link to - do a make obj."; \
	fi
.endif
.endif # !defined(NO_OBJ)

#
# where would that obj directory be?
#
.if !target(whereobj)
whereobj:
	@echo ${.OBJDIR}
.endif

# Same check in bsd.progs.mk
.if ${CANONICALOBJDIR} != ${.CURDIR} && exists(${CANONICALOBJDIR}/) && \
    (${MK_AUTO_OBJ} == "no" || ${.TARGETS:Nclean*:N*clean:Ndestroy*} == "")
cleanobj:
	-rm -rf ${CANONICALOBJDIR}
.else
cleanobj: clean cleandepend
.endif
	@if [ -L ${.CURDIR}/obj ]; then rm -f ${.CURDIR}/obj; fi

# Tell bmake not to look for generated files via .PATH
NOPATH_FILES+=	${CLEANFILES}
.if !empty(NOPATH_FILES)
.NOPATH: ${NOPATH_FILES}
.endif

.if !target(clean)
clean:
.if defined(CLEANFILES) && !empty(CLEANFILES)
	rm -f ${CLEANFILES}
.endif
.if defined(CLEANDIRS) && !empty(CLEANDIRS)
	-rm -rf ${CLEANDIRS}
.endif
.endif
.ORDER: clean all
.if ${MK_AUTO_OBJ} == "yes"
.ORDER: cleanobj all
.ORDER: cleandir all
.endif

.include <bsd.subdir.mk>

cleandir: .WAIT cleanobj

.if make(destroy*) && defined(OBJROOT)
# this (rm -rf objdir) is much faster and more reliable than cleaning.

# just in case we are playing games with these...
_OBJDIR?= ${.OBJDIR}
_CURDIR?= ${.CURDIR}

# destroy almost everything
destroy: destroy-all
destroy-all:

# just remove our objdir
destroy-arch: .NOMETA
.if ${_OBJDIR} != ${_CURDIR}
	cd ${_CURDIR} && rm -rf ${_OBJDIR}
.endif

.if defined(HOST_OBJTOP)
destroy-host: destroy.host
destroy.host: .NOMETA
	cd ${_CURDIR} && rm -rf ${HOST_OBJTOP}/${RELDIR:N.}
.endif

.if make(destroy-all) && ${RELDIR} == "."
destroy-all: destroy-stage
.endif

# remove the stage tree
destroy-stage: .NOMETA
.if defined(STAGE_ROOT)
	cd ${_CURDIR} && rm -rf ${STAGE_ROOT}
.endif

# allow parallel destruction
_destroy_machine_list = common host ${ALL_MACHINE_LIST}
.for m in ${_destroy_machine_list:O:u}
destroy-all: destroy.$m
.if !target(destroy.$m)
destroy.$m: .NOMETA
.if ${_OBJDIR} != ${_CURDIR}
	cd ${_CURDIR} && rm -rf ${OBJROOT}$m*/${RELDIR:N.}
.endif
.endif
.endfor

.endif

.endif # !target(__<bsd.obj.mk>__)
