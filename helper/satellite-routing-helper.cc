#include "satellite-routing-helper.h"
#include "../model/satellite-routing-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteRoutingHelper");

SatelliteRoutingHelper::SatelliteRoutingHelper()
{
}

SatelliteRoutingHelper::~SatelliteRoutingHelper()
{
}

SatelliteRoutingHelper*
SatelliteRoutingHelper::Copy() const
{
    auto* copy = new SatelliteRoutingHelper();
    copy->m_orbitalPlanes = this->m_orbitalPlanes;
    return copy;
}

Ptr<Ipv4RoutingProtocol>
SatelliteRoutingHelper::Create(Ptr<Node> node) const
{
    // Check if a routing protocol has already been aggregated
    Ptr<Ipv4RoutingProtocol> existingRouting = node->GetObject<Ipv4RoutingProtocol>();
    if (existingRouting)
    {
        // If it's a list router, we can add our protocol to it.
        Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(existingRouting);
        if (listRouting)
        {
            Ptr<SatelliteRoutingProtocol> srp = CreateObject<SatelliteRoutingProtocol>();
            srp->SetOrbitalPlanes(m_orbitalPlanes);
            listRouting->AddRoutingProtocol(srp, 0); // Add with high priority
            return existingRouting; // Return the existing list router
        }
        else
        {
            // A different, non-list routing protocol is already installed.
            // We cannot add our protocol. This is an error in the script setup.
            NS_FATAL_ERROR("SatelliteRoutingHelper::Create(): A non-list routing protocol is already installed on the node. Cannot add SatelliteRoutingProtocol.");
            return nullptr; // Unreachable
        }
    }
    else // No routing protocol exists yet, create a list router and add our protocol
    {
        Ptr<Ipv4ListRouting> listRouting = CreateObject<Ipv4ListRouting>();
        node->AggregateObject(listRouting);
        
        Ptr<SatelliteRoutingProtocol> srp = CreateObject<SatelliteRoutingProtocol>();
        srp->SetOrbitalPlanes(m_orbitalPlanes);
        listRouting->AddRoutingProtocol(srp, 0);
        
        return listRouting;
    }
}

void
SatelliteRoutingHelper::SetOrbitalPlanes(const std::vector<NodeContainer>& orbitalPlanes)
{
    m_orbitalPlanes = std::make_shared<const std::vector<NodeContainer>>(orbitalPlanes);
}

} // namespace ns3 