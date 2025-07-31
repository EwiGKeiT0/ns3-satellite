#ifndef SATELLITE_ENERGY_MODEL_HELPER_H
#define SATELLITE_ENERGY_MODEL_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/energy-module.h"

namespace ns3 {

class SatelliteEnergyModelHelper
{
public:
  SatelliteEnergyModelHelper();
  void Set(std::string name, const AttributeValue &value);
  void Install(const NetDeviceContainer &devices, const energy::EnergySourceContainer &sources) const;

private:
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* SATELLITE_ENERGY_MODEL_HELPER_H */