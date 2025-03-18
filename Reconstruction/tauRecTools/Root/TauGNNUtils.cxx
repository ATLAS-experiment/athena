/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNNUtils.h"
#include "tauRecTools/HelperFunctions.h"
#include <algorithm>
#include <iostream>
#define GeV 1000

namespace TauGNNUtils {

GNNVarCalc::GNNVarCalc() : asg::AsgMessaging("TauGNNUtils::GNNVarCalc") {
}

bool GNNVarCalc::compute(const std::string &name, const xAOD::TauJet &tau,
                      double &out) const {
    // Retrieve calculator function
    ScalarCalc func = nullptr;
    try {
        func = m_scalar_map.at(name);
    } catch (const std::out_of_range &e) {
        ATH_MSG_ERROR("Variable '" << name << "' not defined");
        throw;
    }

    // Calculate variable
    return func(tau, out);
}

bool GNNVarCalc::compute(const std::string &name, const xAOD::TauJet &tau,
                      const std::vector<const xAOD::TauTrack *> &tracks,
                      std::vector<double> &out) const {
    out.clear();
    out.reserve(tracks.size());

    // Retrieve calculator function
    TrackCalc func = nullptr;
    try {
        func = m_track_map.at(name);
    } catch (const std::out_of_range &e) {
        ATH_MSG_ERROR("Variable '" << name << "' not defined");
        throw;
    }

    // Calculate variables for selected tracks
    bool success = true;
    double value;
    for (const auto *const trk : tracks) {
        success = success && func(tau, *trk, value);
        out.push_back(value);
    }

    return success;
}

bool GNNVarCalc::compute(const std::string &name, const xAOD::TauJet &tau,
                      const std::vector<xAOD::CaloVertexedTopoCluster> &clusters,
                      std::vector<double> &out) const {
    out.clear();
    out.reserve(clusters.size());

    // Retrieve calculator function
    ClusterCalc func = nullptr;
    try {
        func = m_cluster_map.at(name);
    } catch (const std::out_of_range &e) {
        ATH_MSG_ERROR("Variable '" << name << "' not defined");
        throw;
    }

    // Calculate variables for selected clusters
    bool success = true;
    double value;
    for (const xAOD::CaloVertexedTopoCluster& cluster : clusters) {
        success = success && func(tau, cluster, value);
        out.push_back(value);
    }

    return success;
}

void GNNVarCalc::insert(const std::string &name, ScalarCalc func, const std::vector<std::string>& scalar_vars) {
    if (std::find(scalar_vars.begin(), scalar_vars.end(), name) == scalar_vars.end()) {
      return;
    }
    if (!func) {
        throw std::invalid_argument("Nullptr passed to GNNVarCalc::insert");
    }
    m_scalar_map[name] = func;
}

void GNNVarCalc::insert(const std::string &name, TrackCalc func, const std::vector<std::string>& track_vars) {
    if (std::find(track_vars.begin(), track_vars.end(), name) == track_vars.end()) {
      return;
    }
    if (!func) {
        throw std::invalid_argument("Nullptr passed to GNNVarCalc::insert");
    }
    m_track_map[name] = func;
}

void GNNVarCalc::insert(const std::string &name, ClusterCalc func, const std::vector<std::string>& cluster_vars) {
    if (std::find(cluster_vars.begin(), cluster_vars.end(), name) == cluster_vars.end()) {
      return;
    }
    if (!func) {
        throw std::invalid_argument("Nullptr passed to GNNVarCalc::insert");
    }
    m_cluster_map[name] = func;
}

std::unique_ptr<GNNVarCalc> get_calculator(const std::vector<std::string>& scalar_vars,
					const std::vector<std::string>& track_vars,
					const std::vector<std::string>& cluster_vars) {
    auto calc = std::make_unique<GNNVarCalc>();

    // Scalar variable calculator functions
    calc->insert("absEta", Variables::absEta, scalar_vars);
    calc->insert("isolFrac", Variables::isolFrac, scalar_vars);
    calc->insert("centFrac", Variables::centFrac, scalar_vars);
    calc->insert("etOverPtLeadTrk", Variables::etOverPtLeadTrk, scalar_vars);
    calc->insert("innerTrkAvgDist", Variables::innerTrkAvgDist, scalar_vars);
    calc->insert("absipSigLeadTrk", Variables::absipSigLeadTrk, scalar_vars);
    calc->insert("SumPtTrkFrac", Variables::SumPtTrkFrac, scalar_vars);
    calc->insert("sumEMCellEtOverLeadTrkPt", Variables::sumEMCellEtOverLeadTrkPt, scalar_vars);
    calc->insert("EMPOverTrkSysP", Variables::EMPOverTrkSysP, scalar_vars);
    calc->insert("ptRatioEflowApprox", Variables::ptRatioEflowApprox, scalar_vars);
    calc->insert("mEflowApprox", Variables::mEflowApprox, scalar_vars);
    calc->insert("dRmax", Variables::dRmax, scalar_vars);
    calc->insert("trFlightPathSig", Variables::trFlightPathSig, scalar_vars);
    calc->insert("massTrkSys", Variables::massTrkSys, scalar_vars);
    calc->insert("pt", Variables::pt, scalar_vars);
    calc->insert("pt_tau_log", Variables::pt_tau_log, scalar_vars);
    calc->insert("ptDetectorAxis", Variables::ptDetectorAxis, scalar_vars);
    calc->insert("ptIntermediateAxis", Variables::ptIntermediateAxis, scalar_vars);
    //---added for the eVeto
    /*calc->insert("ptJetSeed_log",              Variables::ptJetSeed_log, scalar_vars);
    calc->insert("absleadTrackEta",            Variables::absleadTrackEta, scalar_vars);
    calc->insert("leadTrackDeltaEta",          Variables::leadTrackDeltaEta, scalar_vars);
    calc->insert("leadTrackDeltaPhi",          Variables::leadTrackDeltaPhi, scalar_vars);
    calc->insert("leadTrackProbNNorHT",        Variables::leadTrackProbNNorHT, scalar_vars);
    calc->insert("EMFracFixed",                Variables::EMFracFixed, scalar_vars);
    calc->insert("etHotShotWinOverPtLeadTrk",  Variables::etHotShotWinOverPtLeadTrk, scalar_vars);
    calc->insert("hadLeakFracFixed",           Variables::hadLeakFracFixed, scalar_vars);
    calc->insert("PSFrac",                     Variables::PSFrac, scalar_vars);
    calc->insert("ClustersMeanCenterLambda",   Variables::ClustersMeanCenterLambda, scalar_vars);
    calc->insert("ClustersMeanFirstEngDens",   Variables::ClustersMeanFirstEngDens, scalar_vars);
    calc->insert("ClustersMeanPresamplerFrac", Variables::ClustersMeanPresamplerFrac, scalar_vars);
    */

    // Track variable calculator functions
    calc->insert("pt_log", Variables::Track::pt_log, track_vars);
    calc->insert("trackPt", Variables::Track::trackPt, track_vars);
    calc->insert("trackEta", Variables::Track::trackEta, track_vars);
    calc->insert("trackPhi", Variables::Track::trackPhi, track_vars);
    calc->insert("pt_tau_log", Variables::Track::pt_tau_log, track_vars);
    calc->insert("pt_jetseed_log", Variables::Track::pt_jetseed_log, track_vars);
    calc->insert("d0_abs_log", Variables::Track::d0_abs_log, track_vars);
    calc->insert("z0sinThetaTJVA_abs_log", Variables::Track::z0sinThetaTJVA_abs_log, track_vars);
    calc->insert("z0sinthetaTJVA", Variables::Track::z0sinthetaTJVA, track_vars);
    calc->insert("z0sinthetaSigTJVA", Variables::Track::z0sinthetaSigTJVA, track_vars);
    calc->insert("d0TJVA", Variables::Track::d0TJVA, track_vars);
    calc->insert("d0SigTJVA", Variables::Track::d0SigTJVA, track_vars);
    calc->insert("dEta", Variables::Track::dEta, track_vars);
    calc->insert("dEtaJetSeedAxis", Variables::Track::dEtaJetSeedAxis, track_vars);
    calc->insert("dPhi", Variables::Track::dPhi, track_vars);
    calc->insert("dPhiJetSeedAxis", Variables::Track::dPhiJetSeedAxis, track_vars);
    calc->insert("nInnermostPixelHits", Variables::Track::nInnermostPixelHits, track_vars);
    calc->insert("nPixelHits", Variables::Track::nPixelHits, track_vars);
    calc->insert("nSCTHits", Variables::Track::nSCTHits, track_vars);
    calc->insert("nIBLHitsAndExp", Variables::Track::nIBLHitsAndExp, track_vars);
    calc->insert("nPixelHitsPlusDeadSensors", Variables::Track::nPixelHitsPlusDeadSensors, track_vars);
    calc->insert("nSCTHitsPlusDeadSensors", Variables::Track::nSCTHitsPlusDeadSensors, track_vars);
    /* Development variables 
    //calc->insert("eProbabilityHT", Variables::Track::eProbabilityHT, track_vars);
    //calc->insert("eProbabilityNN", Variables::Track::eProbabilityNN, track_vars);
    //calc->insert("eProbabilityNNorHT", Variables::Track::eProbabilityNNorHT, track_vars);
    //calc->insert("chargedScoreRNN", Variables::Track::chargedScoreRNN, track_vars);
    //calc->insert("isolationScoreRNN", Variables::Track::isolationScoreRNN, track_vars);
    //calc->insert("conversionScoreRNN", Variables::Track::conversionScoreRNN, track_vars);
    //calc->insert("fakeScoreRNN", Variables::Track::fakeScoreRNN, track_vars);
    */

    //Extension - variables for GNTau
    calc->insert("numberOfInnermostPixelLayerHits", Variables::Track::numberOfInnermostPixelLayerHits, track_vars);
    /* Development variables
    calc->insert("numberOfPixelHits", Variables::Track::numberOfPixelHits, track_vars);
    calc->insert("numberOfPixelSharedHits", Variables::Track::numberOfPixelSharedHits, track_vars);
    calc->insert("numberOfPixelDeadSensors", Variables::Track::numberOfPixelDeadSensors, track_vars);
    calc->insert("numberOfSCTHits", Variables::Track::numberOfSCTHits, track_vars);
    calc->insert("numberOfSCTSharedHits", Variables::Track::numberOfSCTSharedHits, track_vars);
    calc->insert("numberOfSCTDeadSensors", Variables::Track::numberOfSCTDeadSensors, track_vars);
    calc->insert("numberOfTRTHighThresholdHits", Variables::Track::numberOfTRTHighThresholdHits, track_vars);
    calc->insert("numberOfTRTHits", Variables::Track::numberOfTRTHits, track_vars);
    calc->insert("nSiHits", Variables::Track::nSiHits, track_vars);
    calc->insert("expectInnermostPixelLayerHit", Variables::Track::expectInnermostPixelLayerHit, track_vars);
    calc->insert("expectNextToInnermostPixelLayerHit", Variables::Track::expectNextToInnermostPixelLayerHit, track_vars);
    calc->insert("numberOfContribPixelLayers", Variables::Track::numberOfContribPixelLayers, track_vars);
    calc->insert("numberOfPixelHoles", Variables::Track::numberOfPixelHoles, track_vars);
    calc->insert("d0_old", Variables::Track::d0_old, track_vars);
    calc->insert("qOverP", Variables::Track::qOverP, track_vars);
    calc->insert("theta", Variables::Track::theta, track_vars);
    calc->insert("z0TJVA", Variables::Track::z0TJVA, track_vars);
    calc->insert("charge", Variables::Track::charge, track_vars);
    calc->insert("dz0_TV_PV0", Variables::Track::dz0_TV_PV0, track_vars);
    calc->insert("log_sumpt_TV", Variables::Track::log_sumpt_TV, track_vars);
    calc->insert("log_sumpt2_TV", Variables::Track::log_sumpt2_TV, track_vars);
    calc->insert("log_sumpt_PV0", Variables::Track::log_sumpt_PV0, track_vars);
    calc->insert("log_sumpt2_PV0", Variables::Track::log_sumpt2_PV0, track_vars);
    */

    // Cluster variable calculator functions
    calc->insert("et_log", Variables::Cluster::et_log, cluster_vars);
    calc->insert("pt_tau_log", Variables::Cluster::pt_tau_log, cluster_vars);
    calc->insert("pt_jetseed_log", Variables::Cluster::pt_jetseed_log, cluster_vars);
    calc->insert("dEta", Variables::Cluster::dEta, cluster_vars);
    calc->insert("dPhi", Variables::Cluster::dPhi, cluster_vars);
    calc->insert("SECOND_R", Variables::Cluster::SECOND_R, cluster_vars);
    calc->insert("SECOND_LAMBDA", Variables::Cluster::SECOND_LAMBDA, cluster_vars);
    calc->insert("CENTER_LAMBDA", Variables::Cluster::CENTER_LAMBDA, cluster_vars);
    /*---added for the eVeto
    calc->insert("SECOND_LAMBDAOverClustersMeanSecondLambda", Variables::Cluster::SECOND_LAMBDAOverClustersMeanSecondLambda, cluster_vars);
    calc->insert("CENTER_LAMBDAOverClustersMeanCenterLambda", Variables::Cluster::CENTER_LAMBDAOverClustersMeanCenterLambda, cluster_vars);
    calc->insert("FirstEngDensOverClustersMeanFirstEngDens" , Variables::Cluster::FirstEngDensOverClustersMeanFirstEngDens, cluster_vars);
    */

    //Extension - Variables for GNTau
    calc->insert("e", Variables::Cluster::e, cluster_vars);
    calc->insert("et", Variables::Cluster::et, cluster_vars);
    
    /*Development variables
    calc->insert("FIRST_ENG_DENS", Variables::Cluster::FIRST_ENG_DENS, cluster_vars);
    calc->insert("EM_PROBABILITY", Variables::Cluster::EM_PROBABILITY, cluster_vars);
    calc->insert("CENTER_MAG", Variables::Cluster::CENTER_MAG, cluster_vars);
    */ 
    return calc;
}


namespace Variables {
using TauDetail = xAOD::TauJetParameters::Detail;

bool absEta(const xAOD::TauJet &tau, double &out) {
    out = std::abs(tau.eta());
    return true;
}

bool centFrac(const xAOD::TauJet &tau, double &out) {
    float centFrac;
    const auto success = tau.detail(TauDetail::centFrac, centFrac);
    //out = std::min(centFrac, 1.0f);
    out = centFrac;
    return success;
}

bool isolFrac(const xAOD::TauJet &tau, double &out) {
    float isolFrac;
    const auto success = tau.detail(TauDetail::isolFrac, isolFrac);
    //out = std::min(isolFrac, 1.0f);
    out = isolFrac;
    return success;
}

bool etOverPtLeadTrk(const xAOD::TauJet &tau, double &out) {
    float etOverPtLeadTrk;
    const auto success = tau.detail(TauDetail::etOverPtLeadTrk, etOverPtLeadTrk);
    out = etOverPtLeadTrk;
    return success;
}

bool innerTrkAvgDist(const xAOD::TauJet &tau, double &out) {
    float innerTrkAvgDist;
    const auto success = tau.detail(TauDetail::innerTrkAvgDist, innerTrkAvgDist);
    out = innerTrkAvgDist;
    return success;
}

bool absipSigLeadTrk(const xAOD::TauJet &tau, double &out) {
    float ipSigLeadTrk = (tau.nTracks()>0) ? tau.track(0)->d0SigTJVA() : 0.;
    //out = std::min(std::abs(ipSigLeadTrk), 30.0f);
    out = std::abs(ipSigLeadTrk);
    return true;
}

bool sumEMCellEtOverLeadTrkPt(const xAOD::TauJet &tau, double &out) {
    float sumEMCellEtOverLeadTrkPt;
    const auto success = tau.detail(TauDetail::sumEMCellEtOverLeadTrkPt, sumEMCellEtOverLeadTrkPt);
    out = sumEMCellEtOverLeadTrkPt;
    return success;
}

bool SumPtTrkFrac(const xAOD::TauJet &tau, double &out) {
    float SumPtTrkFrac;
    const auto success = tau.detail(TauDetail::SumPtTrkFrac, SumPtTrkFrac);
    out = SumPtTrkFrac;
    return success;
}

bool EMPOverTrkSysP(const xAOD::TauJet &tau, double &out) {
    float EMPOverTrkSysP;
    const auto success = tau.detail(TauDetail::EMPOverTrkSysP, EMPOverTrkSysP);
    out = EMPOverTrkSysP;
    return success;
}

bool ptRatioEflowApprox(const xAOD::TauJet &tau, double &out) {
    float ptRatioEflowApprox;
    const auto success = tau.detail(TauDetail::ptRatioEflowApprox, ptRatioEflowApprox);
    //out = std::min(ptRatioEflowApprox, 4.0f);
    out = ptRatioEflowApprox;
    return success;
}

bool mEflowApprox(const xAOD::TauJet &tau, double &out) {
    float mEflowApprox;
    const auto success = tau.detail(TauDetail::mEflowApprox, mEflowApprox);
    out = mEflowApprox;
    return success;
}

bool dRmax(const xAOD::TauJet &tau, double &out) {
    float dRmax;
    const auto success = tau.detail(TauDetail::dRmax, dRmax);
    out = dRmax;
    return success;
}

bool trFlightPathSig(const xAOD::TauJet &tau, double &out) {
    float trFlightPathSig;
    const auto success = tau.detail(TauDetail::trFlightPathSig, trFlightPathSig);
    out = trFlightPathSig;
    return success;
}

bool massTrkSys(const xAOD::TauJet &tau, double &out) {
    float massTrkSys;
    const auto success = tau.detail(TauDetail::massTrkSys, massTrkSys);
    out = massTrkSys;
    return success;
}

bool pt(const xAOD::TauJet &tau, double &out) {
    out = tau.pt();
    return true;
}

bool pt_tau_log(const xAOD::TauJet &tau, double &out) {
    out = std::log10(std::max(tau.pt() / GeV, 1e-6));
    return true;
}

bool ptDetectorAxis(const xAOD::TauJet &tau, double &out) {
    out = tau.ptDetectorAxis();
    return true;
}

bool ptIntermediateAxis(const xAOD::TauJet &tau, double &out) {
    out = tau.ptIntermediateAxis();
    return true;
}

bool ptJetSeed_log(const xAOD::TauJet &tau, double &out) {
  out = std::log10(std::max(tau.ptJetSeed(), 1e-3));
  return true;
}

bool absleadTrackEta(const xAOD::TauJet &tau, double &out){
  static const SG::ConstAccessor<float> acc_absEtaLeadTrack("ABS_ETA_LEAD_TRACK");
  float absEtaLeadTrack = acc_absEtaLeadTrack(tau);
  out = std::max(0.f, absEtaLeadTrack);
  return true;
}

bool leadTrackDeltaEta(const xAOD::TauJet &tau, double &out){
  static const SG::ConstAccessor<float> acc_absDeltaEta("TAU_ABSDELTAETA");
  float absDeltaEta = acc_absDeltaEta(tau);
  out = std::max(0.f, absDeltaEta);
  return true;
}

bool leadTrackDeltaPhi(const xAOD::TauJet &tau, double &out){
  static const SG::ConstAccessor<float> acc_absDeltaPhi("TAU_ABSDELTAPHI");
  float absDeltaPhi = acc_absDeltaPhi(tau);
  out = std::max(0.f, absDeltaPhi);
  return true;
}

bool leadTrackProbNNorHT(const xAOD::TauJet &tau, double &out){
  auto tracks = tau.allTracks();

  // Sort tracks in descending pt order
  if (!tracks.empty()) {
    auto cmp_pt = [](const xAOD::TauTrack *lhs, const xAOD::TauTrack *rhs) {
      return lhs->pt() > rhs->pt();
    };
    std::sort(tracks.begin(), tracks.end(), cmp_pt);

    const xAOD::TauTrack* tauLeadTrack = tracks.at(0);
    const xAOD::TrackParticle* xTrackParticle = tauLeadTrack->track();
    float eProbabilityHT = xTrackParticle->summaryValue(eProbabilityHT, xAOD::eProbabilityHT);
    static const SG::ConstAccessor<float> acc_eProbabilityNN("eProbabilityNN");
    float eProbabilityNN = acc_eProbabilityNN(*xTrackParticle);
    out = (tauLeadTrack->pt()>2000.) ? eProbabilityNN : eProbabilityHT;
  }
  else {
    out = 0.;
  }
  return true;
}

bool EMFracFixed(const xAOD::TauJet &tau, double &out){
  static const SG::ConstAccessor<float> acc_emFracFixed("EMFracFixed");
  float emFracFixed = acc_emFracFixed(tau);
  out = std::max(emFracFixed, 0.0f);
  return true;
}

bool etHotShotWinOverPtLeadTrk(const xAOD::TauJet &tau, double &out){
  static const SG::ConstAccessor<float> acc_etHotShotWinOverPtLeadTrk("etHotShotWinOverPtLeadTrk");
  float etHotShotWinOverPtLeadTrk = acc_etHotShotWinOverPtLeadTrk(tau);
  out = std::max(etHotShotWinOverPtLeadTrk, 1e-6f);
  return true;
}

bool hadLeakFracFixed(const xAOD::TauJet &tau, double &out){
  static const SG::ConstAccessor<float> acc_hadLeakFracFixed("hadLeakFracFixed");
  float hadLeakFracFixed = acc_hadLeakFracFixed(tau);
  out = std::max(0.f, hadLeakFracFixed);
  return true;
}

bool PSFrac(const xAOD::TauJet &tau, double &out){
  float PSFrac;
  const auto success = tau.detail(TauDetail::PSSFraction, PSFrac);
  out = std::max(0.f,PSFrac);  
  return success;
}

bool ClustersMeanCenterLambda(const xAOD::TauJet &tau, double &out){
  float ClustersMeanCenterLambda;
  const auto success = tau.detail(TauDetail::ClustersMeanCenterLambda, ClustersMeanCenterLambda);
  out = std::max(0.f, ClustersMeanCenterLambda);
  return success;
}

bool ClustersMeanEMProbability(const xAOD::TauJet &tau, double &out){
  float ClustersMeanEMProbability;
  const auto success = tau.detail(TauDetail::ClustersMeanEMProbability, ClustersMeanEMProbability);
  out = std::max(0.f, ClustersMeanEMProbability);
  return success;
}

bool ClustersMeanFirstEngDens(const xAOD::TauJet &tau, double &out){
  float ClustersMeanFirstEngDens;
  const auto success = tau.detail(TauDetail::ClustersMeanFirstEngDens, ClustersMeanFirstEngDens);
  out =  std::max(-10.f, ClustersMeanFirstEngDens);
  return success;
}

bool ClustersMeanPresamplerFrac(const xAOD::TauJet &tau, double &out){
  float ClustersMeanPresamplerFrac;
  const auto success = tau.detail(TauDetail::ClustersMeanPresamplerFrac, ClustersMeanPresamplerFrac);
  out = std::max(0.f, ClustersMeanPresamplerFrac);
  return success;
}

bool ClustersMeanSecondLambda(const xAOD::TauJet &tau, double &out){
  float ClustersMeanSecondLambda;
  const auto success = tau.detail(TauDetail::ClustersMeanSecondLambda, ClustersMeanSecondLambda);
  out = std::max(0.f, ClustersMeanSecondLambda);
  return success;
}

namespace Track {

bool pt_log(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = std::log10(track.pt());
    return true;
}

bool trackPt(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.pt();
    return true;
}

bool trackEta(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.eta();
    return true;
}

bool trackPhi(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.phi();
    return true;
}

bool pt_tau_log(const xAOD::TauJet &tau, const xAOD::TauTrack& /*track*/, double &out) {
    out = std::log10(std::max(tau.pt(), 1e-6));
    return true;
}

bool pt_jetseed_log(const xAOD::TauJet &tau, const xAOD::TauTrack& /*track*/, double &out) {
    out = std::log10(tau.ptJetSeed());
    return true;
}

bool d0_abs_log(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = std::log10(std::abs(track.d0TJVA()) + 1e-6);
    return true;
}

bool z0sinThetaTJVA_abs_log(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = std::log10(std::abs(track.z0sinthetaTJVA()) + 1e-6);
    return true;
}

bool z0sinthetaTJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.z0sinthetaTJVA();
    return true;
}

bool z0sinthetaSigTJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.z0sinthetaSigTJVA();
    return true;
}

bool d0TJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.d0TJVA();
    return true;
}

