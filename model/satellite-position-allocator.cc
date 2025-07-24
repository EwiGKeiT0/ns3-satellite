#include "satellite-position-allocator.h"
#include "ns3/log.h"
#include "ns3/vector.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include <cmath>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SatellitePositionAllocator");

NS_OBJECT_ENSURE_REGISTERED (SatellitePositionAllocator);

TypeId
SatellitePositionAllocator::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatellitePositionAllocator")
        .SetParent<PositionAllocator> ()
        .SetGroupName ("Mobility")
        .AddConstructor<SatellitePositionAllocator> ()
        .AddAttribute ("Planes",
                    "Number of orbital planes in the constellation.",
                    UintegerValue (1),
                    MakeUintegerAccessor (&SatellitePositionAllocator::m_planes),
                    MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("SatsPerPlane",
                    "Number of satellites per orbital plane.",
                    UintegerValue (1),
                    MakeUintegerAccessor (&SatellitePositionAllocator::m_satsPerPlane),
                    MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("Altitude",
                    "Altitude of the satellites in meters.",
                    DoubleValue (700000.0),
                    MakeDoubleAccessor (&SatellitePositionAllocator::m_altitude),
                    MakeDoubleChecker<double> ())
        .AddAttribute ("Inclination",
                    "Inclination of the orbital planes in degrees.",
                    DoubleValue (53.0),
                    MakeDoubleAccessor (&SatellitePositionAllocator::m_inclination),
                    MakeDoubleChecker<double> ())
        ;
    return tid;
}

SatellitePositionAllocator::SatellitePositionAllocator ()
 : m_satIndex(0), m_planes(0), m_satsPerPlane(0), m_altitude(0.0), m_inclination(0.0)
{
    NS_LOG_FUNCTION (this);
}

SatellitePositionAllocator::~SatellitePositionAllocator ()
{
    NS_LOG_FUNCTION (this);
}

Vector
SatellitePositionAllocator::GetNext (void) const
{
    NS_LOG_FUNCTION (this);

    uint32_t currentPlane = m_satIndex / m_satsPerPlane;
    uint32_t satInPlane = m_satIndex % m_satsPerPlane;

    // Calculate RAAN for the plane
    double raanDegrees = (360.0 / m_planes) * currentPlane;
    
    // Calculate initial angle for the satellite in its orbit
    // The phasing factor ensures satellites in adjacent planes are offset
    double phasingFactor = (360.0 / (m_planes * m_satsPerPlane));
    double initialAngleDegrees = (360.0 / m_satsPerPlane) * satInPlane + phasingFactor * currentPlane;

    double radius = 6371000.0 + m_altitude;

    // Position in the 2D orbital plane (x'-y' plane) at t=0
    double initialAngleRad = initialAngleDegrees * M_PI / 180.0;
    double x_orbital = radius * std::cos(initialAngleRad);
    double y_orbital = radius * std::sin(initialAngleRad);
    Vector posOrbital(x_orbital, y_orbital, 0.0);

    // Transformation from orbital plane to ECI frame
    double inclinationRad = m_inclination * M_PI / 180.0;
    double raanRad = raanDegrees * M_PI / 180.0;
    
    double cos_i = std::cos(inclinationRad);
    double sin_i = std::sin(inclinationRad);
    double cos_raan = std::cos(raanRad);
    double sin_raan = std::sin(raanRad);

    // Apply rotations
    double x_inclined = posOrbital.x;
    double y_inclined = posOrbital.y * cos_i;
    double z_inclined = posOrbital.y * sin_i;
    
    double x_final = x_inclined * cos_raan - y_inclined * sin_raan;
    double y_final = x_inclined * sin_raan + y_inclined * cos_raan;
    double z_final = z_inclined;

    m_satIndex++;
    
    return Vector (x_final, y_final, z_final);
}

int64_t
SatellitePositionAllocator::AssignStreams (int64_t stream)
{
    NS_LOG_FUNCTION (this << stream);
    return 0;
}

} // namespace ns3 