#ifndef INTER_SATELLITE_LINK_HELPER_H
#define INTER_SATELLITE_LINK_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include <vector>

namespace ns3 {

/**
 * @brief Helper to create links between satellites, both intra-plane and inter-plane.
 *
 * This helper uses a custom DynamicDelayPointToPointChannel to simulate
 * changing propagation delays due to satellite movement.
 */
class InterSatelliteLinkHelper
{
public:
    InterSatelliteLinkHelper();

    /**
     * @brief Set an attribute on the PointToPointNetDevice type created by the helper.
     * @param name The name of the attribute to set.
     * @param value The value of the attribute.
     */
    void SetDeviceAttribute(std::string name, const AttributeValue &value);

    /**
     * @brief Set the type of queue to use for the devices created by this helper.
     * @tparam Ts Argument types
     * @param type The type of queue
     * @param args Name and AttributeValue pairs to set on the queue.
     */
    template <typename... Ts>
    void SetQueue(std::string type, Ts&&... args);

    /**
     * @brief Install links on the given satellite constellation.
     * @param orbitalPlanes A vector where each element is a NodeContainer representing an orbital plane.
     * @return A NetDeviceContainer with all the created devices.
     */
    NetDeviceContainer Install(const std::vector<NodeContainer>& orbitalPlanes);

private:
    ObjectFactory m_deviceFactory; //!< Factory to create PointToPointNetDevices.
    ObjectFactory m_queueFactory;  //!< Factory to create Queues.
};


/***************************************************************
 *  Implementation of the template declared above.
 ***************************************************************/
template <typename... Ts>
void
InterSatelliteLinkHelper::SetQueue(std::string type, Ts&&... args)
{
    QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

    m_queueFactory.SetTypeId(type);
    m_queueFactory.Set(std::forward<Ts>(args)...);
}


} // namespace ns3

#endif /* INTER_SATELLITE_LINK_HELPER_H */