bool d0SigTJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.d0SigTJVA();
    return true;
}

bool dEta(const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out) {
    out = track.eta() - tau.eta();
    return true;
}

bool dEtaJetSeedAxis(const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out) {
    TLorentzVector tlvSeedJet = tau.p4(xAOD::TauJetParameters::JetSeed);
    out = std::abs(tlvSeedJet.Eta() - track.eta());
    return true;
}

bool dPhi(const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out) {
    out = track.p4().DeltaPhi(tau.p4());
    return true;
}

bool dPhiJetSeedAxis(const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out) {
    TLorentzVector tlvSeedJet = tau.p4(xAOD::TauJetParameters::JetSeed);
    out = tlvSeedJet.DeltaPhi(track.p4());
    return true;
}

bool nInnermostPixelHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t inner_pixel_hits;
    const auto success = track.track()->summaryValue(inner_pixel_hits, xAOD::numberOfInnermostPixelLayerHits);
    out = inner_pixel_hits;
    return success;
}

bool nPixelHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t pixel_hits;
    const auto success = track.track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
    out = pixel_hits;
    return success;
}

bool nSCTHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t sct_hits;
    const auto success = track.track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
    out = sct_hits;
    return success;
}

// same as in tau track classification for trigger
bool nIBLHitsAndExp(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t inner_pixel_hits, inner_pixel_exp;
    const auto success1 = track.track()->summaryValue(inner_pixel_hits, xAOD::numberOfInnermostPixelLayerHits);
    const auto success2 = track.track()->summaryValue(inner_pixel_exp, xAOD::expectInnermostPixelLayerHit);
    out =  inner_pixel_exp ? inner_pixel_hits : 1.;
    return success1 && success2;
}

