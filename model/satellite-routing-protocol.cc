#include "satellite-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/ipv4-header.h"
#include "ns3/mobility-model.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/simulator.h"
#include "ns3/channel.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteRoutingProtocol");

NS_OBJECT_ENSURE_REGISTERED(SatelliteRoutingProtocol);

// Initialize the static map
std::map<Ipv4Address, Ptr<Node>> SatelliteRoutingProtocol::m_ipToNodeMap;

TypeId
SatelliteRoutingProtocol::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatelliteRoutingProtocol")
        .SetParent<Ipv4RoutingProtocol>()
        .SetGroupName("Satellite")
        .AddConstructor<SatelliteRoutingProtocol>();
    return tid;
}

SatelliteRoutingProtocol::SatelliteRoutingProtocol() 
    : m_updateInterval(Seconds(1.0)), m_maxNeighbors(6)
{
    // Set the callback function for our timer. This is done only once.
    m_updateTimer.SetFunction(&SatelliteRoutingProtocol::UpdateActiveNeighbors, this);
}

SatelliteRoutingProtocol::~SatelliteRoutingProtocol() {
    m_updateTimer.Cancel();
}

void
SatelliteRoutingProtocol::DoInitialize()
{
    Ipv4RoutingProtocol::DoInitialize();
    Start();
}

void SatelliteRoutingProtocol::Start()
{
    m_updateTimer.Schedule(Seconds(0.1));
}


void SatelliteRoutingProtocol::AddIpToNodeMapping(Ipv4Address ip, Ptr<Node> node)
{
    m_ipToNodeMap[ip] = node;
}

void SatelliteRoutingProtocol::ClearIpToNodeMapping()
{
    m_ipToNodeMap.clear();
}

const std::map<Ipv4Address, Ptr<Node>>& SatelliteRoutingProtocol::GetIpToNodeMap()
{
    return m_ipToNodeMap;
}

void SatelliteRoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
{
    m_ipv4 = ipv4;
}

void SatelliteRoutingProtocol::SetOrbitalPlanes(const std::vector<NodeContainer>& orbitalPlanes)
{
    m_orbitalPlanes = orbitalPlanes;
}

void SatelliteRoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
    *stream->GetStream() << "SatelliteRoutingProtocol: Routing table for Node " << m_ipv4->GetObject<Node>()->GetId() 
                       << " at time " << Simulator::Now().As(unit) << std::endl;
    *stream->GetStream() << "  Active Neighbors (" << m_activeNeighbors.size() << " total):" << std::endl;
    for (const auto& neighbor : m_activeNeighbors)
    {
        *stream->GetStream() << "    - Neighbor Node ID: " << neighbor.neighborNode->GetId() 
                           << ", Local Device IfIndex: " << neighbor.localDevice->GetIfIndex() << std::endl;
    }
}

void SatelliteRoutingProtocol::NotifyInterfaceUp(uint32_t i) { }
void SatelliteRoutingProtocol::NotifyInterfaceDown(uint32_t i) { }
void SatelliteRoutingProtocol::NotifyAddAddress(uint32_t i, Ipv4InterfaceAddress a) { }
void SatelliteRoutingProtocol::NotifyRemoveAddress(uint32_t i, Ipv4InterfaceAddress a) { }


