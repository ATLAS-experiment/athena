/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVTXPLOTMAKER_H
#define MSVTXPLOTMAKER_H

#include <memory>
#include <string>

#include "GaudiKernel/SystemOfUnits.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"

#include "TROOT.h"
#include "TStyle.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "THStack.h"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
#include "TGraph.h"
#include "TMultiGraph.h"

#include "TMath.h"
#include "TString.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TObjArray.h"
#include "TObjString.h"


class MSVtxPlotMaker {
    public:
        MSVtxPlotMaker(const std::string &datapath, const std::string &pltdir, const std::string &treename="MSVtxValidTree");
        virtual ~MSVtxPlotMaker();
        void makePlots();

    private:
        // structs to pass a set of plots easier between functions

        // number of vertices
        struct NVtxTH1 {
            TH1* h_Nvtx{nullptr};
            TH1* h_Nvtx_b{nullptr};
            TH1* h_Nvtx_e{nullptr};

            NVtxTH1(TH1* h_Nvtx, TH1* h_Nvtx_b, TH1* h_Nvtx_e)
                    : h_Nvtx(h_Nvtx), h_Nvtx_b(h_Nvtx_b), h_Nvtx_e(h_Nvtx_e) {};
        };
        // vertex position graphs
        struct VtxPosTGraph {
            TGraph* zLxy_b{nullptr};
            TGraph* etaphi_b{nullptr};
            TGraph* zLxy_e{nullptr};
            TGraph* etaphi_e{nullptr};
            TGraph* zLxy_out{nullptr};
            TGraph* etaphi_out{nullptr};

            VtxPosTGraph(TGraph* zLxy_b, TGraph* etaphi_b, TGraph* zLxy_e, TGraph* etaphi_e, TGraph* zLxy_out, TGraph* etaphi_out)
                         : zLxy_b(zLxy_b), etaphi_b(etaphi_b), zLxy_e(zLxy_e), etaphi_e(etaphi_e), zLxy_out(zLxy_out), etaphi_out(etaphi_out) {};
        };
        // vertex position histograms
        struct VtxPosTH {
            TH2* h_zLxy{nullptr};
            TH2* h_etaphi{nullptr};
            TH1* h_distanceToIP{nullptr};

            VtxPosTH(TH2* h_zLxy, TH2* h_etaphi, TH1* h_distanceToIP)
                     : h_zLxy(h_zLxy), h_etaphi(h_etaphi), h_distanceToIP(h_distanceToIP) {};
        };
        // vertex chi2 histograms
        struct Chi2TH1 {
            TH1* h_chi2{nullptr};
            TH1* h_chi2nDoF{nullptr};
            TH1* h_chi2prob{nullptr};

            Chi2TH1(TH1* h_chi2, TH1* h_chi2nDoF, TH1* h_chi2prob)
                    : h_chi2(h_chi2), h_chi2nDoF(h_chi2nDoF), h_chi2prob(h_chi2prob) {};
        };
        // angular distances histograms between vertex and its constituents
        struct AngularVtxConstiTH1 {
            TH1* h_dR{nullptr};
            TH1* h_dRmax{nullptr};
            TH1* h_dphimax{nullptr};
            TH1* h_detamax{nullptr};

            AngularVtxConstiTH1(TH1* h_dR, TH1* h_dRmax, TH1* h_dphimax, TH1* h_detamax)
                               : h_dR(h_dR), h_dRmax(h_dRmax), h_dphimax(h_dphimax), h_detamax(h_detamax) {};
        };
        // residuals between reconstructed and matched truth vertices
        struct ResidualTH1 {
            TH1* h_delta_R{nullptr};
            TH1* h_delta_theta{nullptr};
            TH1* h_delta_phi{nullptr};
            TH1* h_delta_Lxy{nullptr};
            TH1* h_delta_z{nullptr};
            TH1* h_delta_phys{nullptr};
            TH1* h_delta_Lxy_posEta{nullptr};
            TH1* h_delta_Lxy_negEta{nullptr};
            TH1* h_delta_z_posEta{nullptr};
            TH1* h_delta_z_negEta{nullptr};

            ResidualTH1(TH1* h_delta_R, TH1* h_delta_theta, TH1* h_delta_phi, TH1* h_delta_Lxy, TH1* h_delta_z, TH1* h_delta_phys,
                        TH1* h_delta_Lxy_posEta, TH1* h_delta_Lxy_negEta, TH1* h_delta_z_posEta, TH1* h_delta_z_negEta)
                        : h_delta_R(h_delta_R), h_delta_theta(h_delta_theta), h_delta_phi(h_delta_phi), h_delta_Lxy(h_delta_Lxy), h_delta_z(h_delta_z), h_delta_phys(h_delta_phys),
                          h_delta_Lxy_posEta(h_delta_Lxy_posEta), h_delta_Lxy_negEta(h_delta_Lxy_negEta), h_delta_z_posEta(h_delta_z_posEta), h_delta_z_negEta(h_delta_z_negEta) {};
        };
        // hits near the vertex 
        struct NHitsTH1 {
            TH1* h_total{nullptr};
            TH1* h_I{nullptr};
            TH1* h_M{nullptr};
            TH1* h_O{nullptr};
            TH1* h_inwardsTotal{nullptr};
            TH1* h_IM{nullptr};
            TH1* h_IO{nullptr};
            TH1* h_MO{nullptr};

