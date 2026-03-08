/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNNDataLoader.h"

using ScalarCalcByRef_t  = std::function<bool(const xAOD::TauJet &, float &)>;
using ScalarCalc_t       = std::function<float(const xAOD::IParticle*)>;

TauGNNDataLoader::TauGNNDataLoader(
  std::shared_ptr<const FlavorTagInference::SaltModel> salt_model, 
  const TauGNNDataLoader::Config& config
) :
    FlavorTagInference::SaltModelEDMLoaderBase(salt_model),
    asg::AsgMessaging("TauGNNDataLoader")
  {
    scalarInputName = config.input_layer_scalar;
    const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig* scalar_input_node = nullptr;
    const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig* track_input_node = nullptr;
    const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig* cluster_input_node = nullptr;
    const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig* hit_input_node = nullptr;
    for (const auto &in_node : graph_config.inputs) {
        if (in_node.name == config.input_layer_scalar) {
            scalar_input_node = &in_node;
            ATH_MSG_DEBUG("Found scalar input node: " << in_node.name);
            break;
        }
    }
    for (const auto &in_node : graph_config.input_sequences) {
        if (in_node.name == config.input_layer_tracks) {
            track_input_node = &in_node;
            ATH_MSG_DEBUG("Found track input node: " << in_node.name);
        }
        if (in_node.name == config.input_layer_clusters) {
            cluster_input_node = &in_node;
            ATH_MSG_DEBUG("Found cluster input node: " << in_node.name);
        }
        if (in_node.name == config.input_layer_hits) {
            hit_input_node = &in_node;
            ATH_MSG_DEBUG("Found hit input node: " << in_node.name);
        }
    }

    // Fill the variable names of each input layer into the corresponding vector
    if (scalar_input_node) {
        for (const auto &in : scalar_input_node->variables) {
            addScalarLoader(in.name, getScalarCalc(in.name));
        }
    } else if(!config.input_layer_scalar.empty()) {
        ATH_MSG_ERROR("Scalar input node '" + config.input_layer_scalar + "' not found in the model input configuration");
        throw std::runtime_error("Scalar input node '" + config.input_layer_scalar + "' not found in the model input configuration");
    }

    if (track_input_node) {
        FlavorTagInference::ConstituentsInputConfig trk_config;
        trk_config.name = "tautracks";
        trk_config.output_name = config.input_layer_tracks;
        trk_config.type = FlavorTagInference::ConstituentsType::TAUTRACK;
        trk_config.order = FlavorTagInference::ConstituentsSortOrder::PT_DESCENDING;
        trk_config.max_n_constituents = config.n_max_tracks;
        trk_config.selection = config.trackClassification ? FlavorTagInference::ConstituentsSelection::TAUTRACK_CLASSIFIED : FlavorTagInference::ConstituentsSelection::ALL;
        trk_config.inputs = {};
        for (const auto &in : track_input_node->variables) {
            if (!config.useTRT && (in.name == "eProbabilityHT")) {
                ATH_MSG_WARNING("Track variable 'eProbabilityHT' requested but useTRT set to false. Using 'eProbabilityHT_noTRT' instead.");
                trk_config.inputs.push_back({"eProbabilityHT_noTRT", FlavorTagInference::ConstituentsEDMType::CUSTOM_GETTER, false});
                continue;
            }
            trk_config.inputs.push_back({in.name, FlavorTagInference::ConstituentsEDMType::CUSTOM_GETTER, false});
        }
        addVectorLoader(config.input_layer_tracks, std::make_shared<FlavorTagInference::ConstituentLoaderTauTrack>(trk_config));
    } else if(!config.input_layer_tracks.empty() && config.n_max_tracks > 0) {
        ATH_MSG_ERROR("Track input node '" + config.input_layer_tracks + "' not found in the model input configuration");
        throw std::runtime_error("Track input node '" + config.input_layer_tracks + "' not found in the model input configuration");
    }

    if (cluster_input_node) {
        FlavorTagInference::ConstituentsInputConfig cls_config;
        cls_config.name = "tauclusters";
        cls_config.output_name = config.input_layer_clusters;
        cls_config.type = FlavorTagInference::ConstituentsType::TAUCLUSTER;
        cls_config.order = FlavorTagInference::ConstituentsSortOrder::PT_DESCENDING;
        cls_config.max_n_constituents = config.n_max_clusters;
        cls_config.selection = FlavorTagInference::ConstituentsSelection::ALL;
        cls_config.inputs = {};
        for (const auto &in : cluster_input_node->variables) {
            cls_config.inputs.push_back({in.name, FlavorTagInference::ConstituentsEDMType::CUSTOM_GETTER, false});
        }
        addVectorLoader(config.input_layer_clusters, std::make_shared<FlavorTagInference::ConstituentLoaderTauCluster>(cls_config, config.max_dr_cluster, config.doVertexCorrection));
    } else if(!config.input_layer_clusters.empty() && config.n_max_clusters > 0) {
        ATH_MSG_ERROR("Cluster input node '" + config.input_layer_clusters + "' not found in the model input configuration");
        throw std::runtime_error("Cluster input node '" + config.input_layer_clusters + "' not found in the model input configuration");
    }

    if (hit_input_node) {
        FlavorTagInference::ConstituentsInputConfig cls_config;
        cls_config.name = "tauhits";
        cls_config.output_name = config.input_layer_hits;
        cls_config.type = FlavorTagInference::ConstituentsType::HIT;
        cls_config.order = FlavorTagInference::ConstituentsSortOrder::UNDEFINED;
        cls_config.max_n_constituents = config.n_max_hits;
        cls_config.selection = FlavorTagInference::ConstituentsSelection::ALL;
        cls_config.inputs = {};
        for (const auto &in : hit_input_node->variables) {
            cls_config.inputs.push_back({in.name, FlavorTagInference::ConstituentsEDMType::CUSTOM_GETTER, false});
        }
        addVectorLoader(config.input_layer_hits, std::make_shared<FlavorTagInference::ConstituentLoaderTauHit>(cls_config, config.hits_decor_name));
    } else if(!config.input_layer_hits.empty() && config.n_max_hits > 0) {
        ATH_MSG_ERROR("Hit input node '" + config.input_layer_hits + "' not found in the model input configuration");
        throw std::runtime_error("Hit input node '" + config.input_layer_hits + "' not found in the model input configuration");
    }
}

