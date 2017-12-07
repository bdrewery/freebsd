SUBDIR= \
	libibverbs \
	libibcm \
	libibumad \
	librdmacm \
	libibmad \
	libosmcomp \
	libosmvendor \
	libcxgb4 \
	libmlx4 \
	libmlx5 \
	libibnetdisc \
	libopensm

SUBDIR_DEPEND_libcxgb4=	libibverbs
SUBDIR_DEPEND_libibcm=	libibverbs
SUBDIR_DEPEND_libibmad=	libibumad
SUBDIR_DEPEND_libibnetdisc=	libosmcomp libibmad libibumad
SUBDIR_DEPEND_libmlx4=	libibverbs
SUBDIR_DEPEND_libmlx5=	libibverbs
SUBDIR_DEPEND_libosmvendor=	libibumad
SUBDIR_DEPEND_librdmacm=	libibverbs
SUBDIR_PARALLEL=

.include <bsd.subdir.mk>
