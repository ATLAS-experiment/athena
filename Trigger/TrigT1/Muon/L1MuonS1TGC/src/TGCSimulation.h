/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGC_TGCSIMULATION_H
#define L1MUONS1TGC_TGCSIMULATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "L1MuonS1TGCToolInterfaces/ICandBuilderTool.h"
#include "L1MuonS1TGCToolInterfaces/IInnerCoincidenceTool.h"
#include "L1MuonS1TGCToolInterfaces/ITrackSelectorTool.h"
#include "L1MuonS1TGCToolInterfaces/Segment.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTrigL1Muon/L1TGCCandDataContainer.h"

namespace L1Muon {

class TGCSimulation : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<TgcRdoContainer> m_keyTgcRdo{
      this, "InputRdo", "TGCRDO", "Location of the input TGC RDO container"};
  SG::WriteHandleKey<TgcSLCandidateContainer> m_validationCandidateKey{
      this, "ValidationCandidateKey", "",
      "Optional pre-Inner-Coincidence transient candidates for validation"};
  SG::WriteHandleKey<L1TGC::SegmentContainer> m_validationSegmentKey{
      this, "ValidationSegmentKey", "",
      "Optional projection segments for validation"};
  SG::WriteHandleKey<xAOD::L1TGCCandDataContainer> m_outputKey{
      this, "OutputKey", "L1TGCCandData",
      "TGC Sector Logic candidate output"};
  SG::WriteHandleKey<xAOD::L1TGCCandDataContainer> m_outputToMdtKey{
      this, "OutputToMdtKey", "L1TGCCandDataToMdt",
      "View of the highest-pT TGC candidates sent to MDTTP"};

  ToolHandle<L1TGC::ICandBuilderTool> m_candidateBuilderTool{
      this, "CandidateBuilderTool", "", "TGC candidate-builder implementation"};
  ToolHandle<L1TGC::IInnerCoincidenceTool> m_innerCoincidenceTool{
      this, "InnerCoincidenceTool", "", "TGC Inner-Coincidence implementation"};
  ToolHandle<L1TGC::ITrackSelectorTool> m_trackSelectorTool{
      this, "TrackSelectorTool", "", "TGC Track-Selector implementation"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "",
                                               "Monitoring tool"};
};

}  // namespace L1Muon

#endif
