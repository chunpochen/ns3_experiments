#include "ns3/core-module.h"                    // 匯入NS-3的核心模組
#include "ns3/network-module.h"                 // 匯入NS-3的網路模組，涉及基本網路功能
#include "ns3/internet-module.h"                // 匯入NS-3的Internet模組，提供IP、UDP、TCP等功能
#include "ns3/point-to-point-module.h"          // 匯入NS-3的點對點模組
#include "ns3/applications-module.h"            // 匯入NS-3的應用程式模組，例如UDP用戶端/伺服器
#include "ns3/ipv4-global-routing-helper.h"     // 匯入IPv4全局路由助手，幫助建立和處理IPv4路由
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
    // LogComponentEnableAll(LOG_PREFIX_TIME);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); // 啟用UDP echo客戶端應用的日誌級別為INFO
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); // 啟用UDP echo服務器應用的日誌級別為INFO

    uint32_t nClients = 1;  // 客戶端數量默認為1
    uint32_t nPackets = 1;  // 封包的數量默認為1

    CommandLine cmd;    // 命令行解析
    cmd.AddValue("nClients", "Number of client node", nClients);    // 設定客戶端數量
    cmd.AddValue("nPackets", "Number of packet", nPackets);         // 設定封包數量
    bool enableFlowMonitor = false;
    cmd.AddValue("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
    cmd.Parse(argc, argv);  // 解析命令行參數

    if (nClients >= 5) { // 不超過五個client
        nClients = 5;
    }

    if (nPackets >= 100) { // 不超過5個packet
        nPackets = 100;
    }

    NodeContainer serverNode;   // 創建服務器節點容器
    serverNode.Create(1);   // 創建一個服務器節點
    NodeContainer clientNodes;  // 創建用戶端節點容器
    clientNodes.Create(nClients);   // 根據nClients的數量創建用戶端節點

    PointToPointHelper p2p; // 創建端對端助手
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));   // 設定端對端鏈路的資料速率
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));   // 設定點端對端路的延遲
    
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

    // 安裝網路堆疊
    InternetStackHelper stack;  // 創建互聯網堆疊
    stack.Install(serverNode);  // 對服務器節點安裝網路堆疊
    stack.Install(clientNodes); // 對用戶端節點安裝網路堆疊
    
    Ipv4AddressHelper address;  // 創建IPv4地址助手
    std::vector<Ipv4InterfaceContainer> clientInterfaces(nClients);
    NetDeviceContainer serverDevices;
    NetDeviceContainer clientDevices;

    // 伺服器節點（n0）與每個用戶端節點（n1-n5）的端對端連接
    for (uint32_t i = 0; i < nClients; ++i) {
        // 建立並設定每個子網路的位址
        std::ostringstream subnet;
        subnet << "10.1." << i+1 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");

        // 安裝點對點設備到節點
        NetDeviceContainer p2pDevices = p2p.Install(serverNode.Get(0), clientNodes.Get(i));

        // 分配IP地址
        Ipv4InterfaceContainer interfaces = address.Assign(p2pDevices);
        clientInterfaces[i] = interfaces;
    }
    // 設定和安裝UDP echo伺服器
    // 創建UDP echo伺服器，服務端口為15
    UdpEchoServerHelper echoServer(9); 
    echoServer.SetAttribute("Port", UintegerValue(15));
    // 在服務器節點上安裝echo服務器
    ApplicationContainer serverApps = echoServer.Install(serverNode.Get(0));    
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(100.0));

    ApplicationContainer clientApps;

    // 創建UDP echo用戶端，指定服務器地址和端口
    UdpEchoClientHelper echoClient(Ipv4Address("10.1.1.1"), 15);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // 建立UDP echo客戶端應用並安裝到每個客戶端節點（n1-n5）
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

    // 創建一個均勻分佈隨機變量用於產生隨機的啟動時間
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    rand->SetAttribute("Min", DoubleValue(2.0));
    rand->SetAttribute("Max", DoubleValue(7.0));

    // 為每個客戶端應用程序設定隨機開始時間
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