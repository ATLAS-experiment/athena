/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "AsgDataHandles/ReadDecorHandle.h"
#include "JetCalibTools/JetDNNCalibStep.h"
#include "JetCalibTools/JetCalibUtils.h"

#include <TLorentzVector.h>
#include <algorithm>
#include <cmath>


StatusCode JetDNNCalibStep::initialize() {

  ATH_CHECK( m_inputVariables.retrieve() );
  ATH_CHECK( m_npvKey.initialize() );
  ATH_CHECK( m_muKey.initialize() );
  ATH_CHECK( m_onnxTool.retrieve() );

  if (static_cast<int>(m_inputVariables.size()) != m_onnxInputShape) {
    ATH_MSG_FATAL("Number of input variables (" << m_inputVariables.size() << ") does not match onnxInputShape (" << m_onnxInputShape << ")");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode JetDNNCalibStep::calibrate(xAOD::JetContainer& jets) const {

  JetHelper::JetContext jc;

  SG::ReadHandle<xAOD::VertexContainer> primaryVertexContainer(m_npvKey);
  float NPV = JetCalibUtils::countNPV(*primaryVertexContainer);
  jc.setValue("NPV" , NPV);

  SG::ReadDecorHandle<xAOD::EventInfo,float> eventInfoDecor(m_muKey);
  float mu = eventInfoDecor(0); 
  jc.setValue("mu", mu);

  // Pass 1: compute normalized input features per jet, resolving degenerate/invalid jets
  // immediately (they never reach the DNN). Surviving jets are batched into a single ONNX
  // inference call below, instead of one call per jet, since the model has a dynamic batch
  // dimension ([-1, 21] in, [-1, 3] out).
  std::vector<xAOD::Jet*> batchJets;
  std::vector<xAOD::JetFourMom_t> batchStartP4;
  std::vector<float> batchInputValues;
  batchJets.reserve(jets.size());
  batchStartP4.reserve(jets.size());
  batchInputValues.reserve(jets.size() * m_inputVariables.size());

  for (xAOD::Jet* jet : jets){

    const xAOD::JetFourMom_t jetStartP4 = jet->getAttribute<xAOD::JetFourMom_t>(m_jetInScale);

    // Sync the jet's active 4-vector to InScale, so any variable reading jet.e()/jet.m()/etc.
    // directly (e.g. log_e/log_m below) sees the same values as jetStartP4, matching
    // GlobalLargeRDNNCalibration's setStartP4() convention.
    jet->setJetP4(jetStartP4);

    // Don't apply calibration for jets with negative or null mass or for one constituent jets
    if (jetStartP4.mass() <= 0 || jet->numConstituents() == 1) {
      jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale, jetStartP4);
      continue;
    }

    std::vector<float> inputTensorValues;
    inputTensorValues.reserve(m_inputVariables.size());

    for (long unsigned int i=0; i < m_inputVariables.size(); i++) {

      // log_e/log_m/log_m_cap40: InputVariable.cpp already returns log(x*scale) for these, with
      // eScale folded in via the VarTool's own Scale property (see JetCalibStepsConfig.py), so
      // eScale must NOT be re-applied below for them, or it would be double-counted.
      static const std::vector<std::string> logScaledVars = {"log_e", "log_m", "log_m_cap40"};
      const std::string& varName = m_inputVariables[i].name();
      bool eScaleAlreadyApplied = std::any_of(logScaledVars.begin(), logScaledVars.end(),
        [&varName](const std::string& n) { return varName.find(n) != std::string::npos; });

      float inputVar = m_inputVariables[i]->getValue(*jet, jc);
      float eScale = m_eScales[i];
      float normOffset = m_normOffsets[i];
      float normScale = m_normScales[i];
      float normalisedVar;

      if (eScaleAlreadyApplied) {
        normalisedVar = inputVar*normScale + normOffset;
      } else {
        normalisedVar = inputVar*eScale*normScale + normOffset;
      }

      ATH_MSG_DEBUG(m_inputVariables[i].name() << ": raw=" << inputVar << " eScale=" << eScale
                    << " normOffset=" << normOffset << " normScale=" << normScale << " normalised=" << normalisedVar);

      inputTensorValues.push_back(normalisedVar);
    }

    int nNan = std::count_if(inputTensorValues.begin(), inputTensorValues.end(),
                              [](float f){ return std::isnan(f) || std::isinf(f); });
    if (nNan > 0) {
      ATH_MSG_WARNING("Encountered NaN or inf value in input features, will not apply calibration to this jet");
      jet->setJetP4(jetStartP4);
      jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale, jetStartP4);
      continue;
    }

    batchJets.push_back(jet);
    batchStartP4.push_back(jetStartP4);
    batchInputValues.insert(batchInputValues.end(), inputTensorValues.begin(), inputTensorValues.end());
  }

  if (batchJets.empty()) {
    return StatusCode::SUCCESS;
  }

  const int64_t nBatch = static_cast<int64_t>(batchJets.size());

  AthInfer::InputDataMap inputData;
  inputData["input_1"] = std::make_pair(
    std::vector<int64_t>{nBatch, m_onnxInputShape}, std::move(batchInputValues)
  );

  AthInfer::OutputDataMap outputData;
  outputData["outputE"] = std::make_pair(std::vector<int64_t>{nBatch, m_onnxOutputShape}, std::vector<float>{});
  outputData["outputM"] = std::make_pair(std::vector<int64_t>{nBatch, m_onnxOutputShape}, std::vector<float>{});

  ATH_CHECK( m_onnxTool->inference(inputData, outputData) );

  const std::vector<float>& outputE = std::get<std::vector<float>>(outputData["outputE"].second);
  const std::vector<float>& outputM = std::get<std::vector<float>>(outputData["outputM"].second);

  // Models with a single output node (e.g. small-R JES-only DNNs) only predict the
  // energy response: there is no mass response to read
  const bool energyOnly = outputM.empty();

  // Pass 2: combine each jet's response and rescale, in the same order the batch was filled
  for (int64_t idx = 0; idx < nBatch; idx++) {

    xAOD::Jet* jet = batchJets[idx];
    const xAOD::JetFourMom_t& jetStartP4 = batchStartP4[idx];

    // First element of each jet's output is the predicted response; remaining elements are unused here
    float predRespE = outputE.at(idx * m_onnxOutputShape);
    float predRespM = energyOnly ? predRespE : outputM.at(idx * m_onnxOutputShape);

    ATH_MSG_DEBUG("jetStartP4: pt=" << jetStartP4.pt() << " eta=" << jetStartP4.eta()
                  << " e=" << jetStartP4.e() << " m=" << jetStartP4.mass());
    ATH_MSG_DEBUG("Predicted response: E=" << predRespE << " M=" << predRespM);

    if (predRespE == 0 || predRespM == 0) {
      ATH_MSG_WARNING("DNN predictions give 0 values, will not apply calibration to this jet");
      jet->setJetP4(jetStartP4);
      jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale, jetStartP4);
      continue;
    }

    // Energy-only models: scale the whole 4-vector by the energy response (so the mass
    // scales with it)
    if (energyOnly) {
      const xAOD::JetFourMom_t calibP4 = jetStartP4 * (1. / predRespE);
      jet->setJetP4(calibP4);
      jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale, calibP4);
      continue;
    }

    // Combine the energy and mass response predictions, following the same approach as GlobalLargeRDNNCalibration
    float calibE = jetStartP4.e() / predRespE;

    // Calibrate the mass at all mass values (log_m_cap40 protects the DNN input from extreme
    // log(mass) values for very light jets, so no output-side mass cutoff is needed here).
    float calibM = jetStartP4.mass() / predRespM;

    // Propagate energy and mass calibration to jet pT
    float calibpT = std::sqrt(calibE*calibE - calibM*calibM) / std::cosh(jetStartP4.eta());

    TLorentzVector TLVjet;
    TLVjet.SetPtEtaPhiM(calibpT, jetStartP4.eta(), jetStartP4.phi(), calibM);
    xAOD::JetFourMom_t calibP4;
    calibP4.SetPxPyPzE(TLVjet.Px(), TLVjet.Py(), TLVjet.Pz(), TLVjet.E());

    jet->setJetP4(calibP4);

    // Set the output scale
    jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale,calibP4);
  }
  return StatusCode::SUCCESS;
}

