/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// AugmentationToolLeadingJets.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Louie Corpe (lcorpe@cern.ch)
//

#include "DerivationFrameworkLLP/AugmentationToolLeadingJets.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>
#include <string>

namespace DerivationFramework {

  AugmentationToolLeadingJets::AugmentationToolLeadingJets(const std::string& t,
      const std::string& n,
      const IInterface* p) : 
    base_class(t,n,p)
  {
  }

  StatusCode AugmentationToolLeadingJets::initialize()
  {
    ATH_CHECK( AthAlgTool::initialize() );
    ATH_CHECK( m_jetKey.initialize() );
    ATH_CHECK( m_decorationKey.initialize() );
    return StatusCode::SUCCESS;
  }

  StatusCode AugmentationToolLeadingJets::addBranches() const
  {

      // Set up the decorators
      SG::WriteDecorHandle<xAOD::JetContainer, bool> decorator (m_decorationKey);

      // CALCULATION OF THE NEW VARIABLE
      // Get Primary vertex
      SG::ReadHandle<xAOD::JetContainer> jets (m_jetKey);
      int counter=0;
      for ( unsigned int i =0 ; i < jets->size() ; i++){
       auto jet = (*jets)[i] ;
       if (fabs(jet->eta()) < 2.5){
         decorator(*jet) = (counter <2); // pick the two leading jets only 
         counter+=1;
       } else {
         decorator(*jet) = 0; // pick the two leading jets only 
       }
      }
      return StatusCode::SUCCESS;
  }
}
