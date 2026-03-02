/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FORWARDELECTRONTOOLSTESTALG_H
#define FORWARDELECTRONTOOLSTESTALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "ElectronPhotonSelectorTools/AsgForwardElectronCalibrationTool.h"
#include <atomic>

class ForwardElectronToolsTestAlg : public AthAlgorithm {
public:
    ForwardElectronToolsTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~ForwardElectronToolsTestAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute()    override;
    virtual StatusCode finalize()   override;

private:
    ToolHandle<AsgForwardElectronCalibrationTool> m_calibTool {
        this, "CalibrationTool", "FwdCalibTool", ""};
    ToolHandle<IAsgElectronLikelihoodTool> m_looseTool {
        this, "LooseTool",  "FwdSelectorTool_Loose",  ""};
    ToolHandle<IAsgElectronLikelihoodTool> m_mediumTool {
        this, "MediumTool", "FwdSelectorTool_Medium", ""};
    ToolHandle<IAsgElectronLikelihoodTool> m_tightTool {
        this, "TightTool",  "FwdSelectorTool_Tight",  ""};

    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronKey {
        this, "ElectronContainerKey", "ForwardElectrons", ""};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey {
        this, "TruthParticleKey", "TruthParticles", ""};

    std::atomic<long> m_nEvents{0}, m_nElectrons{0};
    std::atomic<long> m_nLoose{0}, m_nMedium{0}, m_nTight{0};
};

#endif