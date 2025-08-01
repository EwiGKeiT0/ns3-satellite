#include "satellite-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/names.h"
#include "ns3/core-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "../model/satellite-circular-mobility-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SatelliteHelper");

SatelliteHelper::SatelliteHelper()
{
}

NodeContainer
SatelliteHelper::CreateOribitalPlane(uint32_t satsPerPlane,
                                  double altitude,
                                  double inclination,
                                  double raan)
{
    NS_ASSERT_MSG(satsPerPlane > 2, "Number of satellites per plane must be greater than 2.");
    
    NodeContainer satellites;
    satellites.Create(satsPerPlane);

    for (uint32_t i = 0; i < satsPerPlane; ++i)
    {
        Ptr<Node> node = satellites.Get(i);
        double initialAngle = i * (360.0 / satsPerPlane);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::SatelliteCircularMobilityModel");
        mobility.Install(node);
        Ptr<SatelliteCircularMobilityModel> satMobility =
            node->GetObject<SatelliteCircularMobilityModel>();
        if (satMobility)
        {
            satMobility->SetAttribute("Altitude", DoubleValue(altitude));
            satMobility->SetAttribute("Inclination", DoubleValue(inclination));
            satMobility->SetAttribute("Raan", DoubleValue(raan));
            satMobility->SetAttribute("InitialAngle", DoubleValue(initialAngle));
        }

        Names::Add("Satellite-" + std::to_string(m_planeIndex) + "-" + std::to_string(i), node);
    }

    ++m_planeIndex;

    return satellites;
}

std::vector<NodeContainer>
SatelliteHelper::CreateShell(double altitude,
                             double inclination,
                             uint32_t planes,
                             uint32_t satsPerPlane)
{
    std::vector<NodeContainer> shell;
    for (uint32_t i = 0; i < planes; ++i)
    {
        double raan = i * (360.0 / planes);
        NodeContainer plane = CreateOribitalPlane(satsPerPlane, altitude, inclination, raan);
        shell.push_back(plane);
    }
    return shell;
}

NodeContainer
SatelliteHelper::CreateGroundStation(double latitude, double longitude)
{
    NodeContainer groundStation;
    groundStation.Create(1);
    Ptr<Node> node = groundStation.Get(0);

    // Convert lat/lon to ECEF coordinates
    const double earthRadius = 6371e3; // meters
    double latRad = latitude * M_PI / 180.0;
    double lonRad = longitude * M_PI / 180.0;
    double x = earthRadius * cos(latRad) * cos(lonRad);
    double y = earthRadius * cos(latRad) * sin(lonRad);
    double z = earthRadius * sin(latRad);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(node);
    Ptr<ConstantPositionMobilityModel> mobilityModel =
        node->GetObject<ConstantPositionMobilityModel>();
    if (mobilityModel)
    {
        mobilityModel->SetPosition(Vector(x, y, z));
    }

    Names::Add("GroundStation-" + std::to_string(latitude) + "," + std::to_string(longitude), node);

    return groundStation;
}

} // namespace ns3 