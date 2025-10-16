/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_DELTARTOOL_H
#define DERIVATIONFRAMEWORK_DELTARTOOL_H



#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "ExpressionEvaluation/ExpressionParserUser.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODBase/IParticleContainer.h"

#include <vector>
#include <string>
class EventContext;

namespace DerivationFramework {

  enum EDeltaRToolParser {kDeltaRToolParser1,kDeltaRToolParser2,kDeltaRToolParserNum};
  class DeltaRTool : public extends<ExpressionParserUser<AthAlgTool,kDeltaRToolParserNum>, IAugmentationTool> {
    public:
      DeltaRTool(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
      std::string m_expression;
      std::string m_2ndExpression;
      SG::WriteHandleKey<std::vector<float> > m_sgName {this,"StoreGateEntryName","","SG key of output object"};
      SG::ReadHandleKey<xAOD::IParticleContainer> m_containerName  {this,"ContainerName","","SG key of first container"};
      SG::ReadHandleKey<xAOD::IParticleContainer> m_containerName2 {this,"SecondContainerName","","SG key of first container"};
      
      StatusCode getDeltaRs(std::vector<float>*, const EventContext& ctx) const;
      static float calculateDeltaR(float,float,float,float) ;
  }; 
}

#endif // DERIVATIONFRAMEWORK_DELTARTOOL_H
