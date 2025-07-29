#include "ground-satellite-channel.h"
#include "ground-satellite-phy.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GroundSatelliteChannel");

NS_OBJECT_ENSURE_REGISTERED(GroundSatelliteChannel);

TypeId
GroundSatelliteChannel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GroundSatelliteChannel")
            .SetParent<Channel>()
            .SetGroupName("Satellite")
            .AddConstructor<GroundSatelliteChannel>()
            .AddAttribute("PropagationLossModel",
                          "A pointer to the propagation loss model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&GroundSatelliteChannel::m_loss),
                          MakePointerChecker<PropagationLossModel>())
            .AddAttribute("PropagationDelayModel",
                          "A pointer to the propagation delay model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&GroundSatelliteChannel::m_delay),
                          MakePointerChecker<PropagationDelayModel>());
    return tid;
}

GroundSatelliteChannel::GroundSatelliteChannel()
{
    NS_LOG_FUNCTION(this);
}

GroundSatelliteChannel::~GroundSatelliteChannel()
{
    NS_LOG_FUNCTION(this);
    m_phyList.clear();
}

void
GroundSatelliteChannel::SetPropagationLossModel(const Ptr<PropagationLossModel> loss)
{
    NS_LOG_FUNCTION(this << loss);
    m_loss = loss;
}

void
GroundSatelliteChannel::SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay)
{
    NS_LOG_FUNCTION(this << delay);
    m_delay = delay;
}

void
GroundSatelliteChannel::Add(Ptr<GroundSatellitePhy> phy)
{
    NS_LOG_FUNCTION(this << phy);
    NS_ASSERT_MSG(m_phyList.size() < 2, "GroundSatelliteChannel can only have two PHY devices.");
    m_phyList.push_back(phy);
}

std::size_t
GroundSatelliteChannel::GetNDevices() const
{
    NS_LOG_FUNCTION(this);
    return m_phyList.size();
}

Ptr<NetDevice>
GroundSatelliteChannel::GetDevice(std::size_t i) const
{
    NS_LOG_FUNCTION(this << i);
    // This is a bit of a cheat.  The channel model in ns-3 is really
    // a NetDevice-centric view of the world and we are really a
    // Phy-centric view.  We need to return a NetDevice, so we get
    // the Phy and ask it for its NetDevice.
    return m_phyList[i]->GetDevice();
}

void
GroundSatelliteChannel::Send(Ptr<GroundSatellitePhy> sender,
                             Ptr<const Packet> packet,
                             double txPowerDbm) const
{
    NS_LOG_FUNCTION(this << sender << packet << txPowerDbm);
    NS_ASSERT_MSG(m_phyList.size() == 2, "GroundSatelliteChannel should have exactly two PHY devices for P2P communication.");

    // Find the receiver PHY
    Ptr<GroundSatellitePhy> receiver = (m_phyList[0] == sender) ? m_phyList[1] : m_phyList[0];

    double rxPowerDbm = txPowerDbm;
    if (m_loss)
    {
        rxPowerDbm -= m_loss->CalcRxPower(txPowerDbm,
                                          sender->GetMobility(),
                                          receiver->GetMobility());
    }

    Time delay = Seconds(0);
    if (m_delay)
    {
        delay = m_delay->GetDelay(sender->GetMobility(), receiver->GetMobility());
    }

    Simulator::ScheduleWithContext(
        receiver->GetNode()->GetId(),
        delay,
        &GroundSatellitePhy::StartRx,
        receiver,
        packet->Copy(),
        rxPowerDbm,
        sender->GetDevice()->GetAddress());
}

int64_t
GroundSatelliteChannel::AssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    // This channel does not use random variables, so we return the input stream number.
    return stream;
}

} // namespace ns3 