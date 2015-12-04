#	from: @(#)bsd.subdir.mk	5.9 (Berkeley) 2/1/91
# $FreeBSD$
#
# The include file <bsd.subdir.mk> contains the default targets
# for building subdirectories.
#
# For all of the directories listed in the variable SUBDIRS, the
# specified directory will be visited and the target made. There is
# also a default target which allows the command "make subdir" where
# subdir is any directory listed in the variable SUBDIRS.
#
#
# +++ variables +++
#
# DISTRIBUTION	Name of distribution. [base]
#
# SUBDIR	A list of subdirectories that should be built as well.
#		Each of the targets will execute the same target in the
#		subdirectories. SUBDIR.yes is automatically appeneded
#		to this list.
#
# +++ targets +++
#
#	distribute:
# 		This is a variant of install, which will
# 		put the stuff into the right "distribution".
#
# 	See ALL_SUBDIR_TARGETS for list of targets that will recurse.
# 	Custom targets can be added to SUBDIR_TARGETS in src.conf.
#
# 	Targets defined in STANDALONE_SUBDIR_TARGETS will always be ran
# 	with SUBDIR_PARALLEL and will not respect .WAIT or SUBDIR_DEPEND_
# 	values.
#

.if !target(__<bsd.subdir.mk>__)
__<bsd.subdir.mk>__:

ALL_SUBDIR_TARGETS= all all-man buildconfig buildfiles buildincludes \
		    checkdpadd clean cleandepend cleandir cleanilinks \
		    cleanobj depend distribute files includes installconfig \
		    installfiles installincludes install lint maninstall \
		    manlint obj objlink regress tags \
		    ${SUBDIR_TARGETS}

# Described above.
STANDALONE_SUBDIR_TARGETS?= obj checkdpadd clean cleandepend cleandir \
			    cleanilinks cleanobj

.include <bsd.init.mk>

.if !defined(NEED_SUBDIR)
.if ${.MAKE.LEVEL} == 0 && ${MK_DIRDEPS_BUILD} == "yes" && !empty(SUBDIR) && !(make(clean*) || make(destroy*))
.include <meta.subdir.mk>
# ignore this
_SUBDIR:
.endif
.endif
.if !target(_SUBDIR)

.if defined(SUBDIR)
SUBDIR:=${SUBDIR} ${SUBDIR.yes}
SUBDIR:=${SUBDIR:u}
.endif

DISTRIBUTION?=	base
.if !target(distribute)
distribute: .MAKE
.for dist in ${DISTRIBUTION}
	${_+_}cd ${.CURDIR}; \
	    ${MAKE} install -DNO_SUBDIR DESTDIR=${DISTDIR}/${dist} SHARED=copies
.endfor
.endif

# Convenience targets to run 'build${target}' and 'install${target}' when
# calling 'make ${target}'.
.for __target in files includes
.if !target(${__target})
${__target}:	build${__target} install${__target}
.ORDER:		build${__target} install${__target}
.endif
.endfor

# Make 'install' supports a before and after target.  Actual install
# hooks are placed in 'realinstall'.
.if !target(install)
.for __stage in before real after
.if !target(${__stage}install)
${__stage}install:
.endif
.endfor
install:	beforeinstall realinstall afterinstall
.ORDER:		beforeinstall realinstall afterinstall
.endif

# Subdir code shared among 'make <subdir>', 'make <target>' and SUBDIR_PARALLEL.
_SUBDIR_SH=	\
		if test -d ${.CURDIR}/$${dir}.${MACHINE_ARCH}; then \
			dir=$${dir}.${MACHINE_ARCH}; \
		fi; \
		${ECHODIR} "===> ${DIRPRFX}$${dir} ($${target})"; \
		cd ${.CURDIR}/$${dir}; \
		${MAKE} $${target} DIRPRFX=${DIRPRFX}$${dir}/

# _SUBDIR is defined only for backwards-compatibility and is not used
# by this file.
_SUBDIR: .USEBEFORE
.if defined(SUBDIR) && !empty(SUBDIR) && !defined(NO_SUBDIR)
	@${_+_}target=${.TARGET}; \
	    for dir in ${SUBDIR:N.WAIT}; do ( ${_SUBDIR_SH} ); done
.endif

# Support 'make directory'
${SUBDIR:N.WAIT}: .PHONY .MAKE
	${_+_}@target=all; \
	    dir=${.TARGET}; \
	    ${_SUBDIR_SH};

# Generate build targets for each subdir target combo.
.for __target in ${ALL_SUBDIR_TARGETS}
# Only recurse on directly-called targets.  I.e., don't recurse on dependencies
# such as 'install' becoming {before,real,after}install, just recurse
# 'install'.
.if make(${__target})
# Can ordering be skipped for this and SUBDIR_PARALLEL forced?
_is_standalone_target=	0
_subdir_parallel=	0
_skip_wait=		0
.if ${STANDALONE_SUBDIR_TARGETS:M${__target}}
_is_standalone_target=	1
.endif
.if defined(SUBDIR_PARALLEL) || ${_is_standalone_target} == 1
_subdir_parallel= 1
.endif
# Ignore .WAIT for standalone and non-SUBDIR_PARALLEL builds.
.if ${_is_standalone_target} == 1 || ${_subdir_parallel} == 0
_skip_wait=	1
.endif

.for __dir in ${SUBDIR}
.if ${__dir} == .WAIT
.if ${_skip_wait} == 0
__subdir_targets.${__target}+= .WAIT
.endif
.else
__subdir_targets.${__target}+= ${__target}_subdir_${__dir}
__deps=
.if ${_is_standalone_target} == 0
# For normal SUBDIR_PARALLEL, enforce ordering according to SUBDIR_DEPEND_*
.for __dep in ${SUBDIR_DEPEND_${__dir}}
__deps+= ${__target}_subdir_${__dep}
.endfor
.endif
# The actual _SUBDIR target
${__target}_subdir_${__dir}: .PHONY .MAKE ${__deps}
.if !defined(NO_SUBDIR)
	@${_+_}target=${__target}; \
	    dir=${__dir}; \
	    ${_SUBDIR_SH};
.endif
.endif	# __dir == .WAIT
.endfor	# __dir in ${SUBDIR}
# Hook all subdir targets into the main target so they build first.
${__target}: ${__subdir_targets.${__target}}
# Define ordering for non-SUBDIR_PARALLEL builds.
.if ${_subdir_parallel} == 0
.ORDER:	${__subdir_targets.${__target}}
.endif
.elif !target(${__target})
# Ensure all targets exist.
${__target}:
.endif	# make(${__target})
.endfor	# __target in ${ALL_SUBDIR_TARGETS}

.endif	# !target(_SUBDIR)

.endif
