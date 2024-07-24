/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// AugmentationToolLeadingJets.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Louie Corpe (lcorpe@cern.ch)
//

#include "DerivationFrameworkLLP/AugmentationToolLeadingJets.h"
#include "xAODJet/JetContainer.h"
#include <vector>
#include <string>

namespace DerivationFramework {

  AugmentationToolLeadingJets::AugmentationToolLeadingJets(const std::string& t,
      const std::string& n,
      const IInterface* p) : 
    AthAlgTool(t,n,p)
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
  }

  StatusCode AugmentationToolLeadingJets::addBranches() const
  {

      // Set up the decorators 
      SG::AuxElement::Decorator< bool > decorator("DFDecoratorLeadingJets"); 

      // CALCULATION OF THE NEW VARIABLE
      // Get Primary vertex
      const xAOD::JetContainer* jets =  evtStore()->retrieve< const xAOD::JetContainer >("AntiKt4EMTopoJets");
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
