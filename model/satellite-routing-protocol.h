#ifndef SATELLITE_ROUTING_PROTOCOL_H
#define SATELLITE_ROUTING_PROTOCOL_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/timer.h"
#include <map>
#include <vector>

namespace ns3 {

/**
 * @brief A hybrid geographic routing protocol for LEO satellite networks.
 *
 * This protocol periodically updates a list of "active" neighbors based on a
 * hybrid strategy: mandatory intra-plane links plus opportunistic, distance-based
 * inter-plane links. Forwarding decisions are then made greedily among these
 * active neighbors.
 */
class SatelliteRoutingProtocol : public Ipv4RoutingProtocol
{
public:
    static TypeId GetTypeId(void);
    SatelliteRoutingProtocol();
    virtual ~SatelliteRoutingProtocol();

    // From Ipv4RoutingProtocol
    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) override;
    bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev, const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb, const LocalDeliverCallback& lcb, const ErrorCallback& ecb) override;
    void NotifyInterfaceUp(uint32_t interface) override;
    void NotifyInterfaceDown(uint32_t interface) override;
    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    // Public helpers
    static void AddIpToNodeMapping(Ipv4Address ip, Ptr<Node> node);
    static void AddIpToNodeMapping(const NodeContainer& allSatellites);
    static void ClearIpToNodeMapping();
    static const std::map<Ipv4Address, Ptr<Node>>& GetIpToNodeMap();

    void SetIpv4(Ptr<Ipv4> ipv4) override;
    void SetOrbitalPlanes(const std::vector<NodeContainer>& orbitalPlanes);
    void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const override;
    void DoInitialize() override;

private:
    struct NeighborInfo {
        Ptr<Node> neighborNode;
        Ptr<NetDevice> localDevice;
    };

    void Start();
    void UpdateActiveNeighbors();
    uint32_t GetInterfaceToPeer(Ptr<Node> peer) const;

    Ptr<Ipv4> m_ipv4;
    Timer m_updateTimer;
    Time m_updateInterval;
    uint32_t m_maxNeighbors;
    std::vector<NeighborInfo> m_activeNeighbors;

    // Static data, shared across all instances
    static std::map<Ipv4Address, Ptr<Node>> m_ipToNodeMap;
    // Non-static, each protocol instance needs the full topology
    std::vector<NodeContainer> m_orbitalPlanes; 
};

} // namespace ns3

#endif /* SATELLITE_ROUTING_PROTOCOL_H */ 