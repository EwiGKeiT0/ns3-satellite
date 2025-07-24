#ifndef SATELLITE_CIRCULAR_MOBILITY_MODEL_H
#define SATELLITE_CIRCULAR_MOBILITY_MODEL_H

#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

/**
 * \\brief A simplified mobility model for satellites in a circular orbit.
 *
 * This mobility model calculates the position of a satellite over time based on
 * a simple circular orbit defined by altitude, speed, and inclination.
 * It does not use RAAN or Argument of Perigee for simplification.
 */
class SatelliteCircularMobilityModel : public MobilityModel
{
public:
    static TypeId GetTypeId (void);

    SatelliteCircularMobilityModel ();
    virtual ~SatelliteCircularMobilityModel ();

private:
    // Implemented from MobilityModel
    virtual Vector DoGetPosition (void) const;
    virtual void DoSetPosition (const Vector &position);
    virtual Vector DoGetVelocity (void) const;
    virtual int64_t DoAssignStreams (int64_t stream);

    double m_altitude;            //!< Orbital altitude in meters
    double m_inclinationDegrees;  //!< Orbital inclination in degrees
    double m_raanDegrees;         //!< Right Ascension of the Ascending Node in degrees
    double m_initialAngleDegrees; //!< Initial angle in the orbit in degrees
};

} // namespace ns3

#endif /* SIMPLE_CIRCULAR_MOBILITY_MODEL_H */ 