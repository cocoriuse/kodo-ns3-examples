#ifndef CODE_HEADER_H
#define CODE_HEADER_H


#include <stdint.h>
#include <string>
#include <ns3/header.h>
#include <ns3/mac48-address.h>

namespace ns3 {

class CodeHeader : public Header
{
public:
  CodeHeader ();
  ~CodeHeader ();
  void SetGeneration (uint16_t gen);
  uint16_t GetGeneration (void) const;
  void SetMacSource (Mac48Address source);
  Mac48Address GetMacSource (void) const;
  void SetMacSink (Mac48Address sink);
  Mac48Address GetMacSink (void) const;


  // must be implemented to become a valid new header.
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;
  // allow protocol-specific access to the header data.

private:
  uint16_t m_generation;
  Mac48Address m_Macsource;
  Mac48Address m_Macsink;

};
}
#endif
