/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUPI0SCORECALCULATOR_H
#define TAURECTOOLS_TAUPI0SCORECALCULATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/BDTHelper.h"

#include "xAODPFlow/PFO.h"
#include "AsgTools/PropertyWrapper.h"

#include <string>

/**
 * @brief Selectes pi0Candidates (Pi0 Finder).
 * 
 * @author Veit Scharf
 * @author Will Davey <will.davey@cern.ch> 
 * @author Benedict Winter <benedict.tobias.winter@cern.ch> 
 * @author Stephanie Yuen <stephanie.yuen@cern.ch>
 */

class TauPi0ScoreCalculator : public TauRecToolBase {

public:

  ASG_TOOL_CLASS2(TauPi0ScoreCalculator, TauRecToolBase, ITauToolBase)
  
  TauPi0ScoreCalculator(const std::string& name);
  virtual ~TauPi0ScoreCalculator() = default;

  virtual StatusCode initialize() override;

  using TauRecToolBase::executeTool;
  virtual StatusCode executeTool(xAOD::TauJet& pTau,
				 const EventContext& ctx,
				 xAOD::PFOContainer& pNeutralPFOContainer) const override;

private:
  
  Gaudi::Property<std::string> m_weightfile{this, "BDTWeightFile", ""}; 

  /** @brief Calculate pi0 BDT score */
  float calculateScore(const xAOD::PFO* neutralPFO) const;

  std::unique_ptr<tauRecTools::BDTHelper> m_mvaBDT = nullptr;
};

#endif	// TAURECTOOLS_TAUPI0SCORECALCULATOR_H

