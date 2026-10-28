/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_INDETPERFPLOT_VERTEXTRUTHMATCHING
#define INDETPHYSVALMONITORING_INDETPERFPLOT_VERTEXTRUTHMATCHING
/**
 * @file InDetPerfPlot_VertexTruthMatching.h
 * @author Valentina Cairo, mbasso
 **/

// local includes
#include "InDetPlotBase.h"
#include "InDetPhysValMonitoringUtilities.h"

// Tracking includes:
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexFwd.h"

// xAOD truth object includes:
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthPileupEventContainer.h"
#include "xAODTruth/TruthVertex.h"

// Vertex validation:
#include "InDetTruthVertexValidation/InDetVertexTruthMatchTool.h"
#include "InDetTruthVertexValidation/InDetVertexTruthMatchUtils.h"

#include "InDetPhysValMonitoring/ResolutionHelper.h"

// std includes
#include <string>

// root includes
class TH1;
class TH2;
class TProfile;
class TEfficiency;


///class holding plots for truth matched vertices
class InDetPerfPlot_VertexTruthMatching: public InDetPlotBase {
public:
    InDetPerfPlot_VertexTruthMatching(InDetPlotBase* pParent, const std::string& dirName, const int detailLevel = 10, bool isITk = false);
    void fill(const xAOD::Vertex& vertex, const xAOD::TruthVertex * tvrt =0, float weight=1.0);
    void fill(const xAOD::Vertex* recoHardScatter, const xAOD::VertexContainer& vertexContainer, const std::vector<const xAOD::TruthVertex*>& truthHSVertices, const std::vector<const xAOD::TruthVertex*>& truthPUVertices, float actualMu, float weight=1.0);
private:
    bool m_isITk{false};
    int m_detailLevel{10};
    float m_cutMinTruthRecoRadialDiff = 0.1;
    ///truth type
    TH1* m_vx_type_truth = nullptr;
    TH1* m_vx_x_diff = nullptr;
    TH1* m_vx_x_diff_pull = nullptr;
    TH1* m_vx_y_diff = nullptr;
    TH1* m_vx_y_diff_pull = nullptr;
    TH1* m_vx_z_diff = nullptr;
    TH1* m_vx_z_diff_pull = nullptr;

    ///vertex time
    TH1* m_vx_time_diff = nullptr;
    TH1* m_vx_time_diff_pull = nullptr;

    ///hardscatter classification
    TH1* m_vx_hs_classification = nullptr;
    ///vertex reco efficiency
    TProfile* m_vx_nReco_vs_nTruth_inclusive = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_matched = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_merged = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_split = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_fake = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_dummy = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_clean = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_lowpu = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_highpu = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_hssplit = nullptr;
    TProfile* m_vx_nReco_vs_nTruth_none = nullptr;
    // HS vertex reconstruction efficiency vs PU
    TEfficiency* m_vx_hs_reco_eff = nullptr;
    // HS vertex selection efficiency vs PU
    TEfficiency* m_vx_hs_sel_eff = nullptr;
    // HS vertex selection efficiency vs nReco vertices
    TEfficiency* m_vx_hs_sel_eff_vs_nReco = nullptr;
    // HS vertex reconstruction and selection efficiency vs PU
    TEfficiency* m_vx_hs_reco_sel_eff = nullptr;
    // HS vertex selection efficiency vs PU (distance based)
    TEfficiency* m_vx_hs_sel_eff_dist = nullptr;
    // HS vertex selection efficiency vs PU (actualInteractionsPerCrossing)
    TEfficiency* m_vx_hs_sel_eff_mu = nullptr;
    // HS vertex selection efficiency vs nReco vertices (distance based)
    TEfficiency* m_vx_hs_sel_eff_dist_vs_nReco = nullptr;
    // HS vertex reconstruction efficiency vs n truth vertices
    TEfficiency* m_vx_hs_reco_eff_vs_ntruth = nullptr;
    // HS vertex selection efficiency vs PU (actualInteractionsPerCrossing)
    TEfficiency* m_vx_hs_reco_eff_mu = nullptr;
    // HS vertex selection efficiency vs n truth vertices
    TEfficiency* m_vx_hs_sel_eff_vs_ntruth = nullptr;
    // HS vertex reconstruction and selection efficiency vs n truth vertices
    TEfficiency* m_vx_hs_reco_sel_eff_vs_ntruth = nullptr;


    // For reco (covariance) resolutions:
    TProfile* m_vx_hs_reco_long_reso = nullptr;
    TProfile* m_vx_hs_reco_trans_reso = nullptr;

    // For reco-truth resolutions:
    IDPVM::ResolutionHelper m_resolutionHelper{};
    IDPVM::ResolutionHelper::methods m_resolutionMethod = IDPVM::ResolutionHelper::iterRMS_convergence;

