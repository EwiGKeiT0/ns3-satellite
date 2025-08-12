#include "satellite-sp-routing-protocol.h"
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
#include "ns3/loopback-net-device.h"

#include <limits>
#include <queue>
#include <map>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteSpRoutingProtocol");

NS_OBJECT_ENSURE_REGISTERED(SatelliteSpRoutingProtocol);

// Initialization of static members
std::map<Ipv4Address, Ptr<Node>> SatelliteSpRoutingProtocol::m_ipToNodeMap;
std::vector<std::vector<uint32_t>> SatelliteSpRoutingProtocol::m_adj;
std::map<Ptr<Node>, uint32_t> SatelliteSpRoutingProtocol::m_nodeToIndex;
NodeContainer SatelliteSpRoutingProtocol::m_allSatellites;

TypeId
SatelliteSpRoutingProtocol::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatelliteSpRoutingProtocol")
        .SetParent<Ipv4RoutingProtocol>()
        .SetGroupName("Satellite")
        .AddConstructor<SatelliteSpRoutingProtocol>();
    return tid;
}

SatelliteSpRoutingProtocol::SatelliteSpRoutingProtocol() 
    : m_updateInterval(Seconds(1.0))
{
    m_updateTimer.SetFunction(&SatelliteSpRoutingProtocol::UpdateRoutes, this);
}

SatelliteSpRoutingProtocol::~SatelliteSpRoutingProtocol() {
    m_updateTimer.Cancel();
}

void
SatelliteSpRoutingProtocol::DoInitialize()
{
    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    if (thisNode->GetObject<SatelliteCircularMobilityModel>())
    {
        Ipv4RoutingProtocol::DoInitialize();
        Start();
    }
    else
    {
        Ipv4RoutingProtocol::DoInitialize();
    }
}

void
SatelliteSpRoutingProtocol::DoDispose()
{
    Ipv4RoutingProtocol::DoDispose();
}

void
SatelliteSpRoutingProtocol::Start()
{
    m_updateTimer.Schedule(Seconds(0.1)); 
}

void
SatelliteSpRoutingProtocol::AddNode(Ptr<Node> node)
{
    m_allSatellites.Add(node);
}

void
SatelliteSpRoutingProtocol::AddIpToNodeMapping(Ipv4Address ip, Ptr<Node> node)
{
    m_ipToNodeMap[ip] = node;
}

void
SatelliteSpRoutingProtocol::AddIpToNodeMapping()
{
    ClearIpToNodeMapping();
    for (uint32_t i = 0; i < m_allSatellites.GetN(); ++i)
    {
        Ptr<Node> node = m_allSatellites.Get(i);
        Ptr<Ipv4> ipv4Node = node->GetObject<Ipv4>();
        for (uint32_t j = 1; j < ipv4Node->GetNInterfaces(); ++j) {
            m_ipToNodeMap[ipv4Node->GetAddress(j, 0).GetLocal()] = node;
        }
    }
}

void
SatelliteSpRoutingProtocol::ClearIpToNodeMapping()
{
    m_ipToNodeMap.clear();
}

const std::map<Ipv4Address, Ptr<Node>>&
SatelliteSpRoutingProtocol::GetIpToNodeMap()
{
    return m_ipToNodeMap;
}

void
SatelliteSpRoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
{
    m_ipv4 = ipv4;
}

void
SatelliteSpRoutingProtocol::InitializeTopology()
{
    NS_LOG_INFO("Building global satellite topology once.");
    
    for (uint32_t i=0; i < m_allSatellites.GetN(); ++i)
    {
        m_nodeToIndex[m_allSatellites.Get(i)] = i;
    }

    m_adj.resize(m_allSatellites.GetN());
    for (uint32_t i=0; i < m_allSatellites.GetN(); ++i)
    {
        Ptr<Node> node = m_allSatellites.Get(i);
        for(uint32_t j=0; j < node->GetNDevices(); ++j)
        {
            Ptr<NetDevice> dev = node->GetDevice(j);
            if (DynamicCast<LoopbackNetDevice>(dev))
            {
                continue;
            }
            Ptr<Channel> channel = dev->GetChannel();
            if(channel && channel->GetNDevices() == 2)
            {
                Ptr<NetDevice> peerDev = (channel->GetDevice(0) == dev) ? channel->GetDevice(1) : channel->GetDevice(0);
                Ptr<Node> peerNode = peerDev->GetNode();
                
                // Only add inter-satellite links (ISL) to the shortest path graph
                // Check if both nodes are satellites (have SatelliteCircularMobilityModel)
                bool currentIsSatellite = node->GetObject<SatelliteCircularMobilityModel>() != nullptr;
                bool peerIsSatellite = peerNode->GetObject<SatelliteCircularMobilityModel>() != nullptr;
                
                if(m_nodeToIndex.count(peerNode) && currentIsSatellite && peerIsSatellite)
                {
                    m_adj[i].push_back(m_nodeToIndex[peerNode]);
                }
            }
        }
    }
}

