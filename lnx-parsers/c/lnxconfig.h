#ifndef __LNXCONFIG_H__
#define __LNXCONFIG_H__

#include <stddef.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#include "list.h"

#define LNX_IFNAME_MAX 64

typedef enum {
    ROUTING_MODE_NONE   = 0,   // Unspecified
    ROUTING_MODE_STATIC = 1,   // Static routes only (no RIP, used for hosts)
    ROUTING_MODE_RIP    = 2,   // Use RIP (default for routers)
} routing_mode_t;

struct lnx_interface_t {
    char name[LNX_IFNAME_MAX];

    struct in_addr assigned_ip;
    int prefix_len;


    struct in_addr udp_addr;
    uint16_t udp_port;

    list_link_t link;
};


struct lnx_neighbor_t {
    struct in_addr dest_addr;

    struct in_addr udp_addr;
    uint16_t udp_port;

    char ifname[LNX_IFNAME_MAX];

    list_link_t link;
};

struct lnx_rip_neighbor_t {
    struct in_addr dest;

    list_link_t link;
};

typedef struct {
    struct in_addr network_addr;
    int prefix_len;

    struct in_addr next_hop;

    list_link_t link;
} lnx_static_route_t;


struct lnxconfig_t {
    list_t interfaces; // list of type struct lnx_interface_t
    list_t neighbors; // list of type struct lnx_neighbor_t
    list_t rip_neighbors;  // list of type struct lnx_advertise_neighbor_t
    list_t static_routes; // list of type lnx_static_route_t
    // struct lnx_interface_t *interfaces;
    // struct lnx_neighbor_t *neighbors;
    // struct lnx_advertise_neighbor_t *rip_neighbors;

    routing_mode_t routing_mode;
};

struct lnxconfig_t *lnxconfig_parse(char *config_file);
void lnxconfig_destroy(struct lnxconfig_t *config);

#endif
