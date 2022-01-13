//Open Shortest Path First Protocol

//Including Libraries
#include <fstream>
#include<iostream>
#include<stdlib.h>
#include<map>
#include<stdio.h>
#include "bits/stdc++.h"
#include "cstdlib"
#include<string>
#include<string.h>
//NS3 Libraries
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"
#include "ns3/mobility-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/netanim-module.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OSPF-Ipv4");

AnimationInterface *anim=0; //NetAnim Object

map<int,string> m; //Map for Node connections
map<char,string> m1; //Map for Hosts connections
map<int,string> host; //Map for Links

//Function to display the node connections
void disp(int n)
{
    NS_LOG_INFO("#############################");
      NS_LOG_INFO("\n\n#############################");
    for(int i=0;i<n;i++)
    {
      string s=m[i];
      int n1=s.size();
      NS_LOG_INFO("***********");
      NS_LOG_INFO("Node "+to_string(i)+" is connected with "+to_string(n1)+" nodes");
      NS_LOG_INFO(s); 
    }
    NS_LOG_INFO("***********");
    NS_LOG_INFO("#############################");
}

//Function to store the position of the nth node connection 
string pos(int j)
{
    int i=0;
    string s=m[j];
    int r;
    int n=s.size();
    string ret;
    ret=to_string(j);
    for(i=0;i<n;i++)
    {
        if(s!=""&&s[i]<58)
        {
            r=i;
            break;
        }
    }    
    ret+=to_string(r);
    ret+=s[r];
    int x=s[r]-48;
    string s1=m[x];
    n=s1.size();
    for(i=0;i<n;i++)
    {
        if(s1[i]==ret[0])
        {    
            ret+=to_string(i);
            break;
        }
    }    
    return ret;
}

//Function for breaking the link between interfaces
void setLinkDown(Ptr<Ipv4> interface, uint32_t index,uint32_t start,uint32_t end)
{
  interface->SetDown(index);
  anim->UpdateLinkDescription (start, end, "Link Down");
}

//Function for set up the link between interfaces
void setLinkUp(Ptr<Ipv4> interface, uint32_t index,uint32_t start,uint32_t end)
{
  interface->SetUp(index);
  anim->UpdateLinkDescription (start, end, "");
}

//Function to monitor packets at the receiving end
uint32_t receivedPckts= 0;
void PcktReceived(std::string context, Ptr<const Packet> pkt)
{
  receivedPckts++;
}

