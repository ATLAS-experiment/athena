/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCVALIDATION_TGCL0TRUTHVALIDATIONALG_H
#define L0MUONS1TGCVALIDATION_TGCL0TRUTHVALIDATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GeneratorObjects/McEventCollection.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Segment.h"
#include "L0MuonS1TGCValidation/TgcL0ValidationEvent.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"
#include "xAODTrigL0Muon/SectorLogicCandDataContainer.h"

#include <cstdint>
#include <vector>

namespace L0Muon {

/** @brief Calculates event-local TGC validation quantities.
 *
 * The algorithm has no ROOT or histogram dependency. Its transient output can
 * be consumed by replaceable validation-output backends. The optional Sector
 * Logic check validates the downstream conversion without affecting normal
 * truth-validation users.
 */
class TgcL0TruthValidationAlg final : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<TgcL0CandidateContainer> m_candidateKey{
      this, "CandidateKey", "L0MuonTGCValidationCandidates",
      "Pre-Inner-Coincidence transient candidates"};
  SG::ReadHandleKey<TgcL0SegmentContainer> m_segmentKey{
      this, "SegmentKey", "L0MuonTGCValidationSegments",
      "Projection segments published by reconstruction"};
  SG::ReadHandleKey<McEventCollection> m_truthEventKey{
      this, "TruthEventKey", "TruthEvent", "Input truth-event collection"};
  SG::ReadHandleKey<xAOD::TGCCandDataContainer> m_finalCandidateKey{
      this, "FinalCandidateKey", "L0MuonTGCCandData",
      "Final TGC candidates sent to L0MuonEndcap"};
  SG::ReadHandleKey<xAOD::SectorLogicCandDataContainer> m_sectorLogicKey{
      this, "SectorLogicKey", "L0MuonTGCSectorLogicCandData",
      "TGC Sector Logic candidates sent towards MuCTPI"};
  SG::WriteHandleKey<TgcL0ValidationEvent> m_outputKey{
      this, "OutputKey", "L0MuonTGCValidationEvent",
      "Calculated event-local truth-validation data"};
  ToolHandle<Trk::IExtrapolator> m_extrapolator{
      this, "TrackExtrapolator", "", "Truth-only extrapolator"};

  Gaudi::Property<float> m_maxMeanDeltaR{
      this, "TruthMatchMaxMeanDeltaR", 0.08F,
      "Maximum station-averaged deltaR for truth matching"};
  Gaudi::Property<bool> m_validateSectorLogic{
      this, "ValidateSectorLogic", false,
      "Validate and publish the TGC Sector Logic conversion"};
  Gaudi::Property<float> m_maxWireSegmentDeltaEta{
      this, "TruthWireSegmentMaxDeltaEta", 0.08F,
      "Maximum |delta eta| for truth-to-wire-segment matching"};
  Gaudi::Property<float> m_maxStripSegmentDeltaPhi{
      this, "TruthStripSegmentMaxDeltaPhi", 0.08F,
      "Maximum |delta phi| for truth-to-strip-segment matching"};
  Gaudi::Property<double> m_minPt{
      this, "MinTruthPt", 2000.0, "Minimum truth-muon pT in MeV"};
  Gaudi::Property<double> m_minAbsEta{
      this, "MinTruthAbsEta", 0.95, "Minimum absolute truth eta"};
  Gaudi::Property<double> m_maxAbsEta{
      this, "MaxTruthAbsEta", 2.50, "Maximum absolute truth eta"};
  Gaudi::Property<int> m_requiredTruthStatus{
      this, "RequiredTruthStatus", 1,
      "Generator status required for selected truth muons"};
  Gaudi::Property<int> m_maxAbsBarcode{
      this, "MaxAbsBarcode", 9999,
      "Maximum absolute HepMC barcode for selected truth muons"};
  Gaudi::Property<std::uint16_t> m_requiredBcTagMask{
      this, "RequiredBcTagMask", 0x2U,
      "Required BC-tag bit mask for truth matching; zero disables the filter"};
  Gaudi::Property<std::vector<double>> m_stationAbsZ{
      this, "StationAbsZ", {13436.5, 14728.2, 15148.2},
      "Nominal absolute z positions of the M1, M2, and M3 validation planes"};
  Gaudi::Property<double> m_validationMinR{
      this, "ValidationMinR", 0.0,
      "Minimum radius of the truth-extrapolation validation planes"};
  Gaudi::Property<double> m_validationMaxR{
      this, "ValidationMaxR", 11977.0,
      "Maximum radius of the truth-extrapolation validation planes"};
  Gaudi::Property<double> m_validationPlaneToleranceZ{
      this, "ValidationPlaneToleranceZ", 20.0,
      "Maximum distance in z from an extrapolation target plane"};
};

}  // namespace L0Muon

#endif
