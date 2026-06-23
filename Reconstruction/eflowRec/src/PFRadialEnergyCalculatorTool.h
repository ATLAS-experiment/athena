/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_PFRADIALENERGYCALCULATORTOOL_H
#define EFLOWREC_PFRADIALENERGYCALCULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "eflowCaloObject.h"
#include "IPFBaseTool.h"

class PFRadialEnergyCalculatorTool : public extends<AthAlgTool, IPFBaseTool> {

  public:
  
  using base_class::base_class;

  ~PFRadialEnergyCalculatorTool() {};

  virtual StatusCode execute(const EventContext& ctx, eflowCaloObjectContainer& theEflowCaloObjectContainer) override;

};

#endif
