/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFSUBTRACTIONTRUTHTOOL_H
#define PFSUBTRACTIONTRUTHTOOL_H

#include "PFSubtractionTool.h"
#include "PFSimulateTruthShowerTool.h"

class eflowCaloObject;
struct PFData;


class PFSubtractionTruthTool : public PFSubtractionTool{

public:
  using PFSubtractionTool::PFSubtractionTool;
  ~PFSubtractionTruthTool();

  virtual StatusCode initialize() override;
  virtual StatusCode processPFlowData(const EventContext& ctx, PFData &thePFData) const override;

private:

  void performSubtraction(const EventContext& ctx, const unsigned int& startingPoint, const unsigned int& nCaloObj, PFData &data ) const override;
  void performSubtraction(eflowCaloObject& thisEflowCaloObject) const override;

  /**Toggle whether we fully remove a cell with a truth deposit or reweight it based on truth contribution */
  Gaudi::Property<bool> m_useFullCellTruthSubtraction{this,"useFullCellTruthSubtraction",true,"Toggle whether we fully remove a cell with a truth deposit or reweight it based on truth contribution"};

  ToolHandle<PFSimulateTruthShowerTool> m_theTruthShowerSimulator{this, "PFSimulateTruthShowerTool", "", "The truth shower simulator"};


};

#endif
