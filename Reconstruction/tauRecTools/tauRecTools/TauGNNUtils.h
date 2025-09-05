/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNNUTILS_H
#define TAURECTOOLS_TAUGNNUTILS_H

#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "AsgTools/AsgTool.h"
#include "AsgMessaging/AsgMessaging.h"
#include <unordered_map>




namespace TauGNNUtils {

namespace Variables {

// Functions to calculate (scalar) input variables
// Returns a status code indicating success
namespace Scalar{ 
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
}//namespace Scalar

namespace Track {

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

} // namespace Track


namespace Cluster {

// Functions to calculate input variables for each cluster
// Returns a status code indicating success

bool et_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool pt_tau_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool pt_jetseed_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool dEta(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool dPhi(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool SECOND_R(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool SECOND_LAMBDA(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool CENTER_LAMBDA(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool SECOND_LAMBDAOverClustersMeanSecondLambda(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool CENTER_LAMBDAOverClustersMeanCenterLambda(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool FirstEngDensOverClustersMeanFirstEngDens(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

//Extension - Variables for GNTau
bool e(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool et(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool FIRST_ENG_DENS(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool EM_PROBABILITY(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool CENTER_MAG(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);
} // namespace Cluster

} // namespace Variables

/**
 * @brief Tool to calculate input variables for the GNN-based tau identification
 *
 *   Used to calculate input variables for (onnx)GNN-based tau identification on
 *   the fly by providing a mapping between variable names (strings) and
 *   functions to calculate these variables.
 *
 * @author C. Deutsch
 * @author W. Davey
 * @author N.M. Tamir
 * @author D. Qichen
 *
 */
class GNNVarCalc : public asg::AsgMessaging {
public:
    // Pointers to calculator functions
    using ScalarCalc  = std::function<bool(const xAOD::TauJet &, float &)>;
    using TrackCalc   = std::function<bool(const xAOD::TauJet &, const xAOD::TauTrack &, float &)>;
    using ClusterCalc = std::function<bool(const xAOD::TauJet &, const xAOD::CaloVertexedTopoCluster &, float &)>;  

public:
    GNNVarCalc();
    ~GNNVarCalc() = default;

    // Methods to compute the output (vector) based on the variable name

    // Computes high-level ID variables
    float compute(const std::string &name, const xAOD::TauJet &tau) const;

    // Computes track variables
    std::vector<float> compute(const std::string &name, const xAOD::TauJet &tau,
                 const std::vector<const xAOD::TauTrack *> &tracks) const;

    // Computes cluster variables
    std::vector<float> compute(const std::string &name, const xAOD::TauJet &tau,
                 const std::vector<xAOD::CaloVertexedTopoCluster> &clusters) const;

private:
    // Lookup tables
    inline static const std::unordered_map<std::string, ScalarCalc>  m_scalar_map = {
        {"isolFrac",                  Variables::Scalar::isolFrac},
        {"centFrac",                  Variables::Scalar::centFrac},
        {"etOverPtLeadTrk",           Variables::Scalar::etOverPtLeadTrk},
        {"innerTrkAvgDist",           Variables::Scalar::innerTrkAvgDist},
        {"absipSigLeadTrk",           Variables::Scalar::absipSigLeadTrk},
        {"SumPtTrkFrac",              Variables::Scalar::SumPtTrkFrac},
        {"sumEMCellEtOverLeadTrkPt",  Variables::Scalar::sumEMCellEtOverLeadTrkPt},
        {"EMPOverTrkSysP",            Variables::Scalar::EMPOverTrkSysP},
        {"ptRatioEflowApprox",        Variables::Scalar::ptRatioEflowApprox},
        {"mEflowApprox",              Variables::Scalar::mEflowApprox},
        {"dRmax",                     Variables::Scalar::dRmax},
        {"trFlightPathSig",           Variables::Scalar::trFlightPathSig},
        {"massTrkSys",                Variables::Scalar::massTrkSys},
        {"pt",                        Variables::Scalar::pt}
    };

    inline static const std::unordered_map<std::string, TrackCalc>   m_track_map = {
        {"pt_log",                    Variables::Track::pt_log},
        {"trackPt",                   Variables::Track::trackPt},
        {"trackEta",                  Variables::Track::trackEta},
        {"trackPhi",                  Variables::Track::trackPhi},
        {"pt_tau_log",                Variables::Track::pt_tau_log},
        {"pt_jetseed_log",            Variables::Track::pt_jetseed_log},
        {"d0_abs_log",                Variables::Track::d0_abs_log},
        {"z0sinThetaTJVA_abs_log",    Variables::Track::z0sinThetaTJVA_abs_log},
        {"z0sinthetaTJVA",            Variables::Track::z0sinthetaTJVA},
        {"z0sinthetaSigTJVA",         Variables::Track::z0sinthetaSigTJVA},
        {"d0TJVA",                    Variables::Track::d0TJVA},
        {"d0SigTJVA",                 Variables::Track::d0SigTJVA},
        {"dEta",                      Variables::Track::dEta},
        {"dEtaJetSeedAxis",           Variables::Track::dEtaJetSeedAxis},
        {"dPhi",                      Variables::Track::dPhi},
        {"dPhiJetSeedAxis",           Variables::Track::dPhiJetSeedAxis},
        {"nInnermostPixelHits",       Variables::Track::nInnermostPixelHits},
        {"numberOfInnermostPixelLayerHits", Variables::Track::numberOfInnermostPixelLayerHits},
        {"nPixelHits",                Variables::Track::nPixelHits},
        {"nSCTHits",                  Variables::Track::nSCTHits},
        {"nIBLHitsAndExp",            Variables::Track::nIBLHitsAndExp},
        {"nPixelHitsPlusDeadSensors", Variables::Track::nPixelHitsPlusDeadSensors},
        {"nSCTHitsPlusDeadSensors",   Variables::Track::nSCTHitsPlusDeadSensors},
        {"eProbabilityHT",            Variables::Track::eProbabilityHT}
    };

    inline static const std::unordered_map<std::string, ClusterCalc> m_cluster_map = {
        {"dEta",                      Variables::Cluster::dEta},
        {"dPhi",                      Variables::Cluster::dPhi},
        {"SECOND_R",                  Variables::Cluster::SECOND_R},
        {"SECOND_LAMBDA",             Variables::Cluster::SECOND_LAMBDA},
        {"CENTER_LAMBDA",             Variables::Cluster::CENTER_LAMBDA},
        {"et",                        Variables::Cluster::et}
    };
};

} // namespace TauJetGNNUtils

#endif // TAURECTOOLS_TAUGNNUTILS_H