ScalarCalc_t TauGNNDataLoader::getScalarCalc(const std::string &name) const {
    // Retrieve calculator function
    ScalarCalcByRef_t func = nullptr;
    try {
        func = m_func_map.at(name);
    } catch (const std::out_of_range &e) {
        ATH_MSG_ERROR("Variable '" << name << "' not defined");
        throw std::runtime_error("Variable '" + name + "' not defined");
    }
    return [func](const xAOD::IParticle* p) {
        auto tau = dynamic_cast<const xAOD::TauJet*>(p);
        float out;
        if (!tau) {
            throw std::runtime_error("Invalid TauJet pointer");
        }
        auto success = func(*tau, out);
        if (!success) {
            throw std::runtime_error("Error in scalar variable calculation ");
        }
        return out;
    };
}


namespace TauScalarVars {
using TauDetail = xAOD::TauJetParameters::Detail;

bool eta(const xAOD::TauJet &tau, float &out) {
    out = tau.eta();
    return true;
}

bool absEta(const xAOD::TauJet &tau, float &out) {
    out = std::abs(tau.eta());
    return true;
}

bool centFrac(const xAOD::TauJet &tau, float &out) {
    float centFrac;
    const auto success = tau.detail(TauDetail::centFrac, centFrac);
    //out = std::min(centFrac, 1.0f);
    out = centFrac;
    return success;
}

bool isolFrac(const xAOD::TauJet &tau, float &out) {
    float isolFrac;
    const auto success = tau.detail(TauDetail::isolFrac, isolFrac);
    //out = std::min(isolFrac, 1.0f);
    out = isolFrac;
    return success;
}

bool etOverPtLeadTrk(const xAOD::TauJet &tau, float &out) {
    float etOverPtLeadTrk;
    const auto success = tau.detail(TauDetail::etOverPtLeadTrk, etOverPtLeadTrk);
    out = etOverPtLeadTrk;
    return success;
}

bool innerTrkAvgDist(const xAOD::TauJet &tau, float &out) {
    float innerTrkAvgDist;
    const auto success = tau.detail(TauDetail::innerTrkAvgDist, innerTrkAvgDist);
    out = innerTrkAvgDist;
    return success;
}

bool absipSigLeadTrk(const xAOD::TauJet &tau, float &out) {
    float ipSigLeadTrk = (tau.nTracks()>0) ? tau.track(0)->d0SigTJVA() : 0.;
    //out = std::min(std::abs(ipSigLeadTrk), 30.0f);
    out = std::abs(ipSigLeadTrk);
    return true;
}

bool sumEMCellEtOverLeadTrkPt(const xAOD::TauJet &tau, float &out) {
    float sumEMCellEtOverLeadTrkPt;
    const auto success = tau.detail(TauDetail::sumEMCellEtOverLeadTrkPt, sumEMCellEtOverLeadTrkPt);
    out = sumEMCellEtOverLeadTrkPt;
    return success;
}

bool SumPtTrkFrac(const xAOD::TauJet &tau, float &out) {
    float SumPtTrkFrac;
    const auto success = tau.detail(TauDetail::SumPtTrkFrac, SumPtTrkFrac);
    out = SumPtTrkFrac;
    return success;
}

bool EMPOverTrkSysP(const xAOD::TauJet &tau, float &out) {
    float EMPOverTrkSysP;
    const auto success = tau.detail(TauDetail::EMPOverTrkSysP, EMPOverTrkSysP);
    out = EMPOverTrkSysP;
    return success;
}

bool ptRatioEflowApprox(const xAOD::TauJet &tau, float &out) {
    float ptRatioEflowApprox;
    const auto success = tau.detail(TauDetail::ptRatioEflowApprox, ptRatioEflowApprox);
    //out = std::min(ptRatioEflowApprox, 4.0f);
    out = ptRatioEflowApprox;
    return success;
}

bool mEflowApprox(const xAOD::TauJet &tau, float &out) {
    float mEflowApprox;
    const auto success = tau.detail(TauDetail::mEflowApprox, mEflowApprox);
    out = mEflowApprox;
    return success;
}

bool dRmax(const xAOD::TauJet &tau, float &out) {
    float dRmax;
    const auto success = tau.detail(TauDetail::dRmax, dRmax);
    out = dRmax;
    return success;
}

bool trFlightPathSig(const xAOD::TauJet &tau, float &out) {
    float trFlightPathSig;
    const auto success = tau.detail(TauDetail::trFlightPathSig, trFlightPathSig);
    out = trFlightPathSig;
    return success;
}

bool massTrkSys(const xAOD::TauJet &tau, float &out) {
    float massTrkSys;
    const auto success = tau.detail(TauDetail::massTrkSys, massTrkSys);
    out = massTrkSys;
    return success;
}

bool pt(const xAOD::TauJet &tau, float &out) {
    out = tau.pt();
    return true;
}

bool pt_tau_log(const xAOD::TauJet &tau, float &out) {
    out = std::log10(std::max(tau.pt() / 1000., 1e-6));
    return true;
}

bool ptDetectorAxis(const xAOD::TauJet &tau, float &out) {
    out = tau.ptDetectorAxis();
    return true;
}

bool ptIntermediateAxis(const xAOD::TauJet &tau, float &out) {
    out = tau.ptIntermediateAxis();
    return true;
}

bool ptJetSeed(const xAOD::TauJet &tau, float &out) {
  out = tau.ptJetSeed();
  return true;
}

bool etaJetSeed(const xAOD::TauJet &tau, float &out) {
  out = tau.etaJetSeed();
  return true;
}

bool ptJetSeed_log(const xAOD::TauJet &tau, float &out) {
  out = std::log10(std::max(tau.ptJetSeed(), 1e-3));
  return true;
}

bool absleadTrackEta(const xAOD::TauJet &tau, float &out){
  static const SG::ConstAccessor<float> acc_absEtaLeadTrack("ABS_ETA_LEAD_TRACK");
  float absEtaLeadTrack = acc_absEtaLeadTrack(tau);
  out = std::max(0.f, absEtaLeadTrack);
  return true;
}

bool leadTrackDeltaEta(const xAOD::TauJet &tau, float &out){
  float absDeltaEta =  tau.nTracks() > 0 ? std::abs( tau.track(0)->track()->eta() - tau.eta() ) : -1111.;
  out = std::max(0.f, absDeltaEta);
  return true;
}

bool leadTrackDeltaPhi(const xAOD::TauJet &tau, float &out){
  float absDeltaPhi =  tau.nTracks() > 0 ? std::abs( tau.track(0)->track()->p4().DeltaPhi(tau.p4()) ) : -1111.;
  out = std::max(0.f, absDeltaPhi);
  return true;
}

bool leadTrackProbNNorHT(const xAOD::TauJet &tau, float &out){
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

bool EMFracFixed(const xAOD::TauJet &tau, float &out){
  static const SG::ConstAccessor<float> acc_emFracFixed("EMFracFixed");
  float emFracFixed = acc_emFracFixed(tau);
  out = std::max(emFracFixed, 0.0f);
  return true;
}

bool etHotShotWinOverPtLeadTrk(const xAOD::TauJet &tau, float &out){
  static const SG::ConstAccessor<float> acc_etHotShotWinOverPtLeadTrk("etHotShotWinOverPtLeadTrk");
  float etHotShotWinOverPtLeadTrk = acc_etHotShotWinOverPtLeadTrk(tau);
  out = std::max(etHotShotWinOverPtLeadTrk, 1e-6f);
  return true;
}

bool hadLeakFracFixed(const xAOD::TauJet &tau, float &out){
  static const SG::ConstAccessor<float> acc_hadLeakFracFixed("hadLeakFracFixed");
  float hadLeakFracFixed = acc_hadLeakFracFixed(tau);
  out = std::max(0.f, hadLeakFracFixed);
  return true;
}

bool PSFrac(const xAOD::TauJet &tau, float &out){
  float PSFrac;
  const auto success = tau.detail(TauDetail::PSSFraction, PSFrac);
  out = std::max(0.f,PSFrac);  
  return success;
}

bool ClustersMeanCenterLambda(const xAOD::TauJet &tau, float &out){
  float ClustersMeanCenterLambda;
  const auto success = tau.detail(TauDetail::ClustersMeanCenterLambda, ClustersMeanCenterLambda);
  out = std::max(0.f, ClustersMeanCenterLambda);
  return success;
}

bool ClustersMeanEMProbability(const xAOD::TauJet &tau, float &out){
  float ClustersMeanEMProbability;
  const auto success = tau.detail(TauDetail::ClustersMeanEMProbability, ClustersMeanEMProbability);
  out = std::max(0.f, ClustersMeanEMProbability);
  return success;
}

bool ClustersMeanFirstEngDens(const xAOD::TauJet &tau, float &out){
  float ClustersMeanFirstEngDens;
  const auto success = tau.detail(TauDetail::ClustersMeanFirstEngDens, ClustersMeanFirstEngDens);
  out =  std::max(-10.f, ClustersMeanFirstEngDens);
  return success;
}

bool ClustersMeanPresamplerFrac(const xAOD::TauJet &tau, float &out){
  float ClustersMeanPresamplerFrac;
  const auto success = tau.detail(TauDetail::ClustersMeanPresamplerFrac, ClustersMeanPresamplerFrac);
  out = std::max(0.f, ClustersMeanPresamplerFrac);
  return success;
}

bool ClustersMeanSecondLambda(const xAOD::TauJet &tau, float &out){
  float ClustersMeanSecondLambda;
  const auto success = tau.detail(TauDetail::ClustersMeanSecondLambda, ClustersMeanSecondLambda);
  out = std::max(0.f, ClustersMeanSecondLambda);
  return success;
}
} //namespace Scalar
