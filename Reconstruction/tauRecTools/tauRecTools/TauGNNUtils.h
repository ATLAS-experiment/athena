/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNNUTILS_H
#define TAURECTOOLS_TAUGNNUTILS_H

#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "AsgTools/AsgTool.h"
#include "AsgMessaging/AsgMessaging.h"
#include <unordered_map>


namespace TauGNNUtils {

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
 *
 */
class GNNVarCalc : public asg::AsgMessaging {
public:
    // Pointers to calculator functions
    using ScalarCalc = bool (*)(const xAOD::TauJet &, double &);

    using TrackCalc = bool (*)(const xAOD::TauJet &, const xAOD::TauTrack &,
                               double &);

    using ClusterCalc = bool (*)(const xAOD::TauJet &,
                                 const xAOD::CaloVertexedTopoCluster &, double &);

    using HitCalc = bool (*)(const xAOD::TauJet &,
                                 const xAOD::TrackMeasurementValidation &, double &);

public:
    GNNVarCalc();
    ~GNNVarCalc() = default;

    // Methods to compute the output (vector) based on the variable name

    // Computes high-level ID variables
    bool compute(const std::string &name, const xAOD::TauJet &tau, double &out) const;

    // Computes track variables
    bool compute(const std::string &name, const xAOD::TauJet &tau,
                 const std::vector<const xAOD::TauTrack *> &tracks,
                 std::vector<double> &out) const;

    // Computes cluster variables
    bool compute(const std::string &name, const xAOD::TauJet &tau,
                 const std::vector<xAOD::CaloVertexedTopoCluster> &clusters,
                 std::vector<double> &out) const;

    // Computes hit variables
    bool compute(const std::string &name, const xAOD::TauJet &tau,
                 const std::vector<const xAOD::TrackMeasurementValidation*> &hits,
                 std::vector<double> &out) const;

