/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TauAnalysisTools/TauHFVetoTool.h"

using namespace TauAnalysisTools;

TauHFVetoTool::TauHFVetoTool(const std::string& name) : asg::AsgTool(name) {
}

StatusCode TauHFVetoTool::initialize() {
  ATH_CHECK(m_onnxTool_bveto1p.retrieve());
  ATH_CHECK(m_onnxTool_bveto3p.retrieve());
  ATH_CHECK(m_onnxTool_cveto1p.retrieve());
  ATH_CHECK(m_onnxTool_cveto3p.retrieve());
  return StatusCode::SUCCESS;
}

const xAOD::Jet* TauHFVetoTool::findClosestPFlowJet(const xAOD::TauJet* xTau, const xAOD::JetContainer* vPFlowJets) const {
  // loop through the jets and find the closest one to the tau
  const xAOD::Jet* xClosestJet = nullptr;
  double dMinDeltaR = 999.;
  for (const xAOD::Jet* xJet : *vPFlowJets){
    double dDeltaR = xTau->p4().DeltaR(xJet->p4());
    if (dDeltaR < dMinDeltaR){
      dMinDeltaR = dDeltaR;
      xClosestJet = xJet;
    }
  }
  return xClosestJet;
}

StatusCode TauHFVetoTool::applyHFvetoBDTs(const xAOD::TauJetContainer* Taus, const xAOD::JetContainer* PFlowJets) const {
  // Define Decorators for the auxiliary variables
  SG::Decorator<float> acc_bVetoScore("bVetoScore");
  SG::Decorator<float> acc_cVetoScore("cVetoScore");
  for (const xAOD::TauJet* xTau : *Taus){
    const xAOD::Jet* xAuxJet = findClosestPFlowJet(xTau, PFlowJets);
    auto input = assembleInputValues(xTau, xAuxJet);
    int prongness = xTau->nTracksCharged();
    ATH_CHECK(bVetoScore(prongness, input, acc_bVetoScore(*xTau)));
    ATH_CHECK(cVetoScore(prongness, input, acc_cVetoScore(*xTau)));
  }
  return StatusCode::SUCCESS;
}

StatusCode TauHFVetoTool::inference(const ToolHandle<AthOnnx::IOnnxRuntimeInferenceTool> onnxTool, const std::vector<float>& inputValues, float& output) const {
  auto inputData = inputValues;
  std::vector<float> outputScores;
  std::vector<Ort::Value> inputTensors;
  std::vector<Ort::Value> outputTensors;

  ATH_CHECK(onnxTool->addInput(inputTensors, inputData, 0, 1));
  ATH_CHECK(onnxTool->addOutput(outputTensors, outputScores, 0, 1));
  ATH_CHECK(onnxTool->inference(inputTensors, outputTensors));
  
  output = outputScores[1];

  return StatusCode::SUCCESS;
}

