#include "satellite-net-device.h"
#include "satellite-channel.h"
#include "satellite-phy.h"

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SatelliteNetDevice");

// --- SatelliteNetDevice Implementation ---

NS_OBJECT_ENSURE_REGISTERED(SatelliteNetDevice);

TypeId
SatelliteNetDevice::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatelliteNetDevice")
            .SetParent<NetDevice>()
            .SetGroupName("Satellite")
            .AddConstructor<SatelliteNetDevice>();
    return tid;
}

SatelliteNetDevice::SatelliteNetDevice()
    : m_phy(nullptr),
      m_channel(nullptr),
      m_node(nullptr),
      m_ifIndex(0),
      m_mtu(1500),
      m_linkUp(true)
{
    NS_LOG_FUNCTION(this);
}

SatelliteNetDevice::~SatelliteNetDevice()
{
    NS_LOG_FUNCTION(this);
}

void
SatelliteNetDevice::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_phy = nullptr;
    m_channel = nullptr;
    m_node = nullptr;
    NetDevice::DoDispose();
}

void
SatelliteNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this << index);
    m_ifIndex = index;
}

uint32_t
SatelliteNetDevice::GetIfIndex() const
{
    NS_LOG_FUNCTION(this);
    return m_ifIndex;
}

Ptr<Channel>
SatelliteNetDevice::GetChannel() const
{
    NS_LOG_FUNCTION(this);
    return m_channel;
}

void
SatelliteNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = address;
}

Address
SatelliteNetDevice::GetAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_address;
}

bool
SatelliteNetDevice::SetMtu(const uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

uint16_t
SatelliteNetDevice::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

bool
SatelliteNetDevice::IsLinkUp() const
{
    NS_LOG_FUNCTION(this);
    return m_linkUp;
}

void
SatelliteNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_linkChangeCallback.ConnectWithoutContext(callback);
}

bool
SatelliteNetDevice::IsBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
SatelliteNetDevice::GetBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address::GetBroadcast();
}

bool
SatelliteNetDevice::IsMulticast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
SatelliteNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    return Mac48Address::GetMulticast(multicastGroup);
}

Address
SatelliteNetDevice::GetMulticast(Ipv6Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    return Mac48Address::GetMulticast(multicastGroup);
}

bool
SatelliteNetDevice::IsBridge() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
SatelliteNetDevice::IsPointToPoint() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
SatelliteNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);
    
    SatelliteMacHeader macHeader;
    macHeader.SetSource(m_address);
    macHeader.SetDestination(dest);
    macHeader.SetProtocol(protocolNumber);
    packet->AddHeader(macHeader);

    if (m_phy)
    {
        m_phy->StartTx(packet, dest);
        return true;
    }
    return false;
}

bool
SatelliteNetDevice::SendFrom(Ptr<Packet> packet,
                           const Address& source,
                           const Address& dest,
                           uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << source << dest << protocolNumber);
    // This method is for source-based routing, which we don't support yet.
    return false;
}

Ptr<Node>
SatelliteNetDevice::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

void
SatelliteNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

bool
SatelliteNetDevice::NeedsArp() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

void
SatelliteNetDevice::SetReceiveCallback(ReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_rxCallback = cb;
}

void
SatelliteNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_promiscRxCallback = cb;
}

bool
SatelliteNetDevice::SupportsSendFrom() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

void
SatelliteNetDevice::SetPhy(Ptr<SatellitePhy> phy)
{
    NS_LOG_FUNCTION(this << phy);
    m_phy = phy;
}

void
SatelliteNetDevice::SetChannel(Ptr<SatelliteChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
}

void
SatelliteNetDevice::Receive(Ptr<Packet> packet, const Address& sender)
{
    NS_LOG_FUNCTION(this << packet << sender);

    SatelliteMacHeader macHeader;
    packet->RemoveHeader(macHeader);
    
    Address destAddress = macHeader.GetDestination();
    Address sourceAddress = macHeader.GetSource(); // Get the logical source address

    Mac48Address destMac = Mac48Address::ConvertFrom(destAddress);
    Mac48Address myMac = Mac48Address::ConvertFrom(m_address);

    if (destMac == myMac || destMac.IsBroadcast())
    {
        NS_LOG_INFO("Packet received for " << myMac << " from " << sourceAddress);
        if (!m_rxCallback.IsNull())
        {
            m_rxCallback(this, packet, macHeader.GetProtocol(), sourceAddress);
        }
    }
}

} // namespace ns3 