#include "code-header.h"
#include <ns3/address-utils.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CodeHeader);

CodeHeader::CodeHeader ()
  : m_generation (0)

{
	code=1;
}
CodeHeader::~CodeHeader ()
{

  m_generation = 0;
  code=0;
}


TypeId
CodeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("CodeHeader")
    .SetParent<Header> ()
    .AddConstructor<CodeHeader> ()
  ;
  return tid;
}
TypeId
CodeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
CodeHeader::GetSerializedSize (void) const
{
  return 4;
}
void
CodeHeader::Serialize (Buffer::Iterator start) const
{
  // The 2 byte-constant
  start.WriteHtonU16 (m_generation);



}
uint32_t
CodeHeader::Deserialize (Buffer::Iterator start)
{
  m_generation = start.ReadNtohU16 ();
  return 4; // the number of bytes consumed.
}
void
CodeHeader::Print (std::ostream &os) const
{
  os << m_generation;
}

void
CodeHeader::SetGeneration (uint16_t generation)
{
  m_generation = generation;
}

uint16_t
CodeHeader::GetGeneration (void) const
{
  return m_generation;
}

void CodeHeader::EnableCode (void) 
{
code=1;
}

void CodeHeader::DisableCode (void) 
{
code=0;
}
uint16_t CodeHeader::GetCode (void) 
{
return code;
}

}