bool nPixelHitsPlusDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t pixel_hits, pixel_dead;
    const auto success1 = track.track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
    const auto success2 = track.track()->summaryValue(pixel_dead, xAOD::numberOfPixelDeadSensors);
    out = pixel_hits + pixel_dead;
    return success1 && success2;
}

bool nSCTHitsPlusDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t sct_hits, sct_dead;
    const auto success1 = track.track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
    const auto success2 = track.track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
    out = sct_hits + sct_dead;
    return success1 && success2;
}

bool eProbabilityHT(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    float eProbabilityHT;
    const auto success = track.track()->summaryValue(eProbabilityHT, xAOD::eProbabilityHT);
    out = eProbabilityHT;
    return success;
}

bool eProbabilityNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {  
    static const SG::ConstAccessor<float> acc_eProbabilityNN("eProbabilityNN");
    out = acc_eProbabilityNN(track);
    return true;
}

bool eProbabilityNNorHT(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {  
  auto atrack = track.track();
  float eProbabilityHT = atrack->summaryValue(eProbabilityHT, xAOD::eProbabilityHT);
  static const SG::ConstAccessor<float> acc_eProbabilityNN("eProbabilityNN");
  float eProbabilityNN = acc_eProbabilityNN(*atrack);
  out = (atrack->pt()>2000.) ? eProbabilityNN : eProbabilityHT;
  return true;
}

bool chargedScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
  static const SG::ConstAccessor<float> acc_chargedScoreRNN("rnn_chargedScore");
  out = acc_chargedScoreRNN(track);
  return true;
}

