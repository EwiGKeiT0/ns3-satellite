#ifndef SATELLITE_ENERGY_MODEL_H
#define SATELLITE_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/energy-module.h"

namespace ns3 {

class SatelliteEnergyModel : public energy::DeviceEnergyModel
{
public:
    static TypeId GetTypeId();
    SatelliteEnergyModel();
    ~SatelliteEnergyModel() override;

private:
    void DoDispose() override;
    void DoInitialize() override;
    double DoGetCurrentA() const override;

public:
    void SetEnergySource(Ptr<energy::EnergySource> source) override;
    double GetTotalEnergyConsumption() const override;
    void HandleEnergyDepletion() override;
    void HandleEnergyRecharged() override;
    void HandleEnergyChanged() override;
    void ChangeState(int newState) override;

private:
    void UpdateEnergyState();
    void TransmissionFinished();
    void ReceptionFinished();

    void TxPacketTrace(Ptr<const Packet> packet);
    void RxPacketTrace(Ptr<const Packet> packet);

    Ptr<PointToPointNetDevice> m_device;
    Ptr<energy::EnergySource> m_source;

    double m_txCurrentA;
    double m_rxCurrentA;
    double m_idleCurrentA;

    Time m_lastUpdateTime;
    double m_totalEnergyConsumption;

    bool m_isTransmitting;
    bool m_isReceiving;
};

} // namespace ns3

#endif /* SATELLITE_ENERGY_MODEL_H */