#ifndef SATELLITE_MAC_HEADER_H
#define SATELLITE_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3
{

/**
 * @brief A simple MAC header for the satellite device.
 */
class GroundSatelliteMacHeader : public Header
{
public:
    GroundSatelliteMacHeader();
    ~GroundSatelliteMacHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;

    void SetSource(const Address& address);
    Address GetSource() const;

    void SetProtocol(uint16_t protocol);
    uint16_t GetProtocol() const;

private:
    Mac48Address m_source;
    uint16_t m_protocol;
};

} // namespace ns3

#endif /* SATELLITE_MAC_HEADER_H */ 