std::vector<float> TauHFVetoTool::assembleInputValues(const xAOD::TauJet* xTau, const xAOD::Jet* xAuxJet) const {
  std::vector<float> inputValues;
  // gather charged tau tracks
  std::vector<const xAOD::TauTrack*> tracks = xTau->tracks(xAOD::TauJetParameters::classifiedCharged);
  // sort tracks according to pT, descending
  if (tracks.size() > 1)
    std::sort(tracks.begin(), tracks.end(), [](const xAOD::TauTrack* a, const xAOD::TauTrack* b) {
      return a->pt() > b->pt();
    });
  // gather necessary info related to the auxiliary jet
  const xAOD::BTagging* btag = xAOD::BTaggingUtilities::getBTagging(*xAuxJet);
  double dl1dv01_pb(0);
  btag->pb("DL1dv01", dl1dv01_pb);
  double dl1dv01_pc(0);
  btag->pc("DL1dv01", dl1dv01_pc);
  // cast the double to float
  float dl1dv01_pb_f = static_cast<float>(dl1dv01_pb);
  float dl1dv01_pc_f = static_cast<float>(dl1dv01_pc);
  float AbsDEtaLeadTrk = tracks.size() > 0 ? std::abs(tracks[0]->eta() - xTau->eta()) : -999;
  float AbsDPhiLeadTrk = tracks.size() > 0 ? std::abs(tracks[0]->p4().DeltaPhi(xTau->p4())) : -999;
  // declare accessors for taus and auxiliary jets
  SG::ConstAccessor <float> acc_jetRNNtrans("RNNJetScoreSigTrans");
  SG::ConstAccessor <float> acc_eleRNNtrans("RNNEleScoreSigTrans_v1");
  SG::ConstAccessor <float> acc_etOverPtLeadTrk("etOverPtLeadTrk");
  SG::ConstAccessor <float> acc_dRmax("dRmax");
  SG::ConstAccessor <float> acc_auxJetWidth("Width");
  SG::ConstAccessor <std::vector<ElementLink<DataVector<xAOD::IParticle> > >> acc_GhostTrack("GhostTrack");
  // assemble input values according to prongness
  if (xTau->nTracksCharged() == 1) {
    // assemble inputValues
    inputValues = {
      acc_jetRNNtrans(*xTau), // jetRNNtrans
      acc_eleRNNtrans(*xTau), // eleRNNtrans
      AbsDEtaLeadTrk, // AbsDEtaLeadTrk
      AbsDPhiLeadTrk, // AbsDPhiLeadTrk
      acc_etOverPtLeadTrk(*xTau), // etOverPtLeadTrk
      acc_dRmax(*xTau), // dRmax
      static_cast<float>(xAOD::TrackingHelpers::d0significance(tracks[0]->track())), // trk0d0sig
      static_cast<float>(tracks[0]->track()->z0()), // trk0z0
      dl1dv01_pb_f, // DL1dv01_pb
      dl1dv01_pc_f, // DL1dv01_pc
      acc_auxJetWidth(*xAuxJet), // auxJetWidth
      static_cast<float>(xTau->p4().DeltaR(xAuxJet->p4())), // dRJet
      static_cast<float>(xTau->pt() / xAuxJet->pt()), // ptRatio
      static_cast<float>(acc_GhostTrack(*xAuxJet).size()) // jetNtrk
    };
  } else if (xTau->nTracksCharged() == 3) {
    // assemble inputValues
    inputValues = {
      acc_jetRNNtrans(*xTau), // jetRNNtrans
      acc_eleRNNtrans(*xTau), // eleRNNtrans
      AbsDEtaLeadTrk, // AbsDEtaLeadTrk
      AbsDPhiLeadTrk, // AbsDPhiLeadTrk
      acc_etOverPtLeadTrk(*xTau), // etOverPtLeadTrk
      acc_dRmax(*xTau), // dRmax
      static_cast<float>(xAOD::TrackingHelpers::d0significance(tracks[0]->track())), // trk0d0sig
      static_cast<float>(xAOD::TrackingHelpers::d0significance(tracks[1]->track())), // trk1d0sig
      static_cast<float>(xAOD::TrackingHelpers::d0significance(tracks[2]->track())), // trk2d0sig
      static_cast<float>(tracks[0]->track()->z0()), // trk0z0
      static_cast<float>(tracks[1]->track()->z0()), // trk1z0
      static_cast<float>(tracks[2]->track()->z0()), // trk2z0
      dl1dv01_pb_f, // DL1dv01_pb
      dl1dv01_pc_f, // DL1dv01_pc
      acc_auxJetWidth(*xAuxJet), // auxJetWidth
      static_cast<float>(xTau->p4().DeltaR(xAuxJet->p4())), // dRJet
      static_cast<float>(xTau->pt() / xAuxJet->pt()), // ptRatio
      static_cast<float>(acc_GhostTrack(*xAuxJet).size()) // jetNtrk
    };
  }
  return inputValues;
}

StatusCode TauHFVetoTool::bVetoScore(const int& prongness, const std::vector<float>& input, float& output) const {
  if (prongness == 1) {
    return inference(m_onnxTool_bveto1p, input, output);
  }
  else if (prongness == 3) {
    return inference(m_onnxTool_bveto3p, input, output);
  }
  else {
    output = -999;
    return StatusCode::SUCCESS;
  }
}

StatusCode TauHFVetoTool::cVetoScore(const int& prongness, const std::vector<float>& input, float& output) const {
  if (prongness == 1) {
    return inference(m_onnxTool_cveto1p, input, output);
  }
  else if (prongness == 3) {
    return inference(m_onnxTool_cveto3p, input, output);
  }
  else {
    output = -999;
    return StatusCode::SUCCESS;
  }
}
