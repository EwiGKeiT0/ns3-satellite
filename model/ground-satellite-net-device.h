#ifndef GROUND_SATELLITE_NET_DEVICE_H
#define GROUND_SATELLITE_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/traced-callback.h"
#include "ns3/queue.h"
#include "ns3/data-rate.h"

namespace ns3
{

class GroundSatellitePhy;
class GroundSatelliteChannel;
class Node;
class Packet;

/**
 * @ingroup satellite
 * @brief A ground-to-satellite network device.
 */
class GroundSatelliteNetDevice : public NetDevice
{
public:
    static TypeId GetTypeId();
    GroundSatelliteNetDevice();
    ~GroundSatelliteNetDevice() override;

    // Inherited from NetDevice
    void SetIfIndex(uint32_t index) override;
    uint32_t GetIfIndex() const override;
    Ptr<Channel> GetChannel() const override;
    void SetAddress(Address address) override;
    Address GetAddress() const override;
    bool SetMtu(uint16_t mtu) override;
    uint16_t GetMtu() const override;
    bool IsLinkUp() const override;
    void AddLinkChangeCallback(Callback<void> callback) override;
    bool IsBroadcast() const override;
    Address GetBroadcast() const override;
    bool IsMulticast() const override;
    Address GetMulticast(Ipv4Address multicastGroup) const override;
    Address GetMulticast(Ipv6Address multicastGroup) const override;
    bool IsBridge() const override;
    bool IsPointToPoint() const override;
    bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;
    bool SendFrom(Ptr<Packet> packet,
                  const Address& source,
                  const Address& dest,
                  uint16_t protocolNumber) override;
    Ptr<Node> GetNode() const override;
    void SetNode(Ptr<Node> node) override;
    bool NeedsArp() const override;
    void SetReceiveCallback(ReceiveCallback cb) override;
    void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
    bool SupportsSendFrom() const override;

    // Public methods specific to GroundSatelliteNetDevice
    void SetPhy(Ptr<GroundSatellitePhy> phy);
    void SetChannel(Ptr<GroundSatelliteChannel> channel);
    void Receive(Ptr<Packet> packet, const Address& sender);
    void SetQueue(Ptr<Queue<Packet>> queue);
    void TxMachine(void);
    void TxComplete(void);

    TracedCallback<Ptr<const Packet>> m_macTxTrace;
    TracedCallback<Ptr<const Packet>> m_macRxTrace;

private:
    void DoDispose() override;

    Ptr<GroundSatellitePhy> m_phy;
    Ptr<GroundSatelliteChannel> m_channel;
    Ptr<Node> m_node;
    uint32_t m_ifIndex;
    Address m_address;
    uint16_t m_mtu;
    bool m_linkUp;
    ReceiveCallback m_rxCallback;
    PromiscReceiveCallback m_promiscRxCallback;
    TracedCallback<> m_linkChangeCallback;
    Ptr<Queue<Packet>> m_queue;
    bool m_txMachineState; //!< True if the transmitter is busy.
    DataRate m_dataRate;   //!< The data rate of the device.
};

} // namespace ns3

#endif /* GROUND_SATELLITE_NET_DEVICE_H */ 