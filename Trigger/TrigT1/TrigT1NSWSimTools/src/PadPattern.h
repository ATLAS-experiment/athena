/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PADPATTERN_H
#define PADPATTERN_H

#include <array>
#include <cstdint>

/**
 * @class PadEmulatorPattern
 * @brief Class to host information of sTGC Pad Pattern objects
 *
 * Basic class with all the properties of sTGC Pad Pattern objects, loaded from vhdl files:
 * Pfebs, pad channels, sector type, bandID, phiID
 **/

namespace NSWL1 {

  class PadPattern {

    public:
      PadPattern(const uint32_t bandid, const uint32_t phiid, const std::array<uint32_t,8>& pfebs, const std::array<uint32_t,8>& padchans, const bool isLarge);
      ~PadPattern() = default;

      const std::array<uint32_t,8>& getPfebs() const { return m_pfebs; };
      const std::array<uint32_t,8>& getPadChannels() const { return m_padchans; };
      uint32_t getBandid() const { return m_bandid; };
      uint32_t getPhiid(bool flip=false) const { return flip ? m_phiid_flip : m_phiid; };
      bool isLarge() const { return m_isLarge; };
      bool isSmall() const { return not isLarge(); };
      uint32_t flipPhiIdSign() const;

    private:
      const uint32_t m_bandid;
      const uint32_t m_phiid;
      const uint32_t m_phiid_flip;
      const std::array<uint32_t,8> m_pfebs; // Fixed size 8, i.e. one per layer
      const std::array<uint32_t,8> m_padchans;
      const bool m_isLarge;
  };
}
#endif
