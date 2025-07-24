#ifndef SATELLITE_HELPER_H
#define SATELLITE_HELPER_H

#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

namespace ns3 {

class SatelliteHelper
{
public:
    /**
     * @brief Default constructor for when not creating a constellation.
     */
    SatelliteHelper();

    /**
    * \\brief Construct a helper to create a satellite constellation.
    *
    * \\param planes Number of orbital planes.
    * \\param satsPerPlane Number of satellites per plane.
    * \\param altitude Orbital altitude in meters.
    * \\param inclination Orbital inclination in degrees.
    * \\param raan Right Ascension of the Ascending Node in degrees.
    */
    SatelliteHelper (uint32_t satsPerPlane, double altitude, double inclination, double raan, uint32_t planeIndex);
    
    /**
    * \\brief Create the satellite nodes and install mobility models.
    *
    * This method creates the nodes, installs the SatelliteMobilityModel
    * on each of them, and configures each model with the correct
    * orbital parameters for its position in the constellation.
    *
    * \\return A NodeContainer holding all the created satellite nodes.
    */
    NodeContainer CreateSatellites ();

    /**
     * @brief Install the satellite communication stack on the nodes.
     * @param c The NodeContainer to install the devices on.
     * @return A NetDeviceContainer with the created devices.
     */
    NetDeviceContainer Install(const NodeContainer& c);

    /**
     * @brief Set the propagation loss model for the satellite channel.
     * @param loss The propagation loss model.
     */
    void SetPropagationLossModel(const Ptr<PropagationLossModel> loss);

    /**
     * @brief Set the propagation delay model for the satellite channel.
     * @param delay The propagation delay model.
     */
    void SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay);

private:
    uint32_t m_satsPerPlane;
    double m_altitude;
    double m_inclination;
    double m_raan; // Right Ascension of the Ascending Node in degrees
    uint32_t m_planeIndex;

    ObjectFactory m_phyFactory;
    ObjectFactory m_deviceFactory;
    Ptr<PropagationLossModel> m_loss;
    Ptr<PropagationDelayModel> m_delay;
};

} // namespace ns3

#endif /* SATELLITE_HELPER_H */ 