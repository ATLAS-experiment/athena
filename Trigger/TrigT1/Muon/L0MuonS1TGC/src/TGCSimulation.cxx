/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TGCSimulation.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODL0MuonCand/TGCCandDataAuxContainer.h"
#include "xAODMuonViews/FillContainer.h"

#include <memory>

namespace L0Muon {

StatusCode TGCSimulation::initialize() {
  ATH_CHECK(m_keyTgcRdo.initialize());
  ATH_CHECK(m_outputKey.initialize());
  ATH_CHECK(m_validationCandidateKey.initialize(!m_validationCandidateKey.empty()));
  ATH_CHECK(m_validationSegmentKey.initialize(!m_validationSegmentKey.empty()));
  ATH_CHECK(m_candidateBuilderTool.retrieve());
  ATH_CHECK(m_innerCoincidenceTool.retrieve());
  ATH_CHECK(m_trackSelectorTool.retrieve());
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode TGCSimulation::execute(const EventContext& ctx) const {
  const TgcRdoContainer* tgcRdoContainer{};
  ATH_CHECK(SG::get(tgcRdoContainer, m_keyTgcRdo, ctx));

  auto nTgcRdoCollections = Monitored::Scalar<unsigned int>(
      "nTgcRdoCollections", tgcRdoContainer->size());
  Monitored::Group(m_monTool, nTgcRdoCollections);

  TgcL0CandidateContainer candidates;
  std::unique_ptr<TgcL0SegmentContainer> validationSegments;
  if (!m_validationSegmentKey.empty()) {
    validationSegments = std::make_unique<TgcL0SegmentContainer>();
  }
  ATH_CHECK(m_candidateBuilderTool->build(
      *tgcRdoContainer, candidates, validationSegments.get(), ctx));
  if (validationSegments) {
    SG::WriteHandle<TgcL0SegmentContainer> segmentHandle{
        m_validationSegmentKey, ctx};
    ATH_CHECK(segmentHandle.record(std::move(validationSegments)));
  }
  if (!m_validationCandidateKey.empty()) {
    SG::WriteHandle<TgcL0CandidateContainer> validationCandidates{
        m_validationCandidateKey, ctx};
    ATH_CHECK(validationCandidates.record(
        std::make_unique<TgcL0CandidateContainer>(candidates)));
  }
  ATH_CHECK(m_innerCoincidenceTool->apply(candidates, ctx));

  xAOD::FillContainer<xAOD::TGCCandDataContainer,
                      xAOD::TGCCandDataAuxContainer>
      output{};
  ATH_CHECK(m_trackSelectorTool->select(candidates, *output, ctx));
  ATH_CHECK(output.record(m_outputKey, ctx));
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
