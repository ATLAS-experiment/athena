/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCALIBTOOLS_JET_CALIBRATION_DECORATOR_ALG_H
#define JETCALIBTOOLS_JET_CALIBRATION_DECORATOR_ALG_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "JetInterface/IJetModifier.h"


class JetCalibrationDecoratorAlg : public AthReentrantAlgorithm {
public:
  JetCalibrationDecoratorAlg(const std::string& name,
                             ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  // public handle: lets derivation trains share the CP-algorithm tool instance
  ToolHandle<IJetModifier> m_calibTool;

  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey {
    this, "JetContainer", "AntiKt4EMPFlowJets",
      "Input jet container"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_ptCalibKey {
    this, "ptCalibratedKey", m_jetKey, "pt_calibrated",
      "Decorated calibrated pT"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_etaCalibKey {
    this, "etaCalibratedKey", m_jetKey, "eta_calibrated",
      "Decorated calibrated eta"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_massCalibKey {
    this, "massCalibratedKey", m_jetKey, "mass_calibrated",
      "Decorated calibrated mass"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_phiCalibKey {
    this, "phiCalibratedKey", m_jetKey, "phi_calibrated",
      "Decorated calibrated phi"};
};

#endif