bool isolationScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
  static const SG::ConstAccessor<float> acc_isolationScoreRNN("rnn_isolationScore");
  out = acc_isolationScoreRNN(track);
  return true;
}

bool conversionScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
  static const SG::ConstAccessor<float> acc_conversionScoreRNN("rnn_conversionScore");
  out = acc_conversionScoreRNN(track);
  return true;
}

bool fakeScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
  static const SG::ConstAccessor<float> acc_fakeScoreRNN("rnn_fakeScore");
  out = acc_fakeScoreRNN(track);
  return true;
}

//Extension - variables for GNTau
bool numberOfInnermostPixelLayerHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfInnermostPixelLayerHits);
    out = trk_val;
    return success;
}

bool numberOfPixelHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelHits);
    out = trk_val;
    return success;
}

bool numberOfPixelSharedHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelSharedHits);
    out = trk_val;
    return success;
}

bool numberOfPixelDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelDeadSensors);
    out = trk_val;
    return success;
}

bool numberOfSCTHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfSCTHits);
    out = trk_val;
    return success;
}

bool numberOfSCTSharedHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfSCTSharedHits);
    out = trk_val;
    return success;
}

bool numberOfSCTDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfSCTDeadSensors);
    out = trk_val;
    return success;
}

bool numberOfTRTHighThresholdHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfTRTHighThresholdHits);
    out = trk_val;
    return success;
}