void SatelliteRoutingProtocol::UpdateActiveNeighbors()
{
    NS_LOG_DEBUG("Updating active neighbors for node " << m_ipv4->GetObject<Node>()->GetId());
    
    m_activeNeighbors.clear();
    
    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    int currentPlaneIdx = -1;
    int currentNodeIdx = -1;

    // Find our own position in the topology definition
    for (size_t i = 0; i < m_orbitalPlanes.size(); ++i) {
        for (size_t j = 0; j < m_orbitalPlanes[i].GetN(); ++j) {
            if (m_orbitalPlanes[i].Get(j) == thisNode) {
                currentPlaneIdx = i;
                currentNodeIdx = j;
                break;
            }
        }
        if (currentPlaneIdx != -1) break;
    }
    if (currentPlaneIdx == -1) {
        NS_LOG_WARN("Node " << thisNode->GetId() << " not found in orbital planes topology.");
        m_updateTimer.Schedule(m_updateInterval);
        return;
    }

    // --- Step 1: Discover all physically connected neighbors first ---
    std::vector<NeighborInfo> allPhysicalNeighbors;
    for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i) {
        Ptr<NetDevice> localDevice = m_ipv4->GetNetDevice(i);
        Ptr<Channel> channel = localDevice->GetChannel();
        if (channel && channel->GetNDevices() == 2) {
            Ptr<NetDevice> peerDevice = (channel->GetDevice(0) == localDevice) ? channel->GetDevice(1) : channel->GetDevice(0);
            if (peerDevice) {
                Ptr<Node> peerNode = peerDevice->GetNode();
                allPhysicalNeighbors.push_back({peerNode, localDevice});
            }
        }
    }

    // --- Step 2: Apply routing logic based on the discovered neighbors ---

    // 2a. Force add intra-plane neighbors
    const auto& myPlane = m_orbitalPlanes[currentPlaneIdx];
    if (myPlane.GetN() > 1) {
        Ptr<Node> pre = myPlane.Get((currentNodeIdx + myPlane.GetN() - 1) % myPlane.GetN());
        Ptr<Node> nxt = myPlane.Get((currentNodeIdx + 1) % myPlane.GetN());
        
        for (const auto& neighbor : allPhysicalNeighbors) {
            if (neighbor.neighborNode == pre || neighbor.neighborNode == nxt) {
                m_activeNeighbors.push_back(neighbor);
            }
        }
    }
    
    // 2b. Find best inter-plane neighbors from the remaining physical links
    struct Edge { double dist; NeighborInfo info; };
    std::vector<Edge> interPlaneCandidates;
    Ptr<MobilityModel> thisMobility = thisNode->GetObject<MobilityModel>();

    for (const auto& neighbor : allPhysicalNeighbors) {
        bool isIntraPlane = false;
        for (const auto& active : m_activeNeighbors) {
            if (neighbor.neighborNode == active.neighborNode) {
                isIntraPlane = true;
                break;
            }
        }
        if (!isIntraPlane) {
            interPlaneCandidates.push_back({thisMobility->GetDistanceFrom(neighbor.neighborNode->GetObject<MobilityModel>()), neighbor});
        }
    }
    
    std::sort(interPlaneCandidates.begin(), interPlaneCandidates.end(), [](const Edge& a, const Edge& b){ return a.dist < b.dist; });
    
    uint32_t remainingSlots = m_maxNeighbors - m_activeNeighbors.size();
    for (size_t i = 0; i < remainingSlots && i < interPlaneCandidates.size(); ++i) {
        m_activeNeighbors.push_back(interPlaneCandidates[i].info);
    }

    // Reschedule the timer for the next update.
    m_updateTimer.Schedule(m_updateInterval);
}


bool
SatelliteRoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                     const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb,
                                     const LocalDeliverCallback& lcb, const ErrorCallback& ecb)
{
    // Check if the destination address is one of our own addresses.
    if (m_ipv4->GetInterfaceForAddress(header.GetDestination()) >= 0)
    {
        NS_LOG_INFO("RouteInput: Packet for " << header.GetDestination() << " is for me. Delivering locally.");
        lcb(p, header, idev->GetIfIndex());
        return true;
    }

    // The packet is not for us, so we need to forward it.
    NS_LOG_INFO("RouteInput: Packet for " << header.GetDestination() << " is not for me. Attempting to forward.");

    Socket::SocketErrno sockerr;
    // We need a non-const Ptr to the packet for RouteOutput, as it might be modified.
    Ptr<Packet> packet = p->Copy(); 
    Ptr<Ipv4Route> route = RouteOutput(packet, header, nullptr, sockerr);

    if (route)
    {
        NS_LOG_INFO("  -> Found a route. Forwarding to gateway " << route->GetGateway() 
                    << " via interface " << route->GetOutputDevice()->GetIfIndex());
        // Forward the packet using the unicast callback.
        ucb(route, packet, header);
        return true; // We have successfully handled the packet.
    }
    else
    {
        NS_LOG_WARN("  -> No route found. Dropping packet.");
        // The ErrorCallback could be used here to notify the sender, but for now we just drop.
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false; // We could not handle the packet.
    }
}

