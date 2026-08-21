/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L0MUONENDCAP_L0MUONENDCAPALG_H
#define L0MUONENDCAP_L0MUONENDCAPALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"
#include "xAODTrigL0Muon/SectorLogicCandDataAuxContainer.h"
#include "xAODTrigL0Muon/SectorLogicCandDataContainer.h"

namespace L0Muon {

/**
 * @brief Initial data-flow boundary for the Phase-II endcap trigger.
 *
 * The algorithm converts TGCCandData into the TGC-side SectorLogicCandData
 * container sent towards MuCTPI. Candidate combination and link identifiers
 * will be added in later merge requests.
 */
class L0MuonEndcapAlg final : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<xAOD::TGCCandDataContainer> m_inputKey{
      this, "InputKey", "L0MuonTGCCandData",
      "TGC Sector Logic candidates"};

  SG::WriteHandleKey<xAOD::SectorLogicCandDataContainer> m_outputKey{
      this, "OutputKey", "L0MuonTGCSectorLogicCandData",
      "TGC-side Sector Logic candidates sent towards MuCTPI"};
};

}  // namespace L0Muon

#endif  // L0MUONENDCAP_L0MUONENDCAPALG_H
