/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH_PARTICLELEVEL_JETS_ALG_H
#define TRUTH_PARTICLELEVEL_JETS_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

// Framework includes
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

#include <atomic>

namespace CP {
class ParticleLevelJetsAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{
      this, "jets", "", "the name of the input truth jets container"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "eventInfo", "EventInfo", "the name of the EventInfo container"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_truthLabelKey{
      this, "truthLabelDecoration", m_jetsKey, "HadronConeExclTruthLabelID",
      "the truth flavour label decoration on the input jets"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_decNumTruthBJetsKey{
      this, "numTruthBJetsDecoration", m_eventInfoKey,
      "num_truth_bjets_nocuts",
      "the number of truth b-jets decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_decNumTruthCJetsKey{
      this, "numTruthCJetsDecoration", m_eventInfoKey,
      "num_truth_cjets_nocuts",
      "the number of truth c-jets decoration on EventInfo"};
  mutable std::atomic<bool> m_warnedMissingLabel{false};
};

}  // namespace CP

#endif
