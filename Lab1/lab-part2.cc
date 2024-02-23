#include "ns3/core-module.h" // NS-3的核心模組
#include "ns3/network-module.h"  // 網路模組，用於建立和管理網路節點
#include "ns3/csma-module.h" // CSMA模組，模擬區域網路
#include "ns3/internet-module.h" // Internet模組，提供IP、routing等功能
#include "ns3/point-to-point-module.h"  // 端對端模組，用於建立端對端連接
#include "ns3/applications-module.h" // 應用程式模組，用於建立網路應用
#include "ns3/ipv4-global-routing-helper.h" // IPv4全域routing助手
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
    // LogComponentEnableAll(LOG_PREFIX_TIME); // 啟用時間前綴的所有日誌
    LogComponentEnableAll(LOG_PREFIX_NODE); // 啟用節點前綴的所有日誌

    bool verbose = true; // 開啟詳細日誌
    uint32_t nCsma = 2; // CSMA網路中的節點數量（除了端對端連接的節點）
    uint32_t nPackets = 1; // 發送的封包數量

    CommandLine cmd; // 命令行解析器
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA node/devices", nCsma);
    cmd.AddValue("nPackets", "Number of packet", nPackets);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv); // 解析命令行參數

    if(verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); // 啟用UDP echo客戶端應用的詳細日誌
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); // 啟用UDP echo服務器應用的詳細日誌
    }

    if(nPackets >= 20) 
        nPackets = 20; // 限制最大封包數量為20

    if(nCsma == 0)
        nCsma = 3;  // 確保至少有一個CSMA節點
    else
        nCsma = nCsma;

    NodeContainer p2pNodes1 ,p2pNodes2, csmaNodes; // 定義節點容器
    p2pNodes1.Create(2);  // 創建第一組端對端節點（n0和n1）
    p2pNodes2.Create(2);  // 創建第二組端對端節點（n4和n5）

    csmaNodes.Add(p2pNodes1.Get(1));  // 將n1添加到CSMA網路
    csmaNodes.Create(nCsma);  // 創建CSMA節點（n2和n3）
    csmaNodes.Add(p2pNodes2.Get(0));  // 將n4添加到CSMA網路

    PointToPointHelper pointToPoint; // 創建端對端助手
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps")); // 設定資料速率
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms")); // 設定通道延遲

    NetDeviceContainer p2pDevices1 = pointToPoint.Install(p2pNodes1); // 安裝第一組端對端設備
    NetDeviceContainer p2pDevices2 = pointToPoint.Install(p2pNodes2); // 安裝第一組端對端設備

    CsmaHelper csma; // 創建CSMA助手
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps")); // 設定CSMA資料速率
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560))); // 設定CSMA延遲

    NetDeviceContainer csmaDevices = csma.Install(csmaNodes); // 安裝CSMA設備

    InternetStackHelper stack; // 創建網路堆疊助手
    stack.Install(p2pNodes1); // 安裝到第一組端對端節點
    stack.Install(csmaNodes); // 安裝到CSMA節點
    stack.Install(p2pNodes2); // 安裝到第二組端對端節點

    Ipv4AddressHelper address; // 創建IPv4地址助手
    address.SetBase("10.1.1.0", "255.255.255.0"); // 設置第一組端對端網路的IP地址範圍
    Ipv4InterfaceContainer p2pInterfaces1 = address.Assign(p2pDevices1); // 分配IP地址給第一組端對端節點

    address.SetBase("10.1.2.0", "255.255.255.0"); // 設置CSMA網路的IP地址範圍
    Ipv4InterfaceContainer csmaInterfaces = address.Assign(csmaDevices); // 分配IP地址給CSMA節點

    address.SetBase("10.1.3.0", "255.255.255.0"); // 設置第二組端對端網路的IP地址範圍
    Ipv4InterfaceContainer p2pInterfaces2 = address.Assign(p2pDevices2); // 分配IP地址給第二組端對端節點

    UdpEchoServerHelper echoserver(9); // 創建UDP echo伺服器助手，埠號為9
    // 在CSMA網路的最後一個節點上安裝echo伺服器應用
    ApplicationContainer serverApps = echoserver.Install(p2pNodes2.Get(1)); 
    serverApps.Start(Seconds(1.0)); // 設定伺服器應用啟動時間
    serverApps.Stop(Seconds(50.0)); // 設定伺服器應用結束時間

    // 建立UDP echo客戶端助手，目標地址為第二個端對端裝置右邊的IP地址
    UdpEchoClientHelper echoclient(p2pInterfaces2.GetAddress(1), 9);
    echoclient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoclient.SetAttribute("Interval", TimeValue(Seconds(1.0))); 
    echoclient.SetAttribute("PacketSize", UintegerValue(1024));

    // 建立UDP echo客戶端應用並安裝第一個端對端裝置的最左邊的節點
    ApplicationContainer clientApps1 = echoclient.Install(p2pNodes1.Get(0));
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(20.0));

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    pointToPoint.EnablePcapAll("lab1-part2");
    csma.EnablePcap("lab1-part2", csmaDevices.Get(1), true);

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}