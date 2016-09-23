#ifndef _COMPAT_OPENSOLARIS_UUID_H
#define _COMPAT_OPENSOLARIS_UUID_H

#include <uuid.h>

#define UUID_LEN        16
#define UUID_PRINTABLE_STRING_LENGTH 37
typedef uchar_t         uuid_t[UUID_LEN];

inline int
uuid_is_null(uuid_t uu)
{

        return (uuid_is_nil(uu, NULL));
}

inline void
uuid_generate(uuid_t uu)
{

        uuid_create(&:e 
}

#endif
