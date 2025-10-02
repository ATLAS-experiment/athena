/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Truth3CollectionMaker.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TRUTH3COLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTH3COLLECTIONMAKER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"
class IMCTruthClassifier;

namespace DerivationFramework {

  class Truth3CollectionMaker : public extends<ExpressionParserUser<AthAlgTool>, IAugmentationTool> {
    public: 
      Truth3CollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
      ~Truth3CollectionMaker();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      virtual StatusCode addBranches() const override;

    private:
      mutable std::atomic<unsigned int> m_ntotpart, m_npasspart;
      std::string m_particlesKey;
      //std::string m_verticesKey;
      std::string m_collectionName; 
      std::string m_partString;
      ToolHandle<IMCTruthClassifier> m_classifier;
      bool m_runClassifier;
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTH3COLLECTIONMAKER_H
