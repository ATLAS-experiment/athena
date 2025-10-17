/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the TauTracks from the tau 
  and extract their features for the NN evaluation.
*/

#pragma once

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include <xAODTau/TauJet.h>
#include <xAODTau/TauTrack.h>

// STL includes
#include <string>
#include <vector>
#include <functional>


namespace TauTrackVars {

// Functions to calculate input variables for each track
// Returns a status code indicating success

bool pt_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool trackPt(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

bool trackEta(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

bool trackPhi(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
    
bool pt_tau_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool pt_jetseed_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool d0_abs_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool z0sinThetaTJVA_abs_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool z0sinthetaTJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

bool z0sinthetaSigTJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

bool d0TJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

bool d0SigTJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

bool dEta(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool dEtaJetSeedAxis(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool dPhi(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool dPhiJetSeedAxis(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool nInnermostPixelHits(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool nPixelHits(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool nSCTHits(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

// trigger variants
bool nIBLHitsAndExp (
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool nPixelHitsPlusDeadSensors (
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool nSCTHitsPlusDeadSensors (
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool eProbabilityHT(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool eProbabilityHT_noTRT(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool eProbabilityNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool eProbabilityNNorHT(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool chargedScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool isolationScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool conversionScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

bool fakeScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, float &out);

//Extension - variables for GNTau
bool numberOfInnermostPixelLayerHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfPixelHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfPixelSharedHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfPixelDeadSensors(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfSCTHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfSCTSharedHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfSCTDeadSensors(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfTRTHighThresholdHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfTRTHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool nSiHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool expectInnermostPixelLayerHit(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool expectNextToInnermostPixelLayerHit(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfContribPixelLayers(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool numberOfPixelHoles(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool d0_old(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool qOverP(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool theta(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool z0TJVA(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool charge(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool dz0_TV_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool log_sumpt_TV(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool log_sumpt2_TV(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool log_sumpt_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);
bool log_sumpt2_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &track, float &out);

} // namespace TrackVars


namespace FlavorTagInference {
    // Subclass for IParticles loader inherited from abstract IConstituentsLoader class
    class ConstituentLoaderTauTrack : public IConstituentsLoader {
      public:
        ConstituentLoaderTauTrack(const ConstituentsInputConfig& cfg);

        std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(const xAOD::IParticle& p) const override ;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
      private:
        using FeatureFunc_t = std::function<float(const xAOD::TauTrack&, const xAOD::TauJet&)>;
        using FeatureFuncAsReference_t = std::function<bool(const xAOD::TauJet&, const xAOD::TauTrack&, float&)>;
        std::vector<FeatureFunc_t> m_feature_extractors;
        FeatureFunc_t getFeatureExtractor(const std::string& var_name) const;
        std::vector<const xAOD::TauTrack*> getTauTracks(const xAOD::TauJet* tau) const;
        Inputs getFeatures(const xAOD::TauJet* tau, const std::vector<const xAOD::TauTrack*>& tau_trks) const;
        inline static const std::unordered_map<std::string, FeatureFuncAsReference_t> m_func_map = {
            {"pt_log",                          TauTrackVars::pt_log},
            {"trackPt",                         TauTrackVars::trackPt},
            {"trackEta",                        TauTrackVars::trackEta},
            {"trackPhi",                        TauTrackVars::trackPhi},
            {"pt_tau_log",                      TauTrackVars::pt_tau_log},
            {"pt_jetseed_log",                  TauTrackVars::pt_jetseed_log},
            {"d0_abs_log",                      TauTrackVars::d0_abs_log},
            {"z0sinThetaTJVA_abs_log",          TauTrackVars::z0sinThetaTJVA_abs_log},
            {"z0sinthetaTJVA",                  TauTrackVars::z0sinthetaTJVA},
            {"z0sinthetaSigTJVA",               TauTrackVars::z0sinthetaSigTJVA},
            {"d0TJVA",                          TauTrackVars::d0TJVA},
            {"d0SigTJVA",                       TauTrackVars::d0SigTJVA},
            {"dEta",                            TauTrackVars::dEta},
            {"dEtaJetSeedAxis",                 TauTrackVars::dEtaJetSeedAxis},
            {"dPhi",                            TauTrackVars::dPhi},
            {"dPhiJetSeedAxis",                 TauTrackVars::dPhiJetSeedAxis},
            {"nInnermostPixelHits",             TauTrackVars::nInnermostPixelHits},
            {"numberOfInnermostPixelLayerHits", TauTrackVars::numberOfInnermostPixelLayerHits},
            {"nPixelHits",                      TauTrackVars::nPixelHits},
            {"nSCTHits",                        TauTrackVars::nSCTHits},
            {"nIBLHitsAndExp",                  TauTrackVars::nIBLHitsAndExp},
            {"nPixelHitsPlusDeadSensors",       TauTrackVars::nPixelHitsPlusDeadSensors},
            {"nSCTHitsPlusDeadSensors",         TauTrackVars::nSCTHitsPlusDeadSensors},
            {"eProbabilityHT",                  TauTrackVars::eProbabilityHT},
            {"eProbabilityHT_noTRT",            TauTrackVars::eProbabilityHT_noTRT}
        };
    };
}

