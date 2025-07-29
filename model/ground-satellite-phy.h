#ifndef GROUND_SATELLITE_PHY_H
#define GROUND_SATELLITE_PHY_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/address.h"

namespace ns3
{

class Packet;
class NetDevice;
class MobilityModel;
class Node;
class GroundSatelliteChannel;

class GroundSatellitePhy : public Object
{
public:
    static TypeId GetTypeId();
    GroundSatellitePhy();
    ~GroundSatellitePhy() override;

    /**
     * @brief Set the NetDevice associated with this Phy.
     * @param device The NetDevice.
     */
    void SetDevice(Ptr<NetDevice> device);
    Ptr<NetDevice> GetDevice() const;

    /**
     * @brief Set the Node associated with this Phy.
     * @param node The Node.
     */
    void SetNode(Ptr<Node> node);
    Ptr<Node> GetNode() const;

    Ptr<MobilityModel> GetMobility() const;

    /**
     * @brief Starts the transmission of a packet.
     * @param packet The packet to transmit.
     */
    void StartTx(Ptr<Packet> packet);

    /**
     * @brief Called by the channel to indicate a packet has been received.
     * @param packet The received packet.
     * @param rxPowerDbm The received power in dBm.
     * @param senderAddress The address of the sender.
     */
    void StartRx(Ptr<const Packet> packet, double rxPowerDbm, const Address& senderAddress);

    /**
     * @brief Set the channel associated with this Phy.
     * @param channel The channel.
     */
    void SetChannel(Ptr<GroundSatelliteChannel> channel);

    /**
     * @brief Set the transmission power.
     * @param txPowerDbm The transmission power in dBm.
     */
    void SetTxPower(double txPowerDbm);

private:
    Ptr<NetDevice> m_device; //!< The associated NetDevice
    Ptr<Node> m_node;        //!< The associated Node
    Ptr<GroundSatelliteChannel> m_channel; //!< The associated channel
    double m_txPowerDbm; //!< Transmission power in dBm
};

} // namespace ns3

#endif /* GROUND_SATELLITE_PHY_H */ 