Ptr<Ipv4Route>
SatelliteRoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
    if (!p) return nullptr; 

    // --- Start of Source Address Correction Logic ---
    Ipv4Header Hdr = header; // Create a mutable copy.
    bool sourceAddressNeedsUpdate = (Hdr.GetSource() == Ipv4Address("102.102.102.102"));
    if (sourceAddressNeedsUpdate) {
        NS_LOG_INFO("RouteOutput: Detected temporary source address. Will update after route selection.");
    }
    // --- End of Source Address Correction Logic ---

    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    NS_LOG_INFO("RouteOutput on Node " << thisNode->GetId() 
                << ": Packet from " << Hdr.GetSource() 
                << " to " << Hdr.GetDestination());

    auto it = m_ipToNodeMap.find(Hdr.GetDestination());
    if (it == m_ipToNodeMap.end()) {
        NS_LOG_WARN("  -> Destination " << Hdr.GetDestination() << " not found in IP-to-Node map.");
        sockerr = Socket::ERROR_NOROUTETOHOST;
        return nullptr;
    }
    Ptr<Node> destNode = it->second;
    NS_LOG_INFO("  -> Destination Node ID: " << destNode->GetId());

    // This check should not be necessary if the stack works correctly, 
    // but as a safeguard, we check if the destination is local.
    // If so, RouteOutput should not have been called.
    if (m_ipv4->GetInterfaceForAddress(header.GetDestination()) >= 0) {
       NS_LOG_WARN("  -> Destination is local, RouteOutput should not have been called. Aborting.");
       return nullptr;
    }

    Ptr<MobilityModel> destMobility = destNode->GetObject<MobilityModel>();
    if (!destMobility) { 
        NS_LOG_ERROR("  -> Destination node " << destNode->GetId() << " has no mobility model.");
        sockerr = Socket::ERROR_NOROUTETOHOST; 
        return nullptr; 
    }

    double minDistanceToDest = thisNode->GetObject<MobilityModel>()->GetDistanceFrom(destMobility);
    NeighborInfo bestNextHop;
    bool bestHopFound = false;

    NS_LOG_DEBUG("  -> Finding best next hop among " << m_activeNeighbors.size() << " active neighbors:");
    for (const auto& neighborInfo : m_activeNeighbors) {
        Ptr<MobilityModel> neighborMobility = neighborInfo.neighborNode->GetObject<MobilityModel>();
        double dist = neighborMobility->GetDistanceFrom(destMobility);
        NS_LOG_DEBUG("    - Considering neighbor Node " << neighborInfo.neighborNode->GetId() 
                     << " (dist to dest: " << dist << ")");

        if (dist < minDistanceToDest) {
            minDistanceToDest = dist;
            bestNextHop = neighborInfo;
            bestHopFound = true;
        }
    }

    if (!bestHopFound) {
        if (m_activeNeighbors.size() == 0) {
            NS_LOG_WARN("  -> No active neighbors found to forward the packet.");
            sockerr = Socket::ERROR_NOROUTETOHOST;
            return nullptr;
        } else {
            Ptr<UniformRandomVariable> uniformRv = CreateObject<UniformRandomVariable>();
            uniformRv->SetAttribute("Min", DoubleValue(0.0));
            uniformRv->SetAttribute("Max", DoubleValue((int)m_activeNeighbors.size() - 1));
            bestNextHop = m_activeNeighbors[uniformRv->GetInteger()];
        }
    }
    
    NS_LOG_INFO("  -> Best next hop: Node " << bestNextHop.neighborNode->GetId() 
                << " via local ifIndex " << bestNextHop.localDevice->GetIfIndex()
                << " (dist: " << minDistanceToDest << ")");

    // --- Start of Source Address Finalization Logic ---
    if (sourceAddressNeedsUpdate) {
        Ptr<Ipv4> ipv4 = thisNode->GetObject<Ipv4>();
        int32_t ifIndex = ipv4->GetInterfaceForDevice(bestNextHop.localDevice);
        if (ifIndex >= 0) {
            Ipv4InterfaceAddress ifAddr = ipv4->GetAddress(ifIndex, 0);
            Hdr.SetSource(ifAddr.GetLocal());

            NS_LOG_INFO("  -> Updated source address to " << Hdr.GetSource() 
                        << " based on output ifIndex " << ifIndex);
        }
    }
    // --- End of Source Address Finalization Logic ---
    
    Ptr<Ipv4Route> route = Create<Ipv4Route>();
    route->SetDestination(Hdr.GetDestination());
    route->SetSource(Hdr.GetSource());

    // The gateway is the IP address of the interface on the neighboring node.
    // We need to find this address by traversing the channel from our local device.
    Ptr<Channel> channel = bestNextHop.localDevice->GetChannel();
    Ptr<NetDevice> peerDevice = (channel->GetDevice(0) == bestNextHop.localDevice) 
                              ? channel->GetDevice(1) 
                              : channel->GetDevice(0);
    Ptr<Ipv4> peerIpv4 = peerDevice->GetNode()->GetObject<Ipv4>();
    int32_t peerInterfaceIndex = peerIpv4->GetInterfaceForDevice(peerDevice);

    Ipv4Address gatewayAddress = peerIpv4->GetAddress(peerInterfaceIndex, 0).GetLocal();
    route->SetGateway(gatewayAddress);
    route->SetOutputDevice(bestNextHop.localDevice);

    NS_LOG_INFO("  -> Created route: Dest " << route->GetDestination() 
                << ", Gateway " << route->GetGateway() 
                << ", Output IfIndex " << route->GetOutputDevice()->GetIfIndex());

    return route;
}

} // namespace ns3 