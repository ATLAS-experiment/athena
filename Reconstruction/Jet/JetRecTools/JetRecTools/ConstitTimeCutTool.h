/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETRECTOOLS_CONSTITTIMECUTTOOL_H
#define JETRECTOOLS_CONSTITTIMECUTTOOL_H

// \class ConstitTimeCutTool
// \author Jennifer Roloff
// \date October 2016
//
// This tool places cuts on constituents based on their timing information


#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODBase/IParticleContainer.h"
#include "AthContainers/CurrentContext.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODPFlow/FlowElementContainer.h"

#include <string>

class ConstitTimeCutTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(ConstitTimeCutTool, IJetConstituentModifier)

  public:
  
  ConstitTimeCutTool(const std::string& name);

  // Check that the configuration is sane
  StatusCode initialize() override final;

  private:
  StatusCode process_impl(xAOD::IParticleContainer* cont, const EventContext& ctx) const override final; 
  StatusCode applyTimingCut(xAOD::IParticle* part, const float& time, const float& quality, const float& lambda_center) const; 

  // CaloCluster decorations
  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer>
    m_clusterLarQKey{
      this,
      "ClusterLarQKey",
      "AVG_LAR_Q",
      "AVG_LAR_Q decoration"
    };

  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer>
    m_clusterLambdaKey{
      this,
      "ClusterLambdaKey",
      "CENTER_LAMBDA",
      "CENTER_LAMBDA decoration"
    };

  // FlowElement decorations
  SG::ReadDecorHandleKey<xAOD::FlowElementContainer>
    m_feTimingKey{
      this,
      "FETimingKey",
      "TIMING",
      "TIMING decoration"
    };

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer>
    m_feLarQKey{
      this,
      "FELarQKey",
      "AVG_LAR_Q",
      "AVG_LAR_Q decoration"
    };

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer>
    m_feLambdaKey{
      this,
      "FELambdaKey",
      "CENTER_LAMBDA",
      "CENTER_LAMBDA decoration"
    };

  // Properties.
  float m_lambdaCalDivide; 
  float m_qualityCut; 
  float m_timeCutLargeQ;
  float m_timeCutSmallQ; 
  float m_etaMax;
		
};


#endif
