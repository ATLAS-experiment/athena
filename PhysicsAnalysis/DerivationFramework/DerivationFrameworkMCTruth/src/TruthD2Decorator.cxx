/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthD2Decorator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Robert Les (robert.les@cern.ch)
//

#include "DerivationFrameworkMCTruth/TruthD2Decorator.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>
#include <string>

namespace DerivationFramework {

  TruthD2Decorator::TruthD2Decorator(const std::string& t,
      const std::string& n,
      const IInterface* p) : 
    base_class(t,n,p)
  {

  }

  StatusCode TruthD2Decorator::initialize()  
  { 
      ATH_CHECK(m_jetContainerKey.initialize()); 
      m_decorationName = m_jetContainerKey.key()+".D2";
      ATH_CHECK(m_decorationName.initialize());

      return StatusCode::SUCCESS;
  }


  StatusCode TruthD2Decorator::addBranches(const EventContext& ctx) const
  {
      // Event context

      // Set up the decorators 
      SG::WriteDecorHandle< xAOD::JetContainer, float > decoratorD2(m_decorationName, ctx); 

      // Get the Large-R jet Container
      SG::ReadHandle<xAOD::JetContainer> largeRjets(m_jetContainerKey, ctx);
      
      if(!largeRjets.isValid()) {
        ATH_MSG_ERROR ("Couldn't retrieve JetContainer with key " << m_jetContainerKey.key());
        return StatusCode::FAILURE;
      }

      // loop over jet collection
      for( const auto *jet: *largeRjets){
        //get ECF
        float ecf1 = jet->getAttribute<float>("ECF1");
        float ecf2 = jet->getAttribute<float>("ECF2");
        float ecf3 = jet->getAttribute<float>("ECF3");
        
        //calculate D2 and decorate
        float D2=-999;
        if(std::abs(ecf2)>1e-8)
          D2=ecf3 * std::pow(ecf1, 3.0) / std::pow(ecf2, 3.0);
        decoratorD2(*jet) = D2;
      }

      return StatusCode::SUCCESS;
  }
}
