/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCALIBTOOLS_CALIBRATED_JET_COPY_ALG_H
#define JETCALIBTOOLS_CALIBRATED_JET_COPY_ALG_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "JetInterface/IJetModifier.h"


class CalibratedJetCopyAlg : public AthReentrantAlgorithm {
public:
  CalibratedJetCopyAlg(const std::string& name,
                       ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  ToolHandle<IJetModifier> m_calibTool;

  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey {
    this, "JetContainer", "AntiKt4EMPFlowJets",
      "Input jet container"};

  SG::WriteHandleKey<xAOD::JetContainer> m_copyKey {
    this, "OutputContainer", "",
      "Output calibrated shallow-copy container"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_linkKey {
    this, "calibratedJetLinkKey", m_jetKey, "calibratedJetLink",
      "Link from each input jet to its calibrated copy"};
};

#endif
