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

# Avoid prefixing all subdirs with first CURDIR.
cd "${CURDIR}"
RELDIR="${CURDIR#${SRCTOP}}"
RELDIR="${RELDIR#/}"
curdir=

# Allow local.dirdeps.mk to be used to bootstrap deps
unset MAKELEVEL

list() {
	local mode="${1}"
	local curdir="${2}"
	local makeargs make_results subdirs dirdeps subdir subdir_subst reldir

	# Special handling for top-level.
	makeargs=
	[ -e "${curdir:-.}/Makefile.inc1" ] &&
	    makeargs="TARGET=amd64 TARGET_ARCH=amd64 -f Makefile.inc1"

	subdir_subst=
	false &&
	[ "${curdir}" != "." ] &&
	    subdir_subst="C,^,${curdir}/,"

	if ! make_results=$(${MAKE} ${makeargs} -C "${curdir:-.}" \
	    -V "\${SUBDIR:N.WAIT:${subdir_subst}}" -V DIRDEPS:M* |
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
		if [ -n "${RELDIR}" -a -n "${curdir}" ]; then
			reldir="${RELDIR}/${curdir}"
		else
			reldir="${RELDIR}${curdir}"
		fi
		# Show curdir if not already in DIRDEPS (which happens if
		# there is already a Makefile.depend).
		dirdeps="${dirdeps} "
		if ! [ -z "${dirdeps##${reldir} *}" -o \
		    -z "${dirdeps##${reldir}.${TARGET_SPEC} *}" ]; then
			echo -n "${reldir}.${TARGET_SPEC} "
		fi
		echo "${dirdeps% }"
		;;
	SUBDIR)
		# Don't print top-level directory, which is empty.
		if [ -n "${curdir}" ]; then
			echo "${curdir}"
		fi
		;;
	esac
}

# Utilize Poudriere scripts to parallelize the lookups.
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
