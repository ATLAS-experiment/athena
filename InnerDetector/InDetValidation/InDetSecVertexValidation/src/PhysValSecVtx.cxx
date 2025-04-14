/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PhysValSecVtx.h"

PhysValSecVtx::PhysValSecVtx(const std::string& type, 
                             const std::string& name, 
                             const IInterface* parent)
  : ManagedMonitorToolBase(type, name, parent)
{}

StatusCode PhysValSecVtx::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");    
  ATH_CHECK(ManagedMonitorToolBase::initialize());
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_vertexContainerKey.initialize());

  std::string folder = "SquirrelPlots/SecVtx"; 

  m_secVtxValidationPlots = 
    std::make_unique< SecVtxValidationPlots >(nullptr, 
                                              Form("%s/%s/", 
                                                  folder.c_str(),
                                                  m_vertexContainerKey.key().c_str())
                                              );

  return StatusCode::SUCCESS;
}

StatusCode PhysValSecVtx::bookHistograms()
{
  ATH_MSG_INFO ("Booking hists " << name() << "...");
  ATH_CHECK(bookCollection(m_secVtxValidationPlots.get()));
  return StatusCode::SUCCESS;      
}

StatusCode PhysValSecVtx::fillHistograms()
{
  ATH_MSG_DEBUG ("Filling hists " << name() << "...");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle< xAOD::VertexContainer > inputVertexContainer = SG::makeHandle( m_vertexContainerKey, ctx );
  if (not inputVertexContainer.isValid()) {
    ATH_MSG_FATAL("xAOD::VertexContainer with key " << m_vertexContainerKey.key() << " is not available...");
    return StatusCode::FAILURE;
  }
  const xAOD::VertexContainer *vertexContainer = inputVertexContainer.cptr();

  // Fill histograms
  for (const xAOD::Vertex* vertex : *vertexContainer) {
    m_secVtxValidationPlots->fill(vertex);
  }

  return StatusCode::SUCCESS;
}

StatusCode PhysValSecVtx::procHistograms()
{
  ATH_MSG_INFO ("Finalising hists " << name() << "...");
  m_secVtxValidationPlots->finalize();
  return StatusCode::SUCCESS;
}
