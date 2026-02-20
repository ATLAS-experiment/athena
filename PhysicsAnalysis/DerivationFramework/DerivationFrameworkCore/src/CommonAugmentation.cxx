/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CommonAugmentation.cxx
///////////////////////////////////////////////////////////////////
// Author: James Catmore (James.Catmore@cern.ch)
// This code loops over common tools from the CP groups, which write
// into SG so that other derivations can make use of them without
// re-running the tools.

#include "DerivationFrameworkCore/CommonAugmentation.h"

#include <sstream>                                      // C++ utilities
#include <string>
#include <algorithm>
#include <fstream>

#include "GaudiKernel/ISvcLocator.h"
#include "AthContainers/DataVector.h"
#include "AthLinks/ElementLink.h"
#include "GaudiKernel/Chrono.h"

#include "StoreGate/DataHandle.h"
#include "AthenaKernel/DefaultKey.h"
#include "SGTools/StlVectorClids.h"

///////////////////////////////////////////////////////////////////////////////

DerivationFramework::CommonAugmentation::CommonAugmentation(const std::string& name, ISvcLocator* pSvcLocator) :
AthReentrantAlgorithm(name, pSvcLocator)
{
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode DerivationFramework::CommonAugmentation::initialize() {

    ATH_MSG_INFO("Initializing the common selections in " << name());

    // get the augmentation tools
    ATH_CHECK ( m_augmentationTools.retrieve() );
    ATH_MSG_INFO("The following augmentation tools will be applied....");
    ATH_MSG_INFO(m_augmentationTools);

    // get the chrono auditor
    ATH_CHECK ( m_chronoSvc.retrieve() );
    
    return StatusCode::SUCCESS;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode DerivationFramework::CommonAugmentation::execute(const EventContext& ctx) const {

    // On your marks.... get set....
    Chrono chrono( &(*m_chronoSvc), name() );
    // GO!!

    //=============================================================================
    // AUGMENTATION ===============================================================
    //=============================================================================
    for (const auto &  augmentationTool : m_augmentationTools) {
      if ( augmentationTool->addBranches(ctx).isFailure() ) {
        ATH_MSG_ERROR("Augmentation failed!");
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode DerivationFramework::CommonAugmentation::finalize() {

    ATH_MSG_INFO( "============================================================================");
    ATH_MSG_INFO( " The following CP tools were called by " << name() << " for the whole train:");
    for (const auto &  augmentationTool : m_augmentationTools) {
        ATH_MSG_INFO ( augmentationTool->name() );
    }
    ATH_MSG_INFO( "============================================================================");

    return StatusCode::SUCCESS;

}
