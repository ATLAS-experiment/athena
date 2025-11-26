/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/ConstituentsLoaderTauTrack.h"

namespace FlavorTagInference {
    using FeatureFunc_t = std::function<float(const xAOD::TauTrack&, const xAOD::TauJet&)>;
    using FeatureFuncAsReference_t = std::function<bool(const xAOD::TauJet&, const xAOD::TauTrack&, float&)>;
    ConstituentLoaderTauTrack::ConstituentLoaderTauTrack(const ConstituentsInputConfig& cfg) :
        IConstituentsLoader(cfg)
    {
        for (const InputVariableConfig& input_var : cfg.inputs) {
            m_feature_extractors.push_back(getFeatureExtractor(input_var.name));
        }
    }

    std::vector<const xAOD::TauTrack*> ConstituentLoaderTauTrack::getTauTracks( const xAOD::TauJet* tau ) const
    {
        std::vector<const xAOD::TauTrack*> out = tau->allTracks();

        // Skip unclassified tracks:
        // - the track is a LRT and classifyLRT = false
        // - the track is not among the MaxNtracks highest-pt tracks in the track classifier
        // - track classification is not run (trigger)
        if(m_config.selection == ConstituentsSelection::TAUTRACK_CLASSIFIED){
            std::vector<const xAOD::TauTrack*> classified_tracks;
            std::copy_if(out.begin(), out.end(), std::back_inserter(classified_tracks),
                [](const xAOD::TauTrack* track) {
                    return !track->flag(xAOD::TauJetParameters::unclassified);
                }
            );
            out = std::move(classified_tracks);
        }

        // Sort by descending pt
        if (m_config.order == ConstituentsSortOrder::PT_DESCENDING) {
            std::sort(out.begin(), out.end(),
                [](const xAOD::TauTrack* lhs, const xAOD::TauTrack* rhs) {
                    return lhs->pt() > rhs->pt();
                }
            );
        } else {
            // throw
            throw std::runtime_error("Unsupported sorting order");
        }
        // Truncate tracks
        if (static_cast<size_t>(out.size()) > m_config.max_n_constituents) {
            out.resize(m_config.max_n_constituents, out[0]);
        }
        return out;
    }

    Inputs ConstituentLoaderTauTrack::getFeatures(const xAOD::TauJet* tau, const std::vector<const xAOD::TauTrack*>& tau_trks) const {
        std::vector<int64_t> features_dim = {static_cast<int64_t>(tau_trks.size()), static_cast<int64_t>(m_feature_extractors.size())};
        std::vector<float> features;
        features.reserve(tau_trks.size() * m_feature_extractors.size());
        for (const auto* trk : tau_trks) {
            for (const auto& extractor : m_feature_extractors) {
                features.push_back(extractor(*trk, *tau));
            }
        }
        return Inputs{std::move(features), std::move(features_dim)};
    }

    std::tuple<Inputs, std::vector<const xAOD::IParticle*>> ConstituentLoaderTauTrack::getData(const xAOD::IParticle& i_tau) const {
        auto tau = dynamic_cast<const xAOD::TauJet*>(&i_tau);
        std::vector<const xAOD::TauTrack*> sorted_tau_trks = getTauTracks(tau);
        std::vector<const xAOD::IParticle*> i_sorted_tau_trks(sorted_tau_trks.begin(), sorted_tau_trks.end());

        return std::make_tuple(getFeatures(tau, sorted_tau_trks), i_sorted_tau_trks);
    }

    FeatureFunc_t ConstituentLoaderTauTrack::getFeatureExtractor(const std::string& var_name) const {
        FeatureFuncAsReference_t func_as_ref = nullptr;
        try {
            func_as_ref = m_func_map.at(var_name);
        } catch (const std::out_of_range &e) {
            throw std::runtime_error("Variable '" + var_name + "' not defined");
        }
        return [func_as_ref](const xAOD::TauTrack& trk, const xAOD::TauJet& tau) {
            float out;
            bool success = func_as_ref(tau, trk, out);
            if (!success) {
                throw std::runtime_error("Error in track variable calculation");
            }
            return out;
        };
    }

