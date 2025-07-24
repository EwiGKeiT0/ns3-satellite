#ifndef SATELLITE_POSITION_ALLOCATOR_H
#define SATELLITE_POSITION_ALLOCATOR_H

#include "ns3/position-allocator.h"

namespace ns3 {

class SatellitePositionAllocator : public PositionAllocator
{
public:
    static TypeId GetTypeId (void);

    SatellitePositionAllocator ();
    ~SatellitePositionAllocator () override;

    Vector GetNext (void) const override;
    int64_t AssignStreams (int64_t stream) override;

private:
    mutable uint32_t m_satIndex; //!< Index of the next satellite to allocate
    uint32_t m_planes; //!< Number of orbital planes
    uint32_t m_satsPerPlane; //!< Number of satellites per plane
    double m_altitude; //!< Altitude in meters
    double m_inclination; //!< Inclination in degrees
};

} // namespace ns3

#endif /* SATELLITE_POSITION_ALLOCATOR_H */ 