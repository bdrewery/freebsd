# $FreeBSD$

SYSDIR?=${.CURDIR}/../../../..

# Avoid expanding DTS during tree-walks unless needed.
.if make(test-dts)
test-dts:
.for dts in ${DTS}
	@env MACHINE=`basename ${.CURDIR}` ${SYSDIR}/tools/fdt/make_dtb.sh ${SYSDIR} ${dts} /tmp

.endfor
.endif
