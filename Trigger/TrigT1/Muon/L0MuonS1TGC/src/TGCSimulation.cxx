/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TGCSimulation.h"

#include "StoreGate/ReadHandle.h"
#include "xAODL0MuonCand/TGCCandDataAuxContainer.h"
#include "xAODMuonViews/FillContainer.h"

namespace L0Muon {

StatusCode TGCSimulation::initialize() {
  ATH_CHECK(m_keyTgcRdo.initialize());
  ATH_CHECK(m_outputKey.initialize());
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
  ATH_CHECK(m_candidateBuilderTool->build(*tgcRdoContainer, candidates, ctx));
  ATH_CHECK(m_innerCoincidenceTool->apply(candidates, ctx));

  xAOD::FillContainer<xAOD::TGCCandDataContainer,
                      xAOD::TGCCandDataAuxContainer>
      output{};
  ATH_CHECK(m_trackSelectorTool->select(candidates, *output, ctx));
  ATH_CHECK(output.record(m_outputKey, ctx));
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
