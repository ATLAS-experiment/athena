/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: Robert Les (robert.les@cern.ch)

#include "DerivationFrameworkMCTruth/TruthD2Decorator.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>
#include <string>
#include <cmath>
#include <array>

namespace DerivationFramework {

  StatusCode TruthD2Decorator::initialize()
  {
    ATH_CHECK(m_jetContainerKey.initialize());
    ATH_CHECK(m_decorationName.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode TruthD2Decorator::execute(const EventContext& ctx) const
  {
    // Set up the decorators
    SG::WriteDecorHandle< xAOD::JetContainer, float > decoratorD2(m_decorationName, ctx);

    // Get the Large-R jet Container
    SG::ReadHandle<xAOD::JetContainer> largeRjets(m_jetContainerKey, ctx);

    if(!largeRjets.isValid()) {
      ATH_MSG_ERROR ("Couldn't retrieve JetContainer with key " << m_jetContainerKey.key());
      return StatusCode::FAILURE;
    }

    // loop over jet collection
    const std::array<std::string,3> ECF{"ECF1","ECF2","ECF3"};
    for( const auto *jet: *largeRjets){
      //get ECF
      float ecf1 = jet->getAttribute<float>(ECF[0]);
      float ecf2 = jet->getAttribute<float>(ECF[1]);
      float ecf3 = jet->getAttribute<float>(ECF[2]);

      //calculate D2 and decorate
      float D2=-999;
      if(std::abs(ecf2)>1e-8)
        D2=ecf3 * std::pow(ecf1, 3.0) / std::pow(ecf2, 3.0);
      decoratorD2(*jet) = D2;
    }

    return StatusCode::SUCCESS;
  }
}
