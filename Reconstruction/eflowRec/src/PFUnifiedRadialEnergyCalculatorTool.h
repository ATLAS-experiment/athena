/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFUNIFIEDRADIALENERGYCALCULATORTOOL_H
#define PFUNIFIEDRADIALENERGYCALCULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "IPFUnifiedBaseTool.h"

struct PFData;


class PFUnifiedRadialEnergyCalculatorTool : public extends<AthAlgTool, IPFUnifiedBaseTool> {

  public:
  
  using base_class::base_class;

  ~PFUnifiedRadialEnergyCalculatorTool() {};

  virtual StatusCode processPFlowData(const EventContext& ctx, PFData &thePFData) const override;

};

#endif