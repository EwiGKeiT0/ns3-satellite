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

TypeId PointToPointEnergyModel::GetTypeId(void)
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
      m_state(IDLE)
{
}

PointToPointEnergyModel::~PointToPointEnergyModel()
{
}

void PointToPointEnergyModel::DoDispose(void)
{
    m_device = nullptr;
    m_source = nullptr;
    energy::DeviceEnergyModel::DoDispose();
}

void PointToPointEnergyModel::DoInitialize(void)
{
    energy::DeviceEnergyModel::DoInitialize();
    m_device = DynamicCast<PointToPointNetDevice>(GetObject<NetDevice>());
    NS_ASSERT(m_device != nullptr);

    m_device->TraceConnectWithoutContext("MacTx", MakeCallback(&PointToPointEnergyModel::TxPacketTrace, this));
    m_device->TraceConnectWithoutContext("MacRx", MakeCallback(&PointToPointEnergyModel::RxPacketTrace, this));
}

void PointToPointEnergyModel::SetEnergySource(Ptr<energy::EnergySource> source)
{
    m_source = source;
}

double PointToPointEnergyModel::GetTotalEnergyConsumption(void) const
{
    if (m_source)
    {
        return m_source->GetInitialEnergy() - m_source->GetRemainingEnergy();
    }
    return 0;
}

void PointToPointEnergyModel::HandleEnergyDepletion(void) {}
void PointToPointEnergyModel::HandleEnergyRecharged(void) {}
void PointToPointEnergyModel::HandleEnergyChanged(void) {}

double PointToPointEnergyModel::DoGetCurrentA() const
{
    switch (m_state) {
        case TX:
            return m_txCurrentA;
        case RX:
            return m_rxCurrentA;
        case IDLE:
            return m_idleCurrentA;
        default:
            return 0;
    }
}

void PointToPointEnergyModel::ChangeState(int newState)
{
    if (m_source)
    {
        m_source->UpdateEnergySource();
    }
    m_state = (State)newState;
}

void PointToPointEnergyModel::TxPacketTrace(Ptr<const Packet> packet)
{
    ChangeState(TX);
    DataRateValue dataRateValue;
    m_device->GetAttribute("DataRate", dataRateValue);
    DataRate dataRate = dataRateValue.Get();
    Time txTime = dataRate.CalculateBytesTxTime(packet->GetSize());
    Simulator::Schedule(txTime, &PointToPointEnergyModel::ChangeState, this, IDLE);
}

void PointToPointEnergyModel::RxPacketTrace(Ptr<const Packet> packet)
{
    ChangeState(RX);
    DataRateValue dataRateValue;
    m_device->GetAttribute("DataRate", dataRateValue);
    DataRate dataRate = dataRateValue.Get();
    Time rxTime = dataRate.CalculateBytesTxTime(packet->GetSize());
    Simulator::Schedule(rxTime, &PointToPointEnergyModel::ChangeState, this, IDLE);
}

} // namespace ns3
