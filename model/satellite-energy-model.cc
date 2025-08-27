#include "satellite-energy-model.h"
#include "ground-satellite-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/attribute.h"
#include "ns3/data-rate.h"
#include "ns3/energy-source.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SatelliteEnergyModel");
NS_OBJECT_ENSURE_REGISTERED(SatelliteEnergyModel);

TypeId SatelliteEnergyModel::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatelliteEnergyModel")
        .SetParent<energy::DeviceEnergyModel>()
        .SetGroupName("Satellite")
        .AddConstructor<SatelliteEnergyModel>()
        .AddAttribute("TxCurrentA", "The current consumed by the device when transmitting.",
                      DoubleValue(0.5),
                      MakeDoubleAccessor(&SatelliteEnergyModel::m_txCurrentA),
                      MakeDoubleChecker<double>())
        .AddAttribute("RxCurrentA", "The current consumed by the device when receiving.",
                      DoubleValue(0.4),
                      MakeDoubleAccessor(&SatelliteEnergyModel::m_rxCurrentA),
                      MakeDoubleChecker<double>())
        .AddAttribute("IdleCurrentA", "The current consumed by the device when idle.",
                      DoubleValue(0.0),
                      MakeDoubleAccessor(&SatelliteEnergyModel::m_idleCurrentA),
                      MakeDoubleChecker<double>());
    return tid;
}

SatelliteEnergyModel::SatelliteEnergyModel()
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

SatelliteEnergyModel::~SatelliteEnergyModel()
{
}

void SatelliteEnergyModel::DoDispose()
{
    m_device = nullptr;
    m_source = nullptr;
    energy::DeviceEnergyModel::DoDispose();
}

void SatelliteEnergyModel::DoInitialize()
{
    energy::DeviceEnergyModel::DoInitialize();
    Ptr<NetDevice> netDevice = GetObject<NetDevice>();
    
    if (DynamicCast<PointToPointNetDevice>(netDevice))
    {
        netDevice->TraceConnectWithoutContext("MacTx", MakeCallback(&SatelliteEnergyModel::TxPacketTrace, this));
        netDevice->TraceConnectWithoutContext("MacRx", MakeCallback(&SatelliteEnergyModel::RxPacketTrace, this));
    }
    else
    {
        if (DynamicCast<GroundSatelliteNetDevice>(netDevice))
        {
            netDevice->TraceConnectWithoutContext("MacTx", MakeCallback(&SatelliteEnergyModel::TxPacketTrace, this));
            netDevice->TraceConnectWithoutContext("MacRx", MakeCallback(&SatelliteEnergyModel::RxPacketTrace, this));
        }
        else
        {
            NS_ASSERT_MSG(false, "SatelliteEnergyModel supports only PointToPointNetDevice and GroundSatelliteNetDevice.");
        }
    }

    if (m_source)
    {
        m_lastUpdateTime = Simulator::Now();
    }
}

void SatelliteEnergyModel::SetEnergySource(Ptr<energy::EnergySource> source)
{
    m_source = source;
    m_lastUpdateTime = Simulator::Now();
}

double SatelliteEnergyModel::GetTotalEnergyConsumption() const
{
    Time duration = Simulator::Now() - m_lastUpdateTime;
    double energy = m_totalEnergyConsumption;
    energy += duration.GetSeconds() * DoGetCurrentA() * m_source->GetSupplyVoltage();
    m_source->UpdateEnergySource();
    return energy;
}

void SatelliteEnergyModel::HandleEnergyDepletion() {}
void SatelliteEnergyModel::HandleEnergyRecharged() {}
void SatelliteEnergyModel::HandleEnergyChanged() {}

double SatelliteEnergyModel::DoGetCurrentA() const
{
    double current = m_idleCurrentA;
    if (m_isTransmitting)
    {
        current += m_txCurrentA;
    }
    if (m_isReceiving)
    {
        current += m_rxCurrentA;
    }
    return current;
}

void SatelliteEnergyModel::ChangeState(int newState)
{
    // This function is required to be implemented because it is a pure virtual
    // function in the base class (DeviceEnergyModel). However, our new model
    // manages state internally with boolean flags (m_isTransmitting, m_isReceiving)
    // and the UpdateEnergyState function, so this function is intentionally left empty.
}

void SatelliteEnergyModel::UpdateEnergyState(void)
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

void SatelliteEnergyModel::TransmissionFinished(void)
{
    UpdateEnergyState();
    m_isTransmitting = false;
}

void SatelliteEnergyModel::ReceptionFinished(void)
{
    UpdateEnergyState();
    m_isReceiving = false;
}

void SatelliteEnergyModel::TxPacketTrace(Ptr<const Packet> packet)
{
    UpdateEnergyState();
    m_isTransmitting = true;
    NS_LOG_DEBUG("Transmitting started");

    DataRateValue dataRateValue;
    GetObject<NetDevice>()->GetAttribute("DataRate", dataRateValue);
    DataRate dataRate = dataRateValue.Get();
    Time txTime = dataRate.CalculateBytesTxTime(packet->GetSize());
    Simulator::Schedule(txTime, &SatelliteEnergyModel::TransmissionFinished, this);
}

void SatelliteEnergyModel::RxPacketTrace(Ptr<const Packet> packet)
{
    UpdateEnergyState();
    m_isReceiving = true;
    NS_LOG_DEBUG("Receiving started");

    DataRateValue dataRateValue;
    GetObject<NetDevice>()->GetAttribute("DataRate", dataRateValue);
    DataRate dataRate = dataRateValue.Get();
    Time rxTime = dataRate.CalculateBytesTxTime(packet->GetSize());
    Simulator::Schedule(rxTime, &SatelliteEnergyModel::ReceptionFinished, this);
}

} // namespace ns3
