#! /bin/sh
set -e

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
			echo -n "${reldir} "
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

list "${MODE}" "${curdir}"
