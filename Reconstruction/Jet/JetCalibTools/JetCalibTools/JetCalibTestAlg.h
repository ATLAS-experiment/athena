/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCALIBTOOLS_JETCALIBTESTALG_H
#define JETCALIBTOOLS_JETCALIBTESTALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include "JetCalibTools/JetCalibTool.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "xAODJet/JetContainer.h"

class JetCalibTestAlg : public EL::AnaAlgorithm {
public:
    using AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

private:
    ToolHandle<IJetCalibTool> m_jetCalibTool
        {this, "JetCalibTool", "", "Jet calibration tool"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey
        {this, "JetContainerKey", "AntiKt4EMPFlowJets", "Input jet container"};
};

#endif
