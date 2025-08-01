#ifndef SATELLITE_SP_ROUTING_PROTOCOL_H
#define SATELLITE_SP_ROUTING_PROTOCOL_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/timer.h"
#include <map>
#include <vector>

namespace ns3 {

class SatelliteSpRoutingProtocol : public Ipv4RoutingProtocol
{
public:
    static TypeId GetTypeId(void);
    SatelliteSpRoutingProtocol();
    virtual ~SatelliteSpRoutingProtocol();

    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) override;
    bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev, const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb, const LocalDeliverCallback& lcb, const ErrorCallback& ecb) override;
    void NotifyInterfaceUp(uint32_t interface) override;
    void NotifyInterfaceDown(uint32_t interface) override;
    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    
    // Static helpers for setup
    static void AddNode(Ptr<Node> node);
    static void InitializeTopology();
    static void AddIpToNodeMapping();
    static void AddIpToNodeMapping(Ipv4Address, Ptr<Node>);
    static void ClearIpToNodeMapping();
    static const std::map<Ipv4Address, Ptr<Node>>& GetIpToNodeMap();

    void SetIpv4(Ptr<Ipv4> ipv4) override;
    void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const override;
    void DoInitialize() override;
    void DoDispose() override;

private:
    struct RouteEntry {
        Ptr<Node> nextHopNode;
        uint32_t interface;
    };

    void Start();
    void UpdateRoutes();
    void ComputeRoutes(); 
    uint32_t GetInterfaceToPeer(Ptr<Node> peer) const;

    // Instance-specific members
    Ptr<Ipv4> m_ipv4;
    Timer m_updateTimer;
    Time m_updateInterval;
    // Routing table: maps DESTINATION node to the next hop information
    std::map<Ptr<Node>, RouteEntry> m_routingTable;

    // Static shared data
    static std::vector<std::vector<uint32_t>> m_adj;
    static std::map<Ptr<Node>, uint32_t> m_nodeToIndex;
    static NodeContainer m_allSatellites;
    static std::map<Ipv4Address, Ptr<Node>> m_ipToNodeMap;
};

} 

#endif /* SATELLITE_SP_ROUTING_PROTOCOL_H */
