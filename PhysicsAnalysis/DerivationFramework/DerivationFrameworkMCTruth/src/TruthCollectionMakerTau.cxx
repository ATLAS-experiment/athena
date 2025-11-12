/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerTau.cxx
// Create truth tau collection decorated with tau decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerTau.h"

// Tool header file
// for TruthTausEvent
#include "TauAnalysisTools/BuildTruthTaus.h"

// Athena initialize and finalize
StatusCode DerivationFramework::TruthCollectionMakerTau::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  ATH_CHECK( m_buildTruthTaus.retrieve() );

  return StatusCode::SUCCESS;
}

// Selection and collection creation
StatusCode DerivationFramework::TruthCollectionMakerTau::addBranches(const EventContext& ctx) const
{
  ATH_MSG_VERBOSE("addBranches() ...");

  // One call to build the truth tau collection
  TauAnalysisTools::BuildTruthTaus::TruthTausEvent truthTausEvent;
  ATH_CHECK( m_buildTruthTaus->retrieveTruthTaus( truthTausEvent, ctx ) );

  return StatusCode::SUCCESS;
}

