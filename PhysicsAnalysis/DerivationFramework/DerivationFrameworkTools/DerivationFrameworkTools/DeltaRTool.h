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

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    Gaudi::Property<std::string> m_expression{this, "ObjectRequirements", ""};
    Gaudi::Property<std::string> m_2ndExpression{this, "SecondObjectRequirements", ""};
    SG::WriteHandleKey<std::vector<float> > m_sgName {this,"StoreGateEntryName","","SG key of output object"};
    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerName  {this,"ContainerName","","SG key of first container"};
    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerName2 {this,"SecondContainerName","","SG key of first container"};

    StatusCode getDeltaRs(std::vector<float>*, const EventContext& ctx) const;
    static float calculateDeltaR(float,float,float,float) ;
  };
}

#endif // DERIVATIONFRAMEWORK_DELTARTOOL_H
