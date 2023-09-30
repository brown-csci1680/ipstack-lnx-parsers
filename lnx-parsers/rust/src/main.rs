use std::{
    fs::File,
    io::{BufRead, BufReader},
    net::Ipv4Addr,
};

use ipnet::Ipv4Net;

enum RoutingType {
    None,
    Static,
    RIP,
}

#[derive(PartialEq, Debug)]
struct InterfaceConfig {
    pub name: String,
    // Ip address of the interface + prefix
    pub assigned_prefix: Ipv4Net,
    pub assigned_ip: Ipv4Addr,
    pub udp_addr: Ipv4Addr,
    pub udp_port: u16,
}

struct NeighborConfig {
    pub dest_addr: Ipv4Addr,
    pub udp_addr: Ipv4Addr,
    pub udp_port: u16,
    pub interface_name: String,
}

// route <prefix> (Ipv4net) via <addr> (Ipv4Addr)
pub type StaticRoute = (Ipv4Net, Ipv4Addr);
struct IPConfig {
    pub interfaces: Vec<InterfaceConfig>,
    pub neighbors: Vec<NeighborConfig>,

    pub originating_prefixes: Vec<Ipv4Net>, // Unused in F23, ignore

    pub routing_mode: RoutingType,

    // ROUTERS only so making an option
    pub rip_neighbors: Option<Vec<Ipv4Addr>>,

    pub static_routes: Vec<StaticRoute>, // prefix -> addr
}

impl IPConfig {
    pub fn new(filepath: String) -> Self {
        let open_result = File::open(filepath);
        let file = match open_result {
            Ok(file) => file,
            Err(e) => panic!("Problem opening the file: {:?}", e),
        };

        let reader = BufReader::new(file);
        let mut intfs: Vec<InterfaceConfig> = vec![];
        let mut neighbors: Vec<NeighborConfig> = vec![];
        let mut routing_mode = RoutingType::None;
        let mut originating_prefixes: Vec<Ipv4Net> = vec![];
        let mut rip_neighbors: Vec<Ipv4Addr> = vec![];
        let mut static_routes = vec![];

        for line_res in reader.lines().into_iter() {
            let line = match line_res {
                Ok(l) => l,
                Err(e) => panic!("Error reading line: {:?}", e),
            };

            let tokens = line.split(" ").collect::<Vec<&str>>();
            if (tokens[0] == "#") || (tokens.len() == 0) || tokens[0].starts_with("#") {
                continue;
            }

            match tokens[0] {
                "interface" => intfs.push(Self::parse_interface(line)),
                "neighbor" => neighbors.push(Self::parse_neighbor(line)),
                "routing" => routing_mode = Self::parse_routing(line),
                "route" => static_routes.push(Self::parse_route(line)),
                "rip" => {
                    let (net, addr) = Self::parse_rip(line, &intfs, &neighbors);
                    if net.is_some() {
                        originating_prefixes.push(net.unwrap());
                    } else if addr.is_some() {
                        rip_neighbors.push(addr.unwrap())
                    }
                }
                _ => (),
            };
        }

        let final_rip_n = if rip_neighbors.is_empty() {
            None
        } else {
            Some(rip_neighbors)
        };

        return IPConfig {
            interfaces: intfs,
            neighbors,
            originating_prefixes,
            routing_mode,
            rip_neighbors: final_rip_n,
            static_routes,
        };
    }

    pub fn parse_interface(line: String) -> InterfaceConfig {
        // format: interface <name> <prefix> <bindAddr>
        let err_str = "Interface formatted incorrectly: interface <name> <prefix> <bindAddr>";
        let tokens = line.split_ascii_whitespace().collect::<Vec<&str>>();
        if tokens.len() < 4 {
            panic!("{:?}", err_str)
        }

        // get name
        let name = String::from(tokens[1]);
        // IPv4Net represents an IP addr with a prefix
        let prefix: Ipv4Net = tokens[2].parse().unwrap();
        // parse out the Ipv4 addr and port for UDP socket
        let (udp_addr, udp_port) = Self::str_to_udp(String::from(tokens[3]));

        return InterfaceConfig {
            name,
            assigned_prefix: prefix,
            assigned_ip: prefix.addr(),
            udp_addr,
            udp_port,
        };
    }

    pub fn parse_neighbor(line: String) -> NeighborConfig {
        // format: neighbor <vip> at <bindAddr> via <interface name>
        let err_str =
            "Neighbor formatted incorrectly: neighbor <vip> at <bindAddr> via <interface name>";
        let tokens = line.split_ascii_whitespace().collect::<Vec<&str>>();
        if tokens.len() < 6 {
            panic!("{:?}", err_str)
        }

        let dest_addr: Ipv4Addr = tokens[1].parse().unwrap();
        let (udp_addr, udp_port) = Self::str_to_udp(String::from(tokens[3]));
        return NeighborConfig {
            dest_addr,
            udp_addr,
            udp_port,
            interface_name: String::from(tokens[5]),
        };
    }

