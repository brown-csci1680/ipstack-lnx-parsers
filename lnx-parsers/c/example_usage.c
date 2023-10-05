/*
 * C lnx parser demo
 * This file contains an example of how to use the C lnx parser.
 * Take a look at this for an example of how to call the parser and then
 * traverse the lnxconfig_t struct.
 *
 * For each directive, there is a helper function demonstrating how to
 * fetch and print out each field, which should be a useful starting
 * point to understand how to work with the data.
 *
 * NOTE: This parser uses the "list.h" linked-list implementation
 * provided in the c-utils repo.  If you are using different list.h,
 * you may need to rename the list.h here to avoid conflicts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lnxconfig.h"
#include "list.h"


void print_interface(lnx_interface_t *iface);
void print_neighbor(lnx_neighbor_t *neigh);
void print_rip_neighbor(lnx_rip_neighbor_t *rip_neigh);
void print_static_route(lnx_static_route_t *route);

int main(int argc, char **argv) {
    if (argc != 2) {
	printf("Usage %s <lnx file>\n", argv[0]);
	return 1;
    }

    lnxconfig_t *config = lnxconfig_parse(argv[1]);
    if (config == NULL) {
	printf("Error parsing config file, aborting\n");
	return 1;
    }

    lnx_interface_t *iface;
    list_iterate_begin(&config->interfaces, iface, lnx_interface_t, link) {
	print_interface(iface);
    } list_iterate_end();

    lnx_neighbor_t *neighbor;
    list_iterate_begin(&config->neighbors, neighbor, lnx_neighbor_t, link) {
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

    lnx_rip_neighbor_t *rip_neighbor;
    list_iterate_begin(&config->rip_neighbors, rip_neighbor, lnx_rip_neighbor_t, link) {
	print_rip_neighbor(rip_neighbor);
    } list_iterate_end();

    lnxconfig_destroy(config);

    return 0;
}


void print_interface(lnx_interface_t *iface) {
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

void print_neighbor(lnx_neighbor_t *neigh) {
    char dest_addr[INET_ADDRSTRLEN];
    char udp_addr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &neigh->dest_addr, dest_addr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &neigh->udp_addr, udp_addr, INET_ADDRSTRLEN);

    printf("neighbor %s at %s:%hu via %s\n",
	   dest_addr,
	   udp_addr,
	   neigh->udp_port, neigh->ifname);
}

void print_rip_neighbor(lnx_rip_neighbor_t *rip_neigh) {
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
