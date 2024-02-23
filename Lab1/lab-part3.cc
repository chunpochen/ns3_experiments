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
    
    // 第一個WiFi網路設定    
    YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default(); // 創建預設的WiFi通道幫手
    YansWifiPhyHelper phy1;             // 創建實體層幫手
    phy1.SetChannel(channel1.Create()); // 設置實體層使用的通道
    WifiHelper wifi1;   // 創建WiFi幫手
    WifiMacHelper mac1; // 創建MAC層幫手
    Ssid ssid1 = Ssid("ns-3-ssid1");  // 設置第一個WiFi網絡的SSID
    // 設置站點的MAC層屬性
    mac1.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid1), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices1;  // 創建站點設備容器
    staDevices1 = wifi1.Install(phy1, mac1, wifiStaNodes1);   // 安裝WiFi設備到站點節點
    mac1.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid1)); // 設定接入點的MAC層屬性
    NetDeviceContainer apDevices1;   // 創建接入點設備容器
    apDevices1 = wifi1.Install(phy1, mac1, wifiApNode1); // 安裝WiFi設備到接入點節點


    // 第二個WiFi網路設定
    YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default(); // 創建預設的WiFi通道幫手
    YansWifiPhyHelper phy2;             // 創建實體層幫手
    phy2.SetChannel(channel2.Create()); // 設置實體層使用的通道
    WifiHelper wifi2;   // 創建WiFi幫手
    WifiMacHelper mac2; // 創建MAC層幫手
    Ssid ssid2 = Ssid("ns-3-ssid2");  // 設置第二個WiFi網絡的SSID
    // 設置站點的MAC層屬性
    mac2.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid2), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices2;  // 創建站點設備容器
    staDevices2 = wifi2.Install(phy2, mac2, wifiStaNodes2);   // 安裝WiFi設備到站點節點
    mac2.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid2)); // 設定接入點的MAC層屬性
    NetDeviceContainer apDevices2;   // 創建接入點設備容器
    apDevices2 = wifi2.Install(phy2, mac2, wifiApNode2); // 安裝WiFi設備到接入點節點


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
    mobility.Install(wifiStaNodes1); // 安裝移動性到第一個WiFi網路的站點
    mobility.Install(wifiStaNodes2); // 安裝移動性到第二個WiFi網路的站點
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");   // 設置固定位置移動模型
    mobility.Install(wifiApNode1); // 安裝移動性到第一個WiFi網路的接入點
    mobility.Install(wifiApNode2); // 安裝移動性到第二個WiFi網路的接入點

    // 安裝互聯網協議堆疊   
    InternetStackHelper stack;
    stack.Install(p2pNodes);      // 安裝端對端節點
    stack.Install(wifiApNode1);   // 安裝到第一個WiFi接入點節點
    stack.Install(wifiStaNodes1); // 安裝到第一個WiFi站點節點
    stack.Install(wifiApNode2);   // 安裝到第二個WiFi接入點節點
    stack.Install(wifiStaNodes2); // 安裝到第二個WiFi站點節點 

    // IP Addressing
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);

    // 配置第一個Wi-Fi的IP地址
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface1 = address.Assign(apDevices1); // AP n0
    Ipv4InterfaceContainer staInterface1 = address.Assign(staDevices1); // STA n2, n3, n4

    // 配置第二個Wi-Fi的IP地址
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface2 = address.Assign(apDevices2); // AP n1
    Ipv4InterfaceContainer staInterface2 = address.Assign(staDevices2); // STA n5, n6, n7
    
    // 在 n4 上安裝UDP Echo服務器應用程序
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(wifiStaNodes1.Get(2)); 
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));

    // 在 n7 上安裝UDP Echo客戶端應用程序，並指向 n4 的IP
    UdpEchoClientHelper echoClient(staInterface1.GetAddress(2), 9); // n4的IP地址10.1.2.4
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(wifiStaNodes2.Get(nWifi2 - 1)); // n7是第二個Wi-Fi網路中的第三个節點
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
