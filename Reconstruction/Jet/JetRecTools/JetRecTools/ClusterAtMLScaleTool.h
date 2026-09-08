/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETRECTOOLS_CLUSTERATMLSCALETOOL_H
#define JETRECTOOLS_CLUSTERATMLSCALETOOL_H

////////////////////////////////////////////
/// \class ClusterAtMLScaleTool
///
/// Set ClusterML corrected energy as calibrated energy which is used in the jet algorithm.
/// ML corrected energy is stored in CaloClusters as decoration.
///
//////////////////////////////////////////////////


#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "AsgTools/AsgTool.h" 
#include <string>
#include "StoreGate/ReadDecorHandleKey.h"


class ClusterAtMLScaleTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(ClusterAtMLScaleTool, IJetConstituentModifier)

  public:

  ClusterAtMLScaleTool(const std::string& name);
  ~ClusterAtMLScaleTool();

  // Check that the configuration is reasonable
  virtual StatusCode initialize() override;
  
  private:
  // Property to configure the ML energy decoration key
  
  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer> m_clusterMLCorrectedEnergyKey{
    this,
    "ClusterMLCorrectedEnergyDecorationKey",
    "CaloCalTopoClusters.clusterE_ML",
    "Decoration storing ML-corrected cluster energy"
  };

  // Property to configure the ML energy decoration type
  Gaudi::Property<std::string> m_clusterMLCorrectedEnergyDecorationType{
    this,
    "ClusterMLCorrectedEnergyDecorationType",
    "double",
    "Type of the ML-corrected cluster energy decoration"
  };

  // Implement the correction
  virtual StatusCode process_impl(xAOD::IParticleContainer* cont) const override; 
  StatusCode setClustersToMLScale(xAOD::CaloClusterContainer& cont) const;


  template <typename T>
  StatusCode setClustersToMLScaleImpl(
      xAOD::CaloClusterContainer& cont,
      const EventContext& ctx) const;
		
};


#endif
