/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InvariantMassTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_INVARIANTMASSTOOL_H
#define DERIVATIONFRAMEWORK_INVARIANTMASSTOOL_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODBase/IParticleContainer.h"
#include <string>
#include <vector>

class TVector3;
class EventContext;

namespace DerivationFramework {

 enum  EInvariantMassToolParser { kInvariantMassToolParser1, kInvariantMassToolParser2, kInvariantMassToolParserNum };
 class InvariantMassTool : public extends<ExpressionParserUser<AthAlgTool,kInvariantMassToolParserNum>, IAugmentationTool> {
    public: 
      InvariantMassTool(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches(const EventContext& ctx) const;

    private:
      std::string m_expression;
      std::string m_expression2;
      SG::WriteHandleKey<std::vector<float> > m_sgName {this,"StoreGateEntryName","","SG key of output object"};
      float m_massHypothesis,m_massHypothesis2;
      SG::ReadHandleKey<xAOD::IParticleContainer> m_containerName  {this,"ContainerName","","SG key of first container"};
      SG::ReadHandleKey<xAOD::IParticleContainer> m_containerName2 {this,"SecondContainerName","","SG key of second container"};
      SG::ReadDecorHandleKeyArray<xAOD::IParticleContainer> m_inputDecorNames {this, "InputDecorNames",{},"SG keys for decorations of first (and second) container(s)"};
      StatusCode getInvariantMasses(std::vector<float>*, const EventContext& ctx) const;
      static float calculateInvariantMass(const TVector3& v1, const TVector3&v2,float M1,float M2) ;
  }; 
}

#endif // DERIVATIONFRAMEWORK_INVARIANTMASSTOOL_H
