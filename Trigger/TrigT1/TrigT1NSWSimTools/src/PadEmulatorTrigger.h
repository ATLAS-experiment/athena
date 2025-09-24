/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PADEMULATORTRIGGER_H
#define PADEMULATORTRIGGER_H

#include<bitset>
#include "PadPattern.h"
#include "TrigT1NSWSimExtras.h"

/**
 * @class PadEmulatorTrigger
 * @brief Class to host information of sTGC Pad Trigger objects
 *
 * Basic class with all the properties of sTGC Pad Trigger objects:
 * Wheel, sector, pattern, hitmask, hitmask signature, bandID, sourceID, phiID, BCID
 **/

namespace NSWL1 {

  class PadEmulatorTrigger {

    public:
      PadEmulatorTrigger(const char wheel, const uint32_t sector,
                         const uint32_t bandid, const uint32_t phiid, const uint32_t relbcid,
                         const PadPattern pattern = PadPattern(0, 0, {}, {}, false), const uint32_t hitmask = 0);
      ~PadEmulatorTrigger() = default;

      char getWheel()            const { return m_wheel; };
      uint32_t getSector()       const { return m_sector; };
      PadPattern getPattern()    const { return m_pattern; };
      uint32_t getHitMask()      const { return m_hitmask; };
      uint32_t getBandid()       const { return m_bandid; };
      uint32_t getSourceid()     const { return NSWL1::PAD::wheelSectorToSourceID(m_wheel, m_sector); };
      uint32_t getPhiid()        const { return m_phiid; };
      int getSignedPhiid()       const { return m_phiid_signed; };
      uint32_t getRelbcid()      const { return m_relbcid; };
      const std::string& getSignature() const { return m_hitmasksignature; };

    private:
      const char m_wheel;
      const uint32_t m_sector;
      const PadPattern m_pattern;
      const uint32_t m_hitmask;
      const uint32_t m_bandid;
      const uint32_t m_phiid;
      const int m_phiid_signed;
      const uint32_t m_relbcid;
      std::string m_hitmasksignature;
  };
}
#endif
