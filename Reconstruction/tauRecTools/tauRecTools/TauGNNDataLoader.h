/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "xAODTau/TauJet.h"

#include "AsgMessaging/AsgMessaging.h"

#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/SaltModelEDMLoaderBase.h"
#include "FlavorTagInference/ConstituentsLoader.h"
#include "tauRecTools/ConstituentsLoaderTauCluster.h"
#include "tauRecTools/ConstituentsLoaderTauTrack.h"
#include "tauRecTools/ConstituentsLoaderTauHit.h"

// Functions to calculate (scalar) input variables
// Returns a status code indicating success
namespace TauScalarVars{ 
    bool eta(const xAOD::TauJet &tau, float &out);
    bool absEta(const xAOD::TauJet &tau, float &out);
    bool centFrac(const xAOD::TauJet &tau, float &out);
    bool isolFrac(const xAOD::TauJet &tau, float &out); 
    bool etOverPtLeadTrk(const xAOD::TauJet &tau, float &out);
    bool innerTrkAvgDist(const xAOD::TauJet &tau, float &out);
    bool absipSigLeadTrk(const xAOD::TauJet &tau, float &out);
    bool sumEMCellEtOverLeadTrkPt(const xAOD::TauJet &tau, float &out);
    bool SumPtTrkFrac(const xAOD::TauJet &tau, float &out);
    bool EMPOverTrkSysP(const xAOD::TauJet &tau, float &out);
    bool ptRatioEflowApprox(const xAOD::TauJet &tau, float &out);
    bool mEflowApprox(const xAOD::TauJet &tau, float &out);
    bool dRmax(const xAOD::TauJet &tau, float &out);
    bool trFlightPathSig(const xAOD::TauJet &tau, float &out);
    bool massTrkSys(const xAOD::TauJet &tau, float &out);
    bool pt(const xAOD::TauJet &tau, float &out);
    bool pt_tau_log(const xAOD::TauJet &tau, float &out);
    bool ptDetectorAxis(const xAOD::TauJet &tau, float &out);
    bool ptIntermediateAxis(const xAOD::TauJet &tau, float &out);
    bool ptJetSeed(const xAOD::TauJet &tau, float &out);
    bool etaJetSeed(const xAOD::TauJet &tau, float &out);
    
    //functions to calculate input variables needed for the eVeto RNN
    bool ptJetSeed_log             (const xAOD::TauJet &tau, float &out);
    bool absleadTrackEta           (const xAOD::TauJet &tau, float &out);
    bool leadTrackDeltaEta         (const xAOD::TauJet &tau, float &out);
    bool leadTrackDeltaPhi         (const xAOD::TauJet &tau, float &out);
    bool leadTrackProbNNorHT       (const xAOD::TauJet &tau, float &out);
    bool EMFracFixed               (const xAOD::TauJet &tau, float &out);
    bool etHotShotWinOverPtLeadTrk (const xAOD::TauJet &tau, float &out);
    bool hadLeakFracFixed          (const xAOD::TauJet &tau, float &out);
    bool PSFrac                    (const xAOD::TauJet &tau, float &out);
    bool ClustersMeanCenterLambda  (const xAOD::TauJet &tau, float &out);
    bool ClustersMeanEMProbability (const xAOD::TauJet &tau, float &out);
    bool ClustersMeanFirstEngDens  (const xAOD::TauJet &tau, float &out);
    bool ClustersMeanPresamplerFrac(const xAOD::TauJet &tau, float &out);
    bool ClustersMeanSecondLambda  (const xAOD::TauJet &tau, float &out);
    bool EMPOverTrkSysP            (const xAOD::TauJet &tau, float &out);
}//namespace TauScalarVars

class TauGNNDataLoader : public FlavorTagInference::SaltModelEDMLoaderBase, public asg::AsgMessaging {
    public:
        struct Config {
            std::string nnFile; 
            std::string input_layer_scalar;
            std::string input_layer_tracks;
            std::string input_layer_clusters;
            std::string input_layer_hits;
            std::string output_node_tau;
            std::string output_node_jet;
            size_t n_max_tracks;
            size_t n_max_clusters; 
            float max_dr_cluster;
            size_t n_max_hits; 
            bool doVertexCorrection; 
            bool trackClassification; 
            bool useTRT;
            std::string hits_decor_name;
        };
        TauGNNDataLoader(
            std::shared_ptr<const FlavorTagInference::SaltModel> salt_model, 
            const Config& config
        );
        ~TauGNNDataLoader() = default;
    private:
        using ScalarCalcByRef_t  = std::function<bool(const xAOD::TauJet &, float &)>;
        using ScalarCalc_t       = std::function<float(const xAOD::IParticle*)>;
        ScalarCalc_t getScalarCalc(const std::string &name) const;
        inline static const std::unordered_map<std::string, ScalarCalcByRef_t>  m_func_map = {
            {"isolFrac",                  TauScalarVars::isolFrac},
            {"centFrac",                  TauScalarVars::centFrac},
            {"etOverPtLeadTrk",           TauScalarVars::etOverPtLeadTrk},
            {"innerTrkAvgDist",           TauScalarVars::innerTrkAvgDist},
            {"absipSigLeadTrk",           TauScalarVars::absipSigLeadTrk},
            {"SumPtTrkFrac",              TauScalarVars::SumPtTrkFrac},
            {"sumEMCellEtOverLeadTrkPt",  TauScalarVars::sumEMCellEtOverLeadTrkPt},
            {"EMPOverTrkSysP",            TauScalarVars::EMPOverTrkSysP},
            {"ptRatioEflowApprox",        TauScalarVars::ptRatioEflowApprox},
            {"mEflowApprox",              TauScalarVars::mEflowApprox},
            {"dRmax",                     TauScalarVars::dRmax},
            {"trFlightPathSig",           TauScalarVars::trFlightPathSig},
            {"massTrkSys",                TauScalarVars::massTrkSys},
            {"pt",                        TauScalarVars::pt},
            {"eta",                       TauScalarVars::eta},
            {"ptJetSeed",                 TauScalarVars::ptJetSeed},
            {"etaJetSSeed",               TauScalarVars::etaJetSeed}
        };
};
