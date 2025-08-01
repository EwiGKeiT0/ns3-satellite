#include "satellite-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/ipv4-header.h"
#include "ns3/mobility-model.h"
#include "satellite-circular-mobility-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/simulator.h"
#include "ns3/channel.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteRoutingProtocol");

NS_OBJECT_ENSURE_REGISTERED(SatelliteRoutingProtocol);

Vector
CrossProduct(const Vector& a, const Vector& b) {
    return Vector(a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x);
}

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
    // Check if this node is a satellite or a ground station
    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    if (thisNode->GetObject<SatelliteCircularMobilityModel>())
    {
        // This is a satellite, start the neighbor update process
        Ipv4RoutingProtocol::DoInitialize();
        Start();
    }
    else
    {
        // This is a ground station, do not start the timer
        Ipv4RoutingProtocol::DoInitialize();
    }
}

void
SatelliteRoutingProtocol::DoDispose()
{
    m_orbitalPlanes.reset();
    Ipv4RoutingProtocol::DoDispose();
}

void
SatelliteRoutingProtocol::Start()
{
    m_updateTimer.Schedule(Seconds(0.1));
}


void
SatelliteRoutingProtocol::AddIpToNodeMapping(Ipv4Address ip, Ptr<Node> node)
{
    m_ipToNodeMap[ip] = node;
}

void
SatelliteRoutingProtocol::AddIpToNodeMapping(const NodeContainer& allSatellites)
{
    for(uint32_t i = 0; i < allSatellites.GetN(); ++i)
    {
        Ptr<Node> node = allSatellites.Get(i);
        Ptr<Ipv4> ipv4Node = node->GetObject<Ipv4>();
        for (uint32_t j = 1; j < ipv4Node->GetNInterfaces(); ++j) {
            SatelliteRoutingProtocol::AddIpToNodeMapping(ipv4Node->GetAddress(j, 0).GetLocal(), node);
        }
    }
}

void
SatelliteRoutingProtocol::ClearIpToNodeMapping()
{
    m_ipToNodeMap.clear();
}

const std::map<Ipv4Address, Ptr<Node>>&
SatelliteRoutingProtocol::GetIpToNodeMap()
{
    return m_ipToNodeMap;
}

void
SatelliteRoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
{
    m_ipv4 = ipv4;
}

void
SatelliteRoutingProtocol::SetOrbitalPlanes(
    std::shared_ptr<const std::vector<NodeContainer>> orbitalPlanes)
{
    m_orbitalPlanes = orbitalPlanes;
}

void
SatelliteRoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
    // *stream->GetStream() << "SatelliteRoutingProtocol: Routing table for Node " << m_ipv4->GetObject<Node>()->GetId() 
    //                    << " at time " << Simulator::Now().As(unit) << std::endl;
    // *stream->GetStream() << "  Active Neighbors (" << m_activeNeighbors.size() << " total):" << std::endl;
    for (const auto& neighbor : m_activeNeighbors)
    {
        // *stream->GetStream() << "    - Neighbor Node ID: " << neighbor.neighborNode->GetId() 
        //                    << ", Local Device IfIndex: " << neighbor.localDevice->GetIfIndex() << std::endl;
        *stream->GetStream() << "addLines(" << m_ipv4->GetObject<Node>()->GetId() << ", " << neighbor.neighborNode->GetId() << ");" << std::endl;
    }
}

void
SatelliteRoutingProtocol::NotifyInterfaceUp(uint32_t i) { }
void
SatelliteRoutingProtocol::NotifyInterfaceDown(uint32_t i) { }
void
SatelliteRoutingProtocol::NotifyAddAddress(uint32_t i, Ipv4InterfaceAddress a) { }
void
SatelliteRoutingProtocol::NotifyRemoveAddress(uint32_t i, Ipv4InterfaceAddress a) { }

