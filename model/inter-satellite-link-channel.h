#ifndef DYNAMIC_DELAY_P2P_CHANNEL_H
#define DYNAMIC_DELAY_P2P_CHANNEL_H

#include "ns3/point-to-point-channel.h"
#include "ns3/event-id.h"
#include "ns3/node.h"
#include "ns3/propagation-delay-model.h"

namespace ns3 {

/**
 * @brief A PointToPointChannel that dynamically calculates its propagation delay.
 *
 * This channel overrides the GetDelay() method to calculate the delay
 * on-the-fly based on the current distance between the two connected nodes.
 * It's designed for mobile nodes where the propagation time changes.
 */
class InterSatelliteLinkChannel : public PointToPointChannel
{
public:
    static TypeId GetTypeId(void);

    InterSatelliteLinkChannel();
    ~InterSatelliteLinkChannel() override;

    void Attach(Ptr<PointToPointNetDevice> device);
    Time GetDelay(void) const;
    bool TransmitStart(Ptr<const Packet> p, Ptr<PointToPointNetDevice> src, Time txTime) override;
    
private:
    // Pointers to the two nodes attached to this channel.
    // We make them mutable so they can be modified in the const GetDelay() method.
    // A better design might be to make GetDistanceFrom const in MobilityModel.
    // But for now, this is a pragmatic solution.
    Ptr<Node> m_nodeA;
    Ptr<Node> m_nodeB;
};

} // namespace ns3

#endif /* DYNAMIC_DELAY_P2P_CHANNEL_H */ 