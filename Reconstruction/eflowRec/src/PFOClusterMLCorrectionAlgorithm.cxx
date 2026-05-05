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

  ATH_MSG_DEBUG("Executing");

  ATH_CHECK(shallowCopyChargedFEContainer(ctx));
  ATH_CHECK(shallowCopyAndModifyNeutralFEContainer(ctx));

  return StatusCode::SUCCESS;
}

StatusCode PFOClusterMLCorrectionAlgorithm::shallowCopyChargedFEContainer(const EventContext &ctx) const {
  // Just a shallow copy, no modifications
  SG::ReadHandle<xAOD::FlowElementContainer> chargedFEContainerReadHandle(m_chargedFEContainerReadHandleKey, ctx);

  xAOD::ShallowCopyResult_t<xAOD::FlowElementContainer> shallowCopyPair =
    xAOD::shallowCopy(*chargedFEContainerReadHandle, ctx);

  SG::WriteHandle<xAOD::FlowElementContainer> chargedFEMLContainerWriteHandle(m_chargedFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(chargedFEMLContainerWriteHandle.record(std::move(shallowCopyPair.first), std::move(shallowCopyPair.second)));

  return StatusCode::SUCCESS;
}

StatusCode PFOClusterMLCorrectionAlgorithm::shallowCopyAndModifyNeutralFEContainer(const EventContext &ctx) const {

  // Shallow copy step
  SG::ReadHandle<xAOD::FlowElementContainer> neutralFEContainerReadHandle(m_neutralFEContainerReadHandleKey, ctx);
  xAOD::ShallowCopyResult_t<xAOD::FlowElementContainer> shallowCopyPair =
    xAOD::shallowCopy(*neutralFEContainerReadHandle, ctx);

  // Modification step
  m_correctionTool->correctContainer(*shallowCopyPair.first);

  // Registration of results step
  SG::WriteHandle<xAOD::FlowElementContainer> neutralFEMLContainerWriteHandle(m_neutralFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(neutralFEMLContainerWriteHandle.record(std::move(shallowCopyPair.first), std::move(shallowCopyPair.second)));

  return StatusCode::SUCCESS;
}
