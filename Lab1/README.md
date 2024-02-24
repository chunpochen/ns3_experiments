# Lab1
## lab1-part1

### Objective
The primary aim of this experiment is to demonstrate how to set up a basic network topology in NS-3 and how to configure and run a UDP echo application for simple data transmission.

### Network Design
- **Topology Structure:** The topology consists of one server node (n0) and multiple client nodes (n1-n5), connected through point-to-point links.
- **Server Node:** The server node is equipped with a UDP echo server application.
- **Client Nodes:** The client nodes run a UDP echo client application, sending data packets to the server and receiving echo responses.

## lab1-part2

### Objective
Demonstrate how to establish a mixed network topology in NS-3, incorporating both point-to-point and CSMA connections.
Show how to configure and run a UDP Echo application within this topology for simple data transmission.

### Network Design
- **Topology Structure:** The network consists of point-to-point connections at both ends and a CSMA network in the middle.
- **Point-to-Point Connections:** Two sets of point-to-point connections are located at the ends of the network, serving to link the CSMA network with external nodes.
- **CSMA Network:** The central CSMA network simulates a typical wired Local Area Network (LAN), connecting multiple nodes.
- **UDPEchoApplication:** A UDP Echo server application runs on a node at one end of the network, while a UDP Echo client application runs on a node at the other end. The client sends data packets to the server and receives responses.

## lab1-part3

#### Objective
Demonstrate how to set up a hybrid network topology in NS-3 that includes both Wi-Fi and point-to-point connections.
Show how to configure and run a UDP Echo application within this hybrid topology for basic data transmission.
Analyze node mobility through network animations.

### Network Design
- **Hybrid Topology Structure:** Consists of two Wi-Fi networks, each with an Access Point (AP) and several Station (STA) nodes. These two Wi-Fi networks are interconnected by a point-to-point connection.
- **Point-to-Point Connection:** Links the access points of the two Wi-Fi networks, facilitating data transfer between the two networks.
- **UDPEchoApplication:** Runs a UDP Echo server application on a workstation node in one Wi-Fi network and a UDP Echo client application on a workstation node in the other Wi-Fi network.
