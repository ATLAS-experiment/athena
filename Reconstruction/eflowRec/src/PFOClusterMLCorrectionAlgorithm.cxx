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

  // Read charged container
  SG::ReadHandle<xAOD::FlowElementContainer> chargedFEContainerReadHandle(m_chargedFEContainerReadHandleKey, ctx);
  std::pair<xAOD::FlowElementContainer *, xAOD::ShallowAuxContainer *> shallowCopyPairCharged = xAOD::shallowCopyContainer(*chargedFEContainerReadHandle);
  std::unique_ptr<xAOD::FlowElementContainer> chargedFEMLContainer{shallowCopyPairCharged.first};
  std::unique_ptr<xAOD::ShallowAuxContainer> chargedFEMLContainerAux{shallowCopyPairCharged.second};

  // Read neutral container
  SG::ReadHandle<xAOD::FlowElementContainer> neutralFEContainerReadHandle(m_neutralFEContainerReadHandleKey, ctx);
  std::pair<xAOD::FlowElementContainer *, xAOD::ShallowAuxContainer *> shallowCopyPairNeutral = xAOD::shallowCopyContainer(*neutralFEContainerReadHandle);
  std::unique_ptr<xAOD::FlowElementContainer> neutralFEMLContainer{shallowCopyPairNeutral.first};
  std::unique_ptr<xAOD::ShallowAuxContainer> neutralFEMLContainerAux{shallowCopyPairNeutral.second};

  // neutralFEMLContainer is modified, chargedFEMLContainer is kept constant
  m_correctionTool->correctContainer(*neutralFEMLContainer, *chargedFEMLContainer);

  // Writing containers
  SG::WriteHandle<xAOD::FlowElementContainer> chargedFEMLContainerWriteHandle(m_chargedFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(chargedFEMLContainerWriteHandle.record(std::move(chargedFEMLContainer), std::move(chargedFEMLContainerAux)));
  SG::WriteHandle<xAOD::FlowElementContainer> neutralFEMLContainerWriteHandle(m_neutralFEMLContainerWriteHandleKey, ctx);
  ATH_CHECK(neutralFEMLContainerWriteHandle.record(std::move(neutralFEMLContainer), std::move(neutralFEMLContainerAux)));

  return StatusCode::SUCCESS;
}
