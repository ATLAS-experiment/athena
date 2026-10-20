/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SectorLogicWordEncoder.h"

#include "TrigT1MuctpiBits/MuCTPI_Bits.h"

namespace {

namespace bits = L1Muon::MuCTPIBits;

constexpr std::uint32_t encodeField(const std::uint32_t value,
                                    const std::uint32_t shift,
                                    const std::uint32_t mask) {
  return (value & mask) << shift;
}

}  // namespace

namespace L1Muon::L1TGC {

SectorLogicWords SectorLogicWordEncoder::encode(
    const xAOD::L1TGCCandData& candidate) {
  SectorLogicWords words;
  words.candWord =
      encodeField(candidate.l1Pt(), bits::RUN4_SL2MUCTPI_PT_VAL_SHIFT,
                  bits::RUN4_SL2MUCTPI_PT_VAL_MASK) |
      encodeField(candidate.l1CandCharge(), bits::RUN4_SL2MUCTPI_CHARGE_SHIFT,
                  bits::RUN4_SL2MUCTPI_CHARGE_MASK) |
      encodeField(candidate.l1Phi(), bits::RUN4_SL2MUCTPI_PHI_SHIFT,
                  bits::RUN4_SL2MUCTPI_PHI_MASK) |
      encodeField(candidate.l1Eta(), bits::RUN4_SL2MUCTPI_ETA_SHIFT,
                  bits::RUN4_SL2MUCTPI_ETA_MASK);

  words.candExtraWord =
      encodeField(candidate.l1Threshold(),
                  bits::RUN4_SL2MUCTPI_PTTHRESHOLD_SHIFT,
                  bits::RUN4_SL2MUCTPI_PTTHRESHOLD_MASK) |
      encodeField(candidate.tcId(), bits::RUN4_SL2MUCTPI_TCID_SHIFT,
                  bits::RUN4_SL2MUCTPI_TCID_MASK) |
      encodeField(candidate.coinType(), bits::RUN4_SL2MUCTPI_COINTYPE_SHIFT,
                  bits::RUN4_SL2MUCTPI_COINTYPE_MASK);
  return words;
}

}  // namespace L1Muon::L1TGC
