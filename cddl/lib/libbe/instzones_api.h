#ifndef _COMPAT_OPENSOLARIS_INSTZONES_API_H
#define _COMPAT_OPENSOLARIS_INSTZONES_API_H

typedef void zoneBrandList_t;
typedef intptr_t * zoneList_t;

#define GLOBAL_ZONEID 0

#define getzoneid() (GLOBAL_ZONEID)
#define z_get_nonglobal_zone_list_by_brand(brands) (NULL)
#define z_zlist_get_zonename(zone_list, zone_index) (NULL)
#define z_get_nonglobal_zone_list_by_brand(brands) (NULL)
#define z_zlist_get_zonepath(zone_list, zone_index) (NULL)
#define z_free_brand_list(brand)
#define z_free_zone_list(zone_list)
#define z_set_zone_root(root)
#define z_zlist_get_current_state(zone_list, zone_index) (ZONE_STATE_INCOMPLETE)

#define ZONE_STATE_CONFIGURED           0
#define ZONE_STATE_INCOMPLETE           1
#define ZONE_STATE_INSTALLED            2
#define ZONE_STATE_READY                3
#define ZONE_STATE_RUNNING              4
#define ZONE_STATE_SHUTTING_DOWN        5
#define ZONE_STATE_DOWN                 6
#define ZONE_STATE_MOUNTED              7

#endif
