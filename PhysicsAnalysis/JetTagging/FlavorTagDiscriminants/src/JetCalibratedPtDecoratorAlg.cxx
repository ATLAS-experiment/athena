/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/JetCalibratedPtDecoratorAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODCore/ShallowCopy.h"


namespace FlavorTagDiscriminants {

  JetCalibratedPtDecoratorAlg::JetCalibratedPtDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc)
  {
    declareProperty("JetCalibrationTool", m_calibTool, "Jet calibration tool");
  }

  StatusCode JetCalibratedPtDecoratorAlg::initialize() {
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_ptCalibKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetCalibratedPtDecoratorAlg::execute(
    const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);
    ATH_CHECK(jets.isValid());

    SG::WriteDecorHandle<xAOD::JetContainer, float> ptDec(
      m_ptCalibKey, ctx);

    // Shallow-copy jets so calibration doesn't modify originals
    std::pair<std::unique_ptr<xAOD::JetContainer>,
              std::unique_ptr<xAOD::ShallowAuxContainer>> shallowCopy =
      xAOD::shallowCopyContainer(*jets, ctx);
    std::unique_ptr<xAOD::JetContainer>& copyUP = shallowCopy.first;
    [[maybe_unused]] std::unique_ptr<xAOD::ShallowAuxContainer>& auxUP =
      shallowCopy.second;

    // Apply calibration to the copies
    ATH_CHECK(m_calibTool->modify(*copyUP));

    // Decorate originals with calibrated pT from copies
    for (size_t i = 0; i < jets->size(); ++i) {
      ptDec(*jets->at(i)) = copyUP->at(i)->pt();
    }

    return StatusCode::SUCCESS;
  }

} // namespace FlavorTagDiscriminants
