#include "dynamic-delay-p2p-channel.h"
#include "ns3/attribute.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/double.h" 
#include "ns3/simulator.h"

namespace {
    // Speed of light in vacuum, in m/s
    constexpr double C = 299792458.0; 
}

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DynamicDelayPointToPointChannel");

NS_OBJECT_ENSURE_REGISTERED(DynamicDelayPointToPointChannel);

TypeId
DynamicDelayPointToPointChannel::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::DynamicDelayPointToPointChannel")
        .SetParent<PointToPointChannel>()
        .SetGroupName("Satellite")
        .AddConstructor<DynamicDelayPointToPointChannel>()
        .AddAttribute("NodeA", "The first node connected to this channel.",
                      PointerValue(),
                      MakePointerAccessor(&DynamicDelayPointToPointChannel::m_nodeA),
                      MakePointerChecker<Node>())
        .AddAttribute("NodeB", "The second node connected to this channel.",
                      PointerValue(),
                      MakePointerAccessor(&DynamicDelayPointToPointChannel::m_nodeB),
                      MakePointerChecker<Node>());
    return tid;
}

DynamicDelayPointToPointChannel::DynamicDelayPointToPointChannel() 
    : m_nodeA(nullptr), m_nodeB(nullptr)
{
    NS_LOG_FUNCTION(this);
}

DynamicDelayPointToPointChannel::~DynamicDelayPointToPointChannel()
{
}

void
DynamicDelayPointToPointChannel::Attach(Ptr<PointToPointNetDevice> device)
{
    PointToPointChannel::Attach(device);
    // Nodes can be set via attributes, but we can keep this as a fallback.
    if (m_nodeA == nullptr)
    {
        m_nodeA = device->GetNode();
    }
    else if (m_nodeB == nullptr)
    {
        m_nodeB = device->GetNode();
    }
}

Time
DynamicDelayPointToPointChannel::GetDelay(void) const
{
    if (!m_nodeA || !m_nodeB)
    {
        NS_LOG_WARN("Nodes not set on channel. Returning default delay. NodeA: " << m_nodeA << ", NodeB: " << m_nodeB);
        return PointToPointChannel::GetDelay();
    }

    Ptr<MobilityModel> mobilityA = m_nodeA->GetObject<MobilityModel>();
    Ptr<MobilityModel> mobilityB = m_nodeB->GetObject<MobilityModel>();

    if (!mobilityA || !mobilityB)
    {
        NS_LOG_WARN("Mobility model not found. Returning default delay. MobilityA: " << mobilityA << ", MobilityB: " << mobilityB);
        return PointToPointChannel::GetDelay();
    }

    const double distance = mobilityA->GetDistanceFrom(mobilityB);
    const Time delay = Seconds(distance / C);
    
    NS_LOG_LOGIC("Calculated delay. Distance: " << distance << " m, Delay: " << delay.GetSeconds() << " s");

    return delay;
}


bool
DynamicDelayPointToPointChannel::TransmitStart(Ptr<const Packet> p, Ptr<PointToPointNetDevice> src, Time txTime)
{
    NS_LOG_FUNCTION(this << p << src << txTime);

    // This implementation is now correctly adapted from the base PointToPointChannel.
    // It properly finds the destination device and uses ScheduleWithContext.

    // Determine the destination device on this point-to-point link.
    Ptr<PointToPointNetDevice> dst = nullptr;
    if (GetPointToPointDevice(0) == src)
    {
        dst = GetPointToPointDevice(1);
    }
    else
    {
        // Sanity check that the source is the other device
        NS_ASSERT_MSG(GetPointToPointDevice(1) == src, "TransmitStart called with a source device not on this channel");
        dst = GetPointToPointDevice(0);
    }
    NS_ASSERT_MSG(dst, "Destination device is null, channel not fully connected?");

    // Dynamically calculate the propagation delay
    const Time propDelay = GetDelay();
    const Time totalDelay = txTime + propDelay;

    NS_LOG_LOGIC("Transmitting packet. Propagation Delay: " << propDelay << ", Transmission Time: " << txTime << ", Total Delay: " << totalDelay);

    // Schedule the reception on the destination node, with the correct node context.
    Simulator::ScheduleWithContext(dst->GetNode()->GetId(),
                                   totalDelay,
                                   &PointToPointNetDevice::Receive,
                                   dst,
                                   p->Copy());

    // Note: We cannot call the animation trace (m_txrxPointToPoint) because it is
    // a private member of the base class. This means this dynamic channel will not
    // generate traces for NetAnim. To fix this, the ns-3 base class would need
    // to be modified to make the trace source protected.
    
    return true;
}

} // namespace ns3 