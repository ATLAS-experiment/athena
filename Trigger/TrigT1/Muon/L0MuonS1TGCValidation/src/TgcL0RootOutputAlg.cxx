/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0RootOutputAlg.h"

#include "L0MuonS1TGCValidation/TgcL0ValidationEventCheck.h"
#include "StoreGate/ReadHandle.h"

namespace L0Muon {

StatusCode TgcL0RootOutputAlg::initialize() {
  ATH_CHECK(m_inputKey.initialize());
  m_tree.setPath("L0MuonS1TGCValidation");
  ATH_CHECK(m_tree.init(this));
  return StatusCode::SUCCESS;
}

StatusCode TgcL0RootOutputAlg::execute(const EventContext& ctx) {
  SG::ReadHandle<TgcL0ValidationEvent> input{m_inputKey, ctx};
  ATH_CHECK(input.isValid());

  const TgcL0ValidationCheckResult check =
      checkTgcL0ValidationEvent(*input);
  if (!check.valid) {
    if (m_failOnInvalidData) {
      ATH_MSG_ERROR("Invalid TGC validation data: " << check.message);
      return StatusCode::FAILURE;
    }
    ATH_MSG_WARNING("Skipping invalid TGC validation data: "
                    << check.message);
    return StatusCode::SUCCESS;
  }

  copyToBranches(*input);
  return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode TgcL0RootOutputAlg::finalize() {
  ATH_CHECK(m_tree.write());
  return StatusCode::SUCCESS;
}

void TgcL0RootOutputAlg::copyToBranches(
    const TgcL0ValidationEvent& input) {
  m_runNumber = input.event.runNumber;
  m_eventNumber = input.event.eventNumber;
  m_lumiBlock = input.event.lumiBlock;
  m_bcid = input.event.bcid;

  m_truthPdgId = input.truth.pdgId;
  m_truthBarcode = input.truth.barcode;
  m_truthPt = input.truth.pt;
  m_truthEta = input.truth.eta;
  m_truthPhi = input.truth.phi;
  m_truthCharge = input.truth.charge;
  m_truthExtrapolatedStationMask = input.truth.extrapolatedStationMask;
  m_truthM1Eta = input.truth.m1Eta;
  m_truthM1Phi = input.truth.m1Phi;
  m_truthM2Eta = input.truth.m2Eta;
  m_truthM2Phi = input.truth.m2Phi;
  m_truthM3Eta = input.truth.m3Eta;
  m_truthM3Phi = input.truth.m3Phi;
  m_truthMatched = input.truth.matched;
  m_truthMatchedCandidateIndex = input.truth.matchedCandidateIndex;
  m_truthMatchMeanDeltaR = input.truth.matchMeanDeltaR;
  m_truthUnmatchedReason = input.truth.unmatchedReason;
  m_truthFinalCandidateMatched = input.truth.finalCandidateMatched;
  m_truthMatchedFinalCandidateIndex =
      input.truth.matchedFinalCandidateIndex;
  m_truthFinalCandidateMatchDeltaR =
      input.truth.finalCandidateMatchDeltaR;
  m_truthFinalCandidateUnmatchedReason =
      input.truth.finalCandidateUnmatchedReason;
  m_truthWireSegmentMatched = input.truth.wireSegmentMatched;
  m_truthStripSegmentMatched = input.truth.stripSegmentMatched;
  m_truthMatchedWireSegmentIndex = input.truth.matchedWireSegmentIndex;
  m_truthMatchedStripSegmentIndex = input.truth.matchedStripSegmentIndex;
  m_truthWireSegmentMatchResidual = input.truth.wireSegmentMatchResidual;
  m_truthStripSegmentMatchResidual = input.truth.stripSegmentMatchResidual;

  m_segmentSubdetectorId = input.segments.subdetectorId;
  m_segmentTriggerSector = input.segments.triggerSector;
  m_segmentBcTag = input.segments.bcTag;
  m_segmentProjection = input.segments.projection;
  m_segmentStationMask = input.segments.stationMask;
  m_segmentSummedQuality = input.segments.summedQuality;
  m_segmentNStations = input.segments.nStations;
  m_segmentEta = input.segments.eta;
  m_segmentPhi = input.segments.phi;
  m_segmentResidual = input.segments.residual;
  m_segmentOutputResidual = input.segments.outputResidual;
  m_segmentConsistency = input.segments.consistency;
  m_segmentPivotChannel = input.segments.pivotChannel;
  m_segmentTruthIndex = input.segments.truthIndex;
  m_segmentTruthMatchResidual = input.segments.truthMatchResidual;

  m_candidateSubdetectorId = input.candidates.subdetectorId;
  m_candidateTriggerSector = input.candidates.triggerSector;
  m_candidateReadoutSector = input.candidates.readoutSector;
  m_candidateBcTag = input.candidates.bcTag;
  m_candidateStationMask = input.candidates.stationMask;
  m_candidateWireStationMask = input.candidates.wireStationMask;
  m_candidateStripStationMask = input.candidates.stripStationMask;
  m_candidateEta = input.candidates.eta;
  m_candidatePhi = input.candidates.phi;
  m_candidateDeltaTheta = input.candidates.deltaTheta;
  m_candidateDeltaPhi = input.candidates.deltaPhi;
  m_candidatePt = input.candidates.pt;
  m_candidateThreshold = input.candidates.threshold;
  m_candidateCharge = input.candidates.charge;
  m_candidateGoodMagneticField = input.candidates.goodMagneticField;
  m_candidateTruthIndex = input.candidates.truthIndex;

  m_finalCandidateSourceCandidateIndex =
      input.finalCandidates.sourceCandidateIndex;
  m_finalCandidateReferenceStation =
      input.finalCandidates.referenceStation;
  m_finalCandidateSubdetectorId = input.finalCandidates.subdetectorId;
  m_finalCandidateTriggerSector = input.finalCandidates.triggerSector;
  m_finalCandidateBcTag = input.finalCandidates.bcTag;
  m_finalCandidateTcId = input.finalCandidates.tcId;
  m_finalCandidateRawEta = input.finalCandidates.rawEta;
  m_finalCandidateRawPhi = input.finalCandidates.rawPhi;
  m_finalCandidateEta = input.finalCandidates.eta;
  m_finalCandidatePhi = input.finalCandidates.phi;
  m_finalCandidatePtCode = input.finalCandidates.ptCode;
  m_finalCandidatePt = input.finalCandidates.pt;
  m_finalCandidateThreshold = input.finalCandidates.threshold;
  m_finalCandidateCharge = input.finalCandidates.charge;
  m_finalCandidateInnerCoincidence =
      input.finalCandidates.innerCoincidence;
  m_finalCandidateGoodMagneticField =
      input.finalCandidates.goodMagneticField;
  m_finalCandidateTruthIndex = input.finalCandidates.truthIndex;
  m_finalCandidateTruthMatchDeltaR =
      input.finalCandidates.truthMatchDeltaR;

  m_sectorLogicInputCandidateIndex =
      input.sectorLogic.inputCandidateIndex;
  m_sectorLogicCandWord = input.sectorLogic.candWord;
  m_sectorLogicCandExtraWord = input.sectorLogic.candExtraWord;
  m_sectorLogicBoardId = input.sectorLogic.boardId;
  m_sectorLogicFiberId = input.sectorLogic.fiberId;
  m_sectorLogicBcidOffset = input.sectorLogic.bcidOffset;
  m_sectorLogicVeto = input.sectorLogic.veto;
}

}  // namespace L0Muon