    const std::string& ConstituentLoaderTauTrack::getName() const {
        return m_name;
    }
    const ConstituentsType& ConstituentLoaderTauTrack::getType() const {
        return m_config.type;
    }
    const FTagDataDependencyNames& ConstituentLoaderTauTrack::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& ConstituentLoaderTauTrack::getUsedRemap() const {
        return m_used_remap;
    }

}

namespace TauTrackVars {

bool pt_log(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = std::log10(track.pt());
    return true;
}

bool trackPt(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.pt();
    return true;
}

bool trackEta(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.eta();
    return true;
}

bool trackPhi(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.phi();
    return true;
}

bool pt_tau_log(const xAOD::TauJet &tau, const xAOD::TauTrack& /*track*/, float &out) {
    out = std::log10(std::max(tau.pt(), 1e-6));
    return true;
}

bool pt_jetseed_log(const xAOD::TauJet &tau, const xAOD::TauTrack& /*track*/, float &out) {
    out = std::log10(tau.ptJetSeed());
    return true;
}

bool d0_abs_log(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = std::log10(std::abs(track.d0TJVA()) + 1e-6);
    return true;
}

bool z0sinThetaTJVA_abs_log(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = std::log10(std::abs(track.z0sinthetaTJVA()) + 1e-6);
    return true;
}

bool z0sinthetaTJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.z0sinthetaTJVA();
    return true;
}

bool z0sinthetaSigTJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.z0sinthetaSigTJVA();
    return true;
}

bool d0TJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.d0TJVA();
    return true;
}

bool d0SigTJVA(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.d0SigTJVA();
    return true;
}

bool dEta(const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out) {
    out = track.eta() - tau.eta();
    return true;
}

bool dEtaJetSeedAxis(const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out) {
    TLorentzVector tlvSeedJet = tau.p4(xAOD::TauJetParameters::JetSeed);
    out = std::abs(tlvSeedJet.Eta() - track.eta());
    return true;
}

bool dPhi(const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out) {
    out = track.p4().DeltaPhi(tau.p4());
    return true;
}

bool dPhiJetSeedAxis(const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out) {
    TLorentzVector tlvSeedJet = tau.p4(xAOD::TauJetParameters::JetSeed);
    out = tlvSeedJet.DeltaPhi(track.p4());
    return true;
}

bool nInnermostPixelHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t inner_pixel_hits;
    const auto success = track.track()->summaryValue(inner_pixel_hits, xAOD::numberOfInnermostPixelLayerHits);
    out = inner_pixel_hits;
    return success;
}

bool nPixelHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t pixel_hits;
    const auto success = track.track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
    out = pixel_hits;
    return success;
}

bool nSCTHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t sct_hits;
    const auto success = track.track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
    out = sct_hits;
    return success;
}

// same as in tau track classification for trigger
bool nIBLHitsAndExp(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t inner_pixel_hits, inner_pixel_exp;
    const auto success1 = track.track()->summaryValue(inner_pixel_hits, xAOD::numberOfInnermostPixelLayerHits);
    const auto success2 = track.track()->summaryValue(inner_pixel_exp, xAOD::expectInnermostPixelLayerHit);
    out =  inner_pixel_exp ? inner_pixel_hits : 1.;
    return success1 && success2;
}

bool nPixelHitsPlusDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t pixel_hits, pixel_dead;
    const auto success1 = track.track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
    const auto success2 = track.track()->summaryValue(pixel_dead, xAOD::numberOfPixelDeadSensors);
    out = pixel_hits + pixel_dead;
    return success1 && success2;
}

bool nSCTHitsPlusDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t sct_hits, sct_dead;
    const auto success1 = track.track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
    const auto success2 = track.track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
    out = sct_hits + sct_dead;
    return success1 && success2;
}

bool eProbabilityHT(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    float eProbabilityHT;
    const auto success = track.track()->summaryValue(eProbabilityHT, xAOD::eProbabilityHT);
    out = eProbabilityHT;
    return success;
}

bool eProbabilityHT_noTRT(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &/*track*/, float &out) {
    // Dummy eProbHT = 1.
    out = 1.;
    return true;
}

