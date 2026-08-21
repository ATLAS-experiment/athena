/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCVALIDATION_TGCL0ROOTOUTPUTALG_H
#define L0MUONS1TGCVALIDATION_TGCL0ROOTOUTPUTALG_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "L0MuonS1TGCValidation/TgcL0ValidationEvent.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "StoreGate/ReadHandleKey.h"

#include <cstdint>

namespace L0Muon {

/** @brief MuonTester backend for event-level TGC truth-validation data.
 *
 * The transient validation object remains the boundary between reconstruction
 * and output.  Consequently, MuonTester dependencies are confined to the
 * validation package and do not enter the reconstruction packages.
 */
class TgcL0RootOutputAlg final : public AthHistogramAlgorithm {
 public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

 private:
  void copyToBranches(const TgcL0ValidationEvent& input);

  SG::ReadHandleKey<TgcL0ValidationEvent> m_inputKey{
      this, "InputKey", "L0MuonTGCValidationEvent",
      "Calculated event-local truth-validation data"};
  Gaudi::Property<bool> m_failOnInvalidData{
      this, "FailOnInvalidData", true,
      "Fail instead of skipping structurally inconsistent validation data"};

  MuonVal::MuonTesterTree m_tree{"truthValidation", "L0MUONTGCVALID"};

  MuonVal::ScalarBranch<std::uint32_t>& m_runNumber{
      m_tree.newScalar<std::uint32_t>("runNumber")};
  MuonVal::ScalarBranch<std::uint64_t>& m_eventNumber{
      m_tree.newScalar<std::uint64_t>("eventNumber")};
  MuonVal::ScalarBranch<std::uint32_t>& m_lumiBlock{
      m_tree.newScalar<std::uint32_t>("lumiBlock")};
  MuonVal::ScalarBranch<std::uint32_t>& m_bcid{
      m_tree.newScalar<std::uint32_t>("bcid")};

#define TGCL0_VECTOR_BRANCH(TYPE, MEMBER, NAME) \
  MuonVal::VectorBranch<TYPE>& MEMBER{m_tree.newVector<TYPE>(NAME)}

