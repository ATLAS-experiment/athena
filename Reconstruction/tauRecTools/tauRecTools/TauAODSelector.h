/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUAODSELECTOR_H
#define TAURECTOOLS_TAUAODSELECTOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "AsgTools/PropertyWrapper.h"

class TauAODSelector : public TauRecToolBase {
 
public:
  
  ASG_TOOL_CLASS2( TauAODSelector, TauRecToolBase, ITauToolBase )
  
  TauAODSelector(const std::string& name="TauAODSelector");
  
  virtual ~TauAODSelector() = default;
  
  virtual StatusCode execute(xAOD::TauJet& tau) const override;

private:

  // minimum tau pt below which taus are not written to AOD
  Gaudi::Property<double> m_min0pTauPt{this, "Min0pTauPt", 0.};
  Gaudi::Property<double> m_minTauPt{this, "MinTauPt", 0.};
  Gaudi::Property<bool> m_doEarlyStopping{this, "doEarlyStopping", true};

};

#endif // TAURECTOOLS_TAUAODSELECTOR_H
