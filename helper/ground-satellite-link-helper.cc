#include "ground-satellite-link-helper.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "../model/ground-satellite-net-device.h"
#include "../model/ground-satellite-phy.h"
#include "../model/ground-satellite-channel.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GroundSatelliteLinkHelper");

GroundSatelliteLinkHelper::GroundSatelliteLinkHelper()
{
    m_phyFactory.SetTypeId("ns3::GroundSatellitePhy");
    m_deviceFactory.SetTypeId("ns3::GroundSatelliteNetDevice");
    m_channelFactory.SetTypeId("ns3::GroundSatelliteChannel");

    // Set a default queue type
    SetQueue("ns3::DropTailQueue");
}

void
GroundSatelliteLinkHelper::SetPhyAttribute(std::string name, const AttributeValue& value)
{
    m_phyFactory.Set(name, value);
}

void
GroundSatelliteLinkHelper::SetDeviceAttribute(std::string name, const AttributeValue& value)
{
    m_deviceFactory.Set(name, value);
}

template <typename... Ts>
void
GroundSatelliteLinkHelper::SetQueue(std::string type, Ts&&... args)
{
    QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

    m_queueFactory.SetTypeId(type);
    m_queueFactory.Set(std::forward<Ts>(args)...);
}

void
GroundSatelliteLinkHelper::SetPropagationLossModel(Ptr<PropagationLossModel> loss)
{
    m_loss = loss;
}

void
GroundSatelliteLinkHelper::SetPropagationDelayModel(Ptr<PropagationDelayModel> delay)
{
    m_delay = delay;
}

NetDeviceContainer
GroundSatelliteLinkHelper::Install(const NodeContainer& satellites, const NodeContainer& groundStations) const
{
    NetDeviceContainer allDevices;

    for (auto gsit = groundStations.Begin(); gsit != groundStations.End(); ++gsit)
    {
        Ptr<Node> groundStationNode = *gsit;

        for (auto satit = satellites.Begin(); satit != satellites.End(); ++satit)
        {
            Ptr<Node> satelliteNode = *satit;

            // Create a new P2P link for each satellite-ground station pair
            NetDeviceContainer devices;
            Ptr<GroundSatelliteChannel> channel = m_channelFactory.Create<GroundSatelliteChannel>();

            if (m_loss)
            {
                channel->SetPropagationLossModel(m_loss);
            }
            if (m_delay)
            {
                channel->SetPropagationDelayModel(m_delay);
            }

            // Ground Station side
            Ptr<GroundSatelliteNetDevice> gsDevice = m_deviceFactory.Create<GroundSatelliteNetDevice>();
            gsDevice->SetAddress(Mac48Address::Allocate());
            gsDevice->SetQueue(m_queueFactory.Create<Queue<Packet>>());
            groundStationNode->AddDevice(gsDevice);
            Ptr<GroundSatellitePhy> gsPhy = m_phyFactory.Create<GroundSatellitePhy>();
            gsPhy->SetDevice(gsDevice);
            gsPhy->SetNode(groundStationNode);
            gsPhy->SetChannel(channel);
            gsDevice->SetPhy(gsPhy);
            gsDevice->SetChannel(channel);
            channel->Add(gsPhy);
            devices.Add(gsDevice);

            // Satellite side
            Ptr<GroundSatelliteNetDevice> satDevice = m_deviceFactory.Create<GroundSatelliteNetDevice>();
            satDevice->SetAddress(Mac48Address::Allocate());
            satDevice->SetQueue(m_queueFactory.Create<Queue<Packet>>());
            satelliteNode->AddDevice(satDevice);
            Ptr<GroundSatellitePhy> satPhy = m_phyFactory.Create<GroundSatellitePhy>();
            satPhy->SetDevice(satDevice);
            satPhy->SetNode(satelliteNode);
            satPhy->SetChannel(channel);
            satDevice->SetPhy(satPhy);
            satDevice->SetChannel(channel);
            channel->Add(satPhy);
            devices.Add(satDevice);
            
            allDevices.Add(devices);
        }
    }

    return allDevices;
}

} // namespace ns3 