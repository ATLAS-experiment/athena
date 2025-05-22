/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGADCOMBOHYPO_TRIGADCOMBOHYPOTOOL_H
#define TRIGADCOMBOHYPO_TRIGADCOMBOHYPOTOOL_H

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "DecisionHandling/ComboHypoToolBase.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "xAODJet/Jet.h"
#include "xAODMuon/Muon.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODTau/TauJet.h"
#include "xAODTrigMissingET/TrigMissingET.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"

#include <iostream>
#include <cmath>
#include <numbers>

class TrigADComboHypoTool: public ComboHypoToolBase{

public:
  TrigADComboHypoTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual StatusCode initialize() override;
  //need to see all jets at once not a series of combinations
  virtual StatusCode decide(Combo::LegDecisionsMap& passingLegs, const EventContext& context) const override;
  
private:
  Gaudi::Property<unsigned int> m_maxjs{this, "max_jets",{6}, "Maximum number of jets allowed in the event"};
  Gaudi::Property<unsigned int> m_maxes{this, "max_electrons",{3}, "Maximum number of electrons allowed in the event"};
  Gaudi::Property<unsigned int> m_maxms{this, "max_muons",{3}, "Maximum number of muons allowed in the event"};
  Gaudi::Property<unsigned int> m_maxgs{this, "max_photons",{3}, "Maximum number of photons allowed in the event"};

  Gaudi::Property<double> m_adScoreThres{this, "adScoreThres", {0.}, "HLT AD score threshold"};

  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_adScoreKey{this, "adScoreKey", "Undefined", ""};
  ToolHandle<GenericMonitoringTool> m_monTool{this, "monTool", "", "Monitoring tool"};
  Gaudi::Property<bool> m_monFlag{this, "monFlag", false, "Monitoring flag"};
  
  float getAdScore(
    const std::vector<const xAOD::Jet*> &input_jets,
    const std::vector<const xAOD::Muon*> &input_muons,
    const std::vector<const xAOD::Electron*> &input_electrons,
    const std::vector<const xAOD::Photon*> &input_photons,
    const std::vector<const xAOD::TauJet*> &input_taus,
    const std::vector<const xAOD::TrigMissingET*> &input_mets) const;

  float runInference(std::vector<float> &tensor) const;

  ServiceHandle<AthOnnx::IOnnxRuntimeSvc> m_svc{this, "ONNXRuntimeSvc", "AthOnnx::OnnxRuntimeSvc", "TrigADComboHypoTool ONNXRuntimeSvc"};

  std::unique_ptr<Ort::Session> m_session;

  std::vector<const char *> m_input_node_names;

  std::vector<const char *> m_output_node_names;

  std::vector<int64_t> m_input_node_dims;

  // This path needs to point to the ATLAS calibration area (https://atlas-groupdata.web.cern.ch/atlas-groupdata/, /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/)
  Gaudi::Property<std::string> m_modelFileName{this, "ModelFileName", "TrigAnomalyDetectionHypo/2025-03-10/HLT_AD_v1.onnx"}; // main area v1 (2025.03.10)

};

#endif