bool eProbabilityNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {  
    static const SG::ConstAccessor<float> acc_eProbabilityNN("eProbabilityNN");
    out = acc_eProbabilityNN(track);
    return true;
}

bool eProbabilityNNorHT(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {  
  auto atrack = track.track();
  float eProbabilityHT = atrack->summaryValue(eProbabilityHT, xAOD::eProbabilityHT);
  static const SG::ConstAccessor<float> acc_eProbabilityNN("eProbabilityNN");
  float eProbabilityNN = acc_eProbabilityNN(*atrack);
  out = (atrack->pt()>2000.) ? eProbabilityNN : eProbabilityHT;
  return true;
}

bool chargedScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
  static const SG::ConstAccessor<float> acc_chargedScoreRNN("rnn_chargedScore");
  out = acc_chargedScoreRNN(track);
  return true;
}

bool isolationScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
  static const SG::ConstAccessor<float> acc_isolationScoreRNN("rnn_isolationScore");
  out = acc_isolationScoreRNN(track);
  return true;
}

bool conversionScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
  static const SG::ConstAccessor<float> acc_conversionScoreRNN("rnn_conversionScore");
  out = acc_conversionScoreRNN(track);
  return true;
}

bool fakeScoreRNN(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
  static const SG::ConstAccessor<float> acc_fakeScoreRNN("rnn_fakeScore");
  out = acc_fakeScoreRNN(track);
  return true;
}

//Extension - variables for GNTau
bool numberOfInnermostPixelLayerHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfInnermostPixelLayerHits);
    out = trk_val;
    return success;
}

bool numberOfPixelHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelHits);
    out = trk_val;
    return success;
}

bool numberOfPixelSharedHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelSharedHits);
    out = trk_val;
    return success;
}

bool numberOfPixelDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelDeadSensors);
    out = trk_val;
    return success;
}

bool numberOfSCTHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfSCTHits);
    out = trk_val;
    return success;
}

bool numberOfSCTSharedHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfSCTSharedHits);
    out = trk_val;
    return success;
}

bool numberOfSCTDeadSensors(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfSCTDeadSensors);
    out = trk_val;
    return success;
}

bool numberOfTRTHighThresholdHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfTRTHighThresholdHits);
    out = trk_val;
    return success;
}

bool numberOfTRTHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfTRTHits);
    out = trk_val;
    return success;
}

bool nSiHits(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t pix_hit = 0;uint8_t pix_dead = 0;uint8_t sct_hit = 0;uint8_t sct_dead = 0;
    const auto success1 = track.track()->summaryValue(pix_hit, xAOD::numberOfPixelHits);
    const auto success2 = track.track()->summaryValue(pix_dead, xAOD::numberOfPixelDeadSensors);
    const auto success3 = track.track()->summaryValue(sct_hit, xAOD::numberOfSCTHits);
    const auto success4 = track.track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
    out = pix_hit + pix_dead + sct_hit + sct_dead;
    return success1 && success2 && success3 && success4;
}

bool expectInnermostPixelLayerHit(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::expectInnermostPixelLayerHit);
    out = trk_val;
    return success;
}

bool expectNextToInnermostPixelLayerHit(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::expectNextToInnermostPixelLayerHit);
    out = trk_val;
    return success;
}

bool numberOfContribPixelLayers(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfContribPixelLayers);
    out = trk_val;
    return success;
}

bool numberOfPixelHoles(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    uint8_t trk_val = 0;
    const auto success = track.track()->summaryValue(trk_val, xAOD::numberOfPixelHoles);
    out = trk_val;
    return success;
}

bool d0_old(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.track()->d0();
    //out = trk_val;
    return true;
}

bool qOverP(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.track()->qOverP();
    return true;
}

bool theta(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.track()->theta();
    return true;
}

bool z0TJVA(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out) {
    out = track.track()->z0() + track.track()->vz() - tau.vertex()->z();
    return true;
}

bool charge(const xAOD::TauJet& /*tau*/, const xAOD::TauTrack &track, float &out) {
    out = track.track()->charge();
    return true;
}

} // namespace TrackVars