    TH2* m_resHelper_PUdensity_hsVxTruthLong = nullptr;
    TH1* m_resolution_vs_PUdensity_hsVxTruthLong = nullptr;
    TH1* m_resmean_vs_PUdensity_hsVxTruthLong = nullptr;
    TH2* m_resHelper_PUdensity_hsVxTruthTransv = nullptr;
    TH1* m_resolution_vs_PUdensity_hsVxTruthTransv = nullptr;
    TH1* m_resmean_vs_PUdensity_hsVxTruthTransv = nullptr;
    TH2* m_resHelper_mu_hsVxTruthLong = nullptr;
    TH1* m_resolution_vs_mu_hsVxTruthLong = nullptr;
    TH1* m_resmean_vs_mu_hsVxTruthLong = nullptr;

    TH1* m_vx_hs_z_pull = nullptr;
    TH1* m_vx_hs_y_pull = nullptr;
    TH1* m_vx_hs_x_pull = nullptr;
    TH1* m_vx_all_z_pull = nullptr;
    TH1* m_vx_all_y_pull = nullptr;
    TH1* m_vx_all_x_pull = nullptr;
    
    TH1* m_vx_hs_z_res = nullptr;
    TH1* m_vx_hs_y_res = nullptr;
    TH1* m_vx_hs_x_res = nullptr;
    TH1* m_vx_all_z_res = nullptr;
    TH1* m_vx_all_y_res = nullptr;
    TH1* m_vx_all_x_res = nullptr;

    TH2* m_vx_all_truth_z_res_vs_PU = nullptr;
    TH2* m_vx_all_truth_x_res_vs_PU = nullptr;
    TH2* m_vx_all_truth_y_res_vs_PU = nullptr;
    TH2* m_vx_all_truth_z_pull_vs_PU = nullptr;
    TH2* m_vx_all_truth_x_pull_vs_PU = nullptr;
    TH2* m_vx_all_truth_y_pull_vs_PU = nullptr;

    TH2* m_vx_all_truth_z_res_vs_nTrk = nullptr;
    TH2* m_vx_all_truth_x_res_vs_nTrk = nullptr;
    TH2* m_vx_all_truth_y_res_vs_nTrk = nullptr;
    TH2* m_vx_all_truth_z_pull_vs_nTrk = nullptr;
    TH2* m_vx_all_truth_x_pull_vs_nTrk = nullptr;
    TH2* m_vx_all_truth_y_pull_vs_nTrk = nullptr;

    TH2* m_vx_hs_truth_z_res_vs_PU = nullptr;
    TH2* m_vx_hs_truth_x_res_vs_PU = nullptr;
    TH2* m_vx_hs_truth_y_res_vs_PU = nullptr;
    TH2* m_vx_hs_truth_z_pull_vs_PU = nullptr;
    TH2* m_vx_hs_truth_x_pull_vs_PU = nullptr;
    TH2* m_vx_hs_truth_y_pull_vs_PU = nullptr;

    TH2* m_vx_hs_truth_z_res_vs_nTrk = nullptr;
    TH2* m_vx_hs_truth_x_res_vs_nTrk = nullptr;
    TH2* m_vx_hs_truth_y_res_vs_nTrk = nullptr;
    TH2* m_vx_hs_truth_z_pull_vs_nTrk = nullptr;
    TH2* m_vx_hs_truth_x_pull_vs_nTrk = nullptr;
    TH2* m_vx_hs_truth_y_pull_vs_nTrk = nullptr;

    // New Histograms for Expert Plots for observables for different vertex classifications
    using TH1_map = std::map<int, std::map<InDetVertexTruthMatchUtils::VertexMatchType, TH1*>>;
    TH1_map m_vx_ntracks{};
    TH1_map m_vx_sumpT{};
    TH1_map m_vx_z_asym{};
    TH1_map m_vx_z_asym_weighted{};
    TH1_map m_vx_track_weight{};
    TH1_map m_vx_normalised_track_weight{};
    TH1_map m_vx_chi2Over_ndf{};
    TH1_map m_vx_z0_skewness{};
    TH1_map m_vx_z0_kurtosis{};
    TH1_map m_vx_nVertices{};

    // delta z plot
    TH1* m_vx_all_dz = nullptr;
    TH1* m_vx_hs_mindz = nullptr;
    
    TH1* m_vx_PUdensity = nullptr;
    TH1* m_vx_nTruth = nullptr;
    TH2* m_vx_nTruth_vs_PUdensity = nullptr;

    ///@}
private:
    // plot base has no default implementation of this; we use it to book the histos
    void initializePlots();
    static const xAOD::Vertex* getHSRecoVertexSumPt2(const xAOD::VertexContainer& recoVertices) ;
    template<typename U, typename V>
    float getRadialDiff2(const U* vtx1, const V* vtx2) const;
    float getLocalPUDensity(const xAOD::TruthVertex* vtxOfInterest, const std::vector<const xAOD::TruthVertex*>& truthHSVertices, const std::vector<const xAOD::TruthVertex*>& truthPUVertices, const float radialWindow = 2.0) const;
    static float getRecoLongitudinalReso(const xAOD::Vertex* recoVtx) ;
    static float getRecoTransverseReso(const xAOD::Vertex* recoVtx) ;
    const xAOD::TruthVertex* getTruthVertex(const xAOD::Vertex* recoVtx) const;
    void finalizePlots();
};

#endif
