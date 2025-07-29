#include "satellite-circular-mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SatelliteCircularMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (SatelliteCircularMobilityModel);

// Standard gravitational parameter for Earth in m^3/s^2
const double GM_EARTH = 3.986004418e14;

TypeId
SatelliteCircularMobilityModel::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatelliteCircularMobilityModel")
        .SetParent<MobilityModel> ()
        .SetGroupName ("Mobility")
        .AddConstructor<SatelliteCircularMobilityModel> ()
        .AddAttribute ("Altitude",
                       "The altitude of the satellite's orbit in meters.",
                       DoubleValue (550000.0),
                       MakeDoubleAccessor (&SatelliteCircularMobilityModel::m_altitude),
                       MakeDoubleChecker<double> ())
        .AddAttribute ("Inclination",
                       "The inclination of the orbit in degrees.",
                       DoubleValue (53.0),
                       MakeDoubleAccessor (&SatelliteCircularMobilityModel::m_inclinationDegrees),
                       MakeDoubleChecker<double> ())
        .AddAttribute ("Raan",
                       "The Right Ascension of the Ascending Node in degrees.",
                       DoubleValue (0.0),
                       MakeDoubleAccessor (&SatelliteCircularMobilityModel::m_raanDegrees),
                       MakeDoubleChecker<double> ())
        .AddAttribute ("InitialAngle",
                       "The initial angle of the satellite in its orbit in degrees.",
                       DoubleValue (0.0),
                       MakeDoubleAccessor (&SatelliteCircularMobilityModel::m_initialAngleDegrees),
                       MakeDoubleChecker<double> ());
    return tid;
}

SatelliteCircularMobilityModel::SatelliteCircularMobilityModel ()
  : m_altitude(0.0), m_inclinationDegrees(0.0), m_raanDegrees(0.0), m_initialAngleDegrees(0.0)
{
}

SatelliteCircularMobilityModel::~SatelliteCircularMobilityModel ()
{
}

Vector
SatelliteCircularMobilityModel::DoGetPosition (void) const
{
    double time = Simulator::Now ().GetSeconds ();
    double radius = 6371e3 + m_altitude; // Earth radius + altitude
    double speed = std::sqrt(GM_EARTH / radius);
    double angularVelocity = speed / radius;
    double initialAngleRad = m_initialAngleDegrees * M_PI / 180.0;
    
    double currentAngle = initialAngleRad + angularVelocity * time;

    // 1. Position in the 2D orbital plane (x'-y' plane)
    double x_orbital = radius * std::cos(currentAngle);
    double y_orbital = radius * std::sin(currentAngle);

    // 2. Apply rotations for inclination and RAAN
    double inclinationRad = m_inclinationDegrees * M_PI / 180.0;
    double raanRad = m_raanDegrees * M_PI / 180.0;

    double cos_i = std::cos(inclinationRad);
    double sin_i = std::sin(inclinationRad);
    double cos_raan = std::cos(raanRad);
    double sin_raan = std::sin(raanRad);
    
    // Simplified rotation (assuming Argument of Perigee is 0)
    double x_final = x_orbital * cos_raan - y_orbital * cos_i * sin_raan;
    double y_final = x_orbital * sin_raan + y_orbital * cos_i * cos_raan;
    double z_final = y_orbital * sin_i;
    
    return Vector (x_final, y_final, z_final);
}

void
SatelliteCircularMobilityModel::DoSetPosition (const Vector &position)
{
    // This model calculates position based on orbital parameters, so setting it directly is not supported.
    NotifyCourseChange ();
}

Vector
SatelliteCircularMobilityModel::DoGetVelocity (void) const
{
    double time = Simulator::Now ().GetSeconds ();
    double radius = 6371000.0 + m_altitude;
    double speed = std::sqrt(GM_EARTH / radius);
    double angularVelocity = speed / radius;
    double initialAngleRad = m_initialAngleDegrees * M_PI / 180.0;

    double currentAngle = initialAngleRad + angularVelocity * time;

    // 1. Velocity in the 2D orbital plane
    double vx_orbital = -speed * std::sin(currentAngle);
    double vy_orbital = speed * std::cos(currentAngle);

    // 2. Apply the same rotations as for position
    double inclinationRad = m_inclinationDegrees * M_PI / 180.0;
    double raanRad = m_raanDegrees * M_PI / 180.0;

    double cos_i = std::cos(inclinationRad);
    double sin_i = std::sin(inclinationRad);
    double cos_raan = std::cos(raanRad);
    double sin_raan = std::sin(raanRad);
    
    // Apply simplified rotations to the velocity vector
    double vx_final = vx_orbital * cos_raan - vy_orbital * cos_i * sin_raan;
    double vy_final = vx_orbital * sin_raan + vy_orbital * cos_i * cos_raan;
    double vz_final = vy_orbital * sin_i;
    
    return Vector (vx_final, vy_final, vz_final);
}

int64_t
SatelliteCircularMobilityModel::DoAssignStreams (int64_t stream)
{
    return 0;
}

} // namespace ns3 