bool numberOfTRTHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfTRTHits);
    out = trk_val;
    return success;
}

bool nSiHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t pix_hit = 0;uint8_t pix_dead = 0;uint8_t sct_hit = 0;uint8_t sct_dead = 0;
    const auto success1 = track.track()->summaryValue(pix_hit, xAOD::numberOfPixelHits);
    const auto success2 = track.track()->summaryValue(pix_dead, xAOD::numberOfPixelDeadSensors);
    const auto success3 = track.track()->summaryValue(sct_hit, xAOD::numberOfSCTHits);
    const auto success4 = track.track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
    out = pix_hit + pix_dead + sct_hit + sct_dead;
    return success1 && success2 && success3 && success4;
}

bool expectInnermostPixelLayerHit(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::expectInnermostPixelLayerHit);
    out = trk_val;
    return success;
}

bool expectNextToInnermostPixelLayerHit(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::expectNextToInnermostPixelLayerHit);
    out = trk_val;
    return success;
}

bool numberOfContribPixelLayers(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfContribPixelLayers);
    out = trk_val;
    return success;
}

bool numberOfPixelHoles(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelHoles);
    out = trk_val;
    return success;
}

bool d0_old(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.track()->d0();
    //out = trk_val;
    return true;
}

bool qOverP(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.track()->qOverP();
    return true;
}

