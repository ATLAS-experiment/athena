/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFRADIALENERGYCALCULATORTOOL_H
#define PFRADIALENERGYCALCULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "eflowRec/eflowCaloObject.h"
#include "eflowRec/IPFBaseTool.h"

class PFRadialEnergyCalculatorTool : public extends<AthAlgTool, IPFBaseTool> {

  public:
  
  using base_class::base_class;

  ~PFRadialEnergyCalculatorTool() {};

  virtual StatusCode execute(eflowCaloObjectContainer& theEflowCaloObjectContainer) override;

};

#endif