/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRDO_NSW_PADTRIGGERDATA
#define MUONRDO_NSW_PADTRIGGERDATA

#include "GaudiKernel/MsgStream.h"

#include <iosfwd>
#include <string>
#include <tuple>
#include <vector>
#include <cstdint>

namespace Muon {

using uint32_vt = std::vector<uint32_t>;

class NSW_PadTriggerData {
public:

    // constructor for data
    NSW_PadTriggerData(uint32_t sourceid,
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
                       const uint32_vt& bcid_multiplicity);

    // constructor for sim
    NSW_PadTriggerData(bool side_A, uint32_t sector, uint32_t bcid, uint32_t l1id);

    uint32_t getSourceid()           const { return m_sourceid; }
    uint32_t getFlags()              const { return m_flags; }
    uint32_t getEc()                 const { return m_ec; }
    uint32_t getFragid()             const { return m_fragid; }
    uint32_t getSecid()              const { return m_secid; }
    uint32_t getSpare()              const { return m_spare; }
    uint32_t getOrbit()              const { return m_orbit; }
    uint32_t getBcid()               const { return m_bcid; }
    uint32_t getL1id()               const { return m_l1id; }
    uint32_t getOrbitid()            const { return m_orbitid; }
    uint32_t getOrbit1()             const { return m_orbit1; }
    uint32_t getStatus()             const { return m_status; }
    uint32_t getNumberOfHits()       const { return m_hit_n; }
    uint32_t getNumberOfPfebs()      const { return m_pfeb_n; }
    uint32_t getNumberOfTriggers()   const { return m_trigger_n; }
    uint32_t getNumberOfBcids()      const { return m_bcid_n; }
    const uint32_vt& getHitRelBcids()       const { return m_hit_relbcid; }
    const uint32_vt& getHitPfebs()          const { return m_hit_pfeb; }
    const uint32_vt& getHitTdsChannels()    const { return m_hit_tdschannel; }
    const uint32_vt& getHitVmmChannels()    const { return m_hit_vmmchannel; }
    const uint32_vt& getHitVmms()           const { return m_hit_vmm; }
    const uint32_vt& getHitPadChannels()    const { return m_hit_padchannel; }
    const uint32_vt& getPfebAddrs()         const { return m_pfeb_addr; }
    const uint32_vt& getPfebNChannels()     const { return m_pfeb_nchan; }
    const uint32_vt& getPfebDisconnecteds() const { return m_pfeb_disconnected; }
    const uint32_vt& getTriggerBandIds()    const { return m_trigger_bandid; }
    const uint32_vt& getTriggerPhiIds()     const { return m_trigger_phiid; }
    const uint32_vt& getTriggerRelBcids()   const { return m_trigger_relbcid; }
    const uint32_vt& getBcidRels()          const { return m_bcid_rel; }
    const uint32_vt& getBcidStatuses()      const { return m_bcid_status; }
    const uint32_vt& getBcidMultZeros()     const { return m_bcid_multzero; }
    const uint32_vt& getBcidMultiplicities() const { return m_bcid_multiplicity; }

    std::string string() const;
    friend std::ostream& operator<<(std::ostream& stream, const NSW_PadTriggerData& rhs);
    friend MsgStream& operator<<(MsgStream& stream, const NSW_PadTriggerData& rhs);

    void addTrigger(uint32_t bandid, uint32_t phiid, uint32_t relbcid);

    bool sideA() const;
    bool sideC() const;
    bool largeSector() const;
    bool smallSector() const;

private:

    static std::tuple< uint32_vt, uint32_vt, uint32_vt >
      filterNonNulls(const uint32_vt & bandids, const uint32_vt & phiids, const uint32_vt & bcids) ;

    uint32_t m_sourceid{0};
    uint32_t m_flags{0};
    uint32_t m_ec{0};
    uint32_t m_fragid{0};
    uint32_t m_secid{0};
    uint32_t m_spare{0};
    uint32_t m_orbit{0};
    uint32_t m_bcid{0};
    uint32_t m_l1id{0};
    uint32_t m_orbitid{0};
    uint32_t m_orbit1{0};
    uint32_t m_status{0};
    uint32_t m_hit_n{0};
    uint32_t m_pfeb_n{0};
    uint32_t m_trigger_n{0};
    uint32_t m_bcid_n{0};
    uint32_vt m_hit_relbcid{};
    uint32_vt m_hit_pfeb{};
    uint32_vt m_hit_tdschannel{};
    uint32_vt m_hit_vmmchannel{};
    uint32_vt m_hit_vmm{};
    uint32_vt m_hit_padchannel{};
    uint32_vt m_pfeb_addr{};
    uint32_vt m_pfeb_nchan{};
    uint32_vt m_pfeb_disconnected{};
    uint32_vt m_trigger_bandid{};
    uint32_vt m_trigger_phiid{};
    uint32_vt m_trigger_relbcid{};
    uint32_vt m_bcid_rel{};
    uint32_vt m_bcid_status{};
    uint32_vt m_bcid_multzero{};
    uint32_vt m_bcid_multiplicity{};

    static constexpr uint32_t NULL_BANDID{0xff};
    static constexpr uint32_t NULL_PHIID{0x3f};
    static constexpr uint32_t SIDE_A = 0x6d0000;
    static constexpr uint32_t SIDE_C = 0x6e0000;

};
} // namespace Muon

#endif // MUONRDO_NSW_PADTRIGGERDATA