bool theta(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.track()->theta();
    return true;
}

bool z0TJVA(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out) {
    out = track.track()->z0() + track.track()->vz() - tau.vertex()->z();
    return true;
}

bool charge(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, double &out) {
    out = track.track()->charge();
    return true;
}

bool dz0_TV_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &/*track*/, double &out) {
    static const SG::ConstAccessor<float> acc_dz0TVPV0("dz0_TV_PV0");
    out = acc_dz0TVPV0.withDefault(tau, 0);
    return true;
}

bool log_sumpt_TV(const xAOD::TauJet& tau, const xAOD::TauTrack &/*track*/, double &out) {
    static const SG::ConstAccessor<float> acc_logsumptTV("log_sumpt_TV");
    out = acc_logsumptTV.withDefault(tau, 0);
    return true;
}

bool log_sumpt2_TV(const xAOD::TauJet& tau, const xAOD::TauTrack &/*track*/, double &out) {
    static const SG::ConstAccessor<float> acc_logsumpt2TV("log_sumpt2_TV");
    out = acc_logsumpt2TV.withDefault(tau, 0);
    return true;
}

bool log_sumpt_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &/*track*/, double &out) {
    static const SG::ConstAccessor<float> acc_logsumptPV0("log_sumpt_PV0");
    out = acc_logsumptPV0.withDefault(tau, 0);
    return true;
}

