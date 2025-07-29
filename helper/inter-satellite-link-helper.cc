#include "inter-satellite-link-helper.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/energy-module.h"
#include "../model/inter-satellite-link-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("InterSatelliteLinkHelper");

InterSatelliteLinkHelper::InterSatelliteLinkHelper()
{
    // Configure the attributes for the devices we will create.
    m_deviceFactory.SetTypeId("ns3::PointToPointNetDevice");
    SetDeviceAttribute("DataRate", StringValue("100Gbps"));

    // Set a default queue type
    SetQueue("ns3::DropTailQueue");
}

void InterSatelliteLinkHelper::SetDeviceAttribute(std::string name, const AttributeValue &value)
{
    m_deviceFactory.Set(name, value);
}

template <typename... Ts>
void
InterSatelliteLinkHelper::SetQueue(std::string type, Ts&&... args)
{
    QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

    m_queueFactory.SetTypeId(type);
    m_queueFactory.Set(std::forward<Ts>(args)...);
}

NetDeviceContainer
InterSatelliteLinkHelper::Install(const std::vector<NodeContainer>& orbitalPlanes)
{
    NS_LOG_FUNCTION(this);
    NetDeviceContainer allDevices;

    auto createLink = [this](Ptr<Node> nodeA, Ptr<Node> nodeB) -> NetDeviceContainer {
        // 1. Create our custom channel using an ObjectFactory
        ObjectFactory factory;
        factory.SetTypeId("ns3::InterSatelliteLinkChannel");
        factory.Set("NodeA", PointerValue(nodeA));
        factory.Set("NodeB", PointerValue(nodeB));
        Ptr<InterSatelliteLinkChannel> channel = factory.Create<InterSatelliteLinkChannel>();
        
        // 2. Create the two NetDevices
        NetDeviceContainer devices;
        devices.Add(m_deviceFactory.Create<NetDevice>());
        devices.Add(m_deviceFactory.Create<NetDevice>());

        // 3. Attach devices to nodes and channel
        Ptr<PointToPointNetDevice> devA = DynamicCast<PointToPointNetDevice>(devices.Get(0));
        Ptr<PointToPointNetDevice> devB = DynamicCast<PointToPointNetDevice>(devices.Get(1));
        
        // 4. Install queues on the devices
        devA->SetQueue(m_queueFactory.Create<Queue<Packet>>());
        devB->SetQueue(m_queueFactory.Create<Queue<Packet>>());

        nodeA->AddDevice(devA);
        nodeB->AddDevice(devB);
        
        devA->Attach(channel);
        devB->Attach(channel);
        
        return devices;
    };

    // 1. Create intra-plane (ring) links
    for (const auto& plane : orbitalPlanes)
    {
        for (uint32_t i = 0; i < plane.GetN(); ++i)
        {
            Ptr<Node> nodeA = plane.Get(i);
            Ptr<Node> nodeB = plane.Get((i + 1) % plane.GetN());
            allDevices.Add(createLink(nodeA, nodeB));
        }
    }

    // 2. Create all possible inter-plane links
    for (size_t i = 0; i < orbitalPlanes.size(); ++i)
    {
        for (size_t j = i + 1; j < orbitalPlanes.size(); ++j)
        {
            const auto& planeA = orbitalPlanes[i];
            const auto& planeB = orbitalPlanes[j];
            for (uint32_t n_a = 0; n_a < planeA.GetN(); ++n_a)
            {
                for (uint32_t n_b = 0; n_b < planeB.GetN(); ++n_b)
                {
                    allDevices.Add(createLink(planeA.Get(n_a), planeB.Get(n_b)));
                }
            }
        }
    }

    // BasicEnergySourceHelper basicEnergySourceHelper;
    // basicEnergySourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(100));
    // for (const auto &plane : orbitalPlanes)
    //     basicEnergySourceHelper.Install(plane);

    return allDevices;
}

} // namespace ns3 