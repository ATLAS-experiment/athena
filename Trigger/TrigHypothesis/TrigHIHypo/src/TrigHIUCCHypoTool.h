/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGHIHYPO_TRIGHIUCCHYPOTOOL_H
#define TRIGHIHYPO_TRIGHIUCCHYPOTOOL_H

// Package includes
#include <GaudiKernel/StatusCode.h>
#include "ITrigHIEventShapeHypoTool.h"


// Framework includes
#include "AthenaBaseComps/AthAlgTool.h"

// STL includes
#include <string>

/**
 * @class TrigHIUCCHypoTool
 * @brief the tool to select Ultra Central Collisions (applies a cut on calo energy)
 **/
class TrigHIUCCHypoTool : public extends<AthAlgTool, ITrigHIEventShapeHypoTool> {
public:
  TrigHIUCCHypoTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~TrigHIUCCHypoTool() override {}

  virtual StatusCode initialize() override { return StatusCode::SUCCESS; }
  virtual StatusCode finalize() override  { return StatusCode::SUCCESS; }
  
  virtual StatusCode decide(const xAOD::HIEventShapeContainer*, bool&) const = 0;
  virtual const HLT::Identifier& getId() const = 0;

private:
  HLT::Identifier m_decisionId;
  Gaudi::Property<float> m_FCalEtThreshold{this, "FCalEtThreshold", -1, "Threshold applied on ET from FCal"};
};

#endif // TRIGHIHYPO_TRIGHIUCCHYPOTOOL_H
