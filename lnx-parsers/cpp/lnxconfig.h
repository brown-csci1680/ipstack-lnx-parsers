/*
 * lnxconfig.h - Header-only C++ lnx parser
 *
 * This file contains the public API and structs representing an lnx
 * file.  For an overview of how to use this parser, see demo.cpp.
 */

#ifndef __LNXCONFIG_H__
#define __LNXCONFIG_H__

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <fstream>

namespace lnx {

	/**
	 * This defines the routing mode. There are two modes, rip and static.
	 */
	enum class RoutingMode {
		/**
		 * Don’t use RIP and operate only on local and manually-specified routes.
		 * This is normally used for hosts. It can also be used on routers for
		 * testing purposes, like to make sure things work even without RIP.
		 */
		STATIC,

		/**
		 * Advertise known routes and learn new routes via RIP. Used for routers.
		 */
		RIP
	};

	/**
	 * Represents an interface of this node.
	 */
	struct Interface {
		/**
		 * The name of this interface, e.g. "if0"
		 */
		std::string name;

		/**
		 * Virtual IP address assigned to this interface, e.g. '10.0.0.1'
		 */
		in_addr assigned_ip;

		/**
		 * Integer 0-32 that defines the size of the network, together with the
		 * address. For example, 10.0.0.1/24 means that this interface has address
		 * 10.0.0.1 on the network 10.0.0.0/24 (ie, netmask 255.255.255.0).
		 */
		int prefix_len;

		/**
		 * Bind address for the UDP socket used to send/receive packets on this
		 * interface, e.g. `127.0.0.1`
		 */
		in_addr udp_addr;

		/**
		 * Port for the UDP socket for this interface, e.g. `5000`
		 */
		uint16_t udp_port;
	};

	/**
	 * Defines how to reach other nodes on the same network. Every node always
	 * knows the IP addresses and link-layer info for how to reach its neighbors
	 * (since we have no such thing as ARP).
	 */
	struct Neighbor {
		/**
		 * IP of a neighboring node.
		 */
		in_addr dest_addr;

		/**
		 * UDP address for where to send a packet to reach this node. In combination
		 * with the UDP port, this is the virtual link-layer equivalent of a mac
		 * address.
		 */
		in_addr udp_addr;

		/**
		 * UDP port for where to send a packet to reach this node.
		 */
		uint16_t udp_port;

		/**
		 * Interface where this neighbor can be reached, e.g. "if0". This should be
		 * the interface used when sending packets to this neighbor.
		 */
		std::string ifname;
	};

	/**
	 * If this node is using RIP, this is used to specify IP addresses of other
	 * routers that should receive RIP messages (ie. RIP requests, periodic
	 * updates, triggered updates).
	 */
	struct RIPNeighbor {
		/**
		 * Must be a neighbor IP address defined with a neighbor directive.
		 */
		in_addr dest;
	};

	/**
	 * Manually add a route to a node’s route table.
	 */
	struct StaticRoute {
		/**
		 * The address part of the prefix in the routing table, e.g. for a
		 * prefix 10.5.0.0/24, this will be 10.5.0.0
		 */
		in_addr network_addr;

		/**
		 * The prefix length of the prefix in the routing table, e.g. for a prefix
		 * 10.5.0.0/24, this will be 24
		 */
		int prefix_len;


		/**
		 * The address to route traffic matching the prefix to.
		 */
		in_addr next_hop;
	};

	class Config {
		public:
			Config(char *path_to_lnx_file);

			const RoutingMode &routing_mode() { return m_routing_mode; }
			const std::vector<Interface> &interfaces() { return m_interfaces; }
			const std::vector<Neighbor> &neighbors() { return m_neighbors; }
			const std::vector<RIPNeighbor> &rip_neighbors() { return m_rip_neighbors; }
			const std::vector<StaticRoute> &static_routes() { return m_static_routes; }

		private:
			RoutingMode m_routing_mode;
			void do_parse_error(std::string msg, int lineno);
			void parse_addr(char *ip_str, in_addr *addr, int lineno);
			std::vector<Interface> m_interfaces;
			std::vector<Neighbor> m_neighbors;
			std::vector<RIPNeighbor> m_rip_neighbors;
			std::vector<StaticRoute> m_static_routes;
	};
}

#define TOKEN_MAX_INTERFACE 5
#define TOKEN_MAX_NEIGHBOR 4
#define TOKEN_MAX_RIP_NEIGHBOR 1
#define TOKEN_MAX_ROUTE 3
#define TOKEN_MAX_NAME 16
#define LNX_IFNAME_MAX 64

