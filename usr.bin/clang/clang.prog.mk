# $FreeBSD$

.include "${SRCTOP}/lib/clang/clang.pre.mk"

CFLAGS+=	-I${RELOBJTOP}/lib/clang/libclang
CFLAGS+=	-I${RELOBJTOP}/lib/clang/libllvm

.include "${SRCTOP}/lib/clang/clang.build.mk"

LIBDEPS+=	clang
LIBDEPS+=	llvm

.for lib in ${LIBDEPS}
DPADD+=		${RELOBJTOP}/lib/clang/lib${lib}/lib${lib}.a
LDADD+=		${RELOBJTOP}/lib/clang/lib${lib}/lib${lib}.a
.endfor

PACKAGE=	clang

LIBADD+=	ncursesw
LIBADD+=	pthread

.include <bsd.prog.mk>
