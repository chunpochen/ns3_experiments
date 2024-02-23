#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h" 
#include "ns3/netanim-module.h"

//
// 
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n7   n6   n5   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   *
//                                   AP
//                                     Wifi 10.1.2.0
//

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("lab1-part3");

void CourseChange(std::string context, Ptr<const MobilityModel> model) {
    Vector position = model->GetPosition(); 

    NS_LOG_UNCOND(context <<
                " x = " << position.x << ", y = " << position.y);
}

int main(int argc, char *argv[]) {

    LogComponentEnableAll(LOG_PREFIX_NODE); 

    bool verbose = true; 
    // uint32_t nWifi1 = 3; // First WiFi Network Nodes
    uint32_t nWifi2 = 3;    // Second WiFi Network Nodes
    uint32_t nPackets = 1;  
    bool tracing = false;  

    CommandLine cmd;  
    // cmd.AddValue("nWifi1", "Number of wifi STA devices1 in each network", nWifi1); 
    cmd.AddValue("nWifi2", "Number of wifi STA devices2 in each network", nWifi2); 
    cmd.AddValue("nPackets", "Number of packets to send", nPackets);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);
    cmd.Parse(argc, argv); 

    if (nWifi2 >= 9) { 
        nWifi2 = 9;
    }

    if(nPackets >= 20) {
        nPackets = 20; 
    }

    if(verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); 
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); 
    }

    NodeContainer p2pNodes; 
    p2pNodes.Create(2);    
    PointToPointHelper pointToPoint; 
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer p2pDevices;               
    p2pDevices = pointToPoint.Install(p2pNodes); 
    NodeContainer wifiStaNodes1, wifiStaNodes2;
    wifiStaNodes1.Create(3);      
    wifiStaNodes2.Create(nWifi2); 
    NodeContainer wifiApNode1, wifiApNode2;   
    wifiApNode1 = p2pNodes.Get(0);  
    wifiApNode2 = p2pNodes.Get(1); 
    
    // setup first WiFi network 
    YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default(); 
    YansWifiPhyHelper phy1;            
    phy1.SetChannel(channel1.Create()); 
    WifiHelper wifi1;
    WifiMacHelper mac1; 
    Ssid ssid1 = Ssid("ns-3-ssid1");  
    // setup first MAC
    mac1.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid1), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices1; 
    staDevices1 = wifi1.Install(phy1, mac1, wifiStaNodes1);   
    mac1.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid1)); 
    NetDeviceContainer apDevices1;  
    apDevices1 = wifi1.Install(phy1, mac1, wifiApNode1); 


    // setting second WiFi network 
    YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default(); 
    YansWifiPhyHelper phy2;             
    phy2.SetChannel(channel2.Create());
    WifiHelper wifi2;   
    WifiMacHelper mac2; 
    Ssid ssid2 = Ssid("ns-3-ssid2"); 
    // setup second MAC
    mac2.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid2), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices2;  
    staDevices2 = wifi2.Install(phy2, mac2, wifiStaNodes2);   
    mac2.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid2)); 
    NetDeviceContainer apDevices2;   
    apDevices2 = wifi2.Install(phy2, mac2, wifiApNode2); 

    // Mobility configuration for both WiFi networks
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodes1); 
    mobility.Install(wifiStaNodes2); 
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");   
    mobility.Install(wifiApNode1); 
    mobility.Install(wifiApNode2); 

     
    InternetStackHelper stack;
    stack.Install(p2pNodes);     
    stack.Install(wifiApNode1);   
    stack.Install(wifiStaNodes1); 
    stack.Install(wifiApNode2);   
    stack.Install(wifiStaNodes2); 

    // IP Addressing
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);

    
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface1 = address.Assign(apDevices1); // AP n0
    Ipv4InterfaceContainer staInterface1 = address.Assign(staDevices1); // STA n2, n3, n4

    
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface2 = address.Assign(apDevices2); // AP n1
    Ipv4InterfaceContainer staInterface2 = address.Assign(staDevices2); // STA n5, n6, n7
    
 
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(wifiStaNodes1.Get(2)); 
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));

    UdpEchoClientHelper echoClient(staInterface1.GetAddress(2), 9); // n4 IP 10.1.2.4
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(wifiStaNodes2.Get(nWifi2 - 1)); // 
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(20.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(20.0));

    if (tracing)
    {
        phy1.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy2.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        pointToPoint.EnablePcapAll("lab1-part3");
        phy1.EnablePcap("lab1-part3_1", apDevices1.Get(0));
        phy2.EnablePcap("lab1-part3_2", apDevices2.Get(0));
    }

    std::ostringstream oss;
    oss << "/NodeList/" << wifiStaNodes1.Get(nWifi2 - 1)->GetId()
        << "/$ns3::MobilityModel/CourseChange";
    
    // std::ostringstream oss2;
    // oss2 << "/NodeList/" << wifiStaNodes2.Get(2)->GetId()
    //     << "/$ns3::MobilityModel/CourseChange";

    Config::Connect(oss.str(), MakeCallback(&CourseChange));
    // Config::Connect(oss2.str(), MakeCallback(&CourseChange));
    
    AnimationInterface anim ("animation.xml");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
