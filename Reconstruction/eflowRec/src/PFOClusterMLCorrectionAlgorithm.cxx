/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFOClusterMLCorrectionAlgorithm.h"
#include "xAODCore/ShallowCopy.h"

StatusCode PFOClusterMLCorrectionAlgorithm::initialize()
{
  ATH_MSG_DEBUG("Initializing");
  ATH_CHECK(m_correctionTool.retrieve());
  ATH_CHECK(m_chargedFEContainerReadHandleKey.initialize());
  ATH_CHECK(m_chargedFEMLContainerWriteHandleKey.initialize());
  ATH_CHECK(m_neutralFEContainerReadHandleKey.initialize());
  ATH_CHECK(m_neutralFEMLContainerWriteHandleKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PFOClusterMLCorrectionAlgorithm::execute(const EventContext &ctx) const
{
  try {
    ATH_MSG_DEBUG("Executing");

    // Read charged container
    SG::ReadHandle<xAOD::FlowElementContainer> chargedFEContainerReadHandle(m_chargedFEContainerReadHandleKey, ctx);
    auto [chargedFEMLContainer, chargedFEMLContainerAux] = xAOD::shallowCopy(*chargedFEContainerReadHandle);
    
    // Read neutral container
    SG::ReadHandle<xAOD::FlowElementContainer> neutralFEContainerReadHandle(m_neutralFEContainerReadHandleKey, ctx);
    auto [neutralFEMLContainer, neutralFEMLContainerAux] = xAOD::shallowCopy(*neutralFEContainerReadHandle);
    
    // Modification step
    m_correctionTool->correctContainer(*neutralFEMLContainer, *chargedFEMLContainer, ctx);

    // Writing containers
    SG::WriteHandle<xAOD::FlowElementContainer> chargedFEMLContainerWriteHandle(m_chargedFEMLContainerWriteHandleKey, ctx);
    ATH_CHECK(chargedFEMLContainerWriteHandle.record(std::move(chargedFEMLContainer), std::move(chargedFEMLContainerAux)));
    SG::WriteHandle<xAOD::FlowElementContainer> neutralFEMLContainerWriteHandle(m_neutralFEMLContainerWriteHandleKey, ctx);
    ATH_CHECK(neutralFEMLContainerWriteHandle.record(std::move(neutralFEMLContainer), std::move(neutralFEMLContainerAux)));

    return StatusCode::SUCCESS;
  
  } 
  catch (const std::exception& e) {
    ATH_MSG_ERROR("Standard std::exception caught: " << e.what());
    return StatusCode::FAILURE;
  }
  catch (...) {
    ATH_MSG_ERROR("Unknown exception caught");
    return StatusCode::FAILURE;
  }
 
}