lnx::Config::Config(char *path_to_lnx_file) {
	std::ifstream f(path_to_lnx_file);
	if (!f.is_open()) {
		std::perror("Failed to open file");
		std::exit(1);
	}

	std::string line;
	int tokens;
	int port;
	char ip_buf1[LINE_MAX];
	char ip_buf2[LINE_MAX];
	char first_token[TOKEN_MAX_NAME];
	char name[LNX_IFNAME_MAX];

	int lineno = 0;
	while (std::getline(f, line)) {
		lineno++;

		if (line[0] == '#') {
			continue;
		}

		if ((tokens = std::sscanf(line.c_str(), "%10s", first_token)) != 1) {
			continue;
		}

		if ((strncmp(first_token, "interface", TOKEN_MAX_NAME)) == 0) {
			Interface i;
			tokens = std::sscanf(line.c_str(), "interface %32s %32[^/]/%2d %32[^:]:%d",
				name, ip_buf1, &i.prefix_len, ip_buf2, &port);
			if (tokens != TOKEN_MAX_INTERFACE) {
				do_parse_error("Did not find enough tokens", lineno);
			}
			i.name = name;
			parse_addr(ip_buf1, &i.assigned_ip, lineno);
			// prefix_len assigned above
			parse_addr(ip_buf2, &i.udp_addr, lineno);
			i.udp_port = (uint16_t) port;
			m_interfaces.push_back(i);
		} else if ((strncmp(first_token, "neighbor", TOKEN_MAX_NAME)) == 0) {
			Neighbor n;
			tokens = sscanf(line.c_str(), "neighbor %32s at %32[^:]:%d via %32[^ #]",
				ip_buf1, ip_buf2, &port, name);
			if (tokens != TOKEN_MAX_NEIGHBOR) {
				do_parse_error("Did not find enough tokens", lineno);
			}
			parse_addr(ip_buf1, &n.dest_addr, lineno);
			parse_addr(ip_buf2, &n.udp_addr, lineno);
			n.udp_port = (uint16_t) port;
			n.ifname = name;
			m_neighbors.push_back(n);
		} else if ((strncmp(first_token, "routing", TOKEN_MAX_NAME) == 0)) {
			char *mode_str = ip_buf1; // Reuse this buffer
			tokens = sscanf(line.c_str(), "routing %32s", mode_str);
			if (tokens != 1) {
				do_parse_error("Did not find enough tokens", lineno);
			}
			if (strncmp(mode_str, "rip", TOKEN_MAX_NAME) == 0) {
				m_routing_mode = RoutingMode::RIP;
			} else if (strncmp(mode_str, "static", TOKEN_MAX_NAME) == 0) {
				m_routing_mode = RoutingMode::STATIC;
			} else {
				do_parse_error("Unrecognized routing mode", lineno);
			}
		} else if (strncmp(first_token, "rip", TOKEN_MAX_NAME) == 0) {
			RIPNeighbor r;
			tokens = sscanf(line.c_str(), "rip advertise-to %32s", ip_buf1);
			if (tokens != TOKEN_MAX_RIP_NEIGHBOR) {
				do_parse_error("Did not find enough tokens", lineno);
			}
			parse_addr(ip_buf1, &r.dest, lineno);
			m_rip_neighbors.push_back(r);
		} else if (strncmp(first_token, "route", TOKEN_MAX_NAME) == 0) {
			StaticRoute s;
			tokens = sscanf(line.c_str(), "route %32[^/]/%2d via %32s",
				ip_buf1, &s.prefix_len, ip_buf2);
			if (tokens != TOKEN_MAX_ROUTE) {
				do_parse_error("Did not find enough tokens", lineno);
			}
			parse_addr(ip_buf1, &s.network_addr, lineno);
			parse_addr(ip_buf2, &s.next_hop, lineno);
			m_static_routes.push_back(s);
		}

	}
}

void lnx::Config::do_parse_error(std::string msg, int lineno) {
	std::cerr << "Parse error, line " << lineno << ": " << msg << std::endl;
  std::exit(1);
}

void lnx::Config::parse_addr(char *ip_str, in_addr *addr, int lineno) {
	std::memset(addr, 0, sizeof(in_addr));
	if (inet_pton(AF_INET, ip_str, addr) < 0) {
		do_parse_error("Failed to parse IP address", lineno);
	}
}


#endif // __LNXCONFIG_H__
