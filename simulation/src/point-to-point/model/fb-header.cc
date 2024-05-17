/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 New York University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Adrian S.-W. Tam <adrian.sw.tam@gmail.com>
 */

#include "fb-header.h"
#include "ns3/address-utils.h"
#include "ns3/buffer.h"
#include "ns3/log.h"
#include <iostream>
#include <stdint.h>

NS_LOG_COMPONENT_DEFINE("FbHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(FbHeader);

FbHeader::FbHeader(uint16_t sport,
                   uint16_t dport,
                   uint16_t pg,
                   uint8_t qntz_Fb,
                   uint32_t qoff,
                   uint32_t qdelta)
  : m_sport(sport)
  , m_dport(dport)
  , m_pg(pg)
  , m_qntz_Fb(qntz_Fb)
  , m_qoff(qoff)
  , m_qdelta(qdelta)
{
}

/*
FbHeader::FbHeader (const uint16_t fid, uint8_t qIndex, uint8_t qfb)
        : m_fid(fid), m_qIndex(qIndex), m_qfb(qfb), m_ecnBits(0)
{
        //NS_LOG_LOGIC("CN got the flow id " << std::hex << m_fid.hi << "+" <<
m_fid.lo << std::dec);
}
*/

FbHeader::FbHeader()
  : m_sport()
  , m_dport()
  , m_pg()
  , m_qntz_Fb()
  , m_qoff()
  , m_qdelta()
{
}

FbHeader::~FbHeader() {}

void
FbHeader::SetSport(const uint16_t sport)
{
  m_sport = sport;
}

void
FbHeader::SetDport(const uint16_t dport)
{
  m_dport = dport;
}

void
FbHeader::SetPG(const uint16_t pg)
{
  m_pg = pg;
}

void
FbHeader::SetQntzFb(const uint8_t qntz_Fb)
{
  m_qntz_Fb = qntz_Fb;
}

void
FbHeader::SetQoff(const uint32_t qoff)
{
  m_qoff = qoff;
}

void
FbHeader::SetQdelta(const uint32_t qdelta)
{
  m_qdelta = qdelta;
}

uint16_t
FbHeader::GetSport(void) const
{
  return m_sport;
}

uint16_t
FbHeader::GetDport(void) const
{
  return m_dport;
}

uint16_t
FbHeader::GetPG(void) const
{
  return m_pg;
}

uint8_t
FbHeader::GetQntzFb(void) const
{
  return m_qntz_Fb;
}

uint32_t
FbHeader::GetQoff(void) const
{
  return m_qoff;
}

uint32_t
FbHeader::GetQdelta(void) const
{
  return m_qdelta;
}

TypeId
FbHeader::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::FbHeader").SetParent<Header>().AddConstructor<FbHeader>();
  return tid;
}

TypeId
FbHeader::GetInstanceTypeId(void) const
{
  return GetTypeId();
}

void
FbHeader::Print(std::ostream& os) const
{
  os << "sport=" << m_sport << " dport=" << m_dport << " pg=" << m_pg
     << " qntz_Fb=" << (unsigned)m_qntz_Fb << " qoff=" << m_qoff
     << " qdelta=" << m_qdelta;
}

uint32_t
FbHeader::GetSerializedSize(void) const
{
  return 15;
}
void
FbHeader::Serialize(Buffer::Iterator start) const
{
  start.WriteHtonU16(m_sport);
  start.WriteHtonU16(m_dport);
  start.WriteHtonU16(m_pg);
  start.WriteU8(m_qntz_Fb);
  start.WriteHtonU32(m_qoff);
  start.WriteHtonU32(m_qdelta);
}

uint32_t
FbHeader::Deserialize(Buffer::Iterator start)
{
  m_sport = start.ReadNtohU16();
  m_dport = start.ReadNtohU16();
  m_pg = start.ReadNtohU16();
  m_qntz_Fb = start.ReadU8();
  m_qoff = start.ReadNtohU32();
  m_qdelta = start.ReadNtohU32();

  return GetSerializedSize();
}

}; // namespace ns3
