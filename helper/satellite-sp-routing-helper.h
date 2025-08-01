#ifndef SATELLITE_SP_ROUTING_HELPER_H
#define SATELLITE_SP_ROUTING_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"

namespace ns3 {

/**
 * @brief Helper class to install SatelliteSpRoutingProtocol on nodes.
 */
class SatelliteSpRoutingHelper : public Ipv4RoutingHelper
{
public:
    SatelliteSpRoutingHelper();
    ~SatelliteSpRoutingHelper() override;

    /**
     * @brief Create a new SatelliteSpRoutingHelper.
     * @return a newly-created SatelliteSpRoutingHelper
     */
    SatelliteSpRoutingHelper* Copy() const override;

    /**
     * @brief Installs the satellite SP routing protocol on a node.
     * @param node The node to install the routing protocol on.
     * @return A Ptr to the created routing protocol.
     */
    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

    /**
     * @brief Populates the IP-to-Node mapping in the SatelliteSpRoutingProtocol.
     *
     * This static method should be called after IP addresses are assigned to all
     * satellite nodes.
     */
    static void PopulateIpToNodeMap();

private:
    // No member variables needed now
};

} // namespace ns3

#endif /* SATELLITE_SP_ROUTING_HELPER_H */
