/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**@brief Algorithm demonstrating use of trigger decision information.
          Demonstrates how to read the trigger decision tool in Athena using xAOD input. 
	  Algorithm counts up the total number of events for which a requested trigger fires */

#ifndef ATHEXBASICS_READTRIGGERDECISION_H
#define ATHEXBASICS_READTRIGGERDECISION_H

#include <string>
#include <atomic>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

class ReadTriggerDecision : public AthReentrantAlgorithm {
 public:
  ReadTriggerDecision(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

 private:
    /** Name of the trigger to count */
    Gaudi::Property<std::string> m_triggerName
      {this, "TriggerName", "HLT_e5_idperf_tight_L1EM3", "Name of the trigger to be counted"}; 
    /** Integer counter for the requested trigger */ 
    mutable std::atomic<unsigned int> m_triggerCounter{0};
    /** Tool handle for the trigger decision tool */
    PublicToolHandle<Trig::TrigDecisionTool> m_trigDec{this, "TriggerDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool"};
};

#endif
