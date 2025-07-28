#include "point-to-point-energy-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/attribute.h"
#include "ns3/data-rate.h"
#include "ns3/energy-source.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PointToPointEnergyModel");
NS_OBJECT_ENSURE_REGISTERED(PointToPointEnergyModel);

TypeId PointToPointEnergyModel::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PointToPointEnergyModel")
        .SetParent<energy::DeviceEnergyModel>()
        .SetGroupName("Satellite")
        .AddConstructor<PointToPointEnergyModel>()
        .AddAttribute("TxCurrentA", "The current consumed by the device when transmitting.",
                      DoubleValue(0.02),
                      MakeDoubleAccessor(&PointToPointEnergyModel::m_txCurrentA),
                      MakeDoubleChecker<double>())
        .AddAttribute("RxCurrentA", "The current consumed by the device when receiving.",
                      DoubleValue(0.01),
                      MakeDoubleAccessor(&PointToPointEnergyModel::m_rxCurrentA),
                      MakeDoubleChecker<double>())
        .AddAttribute("IdleCurrentA", "The current consumed by the device when idle.",
                      DoubleValue(0.001),
                      MakeDoubleAccessor(&PointToPointEnergyModel::m_idleCurrentA),
                      MakeDoubleChecker<double>());
    return tid;
}

PointToPointEnergyModel::PointToPointEnergyModel()
    : m_device(nullptr),
      m_source(nullptr),
      m_txCurrentA(0),
      m_rxCurrentA(0),
      m_idleCurrentA(0),
      m_lastUpdateTime(Seconds(0)),
      m_totalEnergyConsumption(0),
      m_isTransmitting(false),
      m_isReceiving(false)
{
}

PointToPointEnergyModel::~PointToPointEnergyModel()
{
}

void PointToPointEnergyModel::DoDispose()
{
    m_device = nullptr;
    m_source = nullptr;
    energy::DeviceEnergyModel::DoDispose();
}

void PointToPointEnergyModel::DoInitialize()
{
    energy::DeviceEnergyModel::DoInitialize();
    m_device = DynamicCast<PointToPointNetDevice>(GetObject<NetDevice>());
    NS_ASSERT(m_device != nullptr);

    m_device->TraceConnectWithoutContext("MacTx", MakeCallback(&PointToPointEnergyModel::TxPacketTrace, this));
    m_device->TraceConnectWithoutContext("MacRx", MakeCallback(&PointToPointEnergyModel::RxPacketTrace, this));

    if (m_source)
    {
        m_lastUpdateTime = Simulator::Now();
    }
}

void PointToPointEnergyModel::SetEnergySource(Ptr<energy::EnergySource> source)
{
    m_source = source;
    m_lastUpdateTime = Simulator::Now();
}

double PointToPointEnergyModel::GetTotalEnergyConsumption() const
{
    Time duration = Simulator::Now() - m_lastUpdateTime;
    double energy = m_totalEnergyConsumption;

    if (m_source)
    {
        energy += duration.GetSeconds() * DoGetCurrentA() * m_source->GetSupplyVoltage();
    }

    return energy;
}

void PointToPointEnergyModel::HandleEnergyDepletion() {}
void PointToPointEnergyModel::HandleEnergyRecharged() {}
void PointToPointEnergyModel::HandleEnergyChanged() {}

double PointToPointEnergyModel::DoGetCurrentA() const
{
    double current = m_idleCurrentA;
    if (m_isTransmitting)
    {
        current += m_txCurrentA - m_idleCurrentA;
    }
    if (m_isReceiving)
    {
        current += m_rxCurrentA - m_idleCurrentA;
    }
    return current;
}

void PointToPointEnergyModel::ChangeState(int newState)
{
    // This function is required to be implemented because it is a pure virtual
    // function in the base class (DeviceEnergyModel). However, our new model
    // manages state internally with boolean flags (m_isTransmitting, m_isReceiving)
    // and the UpdateEnergyState function, so this function is intentionally left empty.
}

void PointToPointEnergyModel::UpdateEnergyState(void)
{
    if (m_source)
    {
        Time now = Simulator::Now();
        Time duration = now - m_lastUpdateTime;
        double energyConsumed = duration.GetSeconds() * DoGetCurrentA() * m_source->GetSupplyVoltage();
        m_totalEnergyConsumption += energyConsumed;
        m_source->UpdateEnergySource();
        m_lastUpdateTime = now;
    }
}

void PointToPointEnergyModel::TransmissionFinished(void)
{
    UpdateEnergyState();
    m_isTransmitting = false;
}

void PointToPointEnergyModel::ReceptionFinished(void)
{
    UpdateEnergyState();
    m_isReceiving = false;
}

void PointToPointEnergyModel::TxPacketTrace(Ptr<const Packet> packet)
{
    UpdateEnergyState();
    m_isTransmitting = true;

    DataRateValue dataRateValue;
    m_device->GetAttribute("DataRate", dataRateValue);
    DataRate dataRate = dataRateValue.Get();
    Time txTime = dataRate.CalculateBytesTxTime(packet->GetSize());
    Simulator::Schedule(txTime, &PointToPointEnergyModel::TransmissionFinished, this);
}

void PointToPointEnergyModel::RxPacketTrace(Ptr<const Packet> packet)
{
    UpdateEnergyState();
    m_isReceiving = true;

    DataRateValue dataRateValue;
    m_device->GetAttribute("DataRate", dataRateValue);
    DataRate dataRate = dataRateValue.Get();
    Time rxTime = dataRate.CalculateBytesTxTime(packet->GetSize());
    Simulator::Schedule(rxTime, &PointToPointEnergyModel::ReceptionFinished, this);
}

} // namespace ns3
