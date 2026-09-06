/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CalibratedJetCopyAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODCore/ShallowCopy.h"


CalibratedJetCopyAlg::CalibratedJetCopyAlg(
  const std::string& name, ISvcLocator* loc)
  : AthReentrantAlgorithm(name, loc)
{
  declareProperty("JetCalibrationTool", m_calibTool, "Jet calibration tool");
}

StatusCode CalibratedJetCopyAlg::initialize() {
  ATH_CHECK(m_calibTool.retrieve());
  ATH_CHECK(m_jetKey.initialize());
  ATH_CHECK(m_copyKey.initialize());
  ATH_CHECK(m_linkKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode CalibratedJetCopyAlg::execute(const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);
  ATH_CHECK(jets.isValid());

  auto [copy, aux] = xAOD::shallowCopy(*jets, ctx);

  ATH_CHECK(m_calibTool->modify(*copy));

  if (!xAOD::setOriginalObjectLink(*jets, *copy)) {
    ATH_MSG_ERROR("Failed to set original object links on " << m_copyKey);
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<xAOD::JetContainer> copyHandle(m_copyKey, ctx);
  ATH_CHECK(copyHandle.record(std::move(copy), std::move(aux)));

  SG::WriteDecorHandle<xAOD::JetContainer, ElementLink<xAOD::JetContainer>>
    linkDec(m_linkKey, ctx);
  for (size_t i = 0; i < jets->size(); ++i) {
    linkDec(*jets->at(i)) =
      ElementLink<xAOD::JetContainer>(*copyHandle, i, ctx);
  }

  return StatusCode::SUCCESS;
}
