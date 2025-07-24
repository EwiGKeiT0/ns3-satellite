#include "satellite-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/core-module.h"
#include "../model/satellite-circular-mobility-model.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteHelper");

SatelliteHelper::SatelliteHelper()
    : m_satsPerPlane(0),
      m_altitude(0.0),
      m_inclination(0.0),
      m_raan(0.0),
      m_planeIndex(0)
{
    m_phyFactory.SetTypeId("ns3::SatellitePhy");
    m_deviceFactory.SetTypeId("ns3::SatelliteNetDevice");
}

SatelliteHelper::SatelliteHelper(uint32_t satsPerPlane,
                                 double altitude,
                                 double inclination,
                                 double raan,
                                 uint32_t planeIndex)
    : m_satsPerPlane(satsPerPlane),
      m_altitude(altitude),
      m_inclination(inclination),
      m_raan(raan),
      m_planeIndex(planeIndex)
{
    m_phyFactory.SetTypeId("ns3::SatellitePhy");
    m_deviceFactory.SetTypeId("ns3::SatelliteNetDevice");
}

NodeContainer
SatelliteHelper::CreateSatellites()
{
    NodeContainer satellites;
    satellites.Create(m_satsPerPlane);

    for (uint32_t j = 0; j < m_satsPerPlane; ++j)
    {
        Ptr<Node> node = satellites.Get(j);
        double initialAngle = j * (360.0 / m_satsPerPlane);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::SatelliteCircularMobilityModel");
        mobility.Install(node);
        Ptr<SatelliteCircularMobilityModel> satMobility = node->GetObject<SatelliteCircularMobilityModel>();
        if (satMobility)
        {
            satMobility->SetAttribute("Altitude", DoubleValue(m_altitude));
            satMobility->SetAttribute("Inclination", DoubleValue(m_inclination));
            satMobility->SetAttribute("Raan", DoubleValue(m_raan));
            satMobility->SetAttribute("InitialAngle", DoubleValue(initialAngle));
        }
        
        Names::Add("Satellite-" + std::to_string(m_planeIndex) + "-" + std::to_string(j), node);
    }

    return satellites;
}

void
SatelliteHelper::SetPropagationLossModel(const Ptr<PropagationLossModel> loss)
{
    m_loss = loss;
}

void
SatelliteHelper::SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay)
{
    m_delay = delay;
}

NetDeviceContainer
SatelliteHelper::Install(const NodeContainer& c)
{
    NetDeviceContainer devices;
    Ptr<SatelliteChannel> channel = CreateObject<SatelliteChannel>();

    if (m_loss)
    {
        channel->SetPropagationLossModel(m_loss);
    }
    if (m_delay)
    {
        channel->SetPropagationDelayModel(m_delay);
    }

    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<SatelliteNetDevice> device = m_deviceFactory.Create<SatelliteNetDevice>();
        Ptr<SatellitePhy> phy = m_phyFactory.Create<SatellitePhy>();

        device->SetAddress(Mac48Address::Allocate());
        node->AddDevice(device);

        phy->SetDevice(device);
        phy->SetNode(node);
        phy->SetChannel(channel);

        device->SetPhy(phy);
        device->SetChannel(channel);
        
        channel->Add(phy);

        devices.Add(device);
    }

    return devices;
}

} // namespace ns3 