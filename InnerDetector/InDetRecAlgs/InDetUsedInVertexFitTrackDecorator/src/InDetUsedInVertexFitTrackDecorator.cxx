/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// InDetUsedInVertexFitTrackDecorator.cxx
///////////////////////////////////////////////////////////////////
// Author: Matthew Basso (matthew.joseph.basso@cern.ch)
// A very simple tool for decorating tracks with their "used-in-fit" info (AMVF fit vertices and weights)

// Local include(s):
#include "InDetUsedInVertexFitTrackDecorator/InDetUsedInVertexFitTrackDecorator.h"

// Constructor with parameters:
InDet::InDetUsedInVertexFitTrackDecorator::InDetUsedInVertexFitTrackDecorator(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode InDet::InDetUsedInVertexFitTrackDecorator::initialize()
{
  // Print configuration
  ATH_MSG_DEBUG("Initializing " << name() << "...");
  ATH_MSG_DEBUG("Using UsedInFitDecoratorTool: " << m_decoTool);

  // Retrieve our tool
  ATH_CHECK(m_decoTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode InDet::InDetUsedInVertexFitTrackDecorator::finalize()
{
  ATH_MSG_DEBUG("Finalizing " << name() << "...");

  // Release our tool
  ATH_CHECK(m_decoTool.release());

  return StatusCode::SUCCESS;
}

StatusCode InDet::InDetUsedInVertexFitTrackDecorator::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("In addBranches(...) for " << name() <<  "...");

  // Decorate our tracks
  m_decoTool->decorate(ctx);

  return StatusCode::SUCCESS;
}
