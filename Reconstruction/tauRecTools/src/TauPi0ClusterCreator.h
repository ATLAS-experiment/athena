/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUPI0CLUSTERCREATOR_H
#define TAURECTOOLS_TAUPI0CLUSTERCREATOR_H

#include "tauRecTools/TauRecToolBase.h"

#include "xAODPFlow/PFOContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "AsgTools/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <string>
#include <vector>
#include <map>

/**
 * @brief Creates Pi0 clusters (Pi0 Finder).
 * 
 * @author Will Davey <will.davey@cern.ch> 
 * @author Benedict Winter <benedict.tobias.winter@cern.ch> 
 * @author Stephanie Yuen <stephanie.yuen@cern.ch>
 */

class TauPi0ClusterCreator : public TauRecToolBase {

public:
  
  ASG_TOOL_CLASS2(TauPi0ClusterCreator, TauRecToolBase, ITauToolBase)
  
  TauPi0ClusterCreator(const std::string& name);

  virtual ~TauPi0ClusterCreator() = default;

  using TauRecToolBase::executeTool;
  virtual StatusCode executeTool(xAOD::TauJet& pTau,
				 const EventContext& ctx,
				 xAOD::PFOContainer& neutralPFOContainer, 
				 xAOD::PFOContainer& hadronicClusterPFOContainer,
				 const xAOD::CaloClusterContainer& pi0CaloClusContainer) const override;
  
private:
  
  /** @brief Configure the neutral PFO*/
  StatusCode configureNeutralPFO(const xAOD::CaloCluster& cluster,
                                 const xAOD::CaloClusterContainer& pi0ClusterContainer,
                                 const xAOD::TauJet& tau,
                                 const std::vector<const xAOD::PFO*>& shotPFOs,
                                 const std::map<unsigned, const xAOD::CaloCluster*>& shotsInCluster,
                                 xAOD::PFO& neutralPFO) const;

  /** @brief Configure the haronic PFO*/
  StatusCode configureHadronicPFO(const xAOD::CaloVertexedTopoCluster& cluster,
                                  double clusterEnergyHad,
                                  xAOD::PFO& hadronicPFO) const;

  std::map<unsigned, const xAOD::CaloCluster*> getShotToClusterMap(
      const std::vector<const xAOD::PFO*>& shotVector,
      std::vector<const xAOD::CaloCluster*>& goodpi0Vecor) const;

  std::vector<unsigned> getShotsMatchedToCluster(
      const std::vector<const xAOD::PFO*>& shotVector,
      const std::map<unsigned, const xAOD::CaloCluster*>& clusterToShotMap,
      const xAOD::CaloCluster& pi0Cluster) const;

  int getNPhotons( const std::vector<const xAOD::PFO*>& shotVector,
                   const std::vector<unsigned>& shotsInCluster) const;

  /** @brief eta moment in PS, EM1 and EM2 w.r.t cluster eta */  
  void getClusterVariables(const xAOD::CaloCluster& cluster,
		           std::vector<int> &nPosECells,
			   float &coreEnergyEM1, 
		           std::vector<float> &deltaEtaFirstMom,
                           std::vector<float> &secondEtaWRTClusterPositionInLayer) const;

  Gaudi::Property<double> m_clusterEtCut {this, "ClusterEtCut", 0.5 * Gaudi::Units::GeV, "Et threshould for pi0 candidate clusters"};
  Gaudi::Property<double> m_maxDeltaRNeutral {this, "MaxDeltaRNeutral", 0.2, "max DeltaR for pi0-tau association"};
  Gaudi::Property<double> m_maxDeltaRJetClust {this, "MaxDeltaRJetClust", 0.4, "max DeltaR for vertexed cluster-tau association"};
  Gaudi::Property<double> m_recoFromAOD {this, "RecoFromAOD", false, "Flag if the reconstruction is happening at AOD-level"};

};

#endif // TAURECTOOLS_TAUPI0CLUSTERCREATOR_H

