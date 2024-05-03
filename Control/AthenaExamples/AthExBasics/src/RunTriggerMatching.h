/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**@brief Algorithm demonstrating trigger matching in Athena
          User provides a trigger string (wildcards possible) and the name of an offline object 
	  container, and the algorithm counts how many offline objects are matched to that trigger */

#ifndef ATHEXBASICS_RUNTRIGGERMATCHING_H
#define ATHEXBASICS_RUNTRIGGERMATCHING_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/R3MatchingTool.h"
#include "xAODBase/IParticleContainer.h"
#include <string>
#include <atomic>

class RunTriggerMatching : public AthReentrantAlgorithm {
 public:
  RunTriggerMatching(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& /*ctx*/) const override;
  virtual StatusCode finalize() override;

 private: 
    /** Integer counter for the requested triggers */ 
    mutable std::atomic<unsigned int> m_matchCounter{0};
    /** String to select triggers to use in the matching with offline objects */
    Gaudi::Property<std::string> m_triggerString{this, "TriggerString", "HLT_mu24.*"};
    /** Read handle for the offline object container - set to muons by default. */
    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerKey{this, "ContainerName", "Muons"};
    /** Tool handle for the trigger decision and matching tools */
    PublicToolHandle<Trig::TrigDecisionTool> m_trigDec{this, "TriggerDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool"};
    PublicToolHandle<Trig::R3MatchingTool> m_matchingTool{this, "R3MatchingTool", "Trig::R3MatchingTool", "R3MatchingTool"};
};

#endif
