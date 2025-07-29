#include "satellite-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/core-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "../model/satellite-circular-mobility-model.h"
#include "../model/ground-satellite-net-device.h"
#include "../model/ground-satellite-phy.h"
#include "../model/ground-satellite-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteHelper");

SatelliteHelper::SatelliteHelper()
    : m_satsPerPlane(0),
      m_altitude(0.0),
      m_inclination(0.0),
      m_raan(0.0),
      m_planeIndex(0)
{
    m_phyFactory.SetTypeId("ns3::GroundSatellitePhy");
    m_deviceFactory.SetTypeId("ns3::GroundSatelliteNetDevice");
    m_channelFactory.SetTypeId("ns3::GroundSatelliteChannel");
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
    m_phyFactory.SetTypeId("ns3::GroundSatellitePhy");
    m_deviceFactory.SetTypeId("ns3::GroundSatelliteNetDevice");
    m_channelFactory.SetTypeId("ns3::GroundSatelliteChannel");
}

NodeContainer
SatelliteHelper::CreateSatellites() const
{
    // NS_ASSERT_MSG(m_satsPerPlane > 2, "Satellites per plane must be greater than 2");
    
    NodeContainer satellites;
    satellites.Create(m_satsPerPlane);

    for (uint32_t i = 0; i < m_satsPerPlane; ++i)
    {
        Ptr<Node> node = satellites.Get(i);
        double initialAngle = i * (360.0 / m_satsPerPlane);

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
        
        Names::Add("Satellite-" + std::to_string(m_planeIndex) + "-" + std::to_string(i), node);
    }

    return satellites;
}

NodeContainer
SatelliteHelper::CreateGroundStation(double latitude, double longitude)
{
    NodeContainer groundStation;
    groundStation.Create(1);
    Ptr<Node> node = groundStation.Get(0);

    // Convert lat/lon to ECEF coordinates
    // Assuming a spherical Earth for simplicity for now.
    // A more accurate model would use the WGS84 model.
    const double earthRadius = 6371e3; // meters
    double latRad = latitude * M_PI / 180.0;
    double lonRad = longitude * M_PI / 180.0;
    double x = earthRadius * cos(latRad) * cos(lonRad);
    double y = earthRadius * cos(latRad) * sin(lonRad);
    double z = earthRadius * sin(latRad);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(node);
    Ptr<ConstantPositionMobilityModel> mobilityModel = node->GetObject<ConstantPositionMobilityModel>();
    if (mobilityModel)
    {
        mobilityModel->SetPosition(Vector(x, y, z));
    }

    Names::Add("GroundStation-" + std::to_string(latitude) + "," + std::to_string(longitude), node);

    return groundStation;
}

void
SatelliteHelper::SetPhyAttribute(std::string name, const AttributeValue& value)
{
    m_phyFactory.Set(name, value);
}

void
SatelliteHelper::SetDeviceAttribute(std::string name, const AttributeValue& value)
{
    m_deviceFactory.Set(name, value);
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
SatelliteHelper::Install(const NodeContainer& satellites, const NodeContainer& groundStations)
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