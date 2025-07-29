#include "ground-satellite-mac-header.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GroundSatelliteMacHeader");

NS_OBJECT_ENSURE_REGISTERED(GroundSatelliteMacHeader);

GroundSatelliteMacHeader::GroundSatelliteMacHeader()
    : m_protocol(0)
{
}

GroundSatelliteMacHeader::~GroundSatelliteMacHeader()
{
}

TypeId
GroundSatelliteMacHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::GroundSatelliteMacHeader")
                            .SetParent<Header>()
                            .SetGroupName("Satellite")
                            .AddConstructor<GroundSatelliteMacHeader>();
    return tid;
}

TypeId
GroundSatelliteMacHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
GroundSatelliteMacHeader::GetSerializedSize() const
{
    return Address::MAX_SIZE + sizeof(m_protocol);
}

void
GroundSatelliteMacHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    uint8_t buf[Address::MAX_SIZE];
    m_source.CopyTo(buf);
    i.Write(buf, Address::MAX_SIZE);
    i.WriteHtonU16(m_protocol);
}

uint32_t
GroundSatelliteMacHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    uint8_t buf[Address::MAX_SIZE];
    i.Read(buf, Address::MAX_SIZE);
    m_source.CopyFrom(buf);
    m_protocol = i.ReadNtohU16();
    return GetSerializedSize();
}

void
GroundSatelliteMacHeader::Print(std::ostream& os) const
{
    os << "GroundSatelliteMacHeader(Source=" << m_source
       << ", Protocol=0x" << std::hex << m_protocol << std::dec << ")";
}

void
GroundSatelliteMacHeader::SetSource(const Address& address)
{
    m_source = Mac48Address::ConvertFrom(address);
}

Address
GroundSatelliteMacHeader::GetSource() const
{
    return m_source;
}

void
GroundSatelliteMacHeader::SetProtocol(uint16_t protocol)
{
    m_protocol = protocol;
}

uint16_t
GroundSatelliteMacHeader::GetProtocol() const
{
    return m_protocol;
}

} // namespace ns3 