    pub fn parse_routing(line: String) -> RoutingType {
        // format: routing <type>
        let tokens = line.split_ascii_whitespace().collect::<Vec<&str>>();
        if tokens.len() < 2 {
            panic!("Routing type formatted incorrectly: routing <type>")
        }

        match tokens[1] {
            "static" => RoutingType::Static,
            "rip" => RoutingType::RIP,
            "none" => RoutingType::None,
            _ => panic!("Invalid routing type!"),
        }
    }

    pub fn parse_route(line: String) -> StaticRoute {
        // format: route <prefix> via <addr>
        let err_str = "Route formatted incorrectly: route <prefix> via <addr>";
        let tokens = line.split_ascii_whitespace().collect::<Vec<&str>>();
        if tokens.len() < 4 {
            panic!("{:?}", err_str)
        }

        // IPv4Net represents an IP addr with a prefix
        let prefix: Ipv4Net = tokens[1].parse().unwrap();
        let addr: Ipv4Addr = tokens[3].parse().unwrap();
        return (prefix, addr);
    }

    fn str_to_udp(input: String) -> (Ipv4Addr, u16) {
        let tokens = input.split(":").collect::<Vec<&str>>();
        return (
            tokens[0].parse::<Ipv4Addr>().unwrap(),
            tokens[1].parse::<u16>().unwrap(),
        );
    }

    pub fn parse_rip(
        line: String,
        ifaces: &Vec<InterfaceConfig>,
        neighbors: &Vec<NeighborConfig>,
    ) -> (Option<Ipv4Net>, Option<Ipv4Addr>) {
        let tokens = line.split_ascii_whitespace().collect::<Vec<&str>>();
        if tokens.len() < 2 {
            panic!("RIP command formatted incorrectly: rip [cmd]")
        }

        let cmd = tokens[1];
        match cmd {
            "originate" => {
                if tokens.len() < 4 && tokens[3] != "prefix" {
                    panic!("RIP originate formatted incorrectly: rip originate prefix <prefix>")
                }

                // IPv4Net represents an IP addr with a prefix
                let prefix: Ipv4Net = tokens[3].parse().unwrap();
                // check that prefix is in config
                (
                    Some(IPConfig::check_originating_prefix(prefix, ifaces)),
                    None,
                )
            }
            "advertise-to" => {
                if tokens.len() < 3 {
                    panic!("RIP advertise-to formatted incorrectly: rip advertise-to <neighbor IP>")
                }

                // IPv4Net represents an IP addr with a prefix
                let addr: Ipv4Addr = tokens[2].parse().unwrap();
                // check that prefix is in config
                (None, Some(IPConfig::check_rip_neighbor(addr, neighbors)))
            }
            _ => panic!("Unrecognized RIP command: {:?}", cmd),
        }
    }

    fn check_originating_prefix(prefix: Ipv4Net, ifaces: &Vec<InterfaceConfig>) -> Ipv4Net {
        for iface in ifaces {
            if iface.assigned_prefix == prefix {
                return prefix;
            }
        }

        panic!("No matching prefix {:?} in config", prefix)
    }

    fn check_rip_neighbor(addr: Ipv4Addr, neighbors: &Vec<NeighborConfig>) -> Ipv4Addr {
        for n in neighbors {
            if n.dest_addr == addr {
                return addr;
            }
        }

        panic!("RIP neighbor {:?} is not a neighbor ip", addr)
    }
}
fn main() {
    println!("Hello, world!");
}

#[cfg(test)]
mod tests {
    use std::net::Ipv4Addr;

    use ipnet::Ipv4Net;

    use crate::{IPConfig, InterfaceConfig};

    #[test]
    fn test_intf() {
        let intf = String::from("interface if0 10.10.10.0/24 1.2.3.4:2004");
        let udp_addr = Ipv4Addr::new(1, 2, 3, 4);
        let assigned_prefix = "10.10.10.0/24".parse::<Ipv4Net>().unwrap();
        let actual = InterfaceConfig {
            name: String::from("if0"),
            assigned_prefix,
            assigned_ip: assigned_prefix.addr(),
            udp_addr,
            udp_port: 2004,
        };
        assert_eq!(actual, IPConfig::parse_interface(intf));
    }

    // static NEIGHBOR: String = String::from("neighbor 10.0.0.2 at 127.0.0.1:5001 via if1");
}