//Main Function
int main (int argc, char **argv)
{
  bool verbose = true;
  bool printRoutingTables = true;
  bool showPings = true;
  bool enableFlowMonitor = true;
  std::string SplitHorizon ("PoisonReverse");
  
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  //Displaying in Command Line 
  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "turn on log components", verbose);
  cmd.AddValue ("printRoutingTables", "Print routing tables at 30, 60 and 90 seconds", printRoutingTables);
  cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
  cmd.AddValue ("splitHorizonStrategy", "Split Horizon strategy to use (NoSplitHorizon, SplitHorizon, PoisonReverse)", SplitHorizon);
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);


  if (verbose)
  {
   LogComponentEnable ("OSPF-Ipv4", LOG_LEVEL_INFO);
   /*LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
   LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
 */ }
  
  
  int nh=3; //No. of Hosts
  int nr=7; //No. of Routers
  int links,n=0,i=0,k=0,j=0,rout=0;
  int noder=(nr+rand())%(nr*nr);
  if(noder>nr*nr)
     links=nr+(noder%nr);
  else
     links=nr*4;
  
  
  
  NS_LOG_INFO ("Creating nodes..");
  NodeContainer hosts;
  NodeContainer routers;
  
  
  InternetStackHelper stack; //Routers
  InternetStackHelper nodes; //Hosts
  
  //Ipv4
  Ipv4AddressHelper ipv4;
  Ipv4InterfaceContainer nethosts[links];
  Ipv4InterfaceContainer netrouters[links];
  Ipv4GlobalRoutingHelper routingHelper;
  //Link
  PointToPointHelper p2p;
  
  //Nodes Creation
  hosts.Create(nh);
  routers.Create(nr);

  stack.SetIpv4StackInstall (true);
  stack.Install(routers);

  nodes.SetIpv4StackInstall(true);
  nodes.Install(hosts);
 
  NodeContainer net[links];
  NetDeviceContainer devices[links];
 
  
  NS_LOG_INFO ("Creating Links...");
  for(int i=0;i<nh;i++)
      host[i]="";
  for(int i=0;i<nr;i++)
      m[i]="";
  for(char i='A';i<='Z';i++)
      m1[i]="";
  char hos='A';
  for(int l=0;l<(links*links);l++)
  {
      string dr=to_string(noder%10)+"Mbps"; //Assigning Data Rate
      string delay=to_string(noder%5)+"ms"; //Assigning Delay
      p2p.SetDeviceAttribute ("DataRate", StringValue (dr));
      p2p.SetChannelAttribute ("Delay", StringValue (delay));
      string ipv;
      if(n<(nh*2)) //Connecting Hosts
      { 
          int f=0;
          int r=rand()%nr;
          for(char h='A';h<=hos;h++)
          if(m[r].find(h)!=string::npos)
          {
              f=1;
          }
          if(f==0)
          {
          char c='A';    
          if(m1[c+(n%nh)].size()<2)
          {
          hos=c+(n%nh);
          net[i].Add(hosts.Get(n%nh));
          net[i].Add(routers.Get(r));
          ipv="10.0."+to_string(k)+".0";
          devices[i]=p2p.Install(net[i]);
          char* ipvad=&ipv[0];
          ipv4.SetBase(Ipv4Address(ipvad),"255.255.255.0");
          nethosts[n]=ipv4.Assign(devices[i]);
          if(m[r]=="")
              m[r]=hos;
          else
              m[r]+=hos;
          if(m1[hos]=="")
              m1[hos]=to_string(r);
          else
              m1[hos]+=to_string(r);
          int ra=rand();
          if(host[n%nh]=="")
            host[n%nh]=to_string(n);
          else    
            host[n%nh]+=to_string(n);
          nethosts[n].SetMetric(0,(ra+i+9)%15);
          nethosts[n].SetMetric(1,(ra+i+9)%15);
          i++;
          n++;
          k++; 
          }
          }
      }
      else
      {
        if(rout<nr-1) //Connecting Router
        {    
          int x=(rout+1);  
          if(m[rout].find(to_string(x))==string::npos &&m[x].find(to_string(rout))==string::npos)
          {
           ipv="10.0."+to_string(k)+".0";
           char* ipvad=&ipv[0];
           ipv4.SetBase(Ipv4Address(ipvad),"255.255.255.0");
          
           net[i].Add(routers.Get(rout));
           net[i].Add(routers.Get(x));
           if(m[rout]=="")
             m[rout]=to_string(x);
           else
            m[rout]+=to_string(x);
           if(m[x]=="")
            m[x]=to_string(rout);
           else     
            m[x]+=to_string(rout);
           devices[i]=p2p.Install(net[i]);
           netrouters[j]=ipv4.Assign(devices[i]);
           int ra=rand();
           netrouters[j].SetMetric(0,(ra+i+9)%15);
           netrouters[j].SetMetric(1,(ra+i+9)%15);
           j++;
           k++;
           i++;
           rout++;
          }
        }
        else
        {
         if(i%rand()==0) //Connecting Routers
           {
           int r=(rand()+i+nr)%nr;
           int r1=(rand()+i+k+nr+1)%nr;
           if(m[r].find(to_string(r1))==string::npos)
           {           
           ipv="10.0."+to_string(k)+".0";
            char* ipvad=&ipv[0];
           ipv4.SetBase(Ipv4Address(ipvad),"255.255.255.0");
            net[i].Add(routers.Get(r));
            net[i].Add(routers.Get(r1));
            if(m[r]=="")
              m[r]=to_string(r1);
            else
              m[r]+=to_string(r1);
            if(m[r1]=="")
              m[r1]=to_string(r);
            else
              m[r1]+=to_string(r);
            devices[i]=p2p.Install(net[i]);
            int ra=rand();
            netrouters[j]=ipv4.Assign(devices[i]);
            netrouters[j].SetMetric(0,(ra+i+9)%15);
            netrouters[j].SetMetric(1,(ra+i+9)%15);
            j++;
            i++;
            k++;
           }           
          }
          else
          {
              int r=(rand()+i+nr)%nr;
              int h=(rand())%nh;
              char c='A';
              int f=0;
              for(char h='A';h<=(c+nh);h++)
              if(m[r].find(h)!=string::npos)
              {
               f=1;
              }
              c+=h;
              if(f==0)
              {    
                 int ra=rand();
               ipv="10.0."+to_string(k)+".0";
               char* ipvad=&ipv[0];
               ipv4.SetBase(Ipv4Address(ipvad),"255.255.255.0");
              
               net[i].Add(hosts.Get(h));
               net[i].Add(routers.Get(r));
               if(m[r]=="")
                 m[r]=c;
               else
                 m[r]+=c;
               if(m1[c]=="")
                 m1[c]=to_string(r);
               else
                 m1[c]+=to_string(r);
               host[h]+=to_string((n-nh));
               devices[i]=p2p.Install(net[i]);
               nethosts[n-nh]=ipv4.Assign(devices[i]);
               nethosts[n-nh].SetMetric(0,(ra+i+9)%(15+i));
               nethosts[n-nh].SetMetric(1,(ra+i+9)%(15+i));
               n++;
               k++;
               i++;
              }
           }
         }
      }
  }
  
   //Installing positions in the respective nodes
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator>alloc= CreateObject<ListPositionAllocator>();
  for(int i=0;i<nh+nr;i++)
  {
      if(i<nh)
      {
          if(i%2==0)
              alloc->Add(Vector(((noder+rand()+200)%200+100),((noder+rand()+200)%200+100), 1));
          else
              alloc->Add(Vector((rand()%200)+200,(rand()%200)+200, 1));
          
      }
      else
      {
          if(i%2==0)
              if(i%4==0)
                 alloc->Add(Vector((rand()+100)%100,(noder+rand())%100+(100+(rand()%i)*10), 1));
              else
                 alloc->Add(Vector((rand()+100)%100,(noder+rand())%100+(100+(rand()%(i-1))*10), 1));
          else
              if(i%3==0)
                 alloc->Add(Vector((noder+rand())%100+(80+rand()%i*10),(rand()+80)%80, 1));
              else
                 alloc->Add(Vector((noder+rand())%100+(80+rand()%i+1*10),(rand()+80)%80, 1));
    }
          
  }
  mobility.SetPositionAllocator(alloc);
  for(int i=0;i<nr+nh;i++)
  {
      if(i<nh)
         mobility.Install(hosts.Get(i));
      else
         mobility.Install(routers.Get(i-nh));
  }    
  
  NS_LOG_INFO ("Created IPv4 and routing");
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
   //Creating Routing Table
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("OSPF-IPv4.txt", std::ios::out);
    if (printRoutingTables)
    {
      for(int j=0;j<nh+nr;j++)
      {
          if(j<nh)
           routingHelper.PrintRoutingTableAt (Seconds (30.0),hosts.Get(j), routingStream);
          else
           routingHelper.PrintRoutingTableAt (Seconds (30.0),routers.Get(j-nh), routingStream);
      }     
      for(int j=0;j<nh+nr;j++)
      {
          if(j<nh)
           routingHelper.PrintRoutingTableAt (Seconds (60.0),hosts.Get(j), routingStream);
          else
           routingHelper.PrintRoutingTableAt (Seconds (60.0),routers.Get(j-nh), routingStream);
      }
      for(int j=0;j<nh+nr;j++)
      {
          if(j<nh)
           routingHelper.PrintRoutingTableAt (Seconds (90.0),hosts.Get(j), routingStream);
          else
           routingHelper.PrintRoutingTableAt (Seconds (90.0),routers.Get(j-nh), routingStream);
      }
    }
  

  uint32_t packetSize = 18000;
  uint32_t maxPacketCount = 50;
  int pack=0;
  int ti=0;
  int v=6;
  Time interPacketInterval = Seconds (1.0);
  NS_LOG_INFO ("Applications Created"); 
  UdpEchoServerHelper echoServer (9);
  for(int i=0;i<v;i++)
  {    
      int s=rand()%nh;
      int c=(rand()+1)%nh;
      if(s==c)
      {    
          if(s<(nh-1))
            s=s+1;
          else
            s=s-1;
      }    
      int k;
      string sh=host[s];
      k=sh[(rand()%sh.size())]-48;
   //Creating server apps
   ApplicationContainer sapps = echoServer.Install (hosts.Get(s));
   sapps.Start (Seconds ((1.0)+(i*50)));
   sapps.Stop (Seconds ((50.0)+(i*50)));
   UdpEchoClientHelper echoClient (Address(nethosts[k].GetAddress(0)), 9);
   echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   echoClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
   echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
   //Creating client apps
   ApplicationContainer capps = echoClient.Install (hosts.Get(c));
   capps.Start (Seconds ((3.0)+(i*50)));
   capps.Stop (Seconds ((50.0)+(i*50)));   
   pack+=maxPacketCount;
   //Calling the monitoring function for received packets
   Ptr<PacketSink> sink= StaticCast<PacketSink>(sapps.Get(0));
   sink->TraceConnect("Rx", "Packet Received", MakeCallback(&PcktReceived));
   ti=i;
  }
  
  
   NS_LOG_INFO ("Created TR File");
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("OSPF-IPv4.tr"));
  
  //Pointer objects to interfaces
