/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

  ATH_MSG_DEBUG("Executing");

  ATH_CHECK(shallowCopyChargedFEContainer(ctx));
  ATH_CHECK(shallowCopyAndModifyNeutralFEContainer(ctx));

  return StatusCode::SUCCESS;
}

StatusCode PFOClusterMLCorrectionAlgorithm::shallowCopyChargedFEContainer(const EventContext &ctx) const {
  // Just a shallow copy, no modifications
  SG::ReadHandle<xAOD::FlowElementContainer> chargedFEContainerReadHandle(m_chargedFEContainerReadHandleKey, ctx);

  std::pair<xAOD::FlowElementContainer *, xAOD::ShallowAuxContainer *> shallowCopyPair = xAOD::shallowCopyContainer(*chargedFEContainerReadHandle);
  std::unique_ptr<xAOD::FlowElementContainer> chargedFEMLContainer{shallowCopyPair.first};
  std::unique_ptr<xAOD::ShallowAuxContainer> chargedFEMLContainerAux{shallowCopyPair.second};
  
  SG::WriteHandle<xAOD::FlowElementContainer> chargedFEMLContainerWriteHandle(m_chargedFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(chargedFEMLContainerWriteHandle.record(std::move(chargedFEMLContainer), std::move(chargedFEMLContainerAux)));

  return StatusCode::SUCCESS;
}

StatusCode PFOClusterMLCorrectionAlgorithm::shallowCopyAndModifyNeutralFEContainer(const EventContext &ctx) const {
  
  // Shallow copy step
  SG::ReadHandle<xAOD::FlowElementContainer> neutralFEContainerReadHandle(m_neutralFEContainerReadHandleKey, ctx);
  std::pair<xAOD::FlowElementContainer *, xAOD::ShallowAuxContainer *> shallowCopyPair = xAOD::shallowCopyContainer(*neutralFEContainerReadHandle);
  std::unique_ptr<xAOD::FlowElementContainer> neutralFEMLContainer{shallowCopyPair.first};
  std::unique_ptr<xAOD::ShallowAuxContainer> neutralFEMLContainerAux{shallowCopyPair.second};

  // Modification step
  m_correctionTool->correctContainer(*neutralFEMLContainer);
  
  // Registration of results step
  SG::WriteHandle<xAOD::FlowElementContainer> neutralFEMLContainerWriteHandle(m_neutralFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(neutralFEMLContainerWriteHandle.record(std::move(neutralFEMLContainer), std::move(neutralFEMLContainerAux)));

  return StatusCode::SUCCESS;
}