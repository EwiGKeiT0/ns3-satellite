#ifndef SATELLITE_NET_DEVICE_H
#define SATELLITE_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/traced-callback.h"
#include "satellite-mac-header.h"

namespace ns3
{

class SatellitePhy;
class SatelliteChannel;
class Node;
class Packet;

/**
* @ingroup satellite
* @brief A satellite network device.
*/
class SatelliteNetDevice : public NetDevice
{
public:
    static TypeId GetTypeId();
    SatelliteNetDevice();
    ~SatelliteNetDevice() override;

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

    // Public methods specific to SatelliteNetDevice
    void SetPhy(Ptr<SatellitePhy> phy);
    void SetChannel(Ptr<SatelliteChannel> channel);
    void Receive(Ptr<Packet> packet, const Address& sender);

private:
    void DoDispose() override;

    Ptr<SatellitePhy> m_phy;
    Ptr<SatelliteChannel> m_channel;
    Ptr<Node> m_node;
    uint32_t m_ifIndex;
    Address m_address;
    uint16_t m_mtu;
    bool m_linkUp;
    ReceiveCallback m_rxCallback;
    PromiscReceiveCallback m_promiscRxCallback;
    TracedCallback<> m_linkChangeCallback;
};

} // namespace ns3

#endif /* SATELLITE_NET_DEVICE_H */ 