Ptr<Ipv4> ipv4a[nr];
Ptr<Ipv4> ipv4b[nr];
int o=0;
NS_LOG_INFO ("Setting Link Down/UP");
for(int i=0;i<nr-1;i++)
{
    if(i%2==0)
    {
          string a=pos(i);
          int si=a.size();
          if(si==4)
          {  
          int p=a[0]-48;
          int g=a[1]-48;
          int p1=a[2]-48;
          int g1=a[3]-48;
          ipv4a[o]=routers.Get(p)->GetObject<Ipv4>();
          ipv4b[o]=routers.Get(p1)->GetObject<Ipv4>();
          //Setting down the links
          Simulator::Schedule (Seconds ((i+1)*5), &setLinkDown,ipv4a[o],g,nh+p,nh+p1);
          Simulator::Schedule (Seconds ((i+1)*10), &setLinkDown,ipv4b[o],g1,nh+p,nh+p1);
          //Setting up the links
          Simulator::Schedule (Seconds (15*(i+1)), &setLinkUp,ipv4a[o],g,nh+p,nh+p1);
          Simulator::Schedule (Seconds (20*(i+1)), &setLinkUp,ipv4b[o],g1,nh+p,nh+p1);
          o++;
          }
    }
}
 

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (300));
  
  NS_LOG_INFO ("Created XML File");
  anim= new AnimationInterface("OSPF-IPv4.xml");
  
  for(int i=0;i<nr+nh;i++)
  {
   if(i<nh)
   {
    string n="Workstation "+to_string(i);  
    anim->UpdateNodeDescription (i, n);
   }
   else
   {    
    string n="Routers "+to_string(i-nh);  
    anim->UpdateNodeDescription (i, n);
   }
  }
  disp(nr);
  
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor= flowHelper.InstallAll();

  /* Now, do the actual simulation. */
  Simulator::Run ();
  Simulator::Destroy ();
  flowMonitor->SerializeToXmlFile("OSPF-IPv4-flow.xml", true, true);  
  
  //Packet loss ratio
  NS_LOG_INFO ("\n#############################\n");
  uint32_t lostPckts= (pack) - receivedPckts;
  double  lostPcktRatio= ((double)lostPckts / (pack)) * 100;
  NS_LOG_INFO("Total number of packets lost : "+to_string(lostPckts));
  NS_LOG_INFO("Packet loss ratio: "+to_string(lostPcktRatio));
  NS_LOG_INFO ("\n#############################");
  //Packet delivery ratio
  NS_LOG_INFO ("\n***************************\n");
  double deliveryPcktRatio= ((double)receivedPckts / (pack)) * 100;
  NS_LOG_INFO("Total number of packets received : "+to_string(receivedPckts));
  NS_LOG_INFO("Packet delivery ratio: "+to_string(deliveryPcktRatio));
  NS_LOG_INFO ("\n***************************\n");
  // Throughtput calculation
  NS_LOG_INFO ("\n#############################\n");
  uint32_t receivedBytes= packetSize * receivedPckts;
  uint32_t time= (ti+1)*50;
  double throughput= (((double)receivedBytes * 8)/ (time * 1000));
  NS_LOG_INFO ("Throughtput in Kbps : "+to_string(throughput));
  NS_LOG_INFO ("\n#############################");
  
  NS_LOG_INFO ("Done.");
}
