#include "satellite-phy.h"
#include "satellite-channel.h"
#include "satellite-net-device.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/core-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SatellitePhy");

NS_OBJECT_ENSURE_REGISTERED(SatellitePhy);

TypeId
SatellitePhy::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatellitePhy")
                            .SetParent<Object>()
                            .SetGroupName("Satellite")
                            .AddConstructor<SatellitePhy>()
                            .AddAttribute("TxPower",
                                          "Transmission power in dBm.",
                                          DoubleValue(30.0),
                                          MakeDoubleAccessor(&SatellitePhy::m_txPowerDbm),
                                          MakeDoubleChecker<double>());
    return tid;
}

SatellitePhy::SatellitePhy()
{
    NS_LOG_FUNCTION(this);
}

SatellitePhy::~SatellitePhy()
{
    NS_LOG_FUNCTION(this);
}

void
SatellitePhy::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
}

Ptr<NetDevice>
SatellitePhy::GetDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_device;
}

void
SatellitePhy::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

Ptr<Node>
SatellitePhy::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

Ptr<MobilityModel>
SatellitePhy::GetMobility() const
{
    NS_LOG_FUNCTION(this);
    return m_node->GetObject<MobilityModel>();
}

void
SatellitePhy::StartTx(Ptr<Packet> packet, const Address& dest)
{
    NS_LOG_FUNCTION(this << packet << dest);
    if (m_channel)
    {
        m_channel->Send(this, packet, m_txPowerDbm, dest);
    }
}

void
SatellitePhy::StartRx(Ptr<const Packet> packet, double rxPowerDbm, const Address& senderAddress)
{
    NS_LOG_FUNCTION(this << packet << rxPowerDbm);
    // The SatelliteNetDevice will log the reception upon successful filtering.
    if (m_device)
    {
        // Note: The downcast is safe because we control the creation process.
        // In a more general case, a dynamic_cast with a check would be better.
        SatelliteNetDevice* dev = dynamic_cast<SatelliteNetDevice*>(PeekPointer(m_device));
        if (dev)
        {
            dev->Receive(packet->Copy(), senderAddress);
        }
    }
}

void
SatellitePhy::SetChannel(Ptr<SatelliteChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
}

} // namespace ns3 