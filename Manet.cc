//Comparing MANET Protocols

//Including necessary libraries
 #include <fstream>
 #include <iostream>
 #include<string>
 #include<string.h>
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/aodv-module.h"
 #include "ns3/dsdv-module.h"
 #include "ns3/dsr-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/yans-wifi-helper.h"
 #include "ns3/flow-monitor-helper.h"
 #include "ns3/animation-interface.h"
   
 using namespace ns3;
 using namespace dsr;
 using namespace std; 
 
 NS_LOG_COMPONENT_DEFINE ("MANET");
 
 AnimationInterface *anim=0;
 
 class RoutingExperiment
 {
 public:
   int i,th;
   RoutingExperiment ();
   void Run (int nSinks, double txp, std::string CSVfileName);
   std::string CommandSetup (int argc, char **argv);
  
 private:
   Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
   void ReceivePacket (Ptr<Socket> socket);
   void CheckThroughput ();
  
   uint32_t port;
   uint32_t bytesTotal;
   uint32_t packetsReceived;
  
   std::string m_CSVfileName;
   int m_nSinks;
   std::string m_protocolName;
   double m_txp;
   bool m_traceMobility;
   uint32_t m_protocol;
 };
  
 RoutingExperiment::RoutingExperiment ()
   : i(0),
     th(0),
     port (9),
     bytesTotal (0),
     packetsReceived (0),
     m_CSVfileName ("DSR.csv"),//File name
     m_traceMobility (true),
     m_protocol (3) // 1.AODV 2.DSDV 3.DSR
 {
 }
  
 static inline std::string  PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
 {
   std::ostringstream oss;
  
   oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();
  
   if (InetSocketAddress::IsMatchingType (senderAddress))
     {
       InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
       oss << " received one packet from " << addr.GetIpv4 ();
     }
   else
     {
       oss << " received one packet!";
     }
   return oss.str ();
 }
  
 void RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
 {
   Ptr<Packet> packet;
   Address senderAddress;
   while ((packet = socket->RecvFrom (senderAddress)))
     {
       bytesTotal += packet->GetSize ();
       packetsReceived += 1;
//        NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
     }
 }
  
 void RoutingExperiment::CheckThroughput ()
 {
   double kbs = (bytesTotal * 8.0) / 1000;
   bytesTotal = 0;
   th+=kbs;
   std::ofstream out (m_CSVfileName.c_str (), std::ios::app);
  
   out << (Simulator::Now ()).GetSeconds () << ","
       << kbs << ","
       << packetsReceived << ","
       << m_nSinks << ","
       << m_protocolName << ","
       << m_txp << ""
       << std::endl;
   i++;    
   out.close ();
   packetsReceived = 0;
   Simulator::Schedule (Seconds (4.0), &RoutingExperiment::CheckThroughput, this);
 }
  
 Ptr<Socket> RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
 {
   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
   Ptr<Socket> sink = Socket::CreateSocket (node, tid);
   InetSocketAddress local = InetSocketAddress (addr, port);
   sink->Bind (local);
   sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));
  
   return sink;
 }
  
 std::string RoutingExperiment::CommandSetup (int argc, char **argv)
 {
   CommandLine cmd (__FILE__);
   cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
   cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
   cmd.AddValue ("protocol", "1=AODV;2=DSDV;3=DSR", m_protocol);
   cmd.Parse (argc, argv);
   return m_CSVfileName;
 }
 
 //Main Function
 int main (int argc, char *argv[])
 {
  bool verbose=true;
     CommandLine cmd (__FILE__);
   cmd.AddValue ("verbose", "turn on log components", verbose);  
   
   if (verbose)
  {
      LogComponentEnable ("MANET", LOG_LEVEL_INFO);
  }
   RoutingExperiment experiment;
   std::string CSVfileName = experiment.CommandSetup (argc,argv);
  
   //blank out the last output file and write the column headers
   std::ofstream out (CSVfileName.c_str ());
   out << "SimulationSecond," <<
   "Throughput," <<
   "PacketsReceived," <<
   "NumberOfSinks," <<
   "RoutingProtocol," <<
   "TransmissionPower" <<
   std::endl;
   out.close ();
  
   int nSinks = 5;//No. of Source/Destination
   double txp = 7.5;
  
   experiment.Run (nSinks, txp, CSVfileName);//Function Call for running 
 }
  
 void RoutingExperiment::Run (int nSinks, double txp, std::string CSVfileName)
 {
   Packet::EnablePrinting ();
   m_nSinks = nSinks;
   m_txp = txp;
   m_CSVfileName = CSVfileName;
  
   int nWifis = 20;
  
   double TotalTime = 200.0;//Simulation Time
   std::string rate ("2048bps");
   std::string phyMode ("DsssRate11Mbps");
   std::string tr_name ("DSR");//Protocol 1.AODV 2.DSDV 3.DSR
   int nodeSpeed = 60; //in m/s
   int nodePause = 0; //in s
   m_protocolName = "protocol";
  
   Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("512"));
   Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));
  
   //Set Non-unicastMode rate to unicast mode
   Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));
  
   NodeContainer adhocNodes;
   adhocNodes.Create (nWifis);
  
   // setting up wifi phy and channel using helpers
   WifiHelper wifi;
   wifi.SetStandard (WIFI_STANDARD_80211b);
  
   YansWifiPhyHelper wifiPhy;
   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
   wifiPhy.SetChannel (wifiChannel.Create ());
  
   // Add a mac and disable rate control
   WifiMacHelper wifiMac;
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                 "DataMode",StringValue (phyMode),
                                 "ControlMode",StringValue (phyMode));
  
   wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
   wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));
  
   wifiMac.SetType ("ns3::AdhocWifiMac");
   NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);
  
   MobilityHelper mobilityAdhoc;
   int64_t streamIndex = 0; // used to get consistent mobility across scenarios
  
   ObjectFactory pos;
   pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
   pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
   pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  
   Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
   streamIndex += taPositionAlloc->AssignStreams (streamIndex);
  
   std::stringstream ssSpeed;
   ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
   std::stringstream ssPause;
   ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
   mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                   "Speed", StringValue (ssSpeed.str ()),
                                   "Pause", StringValue (ssPause.str ()),
                                   "PositionAllocator", PointerValue (taPositionAlloc));
   mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
   mobilityAdhoc.Install (adhocNodes);
   streamIndex += mobilityAdhoc.AssignStreams (adhocNodes, streamIndex);
   NS_UNUSED (streamIndex); // From this point, streamIndex is unused
  
   AodvHelper aodv;
   DsdvHelper dsdv;
   DsrHelper dsr;
   DsrMainHelper dsrMain;
   Ipv4ListRoutingHelper list;
   InternetStackHelper internet;
  
   switch (m_protocol)
     {
     case 1:
       list.Add (aodv, 100);
       m_protocolName = "AODV";
       break;
     case 2:
       list.Add (dsdv, 100);
       m_protocolName = "DSDV";
       break;
     case 3:
       m_protocolName = "DSR";
       break;
     default:
       NS_FATAL_ERROR ("No such protocol:" << m_protocol);
     }
  
   if (m_protocol < 3)
     {
       internet.SetRoutingHelper (list);
       internet.Install (adhocNodes);
     }
   else if (m_protocol == 3)
     {
       internet.Install (adhocNodes);
       dsrMain.Install (dsr, adhocNodes);
     }
  
   NS_LOG_INFO ("assigning ip address");
  
   Ipv4AddressHelper addressAdhoc;
   addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
   Ipv4InterfaceContainer adhocInterfaces;
   adhocInterfaces = addressAdhoc.Assign (adhocDevices);
  
   OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
   onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
   onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
  anim=new AnimationInterface((tr_name+".xml"));
   for (int i = 0; i < nSinks; i++)
     {
       Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), adhocNodes.Get (i));
  
       AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
       onoff1.SetAttribute ("Remote", remoteAddress);
  
       Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
       ApplicationContainer temp = onoff1.Install (adhocNodes.Get (i + nSinks));
       anim->UpdateNodeColor(adhocNodes.Get (i + nSinks),(((i+1)*40)%255),(((i+2)*80)%255),(((i+3)*120)%255));
       anim->UpdateNodeColor(adhocNodes.Get (i),(((i+1)*40)%255),(((i+2)*80)%255),(((i+3)*120)%255));
       anim->UpdateNodeDescription(adhocNodes.Get(i),("Source "+to_string((i+1))));
       anim->UpdateNodeDescription(adhocNodes.Get(i+nSinks),("Destination "+to_string((i+1))));
       temp.Start (Seconds (var->GetValue (50.0,51.0)));
       temp.Stop (Seconds (TotalTime));
     }
  
   std::stringstream ss;
   ss << nWifis;
   std::string nodes = ss.str ();
  
   std::stringstream ss2;
   ss2 << nodeSpeed;
   std::string sNodeSpeed = ss2.str ();
  
   std::stringstream ss3;
   ss3 << nodePause;
   std::string sNodePause = ss3.str ();
  
   std::stringstream ss4;
   ss4 << rate;
   std::string sRate = ss4.str ();

   AsciiTraceHelper ascii;
   Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (tr_name + ".tr").c_str());
   wifiPhy.EnableAsciiAll (osw);
   MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));
  
   Ptr<FlowMonitor> flowmon;
   FlowMonitorHelper flowmonHelper;
   flowmon = flowmonHelper.InstallAll ();
    
   anim->EnablePacketMetadata ();
   
   NS_LOG_INFO ("Run Simulation.");
  
   CheckThroughput ();
  
   Simulator::Stop (Seconds (TotalTime));
   Simulator::Run ();
  
   flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), false, false);
   NS_LOG_INFO ("Throughput: "+to_string((th/i))); 
   Simulator::Destroy ();
 }
  