            NHitsTH1(TH1* h_total, TH1* h_I, TH1* h_M, TH1* h_O, TH1* h_inwardsTotal, TH1* h_IM, TH1* h_IO, TH1* h_MO)
                    : h_total(h_total), h_I(h_I), h_M(h_M), h_O(h_O), h_inwardsTotal(h_inwardsTotal), h_IM(h_IM), h_IO(h_IO), h_MO(h_MO) {};
        };
        // TH1s for different vertex efficiency and purity plot 
        struct EffInputTH1 {
            TH1* h_Lxy{nullptr};
            TH1* h_distanceToIP_b{nullptr};
            TH1* h_z{nullptr};
            TH1* h_distanceToIP_e{nullptr};
            TH1* h_eta{nullptr};

            EffInputTH1(TH1* h_Lxy, TH1* h_distanceToIP_b, TH1* h_z, TH1* h_distanceToIP_e, TH1* h_eta)
                        : h_Lxy(h_Lxy), h_distanceToIP_b(h_distanceToIP_b), h_z(h_z), h_distanceToIP_e(h_distanceToIP_e), h_eta(h_eta) {};
        };


        // core functions
        void setup();
        void setPlotStyle();
        void formatPlots();
        void fillPlots();
        void outputResults();
        // helper functions to setup branches and declare plots
        void setupBranches();
        void formatTGraphs();
        // main filling functions
        void fillTruthVtxPlots(const std::vector<Amg::Vector3D> &truth_vertices);
        void fillReconstructionObjectsHists();
        void fillRecoVtxPlots(const std::vector<Amg::Vector3D> &reco_vertices, const std::vector<std::vector<Amg::Vector3D>> &reco_constituentPos);
        void fillTruthComparisonHists(const std::vector<Amg::Vector3D> &reco_vertices, const std::vector<Amg::Vector3D> &truth_vertices);
        void fillEfficiency_NumeratorDenominatorHists(const std::vector<Amg::Vector3D> &vertices, const std::vector<Amg::Vector3D> &match_candidates,
                                                      std::unique_ptr<EffInputTH1> &denomHists, std::unique_ptr<EffInputTH1> &numHists);
        // helper filling functions
        void fillNvtxHists(const std::vector<Amg::Vector3D> &vertices, std::unique_ptr<NVtxTH1> &hists);
        void fillVtxPosMaps(const std::vector<Amg::Vector3D> &vertices, std::unique_ptr<VtxPosTGraph> &graphs);
        void fillVtxPosHists(const std::vector<Amg::Vector3D> &vertices, std::unique_ptr<VtxPosTH> &hists);
        void fillChi2Hists(double chi2, double NDoF, std::unique_ptr<Chi2TH1> &hists);
        void fillAngularVtxConstiHists(const Amg::Vector3D &vtx, const std::vector<Amg::Vector3D> &consti, std::unique_ptr<AngularVtxConstiTH1> &hists);
        void fillVtxNhitsHists(double total, double inwards, double inner, double middle, double outer, std::unique_ptr<NHitsTH1> &hists);
        void fillResidualHists(double eta, double dR, double d_theta, double d_phi, double d_Lxy, double d_z, double d_phys, std::unique_ptr<ResidualTH1> &hists);
        void fillVtxPosFiducialVolHists(const Amg::Vector3D &vtx, std::unique_ptr<EffInputTH1> &hists);
        // saving plots
        void saveVtxPos(std::unique_ptr<VtxPosTH> &hists, const TString &plotdir);
        void saveTGraph(TMultiGraph* zLxy, TMultiGraph* etaphi, std::unique_ptr<VtxPosTGraph> &graphs, const TString &plotdir);
        void saveTH1(TH1* h, TString plotpath, const char* dectectorLabel="", const char* new_ylabel=nullptr, bool norm=false, bool logy=false);
        void saveTHStack(TH1* h1, TH1* h2, const TString &h1_legend, const TString &h2_legend, const TString &title, const TString &plotpath, int color1=2, int color2=1);
        void saveTEfficiency(TH1* h_num, TH1* h_denom, const TString &title, const TString &plotpath);
        void saveTH2(TH2* h, const TString &plotpath);
        // other helper functions
        void setColorPalette(TStyle *plotStyle);
        TH1* getUnmatchedHist(TH1* h_all, TH1* h_matched, const TString &name_unmatched);


        // input/output variables 
        std::string m_datapath{};
        std::string m_treename{};
        TString m_plotdir{};
        TString m_plotdir_truthVtx{};
        TString m_plotdir_recoVtx{};
        TString m_plotdir_recoVtxHits{};
        TString m_plotdir_vtxResiduals{};
        TString m_plotdir_inputObjects{};
        TString m_plotdir_vtxEfficiency{};
        TString m_plotdir_vtxFakeRate{};
        TTree* m_tree{nullptr}; // TTree is attached to the input file i.e. no smart pointer needed
        std::unique_ptr<TFile> m_input_file{nullptr};
        std::unique_ptr<TFile> m_output_file{nullptr};
        std::unique_ptr<TCanvas> m_c{nullptr};

        // branches
        std::vector<double>* m_truthVtx_x{nullptr};
        std::vector<double>* m_truthVtx_y{nullptr};
        std::vector<double>* m_truthVtx_z{nullptr};

