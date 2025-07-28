#ifndef POINT_TO_POINT_ENERGY_MODEL_H
#define POINT_TO_POINT_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/energy-module.h"

namespace ns3 {

class PointToPointEnergyModel : public energy::DeviceEnergyModel
{
public:
    static TypeId GetTypeId(void);

    PointToPointEnergyModel();
    virtual ~PointToPointEnergyModel();

    // Overridden from DeviceEnergyModel
    void SetEnergySource(Ptr<energy::EnergySource> source) override;
    double GetTotalEnergyConsumption(void) const override;
    void HandleEnergyDepletion(void) override;
    void HandleEnergyRecharged(void) override;
    void HandleEnergyChanged(void) override;

private:
    void DoDispose() override;
    void DoInitialize() override;

    double DoGetCurrentA() const override;
    void ChangeState(int newState) override;
    
    // State transition callbacks
    void TxPacketTrace(Ptr<const Packet> packet);
    void RxPacketTrace(Ptr<const Packet> packet);

    // Member variables
    Ptr<PointToPointNetDevice> m_device; //!< The associated device
    Ptr<energy::EnergySource> m_source; //!< The energy source

    // Energy consumption parameters
    double m_txCurrentA; //!< Transmit current in Amperes
    double m_rxCurrentA; //!< Receive current in Amperes
    double m_idleCurrentA; //!< Idle current in Amperes

    enum State {
        IDLE,
        TX,
        RX
    };

    State m_state; //!< Current state of the energy model
};

} // namespace ns3

#endif /* POINT_TO_POINT_ENERGY_MODEL_H */