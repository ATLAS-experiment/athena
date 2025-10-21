/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONNSWCOMMONDECODE_STGTPPACKETS_H
#define MUONNSWCOMMONDECODE_STGTPPACKETS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <vector>

#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"

namespace Muon::nsw {


class STGTPMMPacket {
  public:

  struct MMSegmentData {
    std::uint32_t monitor{};
    std::uint32_t spare{};
    std::uint32_t lowRes{};
    std::uint32_t phiRes{};
    std::uint32_t dTheta{};
    std::uint32_t phiID{};
    std::uint32_t rIndex{};
  };

   explicit STGTPMMPacket(const std::vector<std::uint32_t>& payload, const int ver);
   virtual ~STGTPMMPacket() = default;
   size_t Size(const int ver);
 
  [[nodiscard]] const std::array<MMSegmentData, STGTPMMData::num_mm>& Segments() const { return m_segmentData; }
  [[nodiscard]] const MMSegmentData& Segment(std::size_t segment) const;

  [[nodiscard]] std::uint32_t BCID() const { return m_BCID; };

  private:  
  std::array<MMSegmentData, STGTPMMData::num_mm> m_segmentData{};
  std::uint32_t m_BCID{};
  std::uint32_t m_valids{};
};

class STGTPPadPacket {
 public:
  explicit STGTPPadPacket(const std::vector<std::uint32_t>& payload, const int ver);
  virtual ~STGTPPadPacket() = default;
  [[nodiscard]] std::uint32_t BCID() const { return m_BCID; };
  [[nodiscard]] std::uint32_t BandID(const std::size_t num) const { return m_bandIDs.at(num); };
  [[nodiscard]] std::uint32_t PhiID(const std::size_t num) const { return m_phiIDs.at(num); };
  [[nodiscard]] std::uint32_t PadIdleFlag() const { return m_idleFlag; };
  [[nodiscard]] std::uint32_t CoincidenceWedge() const { return m_coincWedge; };

 private:
  std::uint32_t m_BCID{};
  std::array<std::uint32_t, STGTPPad::num_pads> m_bandIDs{};
  std::array<std::uint32_t, STGTPPad::num_pads> m_phiIDs{};
  std::uint32_t m_idleFlag{};
  std::uint32_t m_coincWedge{};
  size_t Size(const int version);
  
};

class STGTPSegmentPacket {
 public:
  struct SegmentData {
    std::uint32_t monitor{};
    std::uint32_t spare{};
    std::uint32_t lowRes{};
    std::uint32_t phiRes{};
    std::uint32_t dTheta{};
    std::uint32_t phiID{};
    std::uint32_t rIndex{};
  };

  explicit STGTPSegmentPacket(const std::vector<std::uint32_t>& payload, const int ver);

  virtual ~STGTPSegmentPacket() = default;
  [[nodiscard]] std::uint32_t LUT_ChoiceSelection() const { return m_lut_choice; }
  [[nodiscard]] std::uint32_t NSW_SegmentSelector() const { return m_nsw_segment_selector; }
  [[nodiscard]] std::uint32_t ValidSegmentSelector() const { return m_valid_segment_selector; }

  [[nodiscard]] const std::array<SegmentData, STGTPSegments::num_segments>& Segments() const { return m_segmentData; }
  [[nodiscard]] const SegmentData& Segment(std::size_t segment) const;

  [[nodiscard]] std::uint32_t BCID() const { return m_BCID; };
  [[nodiscard]] std::uint32_t SectorID() const { return m_sectorID; }

 private:
  std::uint32_t m_lut_choice{};
  std::uint32_t m_nsw_segment_selector{};
  std::uint32_t m_valid_segment_selector{};

  std::array<SegmentData, STGTPSegments::num_segments> m_segmentData{};

  std::uint32_t m_BCID{};
  std::uint32_t m_sectorID{};
  size_t Size(const int ver);
};

class STGTPStripPacket {
 public:
  explicit STGTPStripPacket(const std::vector<std::uint32_t>& payload, const int ver);

  virtual ~STGTPStripPacket() = default;

  [[nodiscard]] const std::array<std::uint32_t, STGTPStrips::num_strips>& Strips() const { return m_stripData; }
  [[nodiscard]] std::uint32_t Strip(std::size_t strip) const;
  [[nodiscard]] const std::array<std::uint32_t, STGTPStrips::num_offsets>& Offsets() const { return m_offsets; }
  [[nodiscard]] std::uint32_t Offset(std::size_t offset) const;

  [[nodiscard]] std::uint32_t PhiIdValue() const { return m_phiIdValue; }
  [[nodiscard]] std::uint32_t PhiIdSign() const { return m_phiIdSign; }
  [[nodiscard]] std::uint32_t BandId() const { return m_bandId; }
  [[nodiscard]] std::uint32_t BCID() const { return m_BCID; }

 private:
  std::array<std::uint32_t, STGTPStrips::num_strips> m_stripData{};
  std::array<std::uint32_t, STGTPStrips::num_offsets> m_offsets{};
  std::uint32_t m_phiIdValue{};
  std::uint32_t m_phiIdSign{};
  std::uint32_t m_bandId{};
  std::uint32_t m_BCID{};
  size_t Size(const int ver);
};

}  // namespace Muon::nsw

#endif  // MUONNSWCOMMONDECODE_STGTPPACKETS_H
