/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonRDO/NSW_PadTriggerData.h"
#include <iostream>

namespace Muon {
NSW_PadTriggerData::NSW_PadTriggerData(uint32_t sourceid,
                                       uint32_t flags,
                                       uint32_t ec,
                                       uint32_t fragid,
                                       uint32_t secid,
                                       uint32_t spare,
                                       uint32_t orbit,
                                       uint32_t bcid,
                                       uint32_t l1id,
                                       uint32_t orbitid,
                                       uint32_t orbit1,
                                       uint32_t status,
                                       uint32_t hit_n,
                                       uint32_t pfeb_n,
                                       uint32_t trigger_n,
                                       uint32_t bcid_n,
                                       const uint32_vt& hit_relbcid,
                                       const uint32_vt& hit_pfeb,
                                       const uint32_vt& hit_tdschannel,
                                       const uint32_vt& hit_vmmchannel,
                                       const uint32_vt& hit_vmm,
                                       const uint32_vt& hit_padchannel,
                                       const uint32_vt& pfeb_addr,
                                       const uint32_vt& pfeb_nchan,
                                       const uint32_vt& pfeb_disconnected,
                                       const uint32_vt& trigger_bandid,
                                       const uint32_vt& trigger_phiid,
                                       const uint32_vt& trigger_relbcid,
                                       const uint32_vt& bcid_rel,
                                       const uint32_vt& bcid_status,
                                       const uint32_vt& bcid_multzero,
                                       const uint32_vt& bcid_multiplicity):
  m_sourceid(sourceid),
  m_flags(flags),
  m_ec(ec),
  m_fragid(fragid),
  m_secid(secid),
  m_spare(spare),
  m_orbit(orbit),
  m_bcid(bcid),
  m_l1id(l1id),
  m_orbitid(orbitid),
  m_orbit1(orbit1),
  m_status(status),
  m_hit_n(hit_n),
  m_pfeb_n(pfeb_n),
  m_trigger_n(trigger_n),
  m_bcid_n(bcid_n),
  m_hit_relbcid(hit_relbcid),
  m_hit_pfeb(hit_pfeb),
  m_hit_tdschannel(hit_tdschannel),
  m_hit_vmmchannel(hit_vmmchannel),
  m_hit_vmm(hit_vmm),
  m_hit_padchannel(hit_padchannel),
  m_pfeb_addr(pfeb_addr),
  m_pfeb_nchan(pfeb_nchan),
  m_pfeb_disconnected(pfeb_disconnected),
  m_trigger_bandid(trigger_bandid),
  m_trigger_phiid(trigger_phiid),
  m_trigger_relbcid(trigger_relbcid),
  m_bcid_rel(bcid_rel),
  m_bcid_status(bcid_status),
  m_bcid_multzero(bcid_multzero),
  m_bcid_multiplicity(bcid_multiplicity)
{
  std::tie(m_trigger_bandid, m_trigger_phiid, m_trigger_relbcid) =
    filterNonNulls(m_trigger_bandid, m_trigger_phiid, m_trigger_relbcid);
  m_trigger_n = static_cast<uint32_t>(m_trigger_bandid.size());
}

NSW_PadTriggerData::NSW_PadTriggerData(bool side_A, uint32_t sector, uint32_t bcid, uint32_t l1id)
{
  m_sourceid = (side_A ? SIDE_A : SIDE_C) + 0x20 + sector;
  m_secid = sector;
  m_bcid = bcid;
  m_l1id = l1id;
}

std::string NSW_PadTriggerData::string() const {
  std::stringstream sstream{};
  sstream << "Source ID: " << std::hex << getSourceid() << std::dec << " N(hits): " << getNumberOfHits() << " N(triggers) " << getNumberOfTriggers();
  return sstream.str();
}

std::tuple< uint32_vt, uint32_vt, uint32_vt >
NSW_PadTriggerData::filterNonNulls(const uint32_vt & bandids, const uint32_vt & phiids, const uint32_vt & bcids) {
  uint32_vt bandidsFiltered{}, phiidsFiltered{}, bcidsFiltered{};
  for (size_t it = 0; it < bandids.size(); ++it) {
    if (bandids.at(it) == NULL_BANDID and phiids.at(it) == NULL_PHIID) {
      continue;
    }
    bandidsFiltered.push_back(bandids.at(it));
    phiidsFiltered .push_back(phiids.at(it));
    bcidsFiltered  .push_back(bcids.at(it));
  }
  return std::make_tuple(bandidsFiltered, phiidsFiltered, bcidsFiltered);
}

void NSW_PadTriggerData::addTrigger(uint32_t bandid, uint32_t phiid, uint32_t relbcid) {
  m_trigger_bandid.push_back(bandid);
  m_trigger_phiid.push_back(phiid);
  m_trigger_relbcid.push_back(relbcid);
  m_trigger_n += 1;
}

bool NSW_PadTriggerData::sideA() const {
  return m_sourceid < SIDE_C;
}

bool NSW_PadTriggerData::sideC() const {
  return not sideA();
}

bool NSW_PadTriggerData::largeSector() const {
  return m_secid % 2 == 0;
}

bool NSW_PadTriggerData::smallSector() const {
  return not largeSector();
}

std::ostream& operator<<(std::ostream& stream, const NSW_PadTriggerData& rhs) {
  return stream << rhs.string();
}

MsgStream& operator<<(MsgStream& stream, const NSW_PadTriggerData& rhs) {
  return stream << rhs.string();
}

} // namespace Muon
