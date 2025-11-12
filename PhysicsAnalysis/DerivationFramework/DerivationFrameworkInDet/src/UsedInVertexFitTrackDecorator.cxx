/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// UsedInVertexFitTrackDecorator.cxx
///////////////////////////////////////////////////////////////////
// Author: Matthew Basso (matthew.joseph.basso@cern.ch)
// A very simple tool for decorating tracks with their "used-in-fit" info (AMVF fit vertices and weights)

// Local include(s):
#include "DerivationFrameworkInDet/UsedInVertexFitTrackDecorator.h"

namespace DerivationFramework {

  StatusCode UsedInVertexFitTrackDecorator::initialize()
  {
    // Print configuration
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_MSG_DEBUG("Using UsedInFitDecoratorTool: " << m_decoTool);

    // Retrieve our tool
    ATH_CHECK(m_decoTool.retrieve());

    return StatusCode::SUCCESS;
  }

  StatusCode UsedInVertexFitTrackDecorator::finalize()
  {
    ATH_MSG_DEBUG("Finalizing " << name() << "...");

    // Release our tool
    ATH_CHECK(m_decoTool.release());

    return StatusCode::SUCCESS;
  }

  StatusCode UsedInVertexFitTrackDecorator::addBranches(const EventContext&) const
  {
    ATH_MSG_DEBUG("In addBranches(...) for " << name() <<  "...");

    // Decorate our tracks
    m_decoTool->decorate(); // FIXME Pass EventContext

    return StatusCode::SUCCESS;
  }

} // end: namespace DerivationFramework