        std::vector<double>* m_msVtx_chi2{nullptr};
        std::vector<int>* m_msVtx_Ntrklet{nullptr};
        std::vector<double>* m_msVtx_x{nullptr};
        std::vector<double>* m_msVtx_y{nullptr};
        std::vector<double>* m_msVtx_z{nullptr};
        std::vector<int>* m_msVtx_nMDT{nullptr};
        std::vector<int>* m_msVtx_nMDT_inwards{nullptr};
        std::vector<int>* m_msVtx_nMDT_I{nullptr};
        std::vector<int>* m_msVtx_nMDT_M{nullptr};
        std::vector<int>* m_msVtx_nMDT_O{nullptr};
        std::vector<int>* m_msVtx_nRPC{nullptr};
        std::vector<int>* m_msVtx_nRPC_inwards{nullptr};
        std::vector<int>* m_msVtx_nRPC_I{nullptr};
        std::vector<int>* m_msVtx_nRPC_M{nullptr};
        std::vector<int>* m_msVtx_nRPC_O{nullptr};
        std::vector<int>* m_msVtx_nTGC{nullptr};
        std::vector<int>* m_msVtx_nTGC_inwards{nullptr};
        std::vector<int>* m_msVtx_nTGC_I{nullptr};
        std::vector<int>* m_msVtx_nTGC_M{nullptr};
        std::vector<int>* m_msVtx_nTGC_O{nullptr};

        std::vector<double>* m_obj_x{nullptr}; 
        std::vector<double>* m_obj_y{nullptr}; 
        std::vector<double>* m_obj_z{nullptr}; 
        std::vector<double>* m_obj_phi{nullptr}; 
        std::vector<double>* m_obj_theta{nullptr}; 
        std::vector<double>* m_obj_eta{nullptr}; 
        std::vector<int>* m_obj_vtxLink{nullptr};


        // histograms and graphs

        // truth vertices
        // number of vertices per event
        TH1* m_h_Nvtx_truth = new TH1D("Nvtx_truth", "Nvtx_truth; Number of truth vertices in the barrel or endcaps; Events / bin", 4, 0, 4);
        TH1* m_h_Nvtx_truth_b = new TH1D("Nvtx_truth_b", "Nvtx_truth_b; Number of truth vertices in the barrel; Events / bin", 4, 0, 4);
        TH1* m_h_Nvtx_truth_e = new TH1D("Nvtx_truth_e", "Nvtx_truth_e; Number of truth vertices in the endcaps; Events / bin", 4, 0, 4);
        // position 2D maps: TMultiGraph is composed of TGraphs for vertices in the barrel, the endcaps or in neither (outside the detector region)
        // Lxy-z map
        TMultiGraph* m_zLxy_truth = new TMultiGraph("zLxy_mg_truth", "zLxy_mg_truth"); 
        TGraph* m_zLxy_truth_b = new TGraph(); 
        TGraph* m_zLxy_truth_e = new TGraph(); 
        TGraph* m_zLxy_truth_out = new TGraph(); 
        // eta-phi map
        TMultiGraph* m_etaphi_truth = new TMultiGraph("etaphi_mg_truth", "etaphi_mg_truth"); 
        TGraph* m_etaphi_truth_b = new TGraph(); 
        TGraph* m_etaphi_truth_e = new TGraph(); 
        TGraph* m_etaphi_truth_out = new TGraph(); 
        // position 1D histograms
        TH1* m_h_distanceToIP_truth = new TH1D("distanceToIP_truth", "distanceToIP_truth; Truth vertex |#bf{r}| [m]; Vertices / bin", 50, 0, 20);
        // position 2D histograms
        TH2* m_h_zLxy_truth = new TH2D("zLxy_truth", "zLxy_truth; Truth vertex z [m]; Truth vertex L_{xy} [m]", 128, -16, 16, 52, -1, 12);
        TH2* m_h_etaphi_truth = new TH2D("etaphi_truth", "etaphi_truth; Truth vertex #kern[-1.0]{ #eta}; Truth vertex #kern[-1.0]{ #phi} [rad]", 80, -2.6, 2.6, 40, -TMath::Pi(), TMath::Pi()+1);

