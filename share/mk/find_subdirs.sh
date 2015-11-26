#! /bin/sh
set -e

# XXX: Need to import all options from environment somehow, it may be enough to
# just pass in MAKEFLAGS and then let src.conf work - but what about MAKEARGS?
export NEED_SUBDIR=yes
CURDIR=$(realpath "${1:-.}")
MODE="${2:-SUBDIR}"
: ${MAKE:=make}
: ${SRCTOP:=$(${MAKE} -V SRCTOP:tA)}
: ${TARGET_SPEC:=$(${MAKE} -V TARGET_SPEC)}
# Store tempfiles in the objdir if possible.
if [ -n "${OBJROOT}" ] && [ -d "${OBJROOT}" ]; then
	export TMPDIR="${OBJROOT%/}"
fi

# Avoid prefixing all subdirs with first CURDIR.
cd "${CURDIR}"
RELDIR="${CURDIR#${SRCTOP}}"
RELDIR="${RELDIR#/}"
curdir=

# Allow local.dirdeps.mk to be used to bootstrap deps
unset MAKELEVEL
#export BUILD_DIRDEPS=no

list() {
	local mode="${1}"
	local curdir="${2}"
	local makeargs make_results subdirs dirdeps subdir reldir
	local dirdep dirdeps_qual target_spec machine machine_arch

	if [ -n "${RELDIR}" -a -n "${curdir}" ]; then
		reldir="${RELDIR}/${curdir}"
	else
		reldir="${RELDIR}${curdir}"
	fi

	# Special handling for top-level.
	makeargs=
	[ -e "${curdir:-.}/Makefile.inc1" ] &&
	    makeargs="TARGET=amd64 TARGET_ARCH=amd64 -f Makefile.inc1"

	machine="${TARGET_SPEC%,*}"
	# Trim off arch for host and avoid test dependencies.
	if [ "${machine}" = "host" ]; then
		machine_arch=
		export MK_TESTS=no
	else
		machine_arch="${TARGET_SPEC#*,}"
	fi
	if ! make_results=$(\
	    MACHINE="${machine}" \
	    MACHINE_ARCH="${machine_arch}" \
	    ${MAKE} ${makeargs} -C "${SRCTOP}/${reldir%.host}" \
	    -V "\${SUBDIR:N.WAIT}" -V '${DIRDEPS:${DEP_DIRDEPS_FILTER:ts:}:M*}' |
	    paste -s -d '!' -); then
		echo "Error looking up SUBDIR for ${curdir}" >&2
		return 1
	fi
	subdirs="${make_results%!*}"
	dirdeps="${make_results#*!}"

	if [ -n "${subdirs}" ]; then
		for subdir in ${subdirs}; do
			# Parallelize top-level lookups if possible.
			${COPARALLEL_HANDLE:+coparallel_run} \
			    list "${mode}" "${curdir}${curdir:+/}${subdir}" || return $?
		done
	fi

	case ${mode} in
	DIRDEPS)
		# Show curdir if not already in DIRDEPS (which happens if
		# there is already a Makefile.depend).
		dirdeps="${dirdeps} "
		if ! [ -z "${dirdeps##${reldir}.${TARGET_SPEC} *}" ]; then
			echo -n "${reldir}.${TARGET_SPEC} "
		fi
		echo "${dirdeps% }"
		;;
	DIRDEPS_GRAPH)
		# Show curdir if not already in DIRDEPS (which happens if
		# there is already a Makefile.depend).
		dirdeps="${dirdeps} "
		dirdeps_qual=
		for dirdep in ${dirdeps}; do
			if [ -n "${dirdep%%*.*}" ]; then
				dirdep="${dirdep}.${TARGET_SPEC}"
			fi
			# Skip self-reference.
			if [ -z "${dirdep##${reldir}.${TARGET_SPEC}}" ]; then
				continue;
			fi
			dirdeps_qual="${dirdeps_qual}${dirdeps_qual:+ }${SRCTOP}/${dirdep}"
		done
		# Build the dependency graph.
		echo "${SRCTOP}/${reldir%.host}.${TARGET_SPEC}:${dirdeps_qual:+ }${dirdeps_qual}" \
		    >> "${DIRDEPS_GRAPH}"
		# The directory itself is sometimes prepended.
		dirdeps="${dirdeps#${reldir}.${TARGET_SPEC}}"
		dirdeps="${dirdeps# }"
		export PROCESSED="${PROCESSED} ${reldir}"
		echo "DIRDEPS+= ${reldir}" >> "${DIRDEPS_GRAPH}"
		for dirdep in ${dirdeps}; do
			case " ${PROCESSED} " in
			*\ ${dirdep}\ *) continue ;;
			esac
			target_spec="${TARGET_SPEC}"
			case "${dirdep}" in
			*.host) target_spec="host" ;;
			esac
			export PROCESSED="${PROCESSED} ${dirdep}"
			TARGET_SPEC="${target_spec}" RELDIR= \
			    list "${mode}" "${dirdep}"
		done
		;;
	SUBDIR)
		# Don't print top-level directory, which is empty.
		if [ -n "${curdir}" ]; then
			echo "${curdir}"
		fi
		;;
	esac
}

if [ "${MODE}" = "DIRDEPS_GRAPH" ]; then
	# Use a tempfile that the parent make will include to define
	# the DIRDEPS graph and DIRDEPS to build.
	export DIRDEPS_GRAPH=$(mktemp -t find_subdirs)
fi

# Utilize Poudriere scripts to parallelize the lookups.
false &&
if [ -f /usr/local/share/poudriere/include/hash.sh ] &&
    [ -f /usr/local/share/poudriere/include/parallel.sh ]; then
	. /root/git/poudriere/src/share/poudriere/include/hash.sh
	. /root/git/poudriere/src/share/poudriere/include/parallel.sh
	if type coparallel_start 2>/dev/null >&2; then
		: ${MAKE_JOBS:=$(sysctl -n hw.ncpu)}
		# No real benefit found above 5-7 as this is mostly IO-bound.
		[ ${MAKE_JOBS} -gt 5 ] && MAKE_JOBS=5
		coparallel_start ${MAKE_JOBS}
		trap coparallel_sighandler INT TERM
		trap coparallel_stop EXIT
	fi
fi

list "${MODE}" "${curdir}"

if [ "${MODE}" = "DIRDEPS_GRAPH" ]; then
	${COPARALLEL_HANDLE:+coparallel_wait}
	cat >> "${DIRDEPS_GRAPH}" <<-EOF
	DIRDEPS:= \${DIRDEPS:O:u}
	EOF
	echo "${DIRDEPS_GRAPH}"
fi
