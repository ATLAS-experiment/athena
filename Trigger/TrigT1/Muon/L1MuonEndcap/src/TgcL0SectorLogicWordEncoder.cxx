/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0SectorLogicWordEncoder.h"

#include "TrigT1MuctpiBits/MuCTPI_Bits.h"

namespace {

namespace bits = L0Muon::MuCTPIBits;

constexpr std::uint32_t encodeField(const std::uint32_t value,
                                    const std::uint32_t shift,
                                    const std::uint32_t mask) {
  return (value & mask) << shift;
}

}  // namespace

namespace L0Muon {

TgcL0SectorLogicWords TgcL0SectorLogicWordEncoder::encode(
    const xAOD::TGCCandData& candidate) {
  TgcL0SectorLogicWords words;
  words.candWord =
      encodeField(candidate.pt(), bits::RUN4_SL2MUCTPI_PT_VAL_SHIFT,
                  bits::RUN4_SL2MUCTPI_PT_VAL_MASK) |
      encodeField(candidate.candCharge(), bits::RUN4_SL2MUCTPI_CHARGE_SHIFT,
                  bits::RUN4_SL2MUCTPI_CHARGE_MASK) |
      encodeField(candidate.phi(), bits::RUN4_SL2MUCTPI_PHI_SHIFT,
                  bits::RUN4_SL2MUCTPI_PHI_MASK) |
      encodeField(candidate.eta(), bits::RUN4_SL2MUCTPI_ETA_SHIFT,
                  bits::RUN4_SL2MUCTPI_ETA_MASK);

  words.candExtraWord =
      encodeField(candidate.threshold(),
                  bits::RUN4_SL2MUCTPI_PTTHRESHOLD_SHIFT,
                  bits::RUN4_SL2MUCTPI_PTTHRESHOLD_MASK) |
      encodeField(candidate.tcId(), bits::RUN4_SL2MUCTPI_TCID_SHIFT,
                  bits::RUN4_SL2MUCTPI_TCID_MASK) |
      encodeField(candidate.coinType(), bits::RUN4_SL2MUCTPI_COINTYPE_SHIFT,
                  bits::RUN4_SL2MUCTPI_COINTYPE_MASK);
  return words;
}

}  // namespace L0Muon