        // reconstructed vertices
        // number per event
        TH1* m_h_Nvtx = new TH1D("Nvtx", "Nvtx; Number of reco vertices in barrel or endcaps; Events / bin", 3, 0, 3);
        TH1* m_h_Nvtx_b = new TH1D("Nvtx_b", "Nvtx_b; Number of reco vertices in the barrel; Events / bin", 3, 0, 3);
        TH1* m_h_Nvtx_e = new TH1D("Nvtx_e", "Nvtx_e; Number of reco vertices in the endcaps; Events / bin", 3, 0, 3);
        // position 2D maps: TMultiGraph is composed of TGraphs for vertices in the barrel, the endcaps or in neither (outside the detector region)
        // Lxy-z map of reconstructed vertices
        TMultiGraph* m_zLxy = new TMultiGraph("zLxy_mg", "zLxy_mg"); 
        TGraph* m_zLxy_b = new TGraph(); 
        TGraph* m_zLxy_e = new TGraph(); 
        TGraph* m_zLxy_out = new TGraph(); 
        // eta-phi map of reconstructed vertices
        TMultiGraph* m_etaphi = new TMultiGraph("etaphi_mg", "etaphi_mg"); 
        TGraph* m_etaphi_b = new TGraph(); 
        TGraph* m_etaphi_e = new TGraph(); 
        TGraph* m_etaphi_out = new TGraph(); 
        // position 1D histograms
        TH1* m_h_distanceToIP = new TH1D("distanceToIP", "distanceToIP; Reco vertex |#bf{r}| [m]; Vertices / bin", 50, 0, 20);
        // position 2D histograms
        TH2* m_h_zLxy = new TH2D("zLxy", "zLxy; Reco vertex z [m]; Reco vertex L_{xy} [m]", 128, -16, 16, 52, -1, 12);
        TH2* m_h_etaphi = new TH2D("etaphi", "etaphi; Reco vertex #kern[-1.0]{ #eta}; Reco vertex #kern[-1.0]{ #phi} [rad]", 80, -2.6, 2.6, 40, -TMath::Pi(), TMath::Pi()+1);
        // chi2, chi2 per degree of freedom, and chi2 probability of vertex
        TH1* m_h_chi2_b = new TH1D("chi2_b", "chi2_b; Reco vertex #chi^{2}; Vertices / bin", 100, 0, 10);
        TH1* m_h_chi2_e = new TH1D("chi2_e", "chi2_e; Reco vertex #chi^{2}; Vertices / bin", 100, 0, 10);
        TH1* m_h_chi2nDoF_b = new TH1D("chi2nDoF_b", "chi2nDoF_b; Reco vertex #chi^{2}/n_{DoF}; Vertices / bin", 100, 0, 5);
        TH1* m_h_chi2nDoF_e = new TH1D("chi2nDoF_e", "chi2nDoF_e; Reco vertex #chi^{2}/n_{DoF}; Vertices / bin", 100, 0, 5);
        TH1* m_h_chi2prob_b = new TH1D("chi2_prob_b", "chi2_prob_b; Reco vertex #chi^{2} probability; Vertices / bin", 100, 0, 1);
        TH1* m_h_chi2prob_e = new TH1D("chi2_prob_e", "chi2_prob_e; Reco vertex #chi^{2} probability; Vertices / bin", 100, 0, 1);
        // number of constituents
        TH1* m_h_Nconsti_b = new TH1D("Nconstituents_b", "Nconstituents_b; Reco vertex number of constituents; Vertices / bin", 39, 1, 40);
        TH1* m_h_Nconsti_e = new TH1D("Nconstituents_e", "Nconstituents_e; Reco vertex number of constituents; Vertices / bin", 39, 1, 40);
        // delta R to constituents
        TH1* m_h_VtxConsti_dR_b = new TH1D("VtxConsti_dR_b", "VtxConsti_dR_b; #Delta R(reco vtx, constituent); Number of vertex-constituent pairs / bin", 100, 0, 3);
        TH1* m_h_VtxConsti_dRmax_b = new TH1D("VtxConsti_dRmax_b", "VtxConsti_dRmax_b; max #kern[-0.3]{#Delta } #kern[-0.2]{ R(reco vtx, constituent)}; Vertices / bin", 50, 0, 3);
        TH1* m_h_VtxConsti_dphimax_b = new TH1D("VtxConsti_dphimax_b", "VtxConsti_dphimax_b; max #kern[-0.3]{#Delta } #kern[-0.3]{  #phi (reco vtx, constituent)}; Vertices / bin", 50, -2.5, 2.5);
        TH1* m_h_VtxConsti_detamax_b = new TH1D("VtxConsti_detamax_b", "VtxConsti_detamax_b; max #kern[-0.3]{#Delta } #kern[-0.3]{  #eta (reco vtx, constituent)}; Vertices / bin", 50, -1.0, 1.0);
        TH1* m_h_VtxConsti_dR_e = new TH1D("VtxConsti_dR_e", "VtxConsti_dR_e; #Delta R(reco vtx, constituent); Number of vertex-constituent pairs / bin", 100, 0, 3);
        TH1* m_h_VtxConsti_dRmax_e = new TH1D("VtxConsti_dRmax_e", "VtxConsti_dRmax_e; max #kern[-0.3]{#Delta } #kern[-0.2]{ R(reco vtx, constituent)}; Vertices / bin", 50, 0, 3);
        TH1* m_h_VtxConsti_dphimax_e = new TH1D("VtxConsti_dphimax_e", "VtxConsti_dphimax_e; max #kern[-0.3]{#Delta } #kern[-0.3]{  #phi (reco vtx, constituent)}; Vertices / bin", 50, -2.5, 2.5);
        TH1* m_h_VtxConsti_detamax_e = new TH1D("VtxConsti_detamax_e", "VtxConsti_detamax_e; max #kern[-0.3]{#Delta } #kern[-0.3]{  #eta (reco vtx, constituent)}; Vertices / bin", 50, -1.0, 1.0);