    // Methods to insert calculator functions into the lookup table
    void insert(const std::string &name, ScalarCalc func, const std::vector<std::string>& scalar_vars);
    void insert(const std::string &name, TrackCalc func, const std::vector<std::string>& track_vars);
    void insert(const std::string &name, ClusterCalc func, const std::vector<std::string>& cluster_vars);
    void insert(const std::string &name, HitCalc func, const std::vector<std::string>& hit_vars);

private:
    // Lookup tables
    std::unordered_map<std::string, ScalarCalc> m_scalar_map;
    std::unordered_map<std::string, TrackCalc> m_track_map;
    std::unordered_map<std::string, ClusterCalc> m_cluster_map;
    std::unordered_map<std::string, HitCalc> m_hit_map;
};

// Factory function to create a variable calculator populated with default
// variables
std::unique_ptr<GNNVarCalc> get_calculator(const std::vector<std::string>& scalar_vars,
					const std::vector<std::string>& track_vars,
					const std::vector<std::string>& cluster_vars,
					const std::vector<std::string>& hit_vars);


namespace Variables {

// Functions to calculate (scalar) input variables
// Returns a status code indicating success
bool eta(const xAOD::TauJet &tau, double &out);

bool absEta(const xAOD::TauJet &tau, double &out);

bool centFrac(const xAOD::TauJet &tau, double &out);

bool isolFrac(const xAOD::TauJet &tau, double &out); 

bool etOverPtLeadTrk(const xAOD::TauJet &tau, double &out);

bool innerTrkAvgDist(const xAOD::TauJet &tau, double &out);

bool absipSigLeadTrk(const xAOD::TauJet &tau, double &out);

bool sumEMCellEtOverLeadTrkPt(const xAOD::TauJet &tau, double &out);

bool SumPtTrkFrac(const xAOD::TauJet &tau, double &out);

bool EMPOverTrkSysP(const xAOD::TauJet &tau, double &out);

bool ptRatioEflowApprox(const xAOD::TauJet &tau, double &out);

bool mEflowApprox(const xAOD::TauJet &tau, double &out);

bool dRmax(const xAOD::TauJet &tau, double &out);

bool trFlightPathSig(const xAOD::TauJet &tau, double &out);

bool massTrkSys(const xAOD::TauJet &tau, double &out);

bool pt(const xAOD::TauJet &tau, double &out);

bool pt_tau_log(const xAOD::TauJet &tau, double &out);

bool ptDetectorAxis(const xAOD::TauJet &tau, double &out);

bool ptIntermediateAxis(const xAOD::TauJet &tau, double &out);

bool ptJetSeed(const xAOD::TauJet &tau, double &out);

bool etaJetSeed(const xAOD::TauJet &tau, double &out);

//functions to calculate input variables needed for the eVeto RNN
bool ptJetSeed_log             (const xAOD::TauJet &tau, double &out);
bool absleadTrackEta           (const xAOD::TauJet &tau, double &out);
bool leadTrackDeltaEta         (const xAOD::TauJet &tau, double &out);
bool leadTrackDeltaPhi         (const xAOD::TauJet &tau, double &out);
bool leadTrackProbNNorHT       (const xAOD::TauJet &tau, double &out);
bool EMFracFixed               (const xAOD::TauJet &tau, double &out);
bool etHotShotWinOverPtLeadTrk (const xAOD::TauJet &tau, double &out);
bool hadLeakFracFixed          (const xAOD::TauJet &tau, double &out);
bool PSFrac                    (const xAOD::TauJet &tau, double &out);
bool ClustersMeanCenterLambda  (const xAOD::TauJet &tau, double &out);
bool ClustersMeanEMProbability (const xAOD::TauJet &tau, double &out);
bool ClustersMeanFirstEngDens  (const xAOD::TauJet &tau, double &out);
bool ClustersMeanPresamplerFrac(const xAOD::TauJet &tau, double &out);
bool ClustersMeanSecondLambda  (const xAOD::TauJet &tau, double &out);
bool EMPOverTrkSysP            (const xAOD::TauJet &tau, double &out);


namespace Track {

// Functions to calculate input variables for each track
// Returns a status code indicating success

bool pt_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool trackPt(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

bool trackEta(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

bool trackPhi(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
    
bool pt_tau_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool pt_jetseed_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool d0_abs_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool z0sinThetaTJVA_abs_log(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool z0sinthetaTJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

bool z0sinthetaSigTJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

bool d0TJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

bool d0SigTJVA(
    const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

bool dEta(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool dEtaJetSeedAxis(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool dPhi(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool dPhiJetSeedAxis(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool nInnermostPixelHits(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool nPixelHits(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool nSCTHits(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

// trigger variants
bool nIBLHitsAndExp (
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool nPixelHitsPlusDeadSensors (
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool nSCTHitsPlusDeadSensors (
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool eProbabilityHT(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool eProbabilityNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool eProbabilityNNorHT(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool chargedScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool isolationScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool conversionScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

bool fakeScoreRNN(
    const xAOD::TauJet &tau, const xAOD::TauTrack &track, double &out);

//Extension - variables for GNTau
bool numberOfInnermostPixelLayerHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfPixelHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfPixelSharedHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfPixelDeadSensors(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfSCTHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfSCTSharedHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfSCTDeadSensors(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfTRTHighThresholdHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfTRTHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool nSiHits(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool expectInnermostPixelLayerHit(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool expectNextToInnermostPixelLayerHit(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfContribPixelLayers(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool numberOfPixelHoles(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool d0_old(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool qOverP(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool theta(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool z0TJVA(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool charge(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool dz0_TV_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool log_sumpt_TV(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool log_sumpt2_TV(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool log_sumpt_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);
bool log_sumpt2_PV0(const xAOD::TauJet& tau, const xAOD::TauTrack &track, double &out);

} // namespace Track


namespace Cluster {

// Functions to calculate input variables for each cluster
// Returns a status code indicating success

bool et_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool pt_tau_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool pt_jetseed_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool dEta(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool dPhi(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool SECOND_R(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool SECOND_LAMBDA(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool CENTER_LAMBDA(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool SECOND_LAMBDAOverClustersMeanSecondLambda(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool CENTER_LAMBDAOverClustersMeanCenterLambda(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool FirstEngDensOverClustersMeanFirstEngDens(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

//Extension - Variables for GNTau
bool e(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool et(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool FIRST_ENG_DENS(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool EM_PROBABILITY(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);

bool CENTER_MAG(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, double &out);
} // namespace Cluster

namespace Hit {

// Functions to calculate input variables for each hit
// Returns a status code indicating success

bool j(
    const xAOD::TauJet &tau, const xAOD::TrackMeasurementValidation &hit, double &out);

bool a(
    const xAOD::TauJet &tau, const xAOD::TrackMeasurementValidation &hit, double &out);

bool b(
    const xAOD::TauJet &tau, const xAOD::TrackMeasurementValidation &hit, double &out);

bool layer(
    const xAOD::TauJet &tau, const xAOD::TrackMeasurementValidation &hit, double &out);

} // namespace Hit
} // namespace Variables
} // namespace TauJetGNNUtils

#endif // TAURECTOOLS_TAUGNNUTILS_H
