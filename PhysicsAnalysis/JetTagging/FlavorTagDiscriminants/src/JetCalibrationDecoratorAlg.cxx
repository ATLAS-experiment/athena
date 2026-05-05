/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/JetCalibrationDecoratorAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODCore/ShallowCopy.h"


namespace FlavorTagDiscriminants {

  JetCalibrationDecoratorAlg::JetCalibrationDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc)
  {
    declareProperty("JetCalibrationTool", m_calibTool, "Jet calibration tool");
  }

  StatusCode JetCalibrationDecoratorAlg::initialize() {
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_ptCalibKey.initialize());
    ATH_CHECK(m_etaCalibKey.initialize());
    ATH_CHECK(m_massCalibKey.initialize());
    ATH_CHECK(m_phiCalibKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetCalibrationDecoratorAlg::execute(
    const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);
    ATH_CHECK(jets.isValid());

    SG::WriteDecorHandle<xAOD::JetContainer, float> ptDec(
      m_ptCalibKey, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> etaDec(
      m_etaCalibKey, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> massDec(
      m_massCalibKey, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, float> phiDec(
      m_phiCalibKey, ctx);

    // Shallow-copy jets so calibration doesn't modify originals
    auto [copyUP, auxUP] = xAOD::shallowCopy(*jets, ctx);

    // Apply calibration to the copies
    ATH_CHECK(m_calibTool->modify(*copyUP));

    // Decorate originals with calibrated kinematics from copies
    for (size_t i = 0; i < jets->size(); ++i) {
      const xAOD::Jet& originalJet = *jets->at(i);
      const xAOD::Jet& calibratedJet = *copyUP->at(i);

      ptDec(originalJet) = calibratedJet.pt();
      etaDec(originalJet) = calibratedJet.eta();
      massDec(originalJet) = calibratedJet.m();
      phiDec(originalJet) = calibratedJet.phi();
    }

    return StatusCode::SUCCESS;
  }

} // namespace FlavorTagDiscriminants