        // hits near the vertex (not necessarily used for the reconstruction)
        // total and split by layer 
        // fraction that are further inwards than the vertex,
        // rations by layer: inner/middle, inner/outer, middle/outer 
        // MDT
        TH1* m_h_nMDT_b = new TH1D("nMDT_b", "nMDT_b; Number of MDT hits near the reco vertex; Vertices / bin", 100, 0, 4000);
        TH1* m_h_nMDT_e = new TH1D("nMDT_e", "nMDT_e; Number of MDT hits near the reco vertex; Vertices / bin", 100, 0, 4000);
        TH1* m_h_nMDT_I_b = new TH1D("nMDT_I_b", "nMDT_I_b; Number of inner layer MDT hits near the reco vertex; Vertices / bin", 100, 0, 1000);
        TH1* m_h_nMDT_I_e = new TH1D("nMDT_I_e", "nMDT_I_e; Number of inner layer MDT hits near the reco vertex; Vertices / bin", 100, 0, 1000);
        TH1* m_h_nMDT_M_b = new TH1D("nMDT_M_b", "nMDT_M_b; Number of middle layer MDT hits near the reco vertex; Vertices / bin", 100, 0, 2500);
        TH1* m_h_nMDT_M_e = new TH1D("nMDT_M_e", "nMDT_M_e; Number of middle layer MDT hits near the reco vertex; Vertices / bin", 100, 0, 2500);
        TH1* m_h_nMDT_O_b = new TH1D("nMDT_O_b", "nMDT_O_b; Number of outer layer MDT hits near the reco vertex; Vertices / bin", 100, 0, 2500);
        TH1* m_h_nMDT_O_e = new TH1D("nMDT_O_e", "nMDT_O_e; Number of outer layer MDT hits near the reco vertex; Vertices / bin", 100, 0, 2500);
        TH1* m_h_nMDT_InwardsTotal_b = new TH1D("nMDT_InwardsTotal_b", "nMDT_InwardsTotal_b; fraction of MDT hits inwards of the reco vertex; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_InwardsTotal_e = new TH1D("nMDT_InwardsTotal_e", "nMDT_InwardsTotal_e; fraction of MDT hits inwards of the reco vertex; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_IM_b = new TH1D("nMDT_IM_b", "nMDT_IM_b; Reco vertex inner/middle layer MDT hits; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_IM_e = new TH1D("nMDT_IM_e", "nMDT_IM_e; Reco vertex inner/middle layer MDT hits; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_IO_b = new TH1D("nMDT_IO_b", "nMDT_IO_b; Reco vertex inner/outer layer MDT hits; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_IO_e = new TH1D("nMDT_IO_e", "nMDT_IO_e; Reco vertex inner/outer layer MDT hits; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_MO_b = new TH1D("nMDT_MO_b", "nMDT_MO_b; Reco vertex middle/outer layer MDT hits; Vertices / bin", 100, 0, 3);
        TH1* m_h_nMDT_MO_e = new TH1D("nMDT_MO_e", "nMDT_MO_e; Reco vertex middle/outer layer MDT hits; Vertices / bin", 100, 0, 3);
        // RPC
        TH1* m_h_nRPC = new TH1D("nRPC", "nRPC_e; Number of RPC hits near the reco vertex; Vertices / bin", 50, 0, 1500);
        TH1* m_h_nRPC_I = new TH1D("nRPC_I", "nRPC_I_e; Number of inner layer RPC hits near the reco vertex; Vertices / bin", 50, 0, 500);
        TH1* m_h_nRPC_M = new TH1D("nRPC_M", "nRPC_M_e; Number of middle layer RPC hits near the reco vertex; Vertices / bin", 50, 0, 1000);
        TH1* m_h_nRPC_O = new TH1D("nRPC_O", "nRPC_O_e; Number of outer layer RPC hits near the reco vertex; Vertices / bin", 50, 0, 1000);
        TH1* m_h_nRPC_InwardsTotal = new TH1D("nRPC_InwardsTotal", "nRPC_InwardsTotal_e; fraction of RPC hits inwards of the reco vertex; Vertices / bin", 50, 0, 1);
        TH1* m_h_nRPC_IM = new TH1D("nRPC_IM", "nRPC_IM_e; Reco vertex inner/middle layer RPC hits; Vertices / bin", 50, 0, 3);
        TH1* m_h_nRPC_IO = new TH1D("nRPC_IO", "nRPC_IO_e; Reco vertex inner/outer layer RPC hits; Vertices / bin", 50, 0, 3);
        TH1* m_h_nRPC_MO = new TH1D("nRPC_MO", "nRPC_MO_e; Reco vertex middle/outer layer RPC hits; Vertices / bin", 50, 0, 3);
        // TGC
        TH1* m_h_nTGC = new TH1D("nTGC", "nTGC_e; Number of TGC hits near the reco vertex; Vertices / bin", 50, 0, 1500);
        TH1* m_h_nTGC_I = new TH1D("nTGC_I", "nTGC_I_e; Number of inner layer TGC hits near the reco vertex; Vertices / bin", 50, 0, 500);
        TH1* m_h_nTGC_M = new TH1D("nTGC_M", "nTGC_M_e; Number of middle layer TGC hits near the reco vertex; Vertices / bin", 50, 0, 1000);
        TH1* m_h_nTGC_O = new TH1D("nTGC_O", "nTGC_O_e; Number of outer layer TGC hits near the reco vertex; Vertices / bin", 50, 0, 1000);
        TH1* m_h_nTGC_InwardsTotal = new TH1D("nTGC_InwardsTotal", "nTGC_InwardsTotal_e; fraction of TGC hits inwards of the reco vertex; Vertices / bin", 50, 0, 1);
        TH1* m_h_nTGC_IM = new TH1D("nTGC_IM", "nTGC_IM_e; Reco vertex inner/middle layer TGC hits; Vertices / bin", 50, 0, 3);
        TH1* m_h_nTGC_IO = new TH1D("nTGC_IO", "nTGC_IO_e; Reco vertex inner/outer layer TGC hits; Vertices / bin", 50, 0, 3);
        TH1* m_h_nTGC_MO = new TH1D("nTGC_MO", "nTGC_MO_e; Reco vertex middle/outer layer TGC hits; Vertices / bin", 50, 0, 3);

        // objects available and used for reconstruction 
        // number per event
        TH1* m_h_Nobj = new TH1D("Nobj", "Nobj; Number of objects; Events / bin", 20, 0, 20);  
        TH1* m_h_Nobj_b = new TH1D("Nobj_b", "Nobj_b; Number of objects in the barrel; Events / bin", 20, 0, 20);  
        TH1* m_h_Nobj_e = new TH1D("Nobj_e", "Nobj_e; Number of objects in the endcaps; Events / bin", 20, 0, 20); 
        TH1* m_h_NobjReco = new TH1D("objN_reco", "objN_reco; Number of objects used for reconstruction in the barrel; Events / bin", 20, 0, 20);  
        TH1* m_h_NobjReco_b = new TH1D("objN_reco_b", "objN_reco_b; Number of objects used for reconstruction in the barrel; Events / bin", 20, 0, 20);  
        TH1* m_h_NobjReco_e = new TH1D("objN_reco_e", "objN_reco_e; Number of objects used for reconstruction in the endcaps; Events / bin", 20, 0, 20);  
        // position
        TH1* m_h_obj_phi_b = new TH1D("objPhi_b", "objPhi_b; Object #kern[-0.05]{#phi in the barrel}; Objects / bin", 100, -TMath::Pi(), TMath::Pi());  
        TH1* m_h_obj_phi_e = new TH1D("objPhi_e", "objPhi_e; Object #kern[-0.05]{#phi in the endcaps}; Objects / bin", 100, -TMath::Pi(), TMath::Pi());  
        TH1* m_h_obj_eta = new TH1D("objEta", "objEta; Object #eta; Objects / bin", 50, -2.7, 2.7);  
        TH1* m_h_objReco_phi_b = new TH1D("objPhi_reco_b", "objPhi_reco_b; Object #kern[-0.04]{#phi used for reconstruction in the barrel}; Objects / bin", 100, -TMath::Pi(), TMath::Pi());  
        TH1* m_h_objReco_phi_e = new TH1D("objPhi_reco_e", "objPhi_reco_e; Object #kern[-0.04]{#phi used for reconstruction in the endcaps}; Objects / bin", 100, -TMath::Pi(), TMath::Pi());  
        TH1* m_h_objReco_eta = new TH1D("objEta_reco", "objEta_reco; Object #kern[-0.06]{#eta used for reconstruction}; Objects / bin", 50, -2.7, 2.7);  

        // residuals between reconstructed and the matched truth vertex.
        TH1* m_h_delta_R_b = new TH1D("delta_R_b", "delta_R; #Delta R(reco Vtx, truth Vtx); Vertices / bin", 50, 0, 0.5);
        TH1* m_h_delta_R_e = new TH1D("delta_R_e", "delta_R; #Delta R(reco Vtx, truth Vtx); Vertices / bin", 50, 0, 0.5);
        TH1* m_h_delta_theta_b = new TH1D("delta_theta_b", "delta_theta; #theta_{reco} - #theta_{truth} [rad]; Vertices / 0.01 rad", 100, -0.5, 0.5);
        TH1* m_h_delta_theta_e = new TH1D("delta_theta_e", "delta_theta; #theta_{reco} - #theta_{truth} [rad]; Vertices / 0.01 rad", 100, -0.5, 0.5);
        TH1* m_h_delta_phi_b = new TH1D("delta_phi_b", "delta_phi; #phi_{reco} - #phi_{truth} [rad]; Vertices / 0.02 rad", 50, -0.5, 0.5);
        TH1* m_h_delta_phi_e = new TH1D("delta_phi_e", "delta_phi; #phi_{reco} - #phi_{truth} [rad]; Vertices / 0.02 rad", 50, -0.5, 0.5);
        TH1* m_h_delta_Lxy_b = new TH1D("delta_Lxy_b", "delta_Lxy; L_{xy}^{reco} - L_{xy}^{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_Lxy_e = new TH1D("delta_Lxy_e", "delta_Lxy; L_{xy}^{reco} - L_{xy}^{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_z_b = new TH1D("delta_z_b", "delta_z; z_{reco} - z_{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_z_e = new TH1D("delta_z_e", "delta_z; z_{reco} - z_{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_phys_b = new TH1D("delta_phys_b", "delta_phys; |#bf{r}_{reco} - #bf{r}_{truth}| [cm]; Vertices / 10 cm", 50, 0, 500);
        TH1* m_h_delta_phys_e = new TH1D("delta_phys_e", "delta_phys; |#bf{r}_{reco} - #bf{r}_{truth}| [cm]; Vertices / 10 cm", 75, 0, 750);
        // split into positve and negative eta 
        TH1* m_h_delta_Lxy_posEta_b = new TH1D("delta_Lxy_posEta_b", "delta_Lxy; L_{xy}^{reco} - L_{xy}^{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_Lxy_posEta_e = new TH1D("delta_Lxy_posEta_e", "delta_Lxy; L_{xy}^{reco} - L_{xy}^{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_Lxy_negEta_b = new TH1D("delta_Lxy_negEta_b", "delta_Lxy; L_{xy}^{reco} - L_{xy}^{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_Lxy_negEta_e = new TH1D("delta_Lxy_negEta_e", "delta_Lxy; L_{xy}^{reco} - L_{xy}^{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_z_posEta_b = new TH1D("delta_z_posEta_b", "delta_z; z_{reco} - z_{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_z_negEta_b = new TH1D("delta_z_negEta_b", "delta_z; z_{reco} - z_{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_z_posEta_e = new TH1D("delta_z_posEta_e", "delta_z; z_{reco} - z_{truth} [cm]; Vertices / 10 cm", 100, -500, 500);
        TH1* m_h_delta_z_negEta_e = new TH1D("delta_z_negEta_e", "delta_z; z_{reco} - z_{truth} [cm]; Vertices / 10 cm", 100, -500, 500);

        // histograms for efficiency and fake rate calculations
        // m_h_<vtx type>_<binning variable> contains all reco or truth vertices 
        // m_h_<vtx type 1>_<vtx type 2>_<binning variable> contains all vertices of type <vtx type 1> that can be matched to a vertex of type <vtx type 2>  
        // barrel vertices, binned in the transverse truth decay position
        TH1* m_h_Reco_Lxy_b = new TH1D("vReco_Lxy_b", "reco vertex b; Reco vertex L_{xy} [m]; Vertices / bin", 30, 0, 9); 
        TH1* m_h_RecoTruth_Lxy_b = new TH1D("RecoTruth_Lxy_b", "reco vertex matched to truth in the barrel; Reco vertex L_{xy} [m]; Vertices / bin", 30, 0, 9); 
        TH1* m_h_Truth_Lxy_b = new TH1D("Truth_Lxy_b", "truth vertex b; Truth vertex L_{xy} [m]; Truth vertices / bin", 30, 0, 9);
        TH1* m_h_TruthReco_Lxy_b = new TH1D("TruthReco_Lxy_b", "truth vertex matched to reco in the barrel; Truth vertex L_{xy} [m]; Vertices / bin", 30, 0, 9); 
        // endcap vertices, binned in z truth decay position
        TH1* m_h_Reco_z_e = new TH1D("Reco_z_e", "reco vertex z; Reco vertex |z| [m]; Vertices / bin", 40, 0, 16); 
        TH1* m_h_RecoTruth_z_e = new TH1D("RecoTruth_z_e", "reco vertex z matched to truth in the endcaps; Reco vertex |z| [m]; Vertices / bin", 40, 0, 16); 
        TH1* m_h_Truth_z_e = new TH1D("Truth_z_e", "truth vertex z; Truth vertex |z| [m]; Truth vertices / bin", 40, 0, 16);
        TH1* m_h_TruthReco_z_e = new TH1D("TruthReco_z_e", "truth vertex z matched to reco in the endcaps; Truth vertex |z| [m]; Vertices / bin", 40, 0, 16); 
        // binned in eta
        TH1* m_h_Reco_eta = new TH1D("Reco_eta", "reco vertex eta; Reco vertex #eta; Vertices / bin", 30, -3, 3); 
        TH1* m_h_RecoTruth_eta = new TH1D("RecoTruth_eta", "reco vertex matched to truth; Reco vertex #eta; Vertices / bin", 30, -3, 3); 
        TH1* m_h_Truth_eta = new TH1D("Truth_eta", "truth vertex eta; Truth vertex #eta; Truth vertices / bin", 30, -3, 3);
        TH1* m_h_TruthReco_eta = new TH1D("TruthReco_eta", "truth vertex matched to reco; Truth vertex #eta; Vertices / bin", 30, -3, 3); 
        // barrel vertices, binned in distance from IP
        TH1* m_h_Reco_r_b = new TH1D("Reco_r_b", "reco vertex r; Reco vertex |#bf{r}| [m]; Vertices / bin", 60, 0, 18); 
        TH1* m_h_RecoTruth_r_b = new TH1D("RecoTruth_r_b", "reco vertex matched to truth; Reco vertex |#bf{r}| [m]; Vertices / bin", 60, 0, 18); 
        TH1* m_h_Truth_r_b = new TH1D("Truth_r_b", "truth vertex r; Truth vertex |#bf{r}| [m]; Truth vertices / bin", 60, 0, 18);
        TH1* m_h_TruthReco_r_b = new TH1D("TruthReco_r_b", "truth vertex matched to reco; Truth vertex |#bf{r}| [m]; Vertices / bin", 60, 0, 18); 
        // endcap vertices, binned in distance from IP
        TH1* m_h_Reco_r_e = new TH1D("Reco_r_e", "reco vertex r; Reco vertex |#bf{r}| [m]; Vertices / bin", 60, 0, 18); 
        TH1* m_h_RecoTruth_r_e = new TH1D("RecoTruth_r_e", "reco vertex matched to truth; Reco vertex |#bf{r}| [m]; Vertices / bin", 60, 0, 18); 
        TH1* m_h_Truth_r_e = new TH1D("Truth_r_e", "truth vertex r; Truth vertex |#bf{r}| [m]; Truth vertices / bin", 60, 0, 18);
        TH1* m_h_TruthReco_r_e = new TH1D("TruthReco_r_e", "truth vertex matched to reco; Truth vertex |#bf{r}| [m]; Vertices / bin", 60, 0, 18); 


        // bind the plots into structs for easier passing between functions
        // std::unique_ptr<NVtxTH1> m_h_NVtx_truth = std::make_unique<NVtxTH1>();
        std::unique_ptr<NVtxTH1> m_h_NVtx_truth = std::make_unique<NVtxTH1>(m_h_Nvtx_truth, m_h_Nvtx_truth_b, m_h_Nvtx_truth_e);
        std::unique_ptr<NVtxTH1> m_h_NVtx = std::make_unique<NVtxTH1>(m_h_Nvtx, m_h_Nvtx_b, m_h_Nvtx_e);

        std::unique_ptr<VtxPosTGraph> m_h_VtxPos = std::make_unique<VtxPosTGraph>(m_zLxy_b, m_etaphi_b, m_zLxy_e, m_etaphi_e, m_zLxy_out, m_etaphi_out);
        std::unique_ptr<VtxPosTGraph> m_h_VtxPos_truth = std::make_unique<VtxPosTGraph>(m_zLxy_truth_b, m_etaphi_truth_b, m_zLxy_truth_e, m_etaphi_truth_e, m_zLxy_truth_out, m_etaphi_truth_out);
        std::unique_ptr<VtxPosTH> m_h_VtxPosHists = std::make_unique<VtxPosTH>(m_h_zLxy, m_h_etaphi, m_h_distanceToIP);
        std::unique_ptr<VtxPosTH> m_h_VtxPosHists_truth = std::make_unique<VtxPosTH>(m_h_zLxy_truth, m_h_etaphi_truth, m_h_distanceToIP_truth);

        std::unique_ptr<Chi2TH1> m_h_VtxChi2_b = std::make_unique<Chi2TH1>(m_h_chi2_b, m_h_chi2nDoF_b, m_h_chi2prob_b);
        std::unique_ptr<Chi2TH1> m_h_VtxChi2_e = std::make_unique<Chi2TH1>(m_h_chi2_e, m_h_chi2nDoF_e, m_h_chi2prob_e);

        std::unique_ptr<AngularVtxConstiTH1> m_h_AngularVtxConsti_b = std::make_unique<AngularVtxConstiTH1>(m_h_VtxConsti_dR_b, m_h_VtxConsti_dRmax_b, m_h_VtxConsti_dphimax_b, m_h_VtxConsti_detamax_b);
        std::unique_ptr<AngularVtxConstiTH1> m_h_AngularVtxConsti_e = std::make_unique<AngularVtxConstiTH1>(m_h_VtxConsti_dR_e, m_h_VtxConsti_dRmax_e, m_h_VtxConsti_dphimax_e, m_h_VtxConsti_detamax_e);

        std::unique_ptr<ResidualTH1> m_h_VtxResiduals_b = std::make_unique<ResidualTH1>(m_h_delta_R_b, m_h_delta_theta_b, m_h_delta_phi_b, m_h_delta_Lxy_b, m_h_delta_z_b, m_h_delta_phys_b, m_h_delta_Lxy_posEta_b, m_h_delta_Lxy_negEta_b, m_h_delta_z_posEta_b, m_h_delta_z_negEta_b);
        std::unique_ptr<ResidualTH1> m_h_VtxResiduals_e = std::make_unique<ResidualTH1>(m_h_delta_R_e, m_h_delta_theta_e, m_h_delta_phi_e, m_h_delta_Lxy_e, m_h_delta_z_e, m_h_delta_phys_e, m_h_delta_Lxy_posEta_e, m_h_delta_Lxy_negEta_e, m_h_delta_z_posEta_e, m_h_delta_z_negEta_e);
        std::unique_ptr<NHitsTH1> m_h_Nhits_MDT_b = std::make_unique<NHitsTH1>(m_h_nMDT_b, m_h_nMDT_I_b, m_h_nMDT_M_b, m_h_nMDT_O_b, m_h_nMDT_InwardsTotal_b, m_h_nMDT_IM_b, m_h_nMDT_IO_b, m_h_nMDT_MO_b);
        std::unique_ptr<NHitsTH1> m_h_Nhits_MDT_e = std::make_unique<NHitsTH1>(m_h_nMDT_e, m_h_nMDT_I_e, m_h_nMDT_M_e, m_h_nMDT_O_e, m_h_nMDT_InwardsTotal_e, m_h_nMDT_IM_e, m_h_nMDT_IO_e, m_h_nMDT_MO_e);
        std::unique_ptr<NHitsTH1> m_h_Nhits_RPC = std::make_unique<NHitsTH1>(m_h_nRPC, m_h_nRPC_I, m_h_nRPC_M, m_h_nRPC_O, m_h_nRPC_InwardsTotal, m_h_nRPC_IM, m_h_nRPC_IO, m_h_nRPC_MO);
        std::unique_ptr<NHitsTH1> m_h_Nhits_TGC = std::make_unique<NHitsTH1>(m_h_nTGC, m_h_nTGC_I, m_h_nTGC_M, m_h_nTGC_O, m_h_nTGC_InwardsTotal, m_h_nTGC_IM, m_h_nTGC_IO, m_h_nTGC_MO);
        
        std::unique_ptr<EffInputTH1> m_h_Truth = std::make_unique<EffInputTH1>(m_h_Truth_Lxy_b,  m_h_Truth_r_b, m_h_Truth_z_e,  m_h_Truth_r_e, m_h_Truth_eta);
        std::unique_ptr<EffInputTH1> m_h_TruthRecoMatched = std::make_unique<EffInputTH1>(m_h_TruthReco_Lxy_b,  m_h_TruthReco_r_b, m_h_TruthReco_z_e,  m_h_TruthReco_r_e, m_h_TruthReco_eta);
        std::unique_ptr<EffInputTH1> m_h_Reco = std::make_unique<EffInputTH1>(m_h_Reco_Lxy_b,  m_h_Reco_r_b, m_h_Reco_z_e,  m_h_Reco_r_e, m_h_Reco_eta);
        std::unique_ptr<EffInputTH1> m_h_RecoTruthMatched = std::make_unique<EffInputTH1>(m_h_RecoTruth_Lxy_b,  m_h_RecoTruth_r_b, m_h_RecoTruth_z_e,  m_h_RecoTruth_r_e, m_h_RecoTruth_eta);
};

#endif // MSVTXPLOTMAKER_H
