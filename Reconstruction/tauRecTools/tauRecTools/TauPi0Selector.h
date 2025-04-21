/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUPI0SELECTOR_H
#define	TAURECTOOLS_TAUPI0SELECTOR_H

#include "tauRecTools/TauRecToolBase.h"

#include "AsgTools/PropertyWrapper.h"

#include <string>

/**
 * @brief Apply Et and BDT score cut to pi0s
 * 
 * @author Will Davey <will.davey@cern.ch> 
 * @author Benedict Winter <benedict.tobias.winter@cern.ch> 
 * @author Stephanie Yuen <stephanie.yuen@cern.ch>
 */

class TauPi0Selector : public TauRecToolBase {

public:
  
  ASG_TOOL_CLASS2(TauPi0Selector, TauRecToolBase, ITauToolBase)
  
  TauPi0Selector(const std::string& name);
  virtual ~TauPi0Selector() = default;
  
  virtual StatusCode executePi0nPFO(xAOD::TauJet& pTau, xAOD::PFOContainer& pNeutralPFOContainer) const override;

private:
  /** @brief Get eta bin of Pi0Cluster */
  int getEtaBin(double eta) const;

  Gaudi::Property<std::vector<double>> m_clusterEtCut{this, "ClusterEtCut", {}};
  Gaudi::Property<std::vector<double>> m_clusterBDTCut_1prong{this, "ClusterBDTCut_1prong", {}};
  Gaudi::Property<std::vector<double>> m_clusterBDTCut_mprong{this, "ClusterBDTCut_mprong", {}};

};

#endif	// TAURECTOOLS_TAUPI0SELECTOR_H
