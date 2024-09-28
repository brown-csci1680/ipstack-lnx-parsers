#include "lnxconfig.h"
#include <iostream>
#include <sys/socket.h>

void print_interface(const lnx::Interface &interface);
void print_neighbor(const lnx::Neighbor &neighbor);
void print_static_route(const lnx::StaticRoute &static_route);
void print_rip_neighbor(const lnx::RIPNeighbor &rip_neighbor);

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <lnx file>" << std::endl;
		return 1;
	}

	lnx::Config conf(argv[1]);
	for (auto &i : conf.interfaces()) print_interface(i);
	for (auto &n : conf.neighbors()) print_neighbor(n);
	std::cout << "routing " << (conf.routing_mode() == lnx::RoutingMode::RIP ? "rip" : "static") << std::endl;
	for (auto &s : conf.static_routes()) print_static_route(s);
	for (auto &r : conf.rip_neighbors()) print_rip_neighbor(r);
	std::cout << "rip periodic-update-rate "
		  << conf.rip_periodic_update_rate() << " # in milliseconds" << std::endl;
	std::cout << "rip route-timeout-threshold "
		  << conf.rip_timeout_threshold() << " # in millisecons" << std::endl;

	std::cout << "tcp rto-min "
		  << conf.tcp_rto_min() << " # in microseconds" << std::endl;
	std::cout << "tcp rto-max "
		  << conf.tcp_rto_max() << " # in microseconds" << std::endl;
}

std::string addr_to_str(const in_addr *addr) {
	char buf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, addr, buf, INET_ADDRSTRLEN);
	return buf;
}

void print_interface(const lnx::Interface &interface) {
	std::cout << "interface "
		<< interface.name << " "
		<< addr_to_str(&interface.assigned_ip) << "/"
		<< interface.prefix_len << " "
		<< addr_to_str(&interface.udp_addr) << ":"
		<< interface.udp_port << std::endl;
}

void print_neighbor(const lnx::Neighbor &neighbor) {
	std::cout << "neighbor "
		<< addr_to_str(&neighbor.dest_addr) << " at "
		<< addr_to_str(&neighbor.udp_addr) << ":"
		<< neighbor.udp_port << " via "
		<< neighbor.ifname << std::endl;
}

void print_static_route(const lnx::StaticRoute &static_route) {
	std::cout << "route "
		<< addr_to_str(&static_route.network_addr) << "/"
		<< static_route.prefix_len << " via "
		<< addr_to_str(&static_route.next_hop) << std::endl;
}

void print_rip_neighbor(const lnx::RIPNeighbor &rip_neighbor) {
	std::cout << "rip advertise-to " << addr_to_str(&rip_neighbor.dest) << std::endl;
}
