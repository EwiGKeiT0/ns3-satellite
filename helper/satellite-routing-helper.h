#ifndef SATELLITE_ROUTING_HELPER_H
#define SATELLITE_ROUTING_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include <vector>

namespace ns3 {

/**
 * @brief Helper class to install SatelliteRoutingProtocol on nodes.
 */
class SatelliteRoutingHelper : public Ipv4RoutingHelper
{
public:
    SatelliteRoutingHelper();
    ~SatelliteRoutingHelper() override;

    /**
     * @brief Create a new SatelliteRoutingHelper.
     * @return a newly-created SatelliteRoutingHelper
     */
    SatelliteRoutingHelper* Copy() const override;

    /**
     * @brief Installs the satellite routing protocol on a node.
     * @param node The node to install the routing protocol on.
     * @return A Ptr to the created routing protocol.
     */
    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

    /**
     * @brief Set the orbital topology for the routing protocol.
     * 
     * This is a custom method for this helper to pass topology data
     * to the routing protocol instances it creates.
     * 
     * @param orbitalPlanes The vector of NodeContainers representing the constellation.
     */
    void SetOrbitalPlanes(const std::vector<NodeContainer>& orbitalPlanes);

private:
    // This will hold the topology data to be passed to each protocol instance.
    std::vector<NodeContainer> m_orbitalPlanes;
};

} // namespace ns3

#endif /* SATELLITE_ROUTING_HELPER_H */ 