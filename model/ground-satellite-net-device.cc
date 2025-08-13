#include "ground-satellite-net-device.h"
#include "ground-satellite-channel.h"
#include "ground-satellite-phy.h"
#include "ground-satellite-mac-header.h"

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/llc-snap-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GroundSatelliteNetDevice");

// --- GroundSatelliteNetDevice Implementation ---

NS_OBJECT_ENSURE_REGISTERED(GroundSatelliteNetDevice);

TypeId
GroundSatelliteNetDevice::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GroundSatelliteNetDevice")
            .SetParent<NetDevice>()
            .SetGroupName("Satellite")
            .AddConstructor<GroundSatelliteNetDevice>()
            .AddAttribute("DataRate",
                          "The default data rate for this device.",
                          DataRateValue(DataRate("1Mbps")),
                          MakeDataRateAccessor(&GroundSatelliteNetDevice::m_dataRate),
                          MakeDataRateChecker())
            .AddTraceSource("MacTx",
                            "Trace source indicating a packet has been transmitted.",
                            MakeTraceSourceAccessor(&GroundSatelliteNetDevice::m_macTxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("MacRx",
                            "Trace source indicating a packet has been received.",
                            MakeTraceSourceAccessor(&GroundSatelliteNetDevice::m_macRxTrace),
                            "ns3::Packet::TracedCallback");
    return tid;
}

GroundSatelliteNetDevice::GroundSatelliteNetDevice()
    : m_phy(nullptr),
      m_channel(nullptr),
      m_node(nullptr),
      m_ifIndex(0),
      m_mtu(1500),
      m_linkUp(true),
      m_txMachineState(false)
{
    NS_LOG_FUNCTION(this);
}

GroundSatelliteNetDevice::~GroundSatelliteNetDevice()
{
    NS_LOG_FUNCTION(this);
}

void
GroundSatelliteNetDevice::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_phy = nullptr;
    m_channel = nullptr;
    m_node = nullptr;
    NetDevice::DoDispose();
}

void
GroundSatelliteNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this << index);
    m_ifIndex = index;
}

uint32_t
GroundSatelliteNetDevice::GetIfIndex() const
{
    NS_LOG_FUNCTION(this);
    return m_ifIndex;
}

Ptr<Channel>
GroundSatelliteNetDevice::GetChannel() const
{
    NS_LOG_FUNCTION(this);
    return m_channel;
}

void
GroundSatelliteNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = address;
}

Address
GroundSatelliteNetDevice::GetAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_address;
}

bool
GroundSatelliteNetDevice::SetMtu(const uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

uint16_t
GroundSatelliteNetDevice::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

bool
GroundSatelliteNetDevice::IsLinkUp() const
{
    NS_LOG_FUNCTION(this);
    return m_linkUp;
}

void
GroundSatelliteNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_linkChangeCallback.ConnectWithoutContext(callback);
}

bool
GroundSatelliteNetDevice::IsBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
GroundSatelliteNetDevice::GetBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address::GetBroadcast();
}

bool
GroundSatelliteNetDevice::IsMulticast() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

Address
GroundSatelliteNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    return Mac48Address::GetMulticast(multicastGroup);
}

Address
GroundSatelliteNetDevice::GetMulticast(Ipv6Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    return Mac48Address::GetMulticast(multicastGroup);
}

bool
GroundSatelliteNetDevice::IsBridge() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
GroundSatelliteNetDevice::IsPointToPoint() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

bool
GroundSatelliteNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);
    
    GroundSatelliteMacHeader macHeader;
    macHeader.SetSource(m_address);
    macHeader.SetProtocol(protocolNumber);
    packet->AddHeader(macHeader);

    if (m_queue->Enqueue(packet))
    {
        Simulator::ScheduleNow(&GroundSatelliteNetDevice::TxMachine, this);
        return true;
    }
    return false;
}

bool
GroundSatelliteNetDevice::SendFrom(Ptr<Packet> packet,
                                   const Address& source,
                                   const Address& dest,
                                   uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << source << dest << protocolNumber);
    // This method is for source-based routing, which we don't support yet.
    return false;
}

Ptr<Node>
GroundSatelliteNetDevice::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

void
GroundSatelliteNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

bool
GroundSatelliteNetDevice::NeedsArp() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

void
GroundSatelliteNetDevice::SetReceiveCallback(ReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_rxCallback = cb;
}

void
GroundSatelliteNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_promiscRxCallback = cb;
}

bool
GroundSatelliteNetDevice::SupportsSendFrom() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

void
GroundSatelliteNetDevice::SetPhy(Ptr<GroundSatellitePhy> phy)
{
    NS_LOG_FUNCTION(this << phy);
    m_phy = phy;
}

void
GroundSatelliteNetDevice::SetChannel(Ptr<GroundSatelliteChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
}

void
GroundSatelliteNetDevice::SetQueue(Ptr<Queue<Packet>> queue)
{
    NS_LOG_FUNCTION(this << queue);
    m_queue = queue;
}

void
GroundSatelliteNetDevice::TxMachine()
{
    NS_LOG_FUNCTION(this);
    if (m_txMachineState)
    {
        return;
    }

    Ptr<Packet> packet = m_queue->Dequeue();
    if (packet)
    {
        m_txMachineState = true;
        m_macTxTrace(packet);
        m_phy->StartTx(packet);
    }
}

void
GroundSatelliteNetDevice::TxComplete(void)
{
    NS_LOG_FUNCTION(this);
    m_txMachineState = false;
    TxMachine();
}

void
GroundSatelliteNetDevice::Receive(Ptr<Packet> packet, const Address& sender)
{
    NS_LOG_FUNCTION(this << packet << sender);

    GroundSatelliteMacHeader macHeader;
    packet->RemoveHeader(macHeader);
    m_macRxTrace(packet);

    if (!m_rxCallback.IsNull())
    {
        m_rxCallback(this, packet, macHeader.GetProtocol(), macHeader.GetSource());
    }
}

} // namespace ns3 