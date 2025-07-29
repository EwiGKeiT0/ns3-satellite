#ifndef GROUND_SATELLITE_CHANNEL_H
#define GROUND_SATELLITE_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/pointer.h"
#include "ns3/address.h"

namespace ns3
{

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class GroundSatellitePhy;
class Packet;
class Time;
class Address;

/**
 * @ingroup satellite
 * @brief A channel for ground-to-satellite communication.
 *
 * This class is designed to work with GroundSatellitePhy objects and supports
 * a PropagationLossModel and a PropagationDelayModel. These models must be
 * set by the user before using the channel.
 */
class GroundSatelliteChannel : public Channel
{
public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    GroundSatelliteChannel();
    ~GroundSatelliteChannel() override;

    // Inherited from Channel
    std::size_t GetNDevices() const override;
    Ptr<NetDevice> GetDevice(std::size_t i) const override;

    /**
     * @brief Add a GroundSatellitePhy object to this channel.
     * @param phy The GroundSatellitePhy to add.
     * @note A GroundSatelliteChannel can only have two GroundSatellitePhy devices attached.
     */
    void Add(Ptr<GroundSatellitePhy> phy);

    /**
     * @brief Set the propagation loss model for this channel.
     * @param loss The propagation loss model.
     */
    void SetPropagationLossModel(const Ptr<PropagationLossModel> loss);

    /**
     * @brief Set the propagation delay model for this channel.
     * @param delay The propagation delay model.
     */
    void SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay);

    /**
     * @brief Send a packet over the channel.
     * @param sender The sending PHY object.
     * @param packet The packet to send.
     * @param txPowerDbm The transmission power in dBm.
     *
     * This method is intended to be called from GroundSatellitePhy::StartTx.
     * The channel will deliver the packet to the other PHY object
     * connected to it.
     */
    void Send(Ptr<GroundSatellitePhy> sender, Ptr<const Packet> packet, double txPowerDbm) const;

    /**
     * @brief Assign a fixed random variable stream number to the random variables
     * used by this model.
     * @param stream The first stream index to use.
     * @return The number of stream indices assigned.
     */
    int64_t AssignStreams(int64_t stream);

private:
    /**
     * @brief A list of GroundSatellitePhy pointers.
     */
    using PhyList = std::vector<Ptr<GroundSatellitePhy>>;
    PhyList m_phyList; //!< List of PHY objects connected to the channel.

    Ptr<PropagationLossModel> m_loss;   //!< The propagation loss model.
    Ptr<PropagationDelayModel> m_delay; //!< The propagation delay model.
};

} // namespace ns3

#endif /* GROUND_SATELLITE_CHANNEL_H */ 