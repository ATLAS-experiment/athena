/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOLOCALHADCALIB_GETLCSINGLEPIONSPERF_H
#define CALOLOCALHADCALIB_GETLCSINGLEPIONSPERF_H

/**
 * @class GetLCSinglePionsPerf
 * @version \$Id: GetLCSinglePionsPerf.h,v 1.1 2009-07-01 14:36:01 pospelov Exp $
 * @author Gennady Pospelov <guennadi.pospelov@cern.ch>
 * @date 1-July-2009
 * @brief Top algorithm to get local hadronic calibration performance plots for single pions
 *
 */


#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "Gaudi/Property.h"
#include <vector>
#include <string>
#include <set>
#include <cmath>


class AtlasDetectorID;
class CaloCell_ID;
class CaloDmDescrManager;

class TFile;
class TH1F;
class TProfile;
class TProfile2D;

class GetLCSinglePionsPerf : public AthAlgorithm
{

  public:
    GetLCSinglePionsPerf(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~GetLCSinglePionsPerf();
    virtual StatusCode initialize();
    virtual StatusCode execute(const EventContext& ctx);
    virtual StatusCode finalize();

    static double angle_mollier_factor(double x);

 private:
    typedef std::pair<std::string,xAOD::CaloCluster::MomentType> moment_name_pair;
    typedef std::vector<moment_name_pair> moment_name_vector;

    enum keys_topo {kTOPO_EM, kTOPO_W, kTOPO_OOC, kTOPO};
    enum keys_tagged {kTAGEM, kTAGHAD, kTAGUNK, kTAG};
    enum keys_momsums {kMOM_CLS0, kMOM_CLS1, kMOM_CLS2, kMOM_SUMCLS, kMOM_MISS, kMOM_MCENER};
    enum keys_norm_level {kLEVEL_PARTICLE, kLEVEL_CALIBHIT, kLEVEL_CALIBHIT_IDEAL };

    const AtlasDetectorID* m_id_helper{nullptr};
    const CaloCell_ID* m_calo_id{nullptr};
    const CaloDmDescrManager *m_caloDmDescrManager{nullptr};

    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterBasicCollName{this, "ClusterBasicCollName", "", "Collection name where clusters always have moments"};
    SG::ReadHandleKeyArray<xAOD::CaloClusterContainer> m_clusterCollNames{this, "ClusterCollectionNames", {}, "Cluster collection names after different correction steps"};
    Gaudi::Property<std::string> m_outputFileName{this, "OutputFileName", "CheckSinglePionsReco.root", "Name of output file to save histograms in"};
    TFile * m_outputFile{nullptr};
    SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_CalibrationHitContainerNames{this, "CalibrationHitContainerNames", {}, "Names of CalibrationHitContainers to use"};
    SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_DMCalibrationHitContainerNames{this, "DMCalibrationHitContainerNames", {}, "Names of dead material CalibrationHitContainers to use"};
    moment_name_vector m_validMoments;

    Gaudi::Property<double> m_distance_cut{this, "DistanceCut", 1.5, "Distance cut for clusters to calculate engReco"};

    Gaudi::Property<int>   m_netabin{this, "netabin", 25, "Number of eta bins"};
    Gaudi::Property<float> m_etamin{this, "etamin", 0.0, "Minimum eta"};
    Gaudi::Property<float> m_etamax{this, "etamax", 5.0, "Maximum eta"};
    float m_deta{0};
    Gaudi::Property<int>   m_nphibin{this, "nphibin", 1, "Number of phi bins"};
    Gaudi::Property<float> m_phimin{this, "phimin", -M_PI, "Minimum phi"};
    Gaudi::Property<float> m_phimax{this, "phimax", M_PI, "Maximum phi"};
    float m_dphi{0};
    Gaudi::Property<int>   m_nlogenerbin{this, "nlogenerbin", 22, "Number of log10(energy) bins"};
    Gaudi::Property<float> m_logenermin{this, "logenermin", 2.0, "Minimum log10(energy)"};
    Gaudi::Property<float> m_logenermax{this, "logenermax", 6.4, "Maximum log10(energy)"};
    int   m_nlogenerbin2{3};
    float m_dlogener{0};
    int   m_nnormtype{3};      // pion energy, calibration level, calibration level with ideal previous step
    int   m_ncluscoll{4};      // CaloTopoCluster, CaloWTopoCluster, CaloOOCTopoCluster, CaloCalTopoCluster
    int   m_ntagcases{3};      // TAGGEDEM, TAGGEDHAD, TAGGEDUNKNOWN
    int   m_nmoments{0};       // number of moments assigned to clusters we want to study
    int   m_nmomsums{6};       // ratio of moment assigned to cls0, cls1, cls2, cls_sum, non assigned
//     int   m_nhitsums;         // number of histograms for calibration hits validation
   
    float m_truthPiEngFraction{0.1};
    Gaudi::Property<bool> m_doEngRecOverTruth{this, "doEngRecOverTruth", true, "Create reco over truth energy histograms"};
    Gaudi::Property<bool> m_doEngTag{this, "doEngTag", true, "Create classification performance histograms"};
    Gaudi::Property<bool> m_doEngRecSpect{this, "doEngRecSpect", true, "Create reconstructed energy spectra histograms"};
    Gaudi::Property<bool> m_doEngNoiseClus{this, "doEngNoiseClus", true, "Create noise cluster histograms"};
    Gaudi::Property<bool> m_doClusMoments{this, "doClusMoments", true, "Create cluster moments histograms"};
    Gaudi::Property<bool> m_doRecoEfficiency{this, "doRecoEfficiency", true, "Create reconstruction efficiency histograms"};
    Gaudi::Property<bool> m_useRecoEfficiency{this, "useRecoEfficiency", false, "Use only events where the pion was reconstructed"};
    Gaudi::Property<bool> m_useGoodClus{this, "useGoodClus", false, "Use only good clusters"};
    Gaudi::Property<bool> m_doCalibHitsValidation{this, "doCalibHitsValidation", true, "Create calibration hits validation histograms"};
    Gaudi::Property<bool> m_usePionClusters{this, "usePionClustersOnly", false, "Only use clusters that contain at least m_truthPiEngFraction of the true pion energy"};

    // ereco over truth after each step of calibration
    std::vector<std::vector<std::vector<TProfile *> > > m_engRecOverTruth_vs_eta;   // [ncluscoll][nnormtype][nenerbin]
    std::vector<std::vector<std::vector<TProfile *> > > m_engRecOverTruth_vs_ebeam; // [ncluscoll][nnormtype][netabin]
    // classification performance
    std::vector<std::vector<TProfile *> > m_engTag_vs_eta;   // [ntagcases][nenerbin]
    std::vector<std::vector<TProfile *> > m_engTag_vs_ebeam; // [ntagcases][netabin]
    // reconstructed energy spectras after each step of calibration
    std::vector<std::vector<std::vector<TH1F *> > > m_engRecSpect; // [ncluscoll][nenerbin][netabin]
    // average incident pion energy in bins
    std::vector<std::vector<TH1F *> > m_engPionSpect;   // [nenerbin][netabin]
    // spectra of noise clusters
    std::vector<std::vector<TH1F *> > m_engNoiseClusSpect; // [ncluscoll][netabin]
    std::vector<TProfile *> m_engNoiseClus_vs_eta; // [ncluscoll]
    // cluster moments
    std::vector<std::vector<std::vector<TProfile *> > > m_clusMoment_vs_eta;   // [nmoments][nmomsums][nenerbin]
    std::vector<std::vector<std::vector<TProfile *> > > m_clusMoment_vs_ebeam; // [nmoments][nmomsums][netabin]
    // reconstruction efficiency
    std::vector<TH1F *> m_RecoEfficiency_vs_eta[2]; // [nenerbin]
    std::vector<TH1F *> m_RecoEfficiency_vs_ebeam[2]; // [netabin]
    // calibration hits
    std::vector<TProfile *> m_SumCalibHitOverEbeam_vs_eta;               // [nenerbin]
    //std::vector<TProfile *> m_SumCalibHitOverEbeam_vs_ebeam;             // [netabin]
    std::vector<TProfile *> m_DefaultCalculatorOverEbeam_vs_eta;         // [nenerbin]
    //std::vector<TProfile *> m_DefaultCalculatorOverEbeam_vs_ebeam;       // [netabin]
    std::vector<TProfile *> m_SumCalibHitAssignedOverEbeam_vs_eta; // [nenerbin]
    std::vector<TProfile2D *> m_SumCalibHitAssignedOverEbeam_vs_etaphi; // [nenerbin]

    double m_mc_ener{0};
    double m_mc_eta{0};
    double m_mc_phi{0};
    int m_mc_etabin{0};
    int m_mc_enerbin{0};
    int m_mc_phibin{0};

    int fill_reco (const xAOD::CaloClusterContainer& clusColl,
                   const EventContext& ctx);
    int fill_moments (const xAOD::CaloClusterContainer& clusColl,
                      const EventContext& ctx);
    int fill_calibhits (const xAOD::CaloClusterContainer& clusColl,
                        const EventContext& ctx);

};

#endif // LOCALHADCALIBCHECK_CHECKHITSASSIGNMENT_H

