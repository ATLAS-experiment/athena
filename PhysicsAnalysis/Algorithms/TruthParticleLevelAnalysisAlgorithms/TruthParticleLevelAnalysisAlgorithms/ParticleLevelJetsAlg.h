/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH__PARTICLELEVEL_JETS__ALG_H
#define TRUTH__PARTICLELEVEL_JETS__ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>

// Framework includes
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

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
};

}  // namespace CP

#endif
