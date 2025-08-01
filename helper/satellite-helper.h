#ifndef SATELLITE_HELPER_H
#define SATELLITE_HELPER_H

#include "ns3/node-container.h"
#include <vector>

namespace ns3
{

/**
 * @ingroup satellite
 * @brief A helper to create satellite and ground station nodes.
 */
class SatelliteHelper
{
public:
    SatelliteHelper();

    /**
     * @brief Create a satellite orbital plane.
     * @param satsPerPlane Number of satellites per plane.
     * @param altitude Orbital altitude in meters.
     * @param inclination Orbital inclination in degrees.
     * @param raan Right Ascension of the Ascending Node in degrees.
     * @return A NodeContainer holding all the created satellite nodes.
     */
    NodeContainer CreateOribitalPlane(uint32_t satsPerPlane,
                                   double altitude,
                                   double inclination,
                                   double raan);

    /**
     * @brief Create a satellite orbital shell.
     * @param altitude Orbital altitude in meters.
     * @param inclination Orbital inclination in degrees.
     * @param planes Number of orbital planes.
     * @param satsPerPlane Number of satellites per plane.
     * @return A vector of NodeContainers, each holding the created satellite nodes in one plane.
     */
    std::vector<NodeContainer> CreateShell(double altitude,
                                           double inclination,
                                           uint32_t planes,
                                           uint32_t satsPerPlane);

    /**
     * @brief Create a ground station node.
     * @param latitude Latitude in degrees.
     * @param longitude Longitude in degrees.
     * @return A NodeContainer holding the created ground station node.
     */
    NodeContainer CreateGroundStation(double latitude, double longitude);

private:
    uint32_t m_planeIndex;
};

} // namespace ns3

#endif /* SATELLITE_HELPER_H */ 