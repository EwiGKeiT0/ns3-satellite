#include "point-to-point-energy-model-helper.h"
#include "ns3/node.h"

namespace ns3 {

PointToPointEnergyModelHelper::PointToPointEnergyModelHelper()
{
    m_factory.SetTypeId("ns3::PointToPointEnergyModel");
}

void PointToPointEnergyModelHelper::Set(std::string name, const AttributeValue &value)
{
    m_factory.Set(name, value);
}

void PointToPointEnergyModelHelper::Install(const NetDeviceContainer &devices, const energy::EnergySourceContainer &sources) const
{
    for (uint32_t i = 0; i < devices.GetN(); ++i)
    {
        Ptr<NetDevice> device = devices.Get(i);
        // Assuming the order of devices in NetDeviceContainer matches the order 
        // of nodes used to create the corresponding EnergySourceContainer.
        NS_ASSERT_MSG(i < sources.GetN(), "Mismatch between number of devices and energy sources.");
        Ptr<energy::EnergySource> source = sources.Get(i);
        
        Ptr<energy::DeviceEnergyModel> model = m_factory.Create<energy::DeviceEnergyModel>();
        model->SetEnergySource(source);
        source->AppendDeviceEnergyModel(model);
        device->AggregateObject(model);
    }
}

} // namespace ns3 