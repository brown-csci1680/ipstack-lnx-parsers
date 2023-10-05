#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lnxconfig.h"
#include "list.h"


void print_interface(struct lnx_interface_t *iface) {
    char assigned_ip[INET_ADDRSTRLEN];
    char udp_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &iface->assigned_ip, assigned_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &iface->udp_addr, udp_ip, INET_ADDRSTRLEN);

    printf("interface %s %s/%d %s:%hu\n",
	   iface->name,
	   assigned_ip,
	   iface->prefix_len,
	   udp_ip,
	   iface->udp_port);
}

void print_neighbor(struct lnx_neighbor_t *neigh) {
    char dest_addr[INET_ADDRSTRLEN];
    char udp_addr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &neigh->dest_addr, dest_addr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &neigh->udp_addr, udp_addr, INET_ADDRSTRLEN);

    printf("neighbor %s at %s:%hu via %s\n",
	   dest_addr,
	   udp_addr,
	   neigh->udp_port, neigh->ifname);
}

void print_rip_neighbor(struct lnx_rip_neighbor_t *rip_neigh) {
    char addr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &rip_neigh->dest, addr, INET_ADDRSTRLEN);

    printf("rip advertise-to %s\n", addr);
}

void print_static_route(lnx_static_route_t *route) {
    char network_addr[INET_ADDRSTRLEN];
    char next_hop_addr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &route->network_addr, network_addr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &route->next_hop, next_hop_addr, INET_ADDRSTRLEN);

    printf("route %s/%d via %s\n", network_addr, route->prefix_len, next_hop_addr);
}


int main(int argc, char **argv) {

#if 0
    char *input = "neighbor 10.2.0.3 at 127.0.0.1:5003 via if0 # thing";
    int tokens;

    char ifname[1024];
    char ip_addr[1024];
    char udp_addr[1024];
    int port;
    int len = 42;
    tokens = sscanf(input, "neighbor %32s at %32[^:]:%d via %32[^ #]",
		    ip_addr, udp_addr, &port, ifname);
    printf("Found %d tokens:  ADDR:%s UDP:%s PORT:%d IF:%s\n",
	   tokens, ip_addr, udp_addr, port, ifname);
    exit(0);
#endif
#if 1
    struct lnxconfig_t *config = lnxconfig_parse(argv[1]);

    struct lnx_interface_t *iface;
    list_iterate_begin(&config->interfaces, iface, struct lnx_interface_t, link) {
	print_interface(iface);
    } list_iterate_end();

    struct lnx_neighbor_t *neighbor;
    list_iterate_begin(&config->neighbors, neighbor, struct lnx_neighbor_t, link) {
	print_neighbor(neighbor);
    } list_iterate_end();

    printf("routing %s\n",
	   (config->routing_mode == ROUTING_MODE_NONE) ? "none" : // Should not happen
	   (config->routing_mode == ROUTING_MODE_RIP) ? "rip" :
	   (config->routing_mode == ROUTING_MODE_STATIC) ? "static" : "UNKNOWN");


    lnx_static_route_t *route;
    list_iterate_begin(&config->static_routes, route, lnx_static_route_t, link) {
	print_static_route(route);
    } list_iterate_end();

    struct lnx_rip_neighbor_t *rip_neighbor;
    list_iterate_begin(&config->rip_neighbors, rip_neighbor, struct lnx_rip_neighbor_t, link) {
	print_rip_neighbor(rip_neighbor);
    } list_iterate_end();

#endif
    return 0;
}