bool log_sumpt2_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &/*track*/, double &out) {
    static const SG::ConstAccessor<float> acc_logsumpt2PV0("log_sumpt2_PV0");
    out = acc_logsumpt2PV0.withDefault(tau, 0);
    return true;
}

} // namespace Track


namespace Cluster {
using MomentType = xAOD::CaloCluster::MomentType;

bool et_log(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    out = std::log10(cluster.p4().Et());
    return true;
}

bool pt_tau_log(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster& /*cluster*/, double &out) {
    out = std::log10(std::max(tau.pt(), 1e-6));
    return true;
}

bool pt_jetseed_log(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster& /*cluster*/, double &out) {
    out = std::log10(tau.ptJetSeed());
    return true;
}

bool dEta(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    out = cluster.eta() - tau.eta();
    return true;
}

bool dPhi(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    out = cluster.p4().DeltaPhi(tau.p4());
    return true;
}

bool SECOND_R(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    const auto success = cluster.clust().retrieveMoment(MomentType::SECOND_R, out);
    return success;
}

bool SECOND_LAMBDA(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    const auto success = cluster.clust().retrieveMoment(MomentType::SECOND_LAMBDA, out);
    return success;
}

bool CENTER_LAMBDA(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    const auto success = cluster.clust().retrieveMoment(MomentType::CENTER_LAMBDA, out);
    return success;
}

bool SECOND_LAMBDAOverClustersMeanSecondLambda(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
  static const SG::ConstAccessor<float> acc_ClustersMeanSecondLambda("ClustersMeanSecondLambda");
  float ClustersMeanSecondLambda = acc_ClustersMeanSecondLambda(tau);
  double secondLambda(0);
  const auto success = cluster.clust().retrieveMoment(MomentType::SECOND_LAMBDA, secondLambda);
  out = (ClustersMeanSecondLambda != 0.) ? secondLambda/ClustersMeanSecondLambda : 0.;
  return success;
}

bool CENTER_LAMBDAOverClustersMeanCenterLambda(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
  static const SG::ConstAccessor<float> acc_ClustersMeanCenterLambda("ClustersMeanCenterLambda");
  float ClustersMeanCenterLambda = acc_ClustersMeanCenterLambda(tau);
  double centerLambda(0);
  const auto success = cluster.clust().retrieveMoment(MomentType::CENTER_LAMBDA, centerLambda);
  if (ClustersMeanCenterLambda == 0.){
    out = 250.;
  }else {
    out = centerLambda/ClustersMeanCenterLambda;
  }

  out = std::min(out, 250.);

  return success;
}


bool FirstEngDensOverClustersMeanFirstEngDens(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
  // the ClustersMeanFirstEngDens is the log10 of the energy weighted average of the First_ENG_DENS 
  // divided by ETot to make it dimension-less, 
  // so we need to evaluate the difference of log10(clusterFirstEngDens/clusterTotalEnergy) and the ClustersMeanFirstEngDens
  double clusterFirstEngDens = 0.0;
  bool status = cluster.clust().retrieveMoment(MomentType::FIRST_ENG_DENS, clusterFirstEngDens);
  if (clusterFirstEngDens < 1e-6) clusterFirstEngDens = 1e-6;

  static const SG::ConstAccessor<float> acc_ClusterTotalEnergy("ClusterTotalEnergy");
  float clusterTotalEnergy = acc_ClusterTotalEnergy(tau);
  if (clusterTotalEnergy < 1e-6) clusterTotalEnergy = 1e-6;

  static const SG::ConstAccessor<float> acc_ClustersMeanFirstEngDens("ClustersMeanFirstEngDens");
  float clustersMeanFirstEngDens = acc_ClustersMeanFirstEngDens(tau);

  out = std::log10(clusterFirstEngDens/clusterTotalEnergy) - clustersMeanFirstEngDens;
  
  return status;
}

//Extension - Variables for GNTau
bool e(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    out = cluster.p4().E();
    return true;
}

bool et(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    out = cluster.p4().Et();
    return true;
}

bool FIRST_ENG_DENS(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    double clusterFirstEngDens = 0.0;
    bool status = cluster.clust().retrieveMoment(MomentType::FIRST_ENG_DENS, clusterFirstEngDens);
    out = clusterFirstEngDens;
    return status;
}

bool EM_PROBABILITY(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    double clusterEMprob = 0.0;
    bool status = cluster.clust().retrieveMoment(MomentType::EM_PROBABILITY, clusterEMprob);
    out = clusterEMprob;
    return status;
}

bool CENTER_MAG(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, double &out) {
    double clusterCenterMag = 0.0;
    bool status = cluster.clust().retrieveMoment(MomentType::CENTER_MAG, clusterCenterMag);
    out = clusterCenterMag;
    return status;
}

} // namespace Cluster
} // namespace Variables
} // namespace TauGNNUtils
