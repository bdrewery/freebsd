#ifndef OPENIB_OSD_H
#define OPENIB_OSD_H

#include <byteswap.h>
#include <sys/poll.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define htonll(x) (x)
#define ntohll(x) (x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define htonll(x)  bswap_64(x)
#define ntohll(x)  bswap_64(x)
#endif

#endif // OPENIB_OSD_H
