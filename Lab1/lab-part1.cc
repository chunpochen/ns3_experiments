#include "ns3/core-module.h"                   
#include "ns3/network-module.h"              
#include "ns3/internet-module.h"              
#include "ns3/point-to-point-module.h"      
#include "ns3/applications-module.h"            
#include "ns3/ipv4-global-routing-helper.h"    
#include "ns3/flow-monitor-helper.h"
#include "ns3/mobility-module.h"

// 
//       10.1.2.0           10.1.1.0
// n2 -------------- n0 ---------------n1 
//    point-to-point     point-to-point 
//                   | 
//                 p | 
//                 2 | 10.1.3.0 
//                 p | 
//                   | 
//                   n3 
//

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("Lab1-Part1");

int main(int argc, char *argv[]) {

    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); 
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); 

    uint32_t nClients = 1;  
    uint32_t nPackets = 1;  

    CommandLine cmd;    
    cmd.AddValue("nClients", "Number of client node", nClients);  
    cmd.AddValue("nPackets", "Number of packet", nPackets);     
    bool enableFlowMonitor = false;
    cmd.AddValue("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
    cmd.Parse(argc, argv);  

    if (nClients >= 5) { 
        nClients = 5;
    }

    if (nPackets >= 100) {
        nPackets = 100;
    }

    NodeContainer serverNode;   
    serverNode.Create(1);   
    NodeContainer clientNodes; 
    clientNodes.Create(nClients);   

    PointToPointHelper p2p; 
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));  
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));   
    
    // MobilityHelper mobility;
    // mobility.SetPositionAllocator("ns3::GridPositionAllocator",
    //                                 "MinX", DoubleValue(0.0),
    //                                 "MinY", DoubleValue(0.0),
    //                                 "DeltaX", DoubleValue(5.0),
    //                                 "DeltaY", DoubleValue(10.0),
    //                                 "GridWidth", UintegerValue(3),
    //                                 "LayoutType", StringValue("RowFirst"));
    // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    // for(uint32_t i = 0; i < nClients; i++) {
    //     mobility.Install(clientNodes);
    // }

  
    InternetStackHelper stack;  
    stack.Install(serverNode); 
    stack.Install(clientNodes); 
    
    Ipv4AddressHelper address;  
    std::vector<Ipv4InterfaceContainer> clientInterfaces(nClients);
    NetDeviceContainer serverDevices;
    NetDeviceContainer clientDevices;

    for (uint32_t i = 0; i < nClients; ++i) {

        std::ostringstream subnet;
        subnet << "10.1." << i+1 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");

        NetDeviceContainer p2pDevices = p2p.Install(serverNode.Get(0), clientNodes.Get(i));

        Ipv4InterfaceContainer interfaces = address.Assign(p2pDevices);
        clientInterfaces[i] = interfaces;
    }

    UdpEchoServerHelper echoServer(9); 
    echoServer.SetAttribute("Port", UintegerValue(15));

    ApplicationContainer serverApps = echoServer.Install(serverNode.Get(0));    
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(100.0));

    ApplicationContainer clientApps;

    UdpEchoClientHelper echoClient(Ipv4Address("10.1.1.1"), 15);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    for (uint32_t i = 0; i < nClients; i++) {
        clientApps.Add(echoClient.Install(clientNodes.Get(i)));
    }
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(100.0));

    // // 印出伺服器節點的所有IP位址
    // // 取得第一個節點視為伺服器
    // Ptr<Node> server = serverNode.Get(0);
    // // 從伺服器節點中獲得IPv4協定的實例
    // Ptr<Ipv4> ipv4 = server->GetObject<Ipv4>();
    // // 遍歷伺服器上的所有網路介面
    // for (uint32_t i = 0; i < ipv4->GetNInterfaces(); i++) {
    //     // 遍歷每個介面上的所有IP地址
    //     for (uint32_t j = 0; j < ipv4->GetNAddresses(i); j++) {
    //         // 取得特定介面上的特定位址
    //         Ipv4InterfaceAddress addr = ipv4->GetAddress(i, j);
    //         std::cout << "Interface " << i << " Address " << j << ": " << addr.GetLocal() << std::endl;
    //     }
    // }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    rand->SetAttribute("Min", DoubleValue(2.0));
    rand->SetAttribute("Max", DoubleValue(7.0));

    for (uint32_t i = 0; i < clientApps.GetN(); ++i) {
        clientApps.Get(i)->SetStartTime(Seconds(rand->GetValue()));
    }

    // mobility.Install(serverNode);
    // mobility.Install(clientNodes);

    // Ptr<MobilityModel> mobilityModelA = serverNode.Get(0)->GetObject<MobilityModel>();
    // Ptr<MobilityModel> mobilityModelB = clientNodes.Get(0)->GetObject<MobilityModel>();
    // Vector posA = mobilityModelA->GetPosition();  
    // Vector posB = mobilityModelB->GetPosition();  

    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("lab-part1.tr"));
    p2p.EnablePcapAll("lab-part1");

    // Flow Monitor
    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor;

    if (enableFlowMonitor)
    {
        monitor = flowmonHelper.InstallAll();
    }

    Simulator::Stop(Seconds(100.0));
    Simulator::Run();

    if (enableFlowMonitor)
    {
        flowmonHelper.SerializeToXmlFile("lab-part1.flowmon", false, false);
    }

    Simulator::Destroy();

    // double distance = CalculateDistance(posA, posB);
    // std::cout << "Distance between the nodes: " << distance << " meters" << std::endl;

    return 0;
}
