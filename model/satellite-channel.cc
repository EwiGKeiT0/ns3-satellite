#include "satellite-channel.h"
#include "satellite-phy.h"

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

NS_LOG_COMPONENT_DEFINE("SatelliteChannel");

NS_OBJECT_ENSURE_REGISTERED(SatelliteChannel);

TypeId
SatelliteChannel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatelliteChannel")
            .SetParent<Channel>()
            .SetGroupName("Satellite")
            .AddConstructor<SatelliteChannel>()
            .AddAttribute("PropagationLossModel",
                          "A pointer to the propagation loss model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&SatelliteChannel::m_loss),
                          MakePointerChecker<PropagationLossModel>())
            .AddAttribute("PropagationDelayModel",
                          "A pointer to the propagation delay model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&SatelliteChannel::m_delay),
                          MakePointerChecker<PropagationDelayModel>());
    return tid;
}

SatelliteChannel::SatelliteChannel()
{
    NS_LOG_FUNCTION(this);
}

SatelliteChannel::~SatelliteChannel()
{
    NS_LOG_FUNCTION(this);
    m_phyList.clear();
}

void
SatelliteChannel::SetPropagationLossModel(const Ptr<PropagationLossModel> loss)
{
    NS_LOG_FUNCTION(this << loss);
    m_loss = loss;
}

void
SatelliteChannel::SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay)
{
    NS_LOG_FUNCTION(this << delay);
    m_delay = delay;
}

void
SatelliteChannel::Add(Ptr<SatellitePhy> phy)
{
    NS_LOG_FUNCTION(this << phy);
    m_phyList.push_back(phy);
}

std::size_t
SatelliteChannel::GetNDevices() const
{
    NS_LOG_FUNCTION(this);
    return m_phyList.size();
}

Ptr<NetDevice>
SatelliteChannel::GetDevice(std::size_t i) const
{
    NS_LOG_FUNCTION(this << i);
    // This is a bit of a cheat.  The channel model in ns-3 is really
    // a NetDevice-centric view of the world and we are really a
    // Phy-centric view.  We need to return a NetDevice, so we get
    // the Phy and ask it for its NetDevice.
    return m_phyList[i]->GetDevice();
}

void
SatelliteChannel::Send(Ptr<SatellitePhy> sender,
                       Ptr<const Packet> packet,
                       double txPowerDbm,
                       const Address& dest) const
{
    NS_LOG_FUNCTION(this << sender << packet << txPowerDbm << dest);
    for (PhyList::const_iterator i = m_phyList.begin(); i != m_phyList.end(); ++i)
    {
        Ptr<SatellitePhy> receiver = *i;
        if (receiver == sender)
        {
            continue;
        }

        // If the destination is not broadcast, only send to the intended receiver
        Mac48Address destMac = Mac48Address::ConvertFrom(dest);
        if (!destMac.IsBroadcast() && receiver->GetDevice()->GetAddress() != dest)
        {
            continue;
        }

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
            &SatellitePhy::StartRx,
            receiver,
            packet->Copy(),
            rxPowerDbm,
            sender->GetDevice()->GetAddress());
    }
}

int64_t
SatelliteChannel::AssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    // This channel does not use random variables, so we return the input stream number.
    return stream;
}

} // namespace ns3 