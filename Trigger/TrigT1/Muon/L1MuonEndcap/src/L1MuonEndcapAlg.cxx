/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "L1MuonEndcapAlg.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "StoreGate/ReadHandle.h"
#include "SectorLogicWordEncoder.h"
#include "xAODMuonViews/FillContainer.h"

namespace L1Muon {
namespace {

constexpr int currentBcOffset = 0;
// Board and fiber identifiers remain unset until their conventions are defined.
constexpr std::uint16_t unsetBoardId = 0U;
constexpr std::uint16_t unsetFiberId = 0U;

}  // namespace

StatusCode L1MuonEndcapAlg::initialize() {
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode L1MuonEndcapAlg::execute(const EventContext& ctx) const {
  const xAOD::L1TGCCandDataContainer* inputHandle{};
  ATH_CHECK(SG::get(inputHandle, m_inputKey, ctx));

  xAOD::FillContainer<xAOD::SectorLogicCandDataContainer,
                      xAOD::SectorLogicCandDataAuxContainer>
      output{};

  for (const xAOD::L1TGCCandData* candidate : *inputHandle) {
    if (candidate->tcId() == 0U) continue;

    const L1TGC::SectorLogicWords words =
        L1TGC::SectorLogicWordEncoder::encode(*candidate);
    xAOD::SectorLogicCandData* outputCandidate =
        output->push_back(std::make_unique<xAOD::SectorLogicCandData>());
    outputCandidate->initialize(
        std::vector<std::uint32_t>{words.candWord, words.candExtraWord},
        currentBcOffset, unsetBoardId, unsetFiberId);
  }

  ATH_MSG_DEBUG("Converted " << output->size() << " of "
                              << inputHandle->size()
                              << " TGC candidates to Sector Logic output");

  ATH_CHECK(output.record(m_outputKey, ctx));
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon
