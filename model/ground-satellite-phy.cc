#include "ground-satellite-phy.h"
#include "ground-satellite-channel.h"
#include "ground-satellite-net-device.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/core-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GroundSatellitePhy");

NS_OBJECT_ENSURE_REGISTERED(GroundSatellitePhy);

TypeId
GroundSatellitePhy::GetTypeId()
{
    static TypeId tid = TypeId("ns3::GroundSatellitePhy")
                            .SetParent<Object>()
                            .SetGroupName("Satellite")
                            .AddConstructor<GroundSatellitePhy>()
                            .AddAttribute("TxPower",
                                          "Transmission power in dBm.",
                                          DoubleValue(30.0),
                                          MakeDoubleAccessor(&GroundSatellitePhy::m_txPowerDbm),
                                          MakeDoubleChecker<double>())
                            .AddAttribute("DataRate",
                                          "The transmission data rate.",
                                          DataRateValue(DataRate("1Mbps")),
                                          MakeDataRateAccessor(&GroundSatellitePhy::m_dataRate),
                                          MakeDataRateChecker());
    return tid;
}

GroundSatellitePhy::GroundSatellitePhy()
{
    NS_LOG_FUNCTION(this);
}

GroundSatellitePhy::~GroundSatellitePhy()
{
    NS_LOG_FUNCTION(this);
}

void
GroundSatellitePhy::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
}

Ptr<NetDevice>
GroundSatellitePhy::GetDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_device;
}

void
GroundSatellitePhy::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

Ptr<Node>
GroundSatellitePhy::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

Ptr<MobilityModel>
GroundSatellitePhy::GetMobility() const
{
    NS_LOG_FUNCTION(this);
    return m_node->GetObject<MobilityModel>();
}

void
GroundSatellitePhy::StartTx(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    if (m_channel)
    {
        m_channel->Send(this, packet, m_txPowerDbm);
    }

    Time txTime = Seconds(static_cast<double>(packet->GetSize() * 8) / m_dataRate.GetBitRate());
    auto* dev = dynamic_cast<GroundSatelliteNetDevice*>(PeekPointer(m_device));
    Simulator::Schedule(txTime, &GroundSatelliteNetDevice::TxComplete, dev);
}

void
GroundSatellitePhy::StartRx(Ptr<const Packet> packet, double rxPowerDbm, const Address& senderAddress)
{
    NS_LOG_FUNCTION(this << packet << rxPowerDbm);
    // The GroundSatelliteNetDevice will log the reception upon successful filtering.
    if (m_device)
    {
        // Note: The downcast is safe because we control the creation process.
        // In a more general case, a dynamic_cast with a check would be better.
        auto* dev = dynamic_cast<GroundSatelliteNetDevice*>(PeekPointer(m_device));
        if (dev)
        {
            dev->Receive(packet->Copy(), senderAddress);
        }
    }
}

void
GroundSatellitePhy::SetChannel(Ptr<GroundSatelliteChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
}

void
GroundSatellitePhy::SetTxPower(double txPowerDbm)
{
    NS_LOG_FUNCTION(this << txPowerDbm);
    m_txPowerDbm = txPowerDbm;
}

} // namespace ns3 