void
SatelliteSpRoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
    *stream->GetStream() << "SatelliteSpRoutingProtocol: Routing table for Node " << m_ipv4->GetObject<Node>()->GetId() 
                       << " at time " << Simulator::Now().As(unit) << std::endl;
    *stream->GetStream() << "  Destination Node ID\tNext Hop Node ID\tInterface" << std::endl;
    for (const auto& entry : m_routingTable)
    {
        Ptr<Node> destNode = entry.first;
        Ptr<Node> nextHop = entry.second.nextHopNode;
        uint32_t iface = entry.second.interface;
        *stream->GetStream() << "  " << destNode->GetId() << "\t\t\t" << (nextHop ? nextHop->GetId() : -1) << "\t\t\t" << iface << std::endl;
    }
}

void
SatelliteSpRoutingProtocol::NotifyInterfaceUp(uint32_t i) { }
void
SatelliteSpRoutingProtocol::NotifyInterfaceDown(uint32_t i) { }
void
SatelliteSpRoutingProtocol::NotifyAddAddress(uint32_t i, Ipv4InterfaceAddress a) { }
void
SatelliteSpRoutingProtocol::NotifyRemoveAddress(uint32_t i, Ipv4InterfaceAddress a) { }

void
SatelliteSpRoutingProtocol::UpdateRoutes()
{  
    NS_LOG_DEBUG("Updating routes for node " << m_ipv4->GetObject<Node>()->GetId());
    m_routingTable.clear();
    ComputeRoutes();
    
    m_updateTimer.Schedule(m_updateInterval);
}

uint32_t SatelliteSpRoutingProtocol::GetInterfaceToPeer(Ptr<Node> peer) const
{
    for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i)
    {
        Ptr<NetDevice> dev = m_ipv4->GetNetDevice(i);
        Ptr<Channel> ch = dev->GetChannel();
        if (ch && ch->GetNDevices() == 2)
        {
            Ptr<NetDevice> peerDev = (ch->GetDevice(0) == dev) ? ch->GetDevice(1) : ch->GetDevice(0);
            if (peerDev && peerDev->GetNode() == peer)
            {
                return m_ipv4->GetInterfaceForDevice(dev);
            }
        }
    }
    return -1; // Not found
}

void
SatelliteSpRoutingProtocol::ComputeRoutes()
{
    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    if (m_nodeToIndex.find(thisNode) == m_nodeToIndex.end())
    {
        return; 
    }
    uint32_t numNodes = m_allSatellites.GetN();
    uint32_t srcIndex = m_nodeToIndex[thisNode];

    std::vector<double> dist(numNodes, std::numeric_limits<double>::max());
    std::vector<int> from(numNodes, -1);
    dist[srcIndex] = 0;

    using PQElement = std::pair<double, uint32_t>;
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;
    pq.push({0.0, srcIndex});

    while (!pq.empty())
    {
        double d = pq.top().first;
        uint32_t u_idx = pq.top().second;
        pq.pop();

        if (d > dist[u_idx]) continue;

        Ptr<Node> u_node = m_allSatellites.Get(u_idx);
        for(const auto& v_idx : m_adj[u_idx])
        {
            Ptr<Node> v_node = m_allSatellites.Get(v_idx);
            double weight = u_node->GetObject<MobilityModel>()->GetDistanceFrom(v_node->GetObject<MobilityModel>());
            
            if(dist[u_idx] + weight < dist[v_idx])
            {
                dist[v_idx] = dist[u_idx] + weight;
                if (u_idx == srcIndex)
                {
                    from[v_idx] = v_idx;
                } else
                {
                    from[v_idx] = from[u_idx];
                }
                pq.push({dist[v_idx], v_idx});
            }
        }
    }

    for(uint32_t i=0; i < numNodes; ++i)
    {
        if(i == srcIndex || from[i] == -1) continue;

        Ptr<Node> destNode = m_allSatellites.Get(i);
        
        int nextHopIdx = from[i];

        Ptr<Node> nextHopNode = m_allSatellites.Get(nextHopIdx);
        uint32_t iface = GetInterfaceToPeer(nextHopNode);

        if (iface != (uint32_t)-1)
        {
            m_routingTable[destNode] = {nextHopNode, iface};
        }
    }
}

