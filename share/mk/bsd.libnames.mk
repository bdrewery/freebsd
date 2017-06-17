# $FreeBSD$

# The include file <bsd.libnames.mk> define library names.
# Other include files (e.g. bsd.prog.mk, bsd.lib.mk) include this
# file where necessary.

.if !target(__<bsd.init.mk>__)
.error bsd.libnames.mk cannot be included directly.
.endif

.sinclude <src.libnames.mk>

# Src directory locations are also defined in src.libnames.mk.

LIBCRT0?=	${DESTDIR}${BASE_LIBDIR}/crt0.o

LIB80211?=	${DESTDIR}${BASE_LIBDIR}/lib80211.a
LIBALIAS?=	${DESTDIR}${BASE_LIBDIR}/libalias.a
LIBARCHIVE?=	${DESTDIR}${BASE_LIBDIR}/libarchive.a
LIBASN1?=	${DESTDIR}${BASE_LIBDIR}/libasn1.a
LIBATM?=	${DESTDIR}${BASE_LIBDIR}/libatm.a
LIBAUDITD?=	${DESTDIR}${BASE_LIBDIR}/libauditd.a
LIBAVL?=	${DESTDIR}${BASE_LIBDIR}/libavl.a
LIBBEGEMOT?=	${DESTDIR}${BASE_LIBDIR}/libbegemot.a
LIBBLACKLIST?=	${DESTDIR}${BASE_LIBDIR}/libblacklist.a
LIBBLUETOOTH?=	${DESTDIR}${BASE_LIBDIR}/libbluetooth.a
LIBBSDXML?=	${DESTDIR}${BASE_LIBDIR}/libbsdxml.a
LIBBSM?=	${DESTDIR}${BASE_LIBDIR}/libbsm.a
LIBBSNMP?=	${DESTDIR}${BASE_LIBDIR}/libbsnmp.a
LIBBZ2?=	${DESTDIR}${BASE_LIBDIR}/libbz2.a
LIBC?=		${DESTDIR}${BASE_LIBDIR}/libc.a
LIBCALENDAR?=	${DESTDIR}${BASE_LIBDIR}/libcalendar.a
LIBCAM?=	${DESTDIR}${BASE_LIBDIR}/libcam.a
LIBCAP_DNS?=	${DESTDIR}${BASE_LIBDIR}/libcap_dns.a
LIBCAP_GRP?=	${DESTDIR}${BASE_LIBDIR}/libcap_grp.a
LIBCAP_PWD?=	${DESTDIR}${BASE_LIBDIR}/libcap_pwd.a
LIBCAP_RANDOM?=	${DESTDIR}${BASE_LIBDIR}/libcap_random.a
LIBCAP_SYSCTL?=	${DESTDIR}${BASE_LIBDIR}/libcap_sysctl.a
LIBCASPER?=	${DESTDIR}${BASE_LIBDIR}/libcasper.a
LIBCOMPAT?=	${DESTDIR}${BASE_LIBDIR}/libcompat.a
LIBCOMPILER_RT?=${DESTDIR}${BASE_LIBDIR}/libcompiler_rt.a
LIBCOM_ERR?=	${DESTDIR}${BASE_LIBDIR}/libcom_err.a
LIBCPLUSPLUS?=	${DESTDIR}${BASE_LIBDIR}/libc++.a
LIBCRYPT?=	${DESTDIR}${BASE_LIBDIR}/libcrypt.a
LIBCRYPTO?=	${DESTDIR}${BASE_LIBDIR}/libcrypto.a
LIBCTF?=	${DESTDIR}${BASE_LIBDIR}/libctf.a
LIBCURSES?=	${DESTDIR}${BASE_LIBDIR}/libcurses.a
LIBCUSE?=	${DESTDIR}${BASE_LIBDIR}/libcuse.a
LIBCXGB4?=	${DESTDIR}${BASE_LIBDIR}/libcxgb4.a
LIBCXXRT?=	${DESTDIR}${BASE_LIBDIR}/libcxxrt.a
LIBC_PIC?=	${DESTDIR}${BASE_LIBDIR}/libc_pic.a
LIBDEVCTL?=	${DESTDIR}${BASE_LIBDIR}/libdevctl.a
LIBDEVDCTL?=	${DESTDIR}${BASE_LIBDIR}/libdevdctl.a
LIBDEVINFO?=	${DESTDIR}${BASE_LIBDIR}/libdevinfo.a
LIBDEVSTAT?=	${DESTDIR}${BASE_LIBDIR}/libdevstat.a
LIBDIALOG?=	${DESTDIR}${BASE_LIBDIR}/libdialog.a
LIBDNS?=	${DESTDIR}${BASE_LIBDIR}/libdns.a
LIBDPV?=	${DESTDIR}${BASE_LIBDIR}/libdpv.a
LIBDTRACE?=	${DESTDIR}${BASE_LIBDIR}/libdtrace.a
LIBDWARF?=	${DESTDIR}${BASE_LIBDIR}/libdwarf.a
LIBEDIT?=	${DESTDIR}${BASE_LIBDIR}/libedit.a
LIBEFIVAR?=	${DESTDIR}${BASE_LIBDIR}/libefivar.a
LIBELF?=	${DESTDIR}${BASE_LIBDIR}/libelf.a
LIBEXECINFO?=	${DESTDIR}${BASE_LIBDIR}/libexecinfo.a
LIBFETCH?=	${DESTDIR}${BASE_LIBDIR}/libfetch.a
LIBFIGPAR?=	${DESTDIR}${BASE_LIBDIR}/libfigpar.a
LIBFL?=		"don't use LIBFL, use LIBL"
LIBFORM?=	${DESTDIR}${BASE_LIBDIR}/libform.a
LIBG2C?=	${DESTDIR}${BASE_LIBDIR}/libg2c.a
LIBGEOM?=	${DESTDIR}${BASE_LIBDIR}/libgeom.a
LIBGNUREGEX?=	${DESTDIR}${BASE_LIBDIR}/libgnuregex.a
LIBGPIO?=	${DESTDIR}${BASE_LIBDIR}/libgpio.a
LIBGSSAPI?=	${DESTDIR}${BASE_LIBDIR}/libgssapi.a
LIBGSSAPI_KRB5?= ${DESTDIR}${BASE_LIBDIR}/libgssapi_krb5.a
LIBHDB?=	${DESTDIR}${BASE_LIBDIR}/libhdb.a
LIBHEIMBASE?=	${DESTDIR}${BASE_LIBDIR}/libheimbase.a
LIBHEIMNTLM?=	${DESTDIR}${BASE_LIBDIR}/libheimntlm.a
LIBHEIMSQLITE?=	${DESTDIR}${BASE_LIBDIR}/libheimsqlite.a
LIBHX509?=	${DESTDIR}${BASE_LIBDIR}/libhx509.a
LIBIBCM?=	${DESTDIR}${BASE_LIBDIR}/libibcm.a
LIBIBCOMMON?=	${DESTDIR}${BASE_LIBDIR}/libibcommon.a
LIBIBMAD?=	${DESTDIR}${BASE_LIBDIR}/libibmad.a
LIBIBSDP?=	${DESTDIR}${BASE_LIBDIR}/libibsdp.a
LIBIBUMAD?=	${DESTDIR}${BASE_LIBDIR}/libibumad.a
LIBIBVERBS?=	${DESTDIR}${BASE_LIBDIR}/libibverbs.a
LIBIFCONFIG?=	${DESTDIR}${BASE_LIBDIR}/libifconfig.a
LIBIPSEC?=	${DESTDIR}${BASE_LIBDIR}/libipsec.a
LIBJAIL?=	${DESTDIR}${BASE_LIBDIR}/libjail.a
LIBKADM5CLNT?=	${DESTDIR}${BASE_LIBDIR}/libkadm5clnt.a
LIBKADM5SRV?=	${DESTDIR}${BASE_LIBDIR}/libkadm5srv.a
LIBKAFS5?=	${DESTDIR}${BASE_LIBDIR}/libkafs5.a
LIBKDC?=	${DESTDIR}${BASE_LIBDIR}/libkdc.a
LIBKEYCAP?=	${DESTDIR}${BASE_LIBDIR}/libkeycap.a
LIBKICONV?=	${DESTDIR}${BASE_LIBDIR}/libkiconv.a
LIBKRB5?=	${DESTDIR}${BASE_LIBDIR}/libkrb5.a
LIBKVM?=	${DESTDIR}${BASE_LIBDIR}/libkvm.a
LIBL?=		${DESTDIR}${BASE_LIBDIR}/libl.a
LIBLN?=		"don't use LIBLN, use LIBL"
LIBLZMA?=	${DESTDIR}${BASE_LIBDIR}/liblzma.a
LIBM?=		${DESTDIR}${BASE_LIBDIR}/libm.a
LIBMAGIC?=	${DESTDIR}${BASE_LIBDIR}/libmagic.a
LIBMD?=		${DESTDIR}${BASE_LIBDIR}/libmd.a
LIBMEMSTAT?=	${DESTDIR}${BASE_LIBDIR}/libmemstat.a
LIBMENU?=	${DESTDIR}${BASE_LIBDIR}/libmenu.a
LIBMILTER?=	${DESTDIR}${BASE_LIBDIR}/libmilter.a
LIBMLX4?=	${DESTDIR}${BASE_LIBDIR}/libmlx4.a
LIBMP?=		${DESTDIR}${BASE_LIBDIR}/libmp.a
LIBMT?=		${DESTDIR}${BASE_LIBDIR}/libmt.a
LIBMTHCA?=	${DESTDIR}${BASE_LIBDIR}/libmthca.a
LIBNANDFS?=	${DESTDIR}${BASE_LIBDIR}/libnandfs.a
LIBNCURSES?=	${DESTDIR}${BASE_LIBDIR}/libncurses.a
LIBNCURSESW?=	${DESTDIR}${BASE_LIBDIR}/libncursesw.a
LIBNETGRAPH?=	${DESTDIR}${BASE_LIBDIR}/libnetgraph.a
LIBNGATM?=	${DESTDIR}${BASE_LIBDIR}/libngatm.a
LIBNV?=		${DESTDIR}${BASE_LIBDIR}/libnv.a
LIBNVPAIR?=	${DESTDIR}${BASE_LIBDIR}/libnvpair.a
LIBOPENSM?=	${DESTDIR}${BASE_LIBDIR}/libopensm.a
LIBOPIE?=	${DESTDIR}${BASE_LIBDIR}/libopie.a
LIBOSMCOMP?=	${DESTDIR}${BASE_LIBDIR}/libosmcomp.a
LIBOSMVENDOR?=	${DESTDIR}${BASE_LIBDIR}/libosmvendor.a
LIBPAM?=	${DESTDIR}${BASE_LIBDIR}/libpam.a
LIBPANEL?=	${DESTDIR}${BASE_LIBDIR}/libpanel.a
LIBPANELW?=	${DESTDIR}${BASE_LIBDIR}/libpanelw.a
LIBPCAP?=	${DESTDIR}${BASE_LIBDIR}/libpcap.a
LIBPJDLOG?=	${DESTDIR}${BASE_LIBDIR}/libpjdlog.a
LIBPMC?=	${DESTDIR}${BASE_LIBDIR}/libpmc.a
LIBPROC?=	${DESTDIR}${BASE_LIBDIR}/libproc.a
LIBPROCSTAT?=	${DESTDIR}${BASE_LIBDIR}/libprocstat.a
LIBPTHREAD?=	${DESTDIR}${BASE_LIBDIR}/libpthread.a
LIBRADIUS?=	${DESTDIR}${BASE_LIBDIR}/libradius.a
LIBRDMACM?=	${DESTDIR}${BASE_LIBDIR}/librdmacm.a
LIBROKEN?=	${DESTDIR}${BASE_LIBDIR}/libroken.a
LIBRPCSEC_GSS?=	${DESTDIR}${BASE_LIBDIR}/librpcsec_gss.a
LIBRPCSVC?=	${DESTDIR}${BASE_LIBDIR}/librpcsvc.a
LIBRT?=		${DESTDIR}${BASE_LIBDIR}/librt.a
LIBRTLD_DB?=	${DESTDIR}${BASE_LIBDIR}/librtld_db.a
LIBSBUF?=	${DESTDIR}${BASE_LIBDIR}/libsbuf.a
LIBSDP?=	${DESTDIR}${BASE_LIBDIR}/libsdp.a
LIBSMB?=	${DESTDIR}${BASE_LIBDIR}/libsmb.a
LIBSSL?=	${DESTDIR}${BASE_LIBDIR}/libssl.a
LIBSSP_NONSHARED?=	${DESTDIR}${BASE_LIBDIR}/libssp_nonshared.a
LIBSTAND?=	${DESTDIR}${BASE_LIBDIR}/libstand.a
LIBSTDCPLUSPLUS?= ${DESTDIR}${BASE_LIBDIR}/libstdc++.a
LIBSTDTHREADS?=	${DESTDIR}${BASE_LIBDIR}/libstdthreads.a
LIBSYSDECODE?=	${DESTDIR}${BASE_LIBDIR}/libsysdecode.a
LIBTACPLUS?=	${DESTDIR}${BASE_LIBDIR}/libtacplus.a
LIBTERMCAP?=	${DESTDIR}${BASE_LIBDIR}/libtermcap.a
LIBTERMCAPW?=	${DESTDIR}${BASE_LIBDIR}/libtermcapw.a
LIBTERMLIB?=	"don't use LIBTERMLIB, use LIBTERMCAP"
LIBTINFO?=	"don't use LIBTINFO, use LIBNCURSES"
LIBUFS?=	${DESTDIR}${BASE_LIBDIR}/libufs.a
LIBUGIDFW?=	${DESTDIR}${BASE_LIBDIR}/libugidfw.a
LIBULOG?=	${DESTDIR}${BASE_LIBDIR}/libulog.a
LIBUMEM?=	${DESTDIR}${BASE_LIBDIR}/libumem.a
LIBUSB?=	${DESTDIR}${BASE_LIBDIR}/libusb.a
LIBUSBHID?=	${DESTDIR}${BASE_LIBDIR}/libusbhid.a
LIBUTIL?=	${DESTDIR}${BASE_LIBDIR}/libutil.a
LIBUUTIL?=	${DESTDIR}${BASE_LIBDIR}/libuutil.a
LIBVGL?=	${DESTDIR}${BASE_LIBDIR}/libvgl.a
LIBVMMAPI?=	${DESTDIR}${BASE_LIBDIR}/libvmmapi.a
LIBWIND?=	${DESTDIR}${BASE_LIBDIR}/libwind.a
LIBWRAP?=	${DESTDIR}${BASE_LIBDIR}/libwrap.a
LIBXO?=		${DESTDIR}${BASE_LIBDIR}/libxo.a
LIBXPG4?=	${DESTDIR}${BASE_LIBDIR}/libxpg4.a
LIBY?=		${DESTDIR}${BASE_LIBDIR}/liby.a
LIBYPCLNT?=	${DESTDIR}${BASE_LIBDIR}/libypclnt.a
LIBZ?=		${DESTDIR}${BASE_LIBDIR}/libz.a
LIBZFS?=	${DESTDIR}${BASE_LIBDIR}/libzfs.a
LIBZFS_CORE?=	${DESTDIR}${BASE_LIBDIR}/libzfs_core.a
LIBZPOOL?=	${DESTDIR}${BASE_LIBDIR}/libzpool.a

# enforce the 2 -lpthread and -lc to always be the last in that exact order
.if defined(LDADD)
.if ${LDADD:M-lpthread}
LDADD:=	${LDADD:N-lpthread} -lpthread
.endif
.if ${LDADD:M-lc}
LDADD:=	${LDADD:N-lc} -lc
.endif
.endif

# Only do this for src builds.
.if defined(SRCTOP)
.if defined(_LIBRARIES) && defined(LIB) && \
    ${_LIBRARIES:M${LIB}} != ""
.if !defined(LIB${LIB:tu})
.error ${.CURDIR}: Missing value for LIB${LIB:tu} in ${_this:T}.  Likely should be: LIB${LIB:tu}?= $${DESTDIR}$${BASE_LIBDIR}/lib${LIB}.a
.endif
.endif

# Derive LIB*SRCDIR from LIB*DIR
.for lib in ${_LIBRARIES}
LIB${lib:tu}SRCDIR?=	${SRCTOP}/${LIB${lib:tu}DIR:S,^${OBJTOP}/,,}
.endfor
.endif
