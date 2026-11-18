/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFRADIALENERGYCALCULATORTOOL_H
#define PFRADIALENERGYCALCULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "IPFBaseTool.h"

struct PFData;


class PFRadialEnergyCalculatorTool : public extends<AthAlgTool, IPFBaseTool> {

  public:
  
  using base_class::base_class;

  ~PFRadialEnergyCalculatorTool() {};

  virtual StatusCode processPFlowData(const EventContext& ctx, PFData &thePFData) const override;

};

#endif