bool
SatelliteSpRoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                     const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb,
                                     const LocalDeliverCallback& lcb, const ErrorCallback& ecb)
{
    if (m_ipv4->GetInterfaceForAddress(header.GetDestination()) >= 0)
    {
        NS_LOG_INFO("RouteInput: Packet for " << header.GetDestination() << " is for me. Delivering locally.");
        lcb(p, header, idev->GetIfIndex());
        return true;
    }

    NS_LOG_INFO("RouteInput: Packet for " << header.GetDestination() << " is not for me. Attempting to forward.");

    Socket::SocketErrno sockerr;
    Ptr<Packet> packet = p->Copy(); 
    Ptr<Ipv4Route> route = RouteOutput(packet, header, nullptr, sockerr);

    if (route)
    {
        ucb(route, packet, header);
        return true;
    }
    else
    {
        NS_LOG_WARN("  -> Dropping packet.");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
    }
}

Ptr<Ipv4Route>
SatelliteSpRoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
    if (!p) return nullptr; 

    Ptr<Node> thisNode = m_ipv4->GetObject<Node>();
    Ipv4Address destAddr = header.GetDestination();
    NS_LOG_INFO("RouteOutput on Node " << thisNode->GetId() << " to " << destAddr);

    if (thisNode->GetObject<ConstantPositionMobilityModel>())
    {
        NS_LOG_INFO("  -> Current node is a Ground Station. Finding closest satellite.");
        double minDistance = -1.0;
        Ptr<NetDevice> bestDevice = nullptr;
        
        for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i)
        {
            Ptr<NetDevice> dev = m_ipv4->GetNetDevice(i);
            Ptr<Channel> ch = dev->GetChannel();
            if (ch && ch->GetNDevices() == 2)
            {
                Ptr<NetDevice> peerDev = (ch->GetDevice(0) == dev) ? ch->GetDevice(1) : ch->GetDevice(0);
                if (peerDev && peerDev->GetNode()->GetObject<SatelliteCircularMobilityModel>())
                {
                    double dist = thisNode->GetObject<MobilityModel>()->GetDistanceFrom(peerDev->GetNode()->GetObject<MobilityModel>());
                    if (minDistance < 0 || dist < minDistance)
                    {
                        minDistance = dist;
                        bestDevice = dev;
                    }
                }
            }
        }

        if (bestDevice)
        {
            Ptr<Channel> ch = bestDevice->GetChannel();
            Ptr<NetDevice> peerDev = (ch->GetDevice(0) == bestDevice) ? ch->GetDevice(1) : ch->GetDevice(0);
            Ptr<Ipv4> peerIpv4 = peerDev->GetNode()->GetObject<Ipv4>();
            Ipv4Address gateway = peerIpv4->GetAddress(peerIpv4->GetInterfaceForDevice(peerDev), 0).GetLocal();
            
            Ptr<Ipv4Route> route = Create<Ipv4Route>();
            route->SetDestination(destAddr);
            route->SetSource(m_ipv4->GetAddress(m_ipv4->GetInterfaceForDevice(bestDevice), 0).GetLocal());
            route->SetGateway(gateway);
            route->SetOutputDevice(bestDevice);
            return route;
        }

        NS_LOG_WARN("  -> Ground station has no satellite links.");
        sockerr = Socket::ERROR_NOROUTETOHOST;
        return nullptr;
    }
    
    // IP to Node lookup
    auto it_map = m_ipToNodeMap.find(destAddr);
    if (it_map == m_ipToNodeMap.end())
    {
        NS_LOG_WARN("  -> Destination " << destAddr << " not found in IP-to-Node map.");
        sockerr = Socket::ERROR_NOROUTETOHOST;
        return nullptr;
    }
    Ptr<Node> destNode = it_map->second;

    // Check if destination is a ground station
    if (destNode->GetObject<ConstantPositionMobilityModel>())
    {
        NS_LOG_INFO("  -> Destination is a ground station. Finding closest satellite to destination.");
        
        // Find the satellite closest to the destination ground station
        double minDistance = -1.0;
        Ptr<Node> closestSatellite = nullptr;
        
        for (uint32_t i = 0; i < m_allSatellites.GetN(); ++i)
        {
            Ptr<Node> satellite = m_allSatellites.Get(i);
            if (satellite->GetObject<SatelliteCircularMobilityModel>())
            {
                double dist = destNode->GetObject<MobilityModel>()->GetDistanceFrom(satellite->GetObject<MobilityModel>());
                if (minDistance < 0 || dist < minDistance)
                {
                    minDistance = dist;
                    closestSatellite = satellite;
                }
            }
        }
        
        if (!closestSatellite)
        {
            NS_LOG_WARN("  -> No satellites found to reach ground station.");
            sockerr = Socket::ERROR_NOROUTETOHOST;
            return nullptr;
        }
        
        // If the closest satellite is the current node, forward directly to ground station
        if (closestSatellite == thisNode)
        {
            NS_LOG_INFO("  -> Current satellite is closest to destination ground station. Forwarding directly.");
            
            // Find the interface connected to the destination ground station
            for (uint32_t i = 1; i < m_ipv4->GetNInterfaces(); ++i)
            {
                Ptr<NetDevice> dev = m_ipv4->GetNetDevice(i);
                Ptr<Channel> ch = dev->GetChannel();
                if (ch && ch->GetNDevices() == 2)
                {
                    Ptr<NetDevice> peerDev = (ch->GetDevice(0) == dev) ? ch->GetDevice(1) : ch->GetDevice(0);
                    if (peerDev && peerDev->GetNode() == destNode)
                    {
                        Ptr<Ipv4Route> route = Create<Ipv4Route>();
                        route->SetDestination(destAddr);
                        route->SetSource(m_ipv4->GetAddress(i, 0).GetLocal());
                        route->SetGateway(destAddr); // Direct connection
                        route->SetOutputDevice(dev);
                        
                        NS_LOG_INFO("  -> Direct route found to ground station via interface " << i);
                        return route;
                    }
                }
            }
            
            NS_LOG_WARN("  -> Current satellite should be closest but no direct link found to ground station.");
            sockerr = Socket::ERROR_NOROUTETOHOST;
            return nullptr;
        }
        else
        {
            NS_LOG_INFO("  -> Routing to closest satellite " << closestSatellite->GetId() << " to reach ground station.");
            
            // Route to the closest satellite using the routing table
            auto it_closest = m_routingTable.find(closestSatellite);
            if (it_closest != m_routingTable.end())
            {
                const RouteEntry& entry = it_closest->second;
                Ptr<NetDevice> outDev = m_ipv4->GetNetDevice(entry.interface);
                
                Ptr<Ipv4Route> route = Create<Ipv4Route>();
                route->SetDestination(destAddr); // Keep original destination
                route->SetSource(m_ipv4->GetAddress(entry.interface, 0).GetLocal());
                
                Ptr<Ipv4> peerIpv4 = entry.nextHopNode->GetObject<Ipv4>();
                Ptr<Channel> ch = outDev->GetChannel();
                Ptr<NetDevice> peerDev = (ch->GetDevice(0) == outDev) ? ch->GetDevice(1) : ch->GetDevice(0);
                int32_t peerIfIndex = peerIpv4->GetInterfaceForDevice(peerDev);
                Ipv4Address gateway = peerIpv4->GetAddress(peerIfIndex, 0).GetLocal();
                
                route->SetGateway(gateway);
                route->SetOutputDevice(outDev);
                
                NS_LOG_INFO("  -> Route to closest satellite found. Gateway: " << gateway);
                return route;
            }
            else
            {
                NS_LOG_WARN("  -> No route found to closest satellite " << closestSatellite->GetId());
                sockerr = Socket::ERROR_NOROUTETOHOST;
                return nullptr;
            }
        }
    }

    auto it = m_routingTable.find(destNode);
    if (it != m_routingTable.end())
    {
        const RouteEntry& entry = it->second;
        Ptr<NetDevice> outDev = m_ipv4->GetNetDevice(entry.interface);
        
        Ptr<Ipv4Route> route = Create<Ipv4Route>();
        route->SetDestination(destAddr);
        route->SetSource(m_ipv4->GetAddress(entry.interface, 0).GetLocal());

        Ptr<Ipv4> peerIpv4 = entry.nextHopNode->GetObject<Ipv4>();
        Ptr<Channel> ch = outDev->GetChannel();
        Ptr<NetDevice> peerDev = (ch->GetDevice(0) == outDev) ? ch->GetDevice(1) : ch->GetDevice(0);
        int32_t peerIfIndex = peerIpv4->GetInterfaceForDevice(peerDev);
        Ipv4Address gateway = peerIpv4->GetAddress(peerIfIndex, 0).GetLocal();
        
        route->SetGateway(gateway);
        route->SetOutputDevice(outDev);

        NS_LOG_INFO("  -> Found a route. Forwarding to gateway " << route->GetGateway() 
                    << " via interface " << route->GetOutputDevice()->GetIfIndex());

        return route;
    }

    NS_LOG_WARN("  -> No route found to Node " << destNode->GetId() << " (IP: " << destAddr << ") in SP routing table.");
    sockerr = Socket::ERROR_NOROUTETOHOST;
    return nullptr;
}

} // namespace ns3
