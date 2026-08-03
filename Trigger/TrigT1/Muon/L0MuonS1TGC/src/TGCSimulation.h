/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGC_TGCSIMULATION_H
#define L0MUONS1TGC_TGCSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0CandidateBuilderTool.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0InnerCoincidenceTool.h"
#include "L0MuonS1TGCToolInterfaces/ITgcL0TrackSelectorTool.h"
#include "L0MuonS1TGCToolInterfaces/TgcL0Segment.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"

namespace L0Muon {

class TGCSimulation : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<TgcRdoContainer> m_keyTgcRdo{
      this, "InputRdo", "TGCRDO", "Location of the input TGC RDO container"};
  SG::WriteHandleKey<TgcL0CandidateContainer> m_validationCandidateKey{
      this, "ValidationCandidateKey", "",
      "Optional pre-Inner-Coincidence transient candidates for validation"};
  SG::WriteHandleKey<TgcL0SegmentContainer> m_validationSegmentKey{
      this, "ValidationSegmentKey", "",
      "Optional projection segments for validation"};
  SG::WriteHandleKey<xAOD::TGCCandDataContainer> m_outputKey{
      this, "OutputKey", "L0MuonTGCCandData",
      "TGC Sector Logic candidate output"};

  ToolHandle<ITgcL0CandidateBuilderTool> m_candidateBuilderTool{
      this, "CandidateBuilderTool", "", "TGC candidate-builder implementation"};
  ToolHandle<ITgcL0InnerCoincidenceTool> m_innerCoincidenceTool{
      this, "InnerCoincidenceTool", "", "TGC Inner-Coincidence implementation"};
  ToolHandle<ITgcL0TrackSelectorTool> m_trackSelectorTool{
      this, "TrackSelectorTool", "", "TGC Track-Selector implementation"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "",
                                               "Monitoring tool"};
};

}  // namespace L0Muon

#endif