void
SatelliteRoutingProtocol::UpdateActiveNeighbors()
{
    NS_LOG_DEBUG("Updating active neighbors for node " << m_ipv4->GetObject<Node>()->GetId());
    
    m_activeNeighbors.clear();

    for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i)
    {
        Ptr<NetDevice> localDevice = m_ipv4->GetNetDevice(i);
        Ptr<Channel> channel = localDevice->GetChannel();
        if (channel && channel->GetNDevices() == 2)
        {
            Ptr<NetDevice> peerDevice = (channel->GetDevice(0) == localDevice) ? 
                channel->GetDevice(1) : channel->GetDevice(0);
            if (peerDevice)
            {
                Ptr<Node> peerNode = peerDevice->GetNode();
                if (peerNode && peerNode->GetObject<SatelliteCircularMobilityModel>())
                {
                    m_activeNeighbors.push_back({peerNode, localDevice});
                }
            }
        }
    }
    
    // Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    // int currentPlaneIdx = -1;
    // int currentNodeIdx = -1;

    // // Find our own position in the topology definition
    // for (size_t i = 0; i < m_orbitalPlanes->size(); ++i) {
    //     for (size_t j = 0; j < m_orbitalPlanes->at(i).GetN(); ++j) {
    //         if (m_orbitalPlanes->at(i).Get(j) == thisNode) {
    //             currentPlaneIdx = i;
    //             currentNodeIdx = j;
    //             break;
    //         }
    //     }
    //     if (currentPlaneIdx != -1) break;
    // }
    // if (currentPlaneIdx == -1) {
    //     NS_LOG_WARN("Node " << thisNode->GetId() << " not found in orbital planes topology.");
    //     m_updateTimer.Schedule(m_updateInterval);
    //     return;
    // }

    // // --- Step 1: Discover all physically connected neighbors first ---
    // std::vector<NeighborInfo> allPhysicalNeighbors;
    // for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i) {
    //     Ptr<NetDevice> localDevice = m_ipv4->GetNetDevice(i);
    //     Ptr<Channel> channel = localDevice->GetChannel();
    //     if (channel && channel->GetNDevices() == 2) {
    //         Ptr<NetDevice> peerDevice = (channel->GetDevice(0) == localDevice) ? 
    //             channel->GetDevice(1) : channel->GetDevice(0);
    //         if (peerDevice) {
    //             Ptr<Node> peerNode = peerDevice->GetNode();
    //             // Only consider other satellites as potential inter-satellite neighbors
    //             if (peerNode && peerNode->GetObject<SatelliteCircularMobilityModel>()) {
    //                 allPhysicalNeighbors.push_back({peerNode, localDevice});
    //             }
    //         }
    //     }
    // }

    // --- Step 2: Apply routing logic based on the discovered neighbors ---

    // 2. Find best inter-plane neighbors using geometry
    // const auto& myPlane = m_orbitalPlanes->at(currentPlaneIdx);
    // Ptr<Node> pre = myPlane.Get((currentNodeIdx + myPlane.GetN() - 1) % myPlane.GetN());
    // Ptr<Node> nxt = myPlane.Get((currentNodeIdx + 1) % myPlane.GetN());
    // Ptr<MobilityModel> preMobility = pre->GetObject<MobilityModel>();
    // Ptr<MobilityModel> nxtMobility = nxt->GetObject<MobilityModel>();
    // Ptr<MobilityModel> thisMobility = thisNode->GetObject<MobilityModel>();
    // Vector thisPos = thisMobility->GetPosition();
    // Vector prePos = preMobility->GetPosition();
    // Vector nxtPos = nxtMobility->GetPosition();

    // // Define the plane using a normal vector
    // Vector v1 = prePos - thisPos;
    // Vector v2 = nxtPos - thisPos;
    // Vector normal = CrossProduct(v1, v2);

    // NeighborInfo bestAboveNeighbor;
    // double minAboveDist = -1.0;

    // NeighborInfo bestBelowNeighbor;
    // double minBelowDist = -1.0;

    // for (const auto& neighbor : allPhysicalNeighbors)
    //     if (neighbor.neighborNode == pre || neighbor.neighborNode == nxt) {
    //         m_activeNeighbors.push_back(neighbor);
    //     } else {
    //         Ptr<MobilityModel> candMobility = neighbor.neighborNode->GetObject<MobilityModel>();

    //         Vector candPos = candMobility->GetPosition();
    //         Vector vCand = candPos - thisPos;
    //         double dotProduct = normal * vCand;
    //         double dist = thisMobility->GetDistanceFrom(candMobility);

    //         if (dotProduct > 0) { // Above the plane
    //             if (minAboveDist < 0 || dist < minAboveDist) {
    //                 minAboveDist = dist;
    //                 bestAboveNeighbor = neighbor;
    //             }
    //         } else if (dotProduct < 0) { // Below the plane
    //             if (minBelowDist < 0 || dist < minBelowDist) {
    //                 minBelowDist = dist;
    //                 bestBelowNeighbor = neighbor;
    //             }
    //         }
    //     }
    // if (bestAboveNeighbor.neighborNode) {
    //     m_activeNeighbors.push_back(bestAboveNeighbor);
    // }

    // if (bestBelowNeighbor.neighborNode) {
    //     m_activeNeighbors.push_back(bestBelowNeighbor);
    // }

    // Reschedule the timer for the next update.
    // m_updateTimer.Schedule(m_updateInterval);
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

    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    NS_LOG_INFO("RouteOutput on Node " << thisNode->GetId() 
                << ": Packet from " << header.GetSource() 
                << " to " << header.GetDestination());

    auto it = m_ipToNodeMap.find(header.GetDestination());
    if (it == m_ipToNodeMap.end()) {
        NS_LOG_WARN("  -> Destination " << header.GetDestination() << " not found in IP-to-Node map.");
        sockerr = Socket::ERROR_NOROUTETOHOST;
        return nullptr;
    }
    Ptr<Node> destNode = it->second;
    NS_LOG_INFO("  -> Destination Node ID: " << destNode->GetId());
    
    // Case 1: Current node is a Ground Station
    if (thisNode->GetObject<ConstantPositionMobilityModel>()) {
        NS_LOG_INFO("  -> Current node is a Ground Station. Finding closest satellite to forward to.");

        double minDistance = -1.0;
        Ptr<NetDevice> bestDevice = nullptr;
        
        // Find the satellite neighbor that is closest to this ground station
        for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i) {
            Ptr<NetDevice> dev = m_ipv4->GetNetDevice(i);
            Ptr<Channel> ch = dev->GetChannel();
            if (ch && ch->GetNDevices() == 2) {
                Ptr<NetDevice> peerDev = (ch->GetDevice(0) == dev) ? ch->GetDevice(1) : ch->GetDevice(0);
                Ptr<Node> peerNode = peerDev->GetNode();

                // Ensure the peer is a satellite
                if (peerNode && peerNode->GetObject<SatelliteCircularMobilityModel>()) {
                    double dist = thisNode->GetObject<MobilityModel>()->GetDistanceFrom(peerNode->GetObject<MobilityModel>());
                    if (minDistance < 0 || dist < minDistance) {
                        minDistance = dist;
                        bestDevice = dev;
                    }
                }
            }
        }

        if (bestDevice) {
            // The gateway is the destination itself since we assume the satellite can handle it from there.
            // Or more correctly, the IP of the peer satellite.
            Ptr<Channel> ch = bestDevice->GetChannel();
            Ptr<NetDevice> peerDev = (ch->GetDevice(0) == bestDevice) ? ch->GetDevice(1) : ch->GetDevice(0);
            NS_LOG_INFO("  -> Closest satellite is Node " << peerDev->GetNode()->GetId() << " at distance " << minDistance);
            
            Ptr<Ipv4Route> route = Create<Ipv4Route>();
            route->SetDestination(header.GetDestination());
            route->SetSource(m_ipv4->GetAddress(m_ipv4->GetInterfaceForDevice(bestDevice), 0).GetLocal());
            
            Ptr<Ipv4> peerIpv4 = peerDev->GetNode()->GetObject<Ipv4>();
            int32_t peerIfIndex = peerIpv4->GetInterfaceForDevice(peerDev);
            Ipv4Address gateway = peerIpv4->GetAddress(peerIfIndex, 0).GetLocal();

            route->SetGateway(gateway);
            route->SetOutputDevice(bestDevice);
            return route;
        }

        NS_LOG_WARN("  -> Ground station has no satellite links to forward packet.");
        sockerr = Socket::ERROR_NOROUTETOHOST;
        return nullptr;
    }

    // Case 2: Current node is a Satellite
    Ptr<MobilityModel> destMobility = destNode->GetObject<MobilityModel>();
    if (!destMobility) { 
        NS_LOG_ERROR("  -> Destination node " << destNode->GetId() << " has no mobility model.");
        sockerr = Socket::ERROR_NOROUTETOHOST; 
        return nullptr; 
    }

    double minDistanceToDest = -1.0;
    double ownDistToDest = thisNode->GetObject<MobilityModel>()->GetDistanceFrom(destMobility);
    NeighborInfo bestNextHop;
    bool bestHopFound = false;

    // Check if any neighbor is closer
    for (const auto& neighborInfo : m_activeNeighbors) {
        Ptr<MobilityModel> neighborMobility = neighborInfo.neighborNode->GetObject<MobilityModel>();
        double dist = neighborMobility->GetDistanceFrom(destMobility);
        if (!bestHopFound || dist < minDistanceToDest) {
            minDistanceToDest = dist;
            bestNextHop = neighborInfo;
            bestHopFound = true;
        }
    }

    // Subcase 2a: Destination is a Ground Station
    if (destNode->GetObject<ConstantPositionMobilityModel>()) {
        NS_LOG_INFO("  -> Destination is a Ground Station.");
        
        // If no neighbor is closer, we are the best hop. Route directly to the ground station.
        if (minDistanceToDest > ownDistToDest) {
            NS_LOG_INFO("  -> This satellite is the closest hop to the ground station. Routing directly.");
            // We need to find the specific NetDevice that connects to this ground station.
            for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i) {
                Ptr<NetDevice> dev = m_ipv4->GetNetDevice(i);
                Ptr<Channel> ch = dev->GetChannel();
                if (ch && ch->GetNDevices() == 2) {
                    Ptr<NetDevice> peerDev = (ch->GetDevice(0) == dev) ? ch->GetDevice(1) : ch->GetDevice(0);
                    if (peerDev && peerDev->GetNode() == destNode) {
                        Ptr<Ipv4Route> route = Create<Ipv4Route>();
                        route->SetDestination(header.GetDestination());
                        route->SetSource(m_ipv4->GetAddress(i, 0).GetLocal());
                        route->SetGateway(header.GetDestination()); // The gateway is the final destination
                        route->SetOutputDevice(dev);
                        return route;
                    }
                }
            }
             NS_LOG_WARN("  -> Could not find the device connected to the destination ground station.");
             sockerr = Socket::ERROR_NOROUTETOHOST;
             return nullptr;
        }
    } 
    // Subcase 2b: Destination is another Satellite
    else {
        // if (!bestHopFound) {
        //     NS_LOG_WARN("  -> No greedy hop found for satellite destination. Cannot forward.");
        //     sockerr = Socket::ERROR_NOROUTETOHOST;
        //     return nullptr;
        // }
    }
    
    NS_LOG_INFO("  -> Best inter-satellite next hop: Node " << bestNextHop.neighborNode->GetId() 
                << " via local ifIndex " << bestNextHop.localDevice->GetIfIndex());

    Ptr<Ipv4Route> route = Create<Ipv4Route>();
    route->SetDestination(header.GetDestination());
    route->SetSource(m_ipv4->GetAddress(m_ipv4->GetInterfaceForDevice(bestNextHop.localDevice), 0).GetLocal());

    Ptr<Ipv4> peerIpv4 = bestNextHop.neighborNode->GetObject<Ipv4>();
    int32_t peerInterfaceIndex = peerIpv4->GetInterfaceForDevice(bestNextHop.localDevice->GetChannel()->GetDevice(0) == bestNextHop.localDevice ? bestNextHop.localDevice->GetChannel()->GetDevice(1) : bestNextHop.localDevice->GetChannel()->GetDevice(0));
    Ipv4Address gatewayAddress = peerIpv4->GetAddress(peerInterfaceIndex, 0).GetLocal();
    
    route->SetGateway(gatewayAddress);
    route->SetOutputDevice(bestNextHop.localDevice);
    
    return route;
}

} // namespace ns3 