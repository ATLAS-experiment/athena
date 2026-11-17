/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXMONITORED_MONITOREDALG_H
#define ATHEXMONITORED_MONITOREDALG_H 1

#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include <string>

class MonitoredAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring tool"};
  uint32_t m_seed = 1234;
};


#endif
