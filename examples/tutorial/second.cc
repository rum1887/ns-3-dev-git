/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

static NodeContainer
Createp2pNodes (int nodeCount)
{
  NodeContainer p2pNodes;
  p2pNodes.Create(nodeCount);
  return p2pNodes;
}

static NodeContainer
CreateCsmaBus(NodeContainer p2pNodes ,int extra)
{
    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get (1));
    csmaNodes.Create (extra);
    return csmaNodes;
}

static PointToPointHelper
CreatePointToPointCommunication (std::string dataRate, std::string delay)
{
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(delay));
    return pointToPoint;
}

static CsmaHelper
CreateCsmaCommunication (std::string dataRate, std::uint32_t delay)
{   CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue (dataRate));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (delay)));
    return csma;

}

static void
Init (NodeContainer p2pNodes,NodeContainer csmaNodes, PointToPointHelper &pointTopoint,CsmaHelper &csma, NetDeviceContainer &csmaDevices,Ipv4InterfaceContainer &csmainterfaces)
{
    pointTopoint = CreatePointToPointCommunication ("5Mbps", "2ms");
    csma = CreateCsmaCommunication ("100Mbps",6560);

    NetDeviceContainer p2pDevices;
    p2pDevices = pointTopoint.Install(p2pNodes);
    
    csmaDevices = csma.Install (csmaNodes);

    InternetStackHelper stack;
    stack.Install (p2pNodes.Get(0));
    stack.Install (csmaNodes);
    
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);
                                              
    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign (csmaDevices);
}

static void
InstallServer (NodeContainer csmaNodes,uint32_t nCsma)
{
    UdpEchoServerHelper echoServer (9);
    
    ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));
}

static void
InstallClient (NodeContainer p2pNodes,NodeContainer csmaNodes, uint32_t nCsma, Ipv4InterfaceContainer &csmaInterfaces)
{
    UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
    ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));
}

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 3;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

    PointToPointHelper pointToPoint ;
    CsmaHelper csma;
    Ipv4InterfaceContainer csmaInterfaces;
    NetDeviceContainer csmaDevices;
    
    NodeContainer p2pNodes = Createp2pNodes(2);
    NodeContainer csmaNodes = CreateCsmaBus(p2pNodes , nCsma);
    Init(p2pNodes,csmaNodes,pointToPoint,csma,csmaDevices,csmaInterfaces);
    InstallServer (csmaNodes,nCsma);
    InstallClient ( p2pNodes, csmaNodes,  nCsma, csmaInterfaces);
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    pointToPoint.EnablePcapAll("second");
    csma.EnablePcap("second", csmaDevices.Get(1), true);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