  TGCL0_VECTOR_BRANCH(int, m_truthPdgId, "truthPdgId");
  TGCL0_VECTOR_BRANCH(int, m_truthBarcode, "truthBarcode");
  TGCL0_VECTOR_BRANCH(float, m_truthPt, "truthPt");
  TGCL0_VECTOR_BRANCH(float, m_truthEta, "truthEta");
  TGCL0_VECTOR_BRANCH(float, m_truthPhi, "truthPhi");
  TGCL0_VECTOR_BRANCH(float, m_truthCharge, "truthCharge");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_truthExtrapolatedStationMask,
                      "truthExtrapolatedStationMask");
  TGCL0_VECTOR_BRANCH(float, m_truthM1Eta, "truthM1Eta");
  TGCL0_VECTOR_BRANCH(float, m_truthM1Phi, "truthM1Phi");
  TGCL0_VECTOR_BRANCH(float, m_truthM2Eta, "truthM2Eta");
  TGCL0_VECTOR_BRANCH(float, m_truthM2Phi, "truthM2Phi");
  TGCL0_VECTOR_BRANCH(float, m_truthM3Eta, "truthM3Eta");
  TGCL0_VECTOR_BRANCH(float, m_truthM3Phi, "truthM3Phi");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_truthMatched, "truthMatched");
  TGCL0_VECTOR_BRANCH(int, m_truthMatchedCandidateIndex,
                      "truthMatchedCandidateIndex");
  TGCL0_VECTOR_BRANCH(float, m_truthMatchMeanDeltaR,
                      "truthMatchMeanDeltaR");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_truthUnmatchedReason,
                      "truthUnmatchedReason");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_truthWireSegmentMatched,
                      "truthWireSegmentMatched");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_truthStripSegmentMatched,
                      "truthStripSegmentMatched");
  TGCL0_VECTOR_BRANCH(int, m_truthMatchedWireSegmentIndex,
                      "truthMatchedWireSegmentIndex");
  TGCL0_VECTOR_BRANCH(int, m_truthMatchedStripSegmentIndex,
                      "truthMatchedStripSegmentIndex");
  TGCL0_VECTOR_BRANCH(float, m_truthWireSegmentMatchResidual,
                      "truthWireSegmentMatchResidual");
  TGCL0_VECTOR_BRANCH(float, m_truthStripSegmentMatchResidual,
                      "truthStripSegmentMatchResidual");

  TGCL0_VECTOR_BRANCH(std::uint16_t, m_segmentSubdetectorId,
                      "segmentSubdetectorId");
  TGCL0_VECTOR_BRANCH(std::uint16_t, m_segmentTriggerSector,
                      "segmentTriggerSector");
  TGCL0_VECTOR_BRANCH(std::uint16_t, m_segmentBcTag, "segmentBcTag");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_segmentProjection,
                      "segmentProjection");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_segmentStationMask,
                      "segmentStationMask");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_segmentSummedQuality,
                      "segmentSummedQuality");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_segmentNStations,
                      "segmentNStations");
  TGCL0_VECTOR_BRANCH(float, m_segmentEta, "segmentEta");
  TGCL0_VECTOR_BRANCH(float, m_segmentPhi, "segmentPhi");
  TGCL0_VECTOR_BRANCH(float, m_segmentResidual, "segmentResidual");
  TGCL0_VECTOR_BRANCH(float, m_segmentOutputResidual,
                      "segmentOutputResidual");
  TGCL0_VECTOR_BRANCH(float, m_segmentConsistency, "segmentConsistency");
  TGCL0_VECTOR_BRANCH(std::uint16_t, m_segmentPivotChannel,
                      "segmentPivotChannel");
  TGCL0_VECTOR_BRANCH(int, m_segmentTruthIndex, "segmentTruthIndex");
  TGCL0_VECTOR_BRANCH(float, m_segmentTruthMatchResidual,
                      "segmentTruthMatchResidual");

  TGCL0_VECTOR_BRANCH(std::uint16_t, m_candidateSubdetectorId,
                      "candidateSubdetectorId");
  TGCL0_VECTOR_BRANCH(std::uint16_t, m_candidateTriggerSector,
                      "candidateTriggerSector");
  TGCL0_VECTOR_BRANCH(std::uint16_t, m_candidateReadoutSector,
                      "candidateReadoutSector");
  TGCL0_VECTOR_BRANCH(std::uint16_t, m_candidateBcTag, "candidateBcTag");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_candidateStationMask,
                      "candidateStationMask");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_candidateWireStationMask,
                      "candidateWireStationMask");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_candidateStripStationMask,
                      "candidateStripStationMask");
  TGCL0_VECTOR_BRANCH(float, m_candidateEta, "candidateEta");
  TGCL0_VECTOR_BRANCH(float, m_candidatePhi, "candidatePhi");
  TGCL0_VECTOR_BRANCH(float, m_candidateDeltaTheta, "candidateDeltaTheta");
  TGCL0_VECTOR_BRANCH(float, m_candidateDeltaPhi, "candidateDeltaPhi");
  TGCL0_VECTOR_BRANCH(float, m_candidatePt, "candidatePt");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_candidateThreshold,
                      "candidateThreshold");
  TGCL0_VECTOR_BRANCH(std::int8_t, m_candidateCharge, "candidateCharge");
  TGCL0_VECTOR_BRANCH(std::uint8_t, m_candidateGoodMagneticField,
                      "candidateGoodMagneticField");
  TGCL0_VECTOR_BRANCH(int, m_candidateTruthIndex, "candidateTruthIndex");

#undef TGCL0_VECTOR_BRANCH
};

}  // namespace L0Muon

#endif
