/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0SectorLogicWordEncoder.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "AthContainers/AuxStoreInternal.h"
#include "xAODL0MuonCand/TGCCandData.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"
#include "xAODTrigL1Muon/SectorLogicCandData.h"
#include "xAODTrigL1Muon/SectorLogicCandDataContainer.h"

namespace {

bool check(const bool condition, const std::string_view description) {
  if (!condition) std::cerr << "Test failed: " << description << '\n';
  return condition;
}

}  // namespace

int main() {
  xAOD::TGCCandDataContainer candidates;
  SG::AuxStoreInternal candidateAux;
  candidates.setStore(&candidateAux);
  xAOD::TGCCandData* candidate =
      candidates.push_back(std::make_unique<xAOD::TGCCandData>());
  candidate->initialize(0x65U, 17U, 0x2U);
  candidate->setEta(-1.25F);
  candidate->setPhi(2.1F);
  candidate->setPt(42.5F);
  candidate->setThreshold(11U);
  candidate->setCandCharge(1U);
  candidate->setCoinType(0U);
  candidate->setHasInnerCoincidence(true);
  candidate->setGoodMagneticField(true);
  candidate->setTcId(5U);

  const L0Muon::TgcL0SectorLogicWords words =
      L0Muon::TgcL0SectorLogicWordEncoder::encode(*candidate);

  bool success = true;
  success &= check(words.candWord == 0x55ea912fU, "candidate word");
  success &= check(words.candExtraWord == 0x35bU, "candidate extra word");

  xAOD::SectorLogicCandDataContainer decodedCandidates;
  SG::AuxStoreInternal decodedAux;
  decodedCandidates.setStore(&decodedAux);
  xAOD::SectorLogicCandData* decoded = decodedCandidates.push_back(
      std::make_unique<xAOD::SectorLogicCandData>());
  decoded->initialize(
      std::vector<std::uint32_t>{words.candWord, words.candExtraWord},
      0, 0U, 0U);

  success &=
      check(decoded->candWord() == words.candWord, "stored candidate word");
  success &= check(decoded->candExtraWord() == words.candExtraWord,
                   "stored candidate extra word");
  success &= check(decoded->BCIDOffset() == 0, "current-BC offset");
  success &= check(decoded->boardID() == 0U, "unset board ID");
  success &= check(decoded->fiberID() == 0U, "unset fiber ID");
  success &= check(decoded->rawEta() == candidate->eta(), "eta code");
  success &= check(decoded->rawPhi() == candidate->phi(), "phi code");
  success &= check(decoded->pT() == candidate->pt(), "pT code");
  success &=
      check(decoded->charge() == candidate->candCharge(), "charge code");
  success &= check(decoded->ptThresh() == candidate->threshold(),
                   "threshold code");
  success &= check(decoded->TCID() == candidate->tcId(), "TCID code");
  success &=
      check(decoded->coinType() == candidate->coinType(), "CoinType code");
  success &= check(decoded->mdtFlag() == 0U, "MDT flag is clear");
  success &= check(decoded->isMDT() == 0U, "MDT-processing bit is clear");
  success &= check(decoded->tileCoin() == 0U, "Tile bit is clear");
  success &= check(decoded->exotTrig() == 0U, "exotic-trigger bits are clear");
  success &= check(decoded->numMDTSeg() == 0U, "MDT segment count is clear");
  success &=
      check(decoded->mdtSegQual() == 0U, "MDT segment quality is clear");

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
