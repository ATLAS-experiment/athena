/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "L0MuonEndcapAlg.h"

#include "StoreGate/ReadHandle.h"
#include "xAODMuonViews/FillContainer.h"

namespace L0Muon {

StatusCode L0MuonEndcapAlg::initialize() {
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode L0MuonEndcapAlg::execute(const EventContext& ctx) const {
  const xAOD::TGCCandDataContainer* inputHandle{};
  ATH_CHECK(SG::get(inputHandle, m_inputKey, ctx));

  xAOD::FillContainer<xAOD::SectorLogicCandDataContainer,
                      xAOD::SectorLogicCandDataAuxContainer>
      output{};

  ATH_MSG_DEBUG("Received " << inputHandle->size()
                             << " TGC candidates; endcap candidate conversion "
                                "is not implemented in this skeleton");

  ATH_CHECK(output.record(m_outputKey, ctx));
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
