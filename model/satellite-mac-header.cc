#include "satellite-mac-header.h"
#include "ns3/log.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatelliteMacHeader);

TypeId
SatelliteMacHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatelliteMacHeader")
        .SetParent<Header>()
        .SetGroupName("Satellite")
        .AddConstructor<SatelliteMacHeader>();
    return tid;
}

TypeId
SatelliteMacHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

SatelliteMacHeader::SatelliteMacHeader() = default;

void
SatelliteMacHeader::Print(std::ostream& os) const
{
    os << "src=" << m_source << ", dst=" << m_destination;
}

uint32_t
SatelliteMacHeader::GetSerializedSize() const
{
    return 14; // 2 * 6 bytes for MAC addresses + 2 bytes for protocol
}

void
SatelliteMacHeader::Serialize(Buffer::Iterator start) const
{
    uint8_t buffer[6];
    m_source.CopyTo(buffer);
    start.Write(buffer, 6);
    m_destination.CopyTo(buffer);
    start.Write(buffer, 6);
    start.WriteHtonU16(m_protocol);
}

uint32_t
SatelliteMacHeader::Deserialize(Buffer::Iterator start)
{
    uint8_t buffer[6];
    start.Read(buffer, 6);
    m_source.CopyFrom(buffer);
    start.Read(buffer, 6);
    m_destination.CopyFrom(buffer);
    m_protocol = start.ReadNtohU16();
    return GetSerializedSize();
}

void
SatelliteMacHeader::SetSource(const Address& address)
{
    m_source = Mac48Address::ConvertFrom(address);
}

Address
SatelliteMacHeader::GetSource() const
{
    return m_source;
}

void
SatelliteMacHeader::SetDestination(const Address& address)
{
    m_destination = Mac48Address::ConvertFrom(address);
}

Address
SatelliteMacHeader::GetDestination() const
{
    return m_destination;
}

void
SatelliteMacHeader::SetProtocol(uint16_t protocol)
{
    m_protocol = protocol;
}

uint16_t
SatelliteMacHeader::GetProtocol() const
{
    return m_protocol;
}

} // namespace ns3 