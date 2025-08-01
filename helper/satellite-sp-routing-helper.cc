#include "satellite-sp-routing-helper.h"
#include "../model/satellite-sp-routing-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteSpRoutingHelper");

SatelliteSpRoutingHelper::SatelliteSpRoutingHelper()
{
}

SatelliteSpRoutingHelper::~SatelliteSpRoutingHelper()
{
}

SatelliteSpRoutingHelper*
SatelliteSpRoutingHelper::Copy() const
{
    // No member variables to copy, just return a new instance.
    return new SatelliteSpRoutingHelper();
}

Ptr<Ipv4RoutingProtocol>
SatelliteSpRoutingHelper::Create(Ptr<Node> node) const
{
    Ptr<Ipv4RoutingProtocol> existingRouting = node->GetObject<Ipv4RoutingProtocol>();
    Ptr<Ipv4ListRouting> listRouting;

    if (existingRouting)
    {
        listRouting = DynamicCast<Ipv4ListRouting>(existingRouting);
        if (!listRouting)
        {
            NS_FATAL_ERROR("SatelliteSpRoutingHelper::Create(): A non-list routing protocol is "
                           "already installed. Cannot add SatelliteSpRoutingProtocol.");
            return nullptr; 
        }
    }
    else
    {
        listRouting = CreateObject<Ipv4ListRouting>();
        node->AggregateObject(listRouting);
    }

    SatelliteSpRoutingProtocol::AddNode(node);
    
    // The protocol no longer needs orbital planes passed during creation.
    // Topology is initialized statically via InitializeTopology().
    Ptr<SatelliteSpRoutingProtocol> srp = CreateObject<SatelliteSpRoutingProtocol>();
    listRouting->AddRoutingProtocol(srp, 0); // Priority 0 is high
    
    return listRouting;
}

void
SatelliteSpRoutingHelper::PopulateIpToNodeMap()
{
    SatelliteSpRoutingProtocol::ClearIpToNodeMapping();
    SatelliteSpRoutingProtocol::AddIpToNodeMapping();
}

} // namespace ns3
