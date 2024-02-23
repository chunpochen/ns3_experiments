#include "ns3/core-module.h" 
#include "ns3/network-module.h" 
#include "ns3/csma-module.h"
#include "ns3/internet-module.h" 
#include "ns3/point-to-point-module.h"  
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"

//
//          10.1.1.0                                    10.1.3.0 
// n0 ---------------------   n1   n2   n3   n4   ---------------------n5 
//       point-to-point       |    |    |    |       point-to-point 
//                           ================ 
//                             LAN 10.1.2.0
//

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("lab1-part2");

int main(int argc, char *argv[]) {

    LogComponentEnableAll(LOG_PREFIX_NODE); 

    bool verbose = true; 
    uint32_t nCsma = 2; 
    uint32_t nPackets = 1; 

    CommandLine cmd; 
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA node/devices", nCsma);
    cmd.AddValue("nPackets", "Number of packet", nPackets);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv); 

    if(verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); 
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); 
    }

    if(nPackets >= 20) 
        nPackets = 20; 

    if(nCsma == 0)
        nCsma = 3;  
    else
        nCsma = nCsma;

    NodeContainer p2pNodes1 ,p2pNodes2, csmaNodes; // 定義節點容器
    p2pNodes1.Create(2); 
    p2pNodes2.Create(2);  

    csmaNodes.Add(p2pNodes1.Get(1)); 
    csmaNodes.Create(nCsma);  ）
    csmaNodes.Add(p2pNodes2.Get(0));

    PointToPointHelper pointToPoint; 
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms")); 

    NetDeviceContainer p2pDevices1 = pointToPoint.Install(p2pNodes1); 
    NetDeviceContainer p2pDevices2 = pointToPoint.Install(p2pNodes2); 

    CsmaHelper csma; 
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps")); 
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560))); 

    NetDeviceContainer csmaDevices = csma.Install(csmaNodes); 

    InternetStackHelper stack; 
    stack.Install(p2pNodes1); 
    stack.Install(csmaNodes); 
    stack.Install(p2pNodes2); 

    Ipv4AddressHelper address; 
    address.SetBase("10.1.1.0", "255.255.255.0"); 
    Ipv4InterfaceContainer p2pInterfaces1 = address.Assign(p2pDevices1); 

    address.SetBase("10.1.2.0", "255.255.255.0"); 
    Ipv4InterfaceContainer csmaInterfaces = address.Assign(csmaDevices); 

    address.SetBase("10.1.3.0", "255.255.255.0"); 
    Ipv4InterfaceContainer p2pInterfaces2 = address.Assign(p2pDevices2); 

    UdpEchoServerHelper echoserver(9); 
  
    ApplicationContainer serverApps = echoserver.Install(p2pNodes2.Get(1)); 
    serverApps.Start(Seconds(1.0)); 
    serverApps.Stop(Seconds(50.0)); 

    UdpEchoClientHelper echoclient(p2pInterfaces2.GetAddress(1), 9);
    echoclient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoclient.SetAttribute("Interval", TimeValue(Seconds(1.0))); 
    echoclient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps1 = echoclient.Install(p2pNodes1.Get(0));
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(20.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    pointToPoint.EnablePcapAll("lab1-part2");
    csma.EnablePcap("lab1-part2", csmaDevices.Get(1), true);

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
