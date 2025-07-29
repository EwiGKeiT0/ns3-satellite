#ifndef GROUND_SATELLITE_LINK_HELPER_H
#define GROUND_SATELLITE_LINK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/queue.h"

namespace ns3
{

/**
 * @ingroup satellite
 * @brief A helper to create and configure ground-to-satellite links.
 */
class GroundSatelliteLinkHelper
{
public:
    GroundSatelliteLinkHelper();

    /**
     * @brief Set an attribute on the underlying Phy.
     * @param name The name of the attribute to set.
     * @param value The value of the attribute.
     */
    void SetPhyAttribute(std::string name, const AttributeValue& value);

    /**
     * @brief Set an attribute on the underlying NetDevice.
     * @param name The name of the attribute to set.
     * @param value The value of the attribute.
     */
    void SetDeviceAttribute(std::string name, const AttributeValue& value);

    /**
     * @brief Set the type of queue to use for the devices created by this helper.
     * @tparam Ts Argument types
     * @param type The type of queue
     * @param args Name and AttributeValue pairs to set on the queue.
     */
    template <typename... Ts>
    void SetQueue(std::string type, Ts&&... args);

    /**
     * @brief Set the propagation loss model for the channels.
     * @param loss The propagation loss model.
     */
    void SetPropagationLossModel(Ptr<PropagationLossModel> loss);

    /**
     * @brief Set the propagation delay model for the channels.
     * @param delay The propagation delay model.
     */
    void SetPropagationDelayModel(Ptr<PropagationDelayModel> delay);

    /**
     * @brief Install the ground-to-satellite communication stack between
     *        a set of satellites and a set of ground stations.
     * @param satellites The container of satellite nodes.
     * @param groundStations The container of ground station nodes.
     * @return A NetDeviceContainer with all the created devices.
     */
    NetDeviceContainer Install(const NodeContainer& satellites, const NodeContainer& groundStations) const;

private:
    ObjectFactory m_phyFactory;
    ObjectFactory m_deviceFactory;
    ObjectFactory m_channelFactory;
    ObjectFactory m_queueFactory;
    Ptr<PropagationLossModel> m_loss;
    Ptr<PropagationDelayModel> m_delay;
};

} // namespace ns3

#endif /* GROUND_SATELLITE_LINK_HELPER_H */ 