/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COMMISSIONREC_COMTIMEREC
#define COMMISSIONREC_COMTIMEREC

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CosmicTriggerTimeTool.h"

class ComTimeRec : public AthAlgorithm
{

public:

  ComTimeRec(const std::string& name, ISvcLocator* pSvcLocator);
  ~ComTimeRec();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:

  std::string m_comTimeKey;
  std::string m_mode;
  std::string m_method;
  std::string m_TTRKey;
  double m_samplingPeriod;
  double m_velocity;
  CosmicTriggerTimeTool* m_triggerTimeTool;

};

#endif
