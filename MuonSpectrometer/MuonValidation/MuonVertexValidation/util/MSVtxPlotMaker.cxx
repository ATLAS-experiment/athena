/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Utils.h"
#include "PlotAnnotations.h"
#include "MSVtxPlotMaker.h"


using namespace MuonVertexValidationMacroUtils;
using namespace MuonVertexValidationMacroPlotAnnotations;

MSVtxPlotMaker::MSVtxPlotMaker(const std::string &datapath, const std::string &pltdir, const std::string &treename) :
                m_datapath(datapath), m_treename(treename),
                m_plotdir(pltdir), 
                m_plotdir_truthVtx(m_plotdir+"truthVtx/"), 
                m_plotdir_recoVtx(m_plotdir+"recoVtx/"), 
                m_plotdir_recoVtxHits(m_plotdir_recoVtx+"hits/"), 
                m_plotdir_vtxResiduals(m_plotdir+"vtxResiduals/"), 
                m_plotdir_inputObjects(m_plotdir+"inputObjects/"),
                m_plotdir_vtxEfficiency(m_plotdir+"vtxEfficiency/"),
                m_plotdir_vtxFakeRate(m_plotdir+"vtxFakeRate/")
                {}


MSVtxPlotMaker::~MSVtxPlotMaker() = default;


// --- core functions --- //

void MSVtxPlotMaker::makePlots(){
    // Generates a series of plots from the input data (m_datapath and m_treename) and stores these at m_plotdir
    // as a figure and in a root file. 
    setup();
    setPlotStyle();
    formatPlots();
    fillPlots();
    outputResults();
}


void MSVtxPlotMaker::setup(){
    // set up input file and its tree as well as the output root file.
    m_input_file = std::make_unique<TFile>(m_datapath.c_str(), "read");
    m_tree = (TTree*)m_input_file->Get(m_treename.c_str());
    setupBranches();

    gSystem->mkdir(m_plotdir, kTRUE);
    gSystem->mkdir(m_plotdir_truthVtx, kTRUE);
    gSystem->mkdir(m_plotdir_recoVtx, kTRUE);
    gSystem->mkdir(m_plotdir_recoVtxHits, kTRUE);
    gSystem->mkdir(m_plotdir_vtxResiduals, kTRUE);
    gSystem->mkdir(m_plotdir_inputObjects, kTRUE);
    gSystem->mkdir(m_plotdir_vtxEfficiency, kTRUE);
    gSystem->mkdir(m_plotdir_vtxFakeRate, kTRUE);
    m_output_file = std::make_unique<TFile>(m_plotdir+"Histograms.root", "recreate");

    return;
}


void MSVtxPlotMaker::setPlotStyle(){
    // set the plot style and the color palette
    gROOT->SetStyle("ATLAS");
    TStyle* plotStyle = gROOT->GetStyle("ATLAS");
    plotStyle->SetOptTitle(0);
    plotStyle->SetHistLineWidth(1.);

    setColorPalette(plotStyle);
    plotStyle->cd();
    gROOT->ForceStyle();
        
    return;
}


void MSVtxPlotMaker::formatPlots(){
    // Call formating routines for the plots
    formatTGraphs();

    return;
}


void MSVtxPlotMaker::fillPlots(){
    // main loop over the events which fills the plots

    for(int i=0; i<m_tree->GetEntries(); ++i){
        m_tree->GetEntry(i);

        const std::vector<Amg::Vector3D> truth_vertices = getVertexPos(*m_truthVtx_x, *m_truthVtx_y, *m_truthVtx_z);
        const std::vector<Amg::Vector3D> reco_vertices = getVertexPos(*m_msVtx_x, *m_msVtx_y, *m_msVtx_z);
        const std::vector<std::vector<Amg::Vector3D>> reco_constituentPos = getConstituentPos(reco_vertices.size(), *m_obj_vtxLink,
                                                                                              *m_obj_x, *m_obj_y, *m_obj_z);

        fillTruthVtxPlots(truth_vertices);
        fillReconstructionObjectsHists();
        fillRecoVtxPlots(reco_vertices, reco_constituentPos);
        fillTruthComparisonHists(reco_vertices, truth_vertices);

        // histograms for vertex efficiency: fraction of truth vertices that are reconstructed  
        fillEfficiency_NumeratorDenominatorHists(truth_vertices, reco_vertices, m_h_Truth, m_h_TruthRecoMatched);
        // histograms for vertex purity: fraction of reconstructed vertices that are truth vertices 
        fillEfficiency_NumeratorDenominatorHists(reco_vertices, truth_vertices, m_h_Reco, m_h_RecoTruthMatched);
    }

    return;
}


void MSVtxPlotMaker::outputResults(){
    // draw, save, write the plots
    m_c = std::make_unique<TCanvas>();
    m_output_file->cd();


    // truth vertices
    saveTH1(m_h_Nvtx_truth, m_plotdir_truthVtx+"Nvtx_truth");
    saveTH1(m_h_Nvtx_truth_b, m_plotdir_truthVtx+"Nvtx_truth_b");
    saveTH1(m_h_Nvtx_truth_e, m_plotdir_truthVtx+"Nvtx_truth_e");
    // position
    saveTGraph(m_zLxy_truth, m_etaphi_truth, m_h_VtxPos_truth, m_plotdir_truthVtx);
    saveVtxPos(m_h_VtxPosHists_truth, m_plotdir_truthVtx);


    // reconstructed vertices
    saveTH1(m_h_Nvtx, m_plotdir_recoVtx+"Nvtx");
    saveTH1(m_h_Nvtx_b, m_plotdir_recoVtx+"Nvtx_b");
    saveTH1(m_h_Nvtx_e, m_plotdir_recoVtx+"Nvtx_e");
    // position
    saveTGraph(m_zLxy, m_etaphi, m_h_VtxPos, m_plotdir_recoVtx);
    saveVtxPos(m_h_VtxPosHists, m_plotdir_recoVtx);
    // chi squared 
    saveTH1(m_h_chi2_b, m_plotdir_recoVtx+"chi2_b");
    saveTH1(m_h_chi2_e, m_plotdir_recoVtx+"chi2_e");
    saveTH1(m_h_chi2nDoF_b, m_plotdir_recoVtx+"chi2nDoF_b");
    saveTH1(m_h_chi2nDoF_e, m_plotdir_recoVtx+"chi2nDoF_e");
    saveTH1(m_h_chi2prob_b, m_plotdir_recoVtx+"chi2_prob_b");
    saveTH1(m_h_chi2prob_e, m_plotdir_recoVtx+"chi2_prob_e");
    // constituents
    saveTH1(m_h_Nconsti_b, m_plotdir_recoVtx+"Nconstituents_b");
    saveTH1(m_h_Nconsti_e, m_plotdir_recoVtx+"Nconstituents_e");
    saveTH1(m_h_VtxConsti_dR_b, m_plotdir_recoVtx+"VtxConsti_dR_b");
    saveTH1(m_h_VtxConsti_dRmax_b, m_plotdir_recoVtx+"VtxConsti_dRmax_b");
    saveTH1(m_h_VtxConsti_dphimax_b, m_plotdir_recoVtx+"VtxConsti_dphimax_b");
    saveTH1(m_h_VtxConsti_detamax_b, m_plotdir_recoVtx+"VtxConsti_detamax_b");
    saveTH1(m_h_VtxConsti_dR_e, m_plotdir_recoVtx+"VtxConsti_dR_e");
    saveTH1(m_h_VtxConsti_dRmax_e, m_plotdir_recoVtx+"VtxConsti_dRmax_e");
    saveTH1(m_h_VtxConsti_dphimax_e, m_plotdir_recoVtx+"VtxConsti_dphimax_e");
    saveTH1(m_h_VtxConsti_detamax_e, m_plotdir_recoVtx+"VtxConsti_detamax_e");
    // MDT hits
    saveTH1(m_h_nMDT_b, m_plotdir_recoVtxHits+"nMDT_b");
    saveTH1(m_h_nMDT_e, m_plotdir_recoVtxHits+"nMDT_e");
    saveTH1(m_h_nMDT_I_b, m_plotdir_recoVtxHits+"nMDT_I_b");
    saveTH1(m_h_nMDT_I_e, m_plotdir_recoVtxHits+"nMDT_I_e");
    saveTH1(m_h_nMDT_M_b, m_plotdir_recoVtxHits+"nMDT_M_b");
    saveTH1(m_h_nMDT_M_e, m_plotdir_recoVtxHits+"nMDT_M_e");
    saveTH1(m_h_nMDT_O_b, m_plotdir_recoVtxHits+"nMDT_O_b");
    saveTH1(m_h_nMDT_O_e, m_plotdir_recoVtxHits+"nMDT_O_e");
    saveTH1(m_h_nMDT_InwardsTotal_b, m_plotdir_recoVtxHits+"nMDT_InwardsTotal_b");
    saveTH1(m_h_nMDT_InwardsTotal_e, m_plotdir_recoVtxHits+"nMDT_InwardsTotal_e");
    saveTH1(m_h_nMDT_IM_b, m_plotdir_recoVtxHits+"nMDT_IM_b");
    saveTH1(m_h_nMDT_IM_e, m_plotdir_recoVtxHits+"nMDT_IM_e");
    saveTH1(m_h_nMDT_IO_b, m_plotdir_recoVtxHits+"nMDT_IO_b");
    saveTH1(m_h_nMDT_IO_e, m_plotdir_recoVtxHits+"nMDT_IO_e");
    saveTH1(m_h_nMDT_MO_b, m_plotdir_recoVtxHits+"nMDT_MO_b");
    saveTH1(m_h_nMDT_MO_e, m_plotdir_recoVtxHits+"nMDT_MO_e");
    // RPC hits
    saveTH1(m_h_nRPC, m_plotdir_recoVtxHits+"nRPC");
    saveTH1(m_h_nRPC_I, m_plotdir_recoVtxHits+"nRPC_I");
    saveTH1(m_h_nRPC_M, m_plotdir_recoVtxHits+"nRPC_M");
    saveTH1(m_h_nRPC_O, m_plotdir_recoVtxHits+"nRPC_O");
    saveTH1(m_h_nRPC_InwardsTotal, m_plotdir_recoVtxHits+"nRPC_InwardsTotal");
    saveTH1(m_h_nRPC_IM, m_plotdir_recoVtxHits+"nRPC_IM");
    saveTH1(m_h_nRPC_IO, m_plotdir_recoVtxHits+"nRPC_IO");
    saveTH1(m_h_nRPC_MO, m_plotdir_recoVtxHits+"nRPC_MO");
    // TGC hits
    saveTH1(m_h_nTGC, m_plotdir_recoVtxHits+"nTGC");
    saveTH1(m_h_nTGC_I, m_plotdir_recoVtxHits+"nTGC_I");
    saveTH1(m_h_nTGC_M, m_plotdir_recoVtxHits+"nTGC_M");
    saveTH1(m_h_nTGC_O, m_plotdir_recoVtxHits+"nTGC_O");
    saveTH1(m_h_nTGC_InwardsTotal, m_plotdir_recoVtxHits+"nTGC_InwardsTotal");
    saveTH1(m_h_nTGC_IM, m_plotdir_recoVtxHits+"nTGC_IM");
    saveTH1(m_h_nTGC_IO, m_plotdir_recoVtxHits+"nTGC_IO");
    saveTH1(m_h_nTGC_MO, m_plotdir_recoVtxHits+"nTGC_MO");


    // input objects for the vertex reconstruction
    // use counts from eta and phi histograms to fill the total number of objects available and used for reconstruction
    saveTHStack(m_h_NobjReco, m_h_Nobj, 
                TString::Format("#splitline{Used in}{reconstruction: %d}", int(m_h_objReco_eta->GetEntries())), TString::Format("Total: %d", int(m_h_obj_eta->GetEntries())),
                TString("objN_stack; Number of objects; Events / bin"), m_plotdir_inputObjects+"objN_stack");
    saveTHStack(m_h_NobjReco_b, m_h_Nobj_b, 
                TString::Format("#splitline{Used in}{reconstruction: %d}", int(m_h_objReco_phi_b->GetEntries())), TString::Format("Total: %d", int(m_h_obj_phi_b->GetEntries())),
                TString("objN_stack_b; Number of objects in the barrel; Events / bin"), m_plotdir_inputObjects+"objN_stack_b");
    saveTHStack(m_h_NobjReco_e, m_h_Nobj_e, 
                TString::Format("#splitline{Used in}{reconstruction: %d}", int(m_h_objReco_phi_e->GetEntries())), TString::Format("Total: %d", int(m_h_obj_phi_e->GetEntries())),
                TString("objN_stack_e; Number of objects in the endcaps; Events / bin"), m_plotdir_inputObjects+"objN_stack_e");
    saveTHStack(m_h_objReco_eta, m_h_obj_eta, 
                TString::Format("#splitline{Used in}{reconstruction: %d}", int(m_h_objReco_eta->GetEntries())), TString::Format("Total: %d", int(m_h_obj_eta->GetEntries())),
                TString("objEta_stack; Object #kern[-0.425]{#eta }; Events / bin"), m_plotdir_inputObjects+"objEta_stack");
    saveTHStack(m_h_objReco_phi_b, m_h_obj_phi_b, 
                TString::Format("#splitline{Used in}{reconstruction: %d}", int(m_h_objReco_phi_b->GetEntries())), TString::Format("Total: %d", int(m_h_obj_phi_b->GetEntries())),
                TString("objPhi_stack_b; Object #kern[-0.09]{#phi in the barrel}; Events / bin"), m_plotdir_inputObjects+"objPhi_stack_b");
    saveTHStack(m_h_objReco_phi_e, m_h_obj_phi_e, 
                TString::Format("#splitline{Used in}{reconstruction: %d}", int(m_h_objReco_phi_e->GetEntries())), TString::Format("Total: %d", int(m_h_obj_phi_e->GetEntries())),
                TString("objPhi_stack_e; Object #kern[-0.09]{#phi in the endcaps}; Events / bin"), m_plotdir_inputObjects+"objPhi_stack_e");
    // objects used for reconstruction
    saveTH1(m_h_NobjReco, m_plotdir_inputObjects+"objN_reco");
    saveTH1(m_h_NobjReco_b, m_plotdir_inputObjects+"objN_reco_b");
    saveTH1(m_h_NobjReco_e, m_plotdir_inputObjects+"objN_reco_e");
    saveTH1(m_h_objReco_eta, m_plotdir_inputObjects+"objEta_reco");
    saveTH1(m_h_objReco_phi_b, m_plotdir_inputObjects+"objPhi_reco_b");
    saveTH1(m_h_objReco_phi_e, m_plotdir_inputObjects+"objPhi_reco_e");


    // vertex residuals
    // barrel
    saveTH1(m_h_delta_R_b, m_plotdir_vtxResiduals+"delta_R_b");
    saveTH1(m_h_delta_theta_b, m_plotdir_vtxResiduals+"delta_theta_b");
    saveTH1(m_h_delta_phi_b, m_plotdir_vtxResiduals+"delta_phi_b");
    saveTH1(m_h_delta_Lxy_b, m_plotdir_vtxResiduals+"delta_Lxy_b");
    saveTH1(m_h_delta_z_b, m_plotdir_vtxResiduals+"delta_z_b");
    saveTH1(m_h_delta_phys_b, m_plotdir_vtxResiduals+"delta_phys_b");
    // endcaps
    saveTH1(m_h_delta_R_e, m_plotdir_vtxResiduals+"delta_R_e");
    saveTH1(m_h_delta_theta_e, m_plotdir_vtxResiduals+"delta_theta_e");
    saveTH1(m_h_delta_phi_e, m_plotdir_vtxResiduals+"delta_phi_e");
    saveTH1(m_h_delta_Lxy_e, m_plotdir_vtxResiduals+"delta_Lxy_e");
    saveTH1(m_h_delta_z_e, m_plotdir_vtxResiduals+"delta_z_e");
    saveTH1(m_h_delta_phys_e, m_plotdir_vtxResiduals+"delta_phys_e");
    // split into positive and negative eta
    saveTH1(m_h_delta_Lxy_posEta_b, m_plotdir_vtxResiduals+"delta_Lxy_posEta_b", TString::Format("0 < #kern[-0.45]{#eta <} %.2f", fidVol_barrel_etaCut).Data());
    saveTH1(m_h_delta_Lxy_negEta_b, m_plotdir_vtxResiduals+"delta_Lxy_negEta_b", TString::Format("-%.2f < #kern[-0.45]{#eta <} 0", fidVol_barrel_etaCut).Data());
    saveTH1(m_h_delta_Lxy_posEta_e, m_plotdir_vtxResiduals+"delta_Lxy_posEta_e", TString::Format("%.2f < #kern[-0.45]{#eta <} %.2f", fidVol_endcaps_etaCut_low, fidVol_endcaps_etaCut_up).Data());
    saveTH1(m_h_delta_Lxy_negEta_e, m_plotdir_vtxResiduals+"delta_Lxy_negEta_e", TString::Format("-%.2f < #kern[-0.45]{#eta <} -%.2f", fidVol_endcaps_etaCut_up, fidVol_endcaps_etaCut_low).Data());
    saveTH1(m_h_delta_z_posEta_b, m_plotdir_vtxResiduals+"delta_z_posEta_b", TString::Format("0 < #kern[-0.45]{#eta <} %.2f", fidVol_barrel_etaCut).Data());
    saveTH1(m_h_delta_z_negEta_b, m_plotdir_vtxResiduals+"delta_z_negEta_b", TString::Format("-%.2f < #kern[-0.45]{#eta <} 0", fidVol_barrel_etaCut).Data());
    saveTH1(m_h_delta_z_posEta_e, m_plotdir_vtxResiduals+"delta_z_posEta_e", TString::Format("%.2f < #kern[-0.45]{#eta <} %.2f", fidVol_endcaps_etaCut_low, fidVol_endcaps_etaCut_up).Data());
    saveTH1(m_h_delta_z_negEta_e, m_plotdir_vtxResiduals+"delta_z_negEta_e", TString::Format("-%.2f < #kern[-0.45]{#eta <} -%.2f", fidVol_endcaps_etaCut_up, fidVol_endcaps_etaCut_low).Data());


    // vertex efficiency
    // transverse distance: barrel
    saveTHStack(m_h_TruthReco_Lxy_b, m_h_Truth_Lxy_b,
                TString::Format("#splitline{Truth vertices}{matched to reco: %d}", int(m_h_TruthReco_Lxy_b->GetEntries())), TString::Format("Truth vertices: %d", int(m_h_Truth_Lxy_b->GetEntries())),
                TString("stack_vtxeff_Lxy_b; Truth vertex L_{xy} [m]; Vertices / bin"), m_plotdir_vtxEfficiency+"num_denom_vtxeff_Lxy_b");
    saveTEfficiency(m_h_TruthReco_Lxy_b, m_h_Truth_Lxy_b,
                    TString("vtxeff_Lxy_b; Truth vertex L_{xy} [m]; Efficiency: n_{truth}^{reco matched} / n_{truth}"), m_plotdir_vtxEfficiency+"vtxeff_Lxy_b");

    // longitudinal distance: endcap
    saveTHStack(m_h_TruthReco_z_e, m_h_Truth_z_e,
                TString::Format("#splitline{Truth vertices}{matched to reco: %d}", int(m_h_TruthReco_z_e->GetEntries())), TString::Format("Truth vertices: %d", int(m_h_Truth_z_e->GetEntries())),
                TString("stack_vtxeff_z_e; Truth vertex |z| [m]; Vertices / bin"), m_plotdir_vtxEfficiency+"num_denom_vtxeff_z_e");
    saveTEfficiency(m_h_TruthReco_z_e, m_h_Truth_z_e,
                    TString("vtxeff_z_e; Truth vertex |z| [m]; Efficiency: n_{truth}^{reco matched} / n_{truth}"), m_plotdir_vtxEfficiency+"vtxeff_z_e");

    // eta-binned
    saveTHStack(m_h_TruthReco_eta, m_h_Truth_eta,
                TString::Format("#splitline{Truth vertices}{matched to reco: %d}", int(m_h_TruthReco_eta->GetEntries())), TString::Format("Truth vertices: %d", int(m_h_Truth_eta->GetEntries())),
                TString("stack_vtxeff_eta; Truth vertex #kern[-0.425]{#eta }; Vertices / bin"), m_plotdir_vtxEfficiency+"num_denom_vtxeff_eta");
    saveTEfficiency(m_h_TruthReco_eta, m_h_Truth_eta,
                    TString("vtxeff_eta; Truth vertex #kern[-0.425]{#eta }; Efficiency: n_{truth}^{reco matched} / n_{truth}"), m_plotdir_vtxEfficiency+"vtxeff_eta");

    // distance form IP: barrel
    saveTHStack(m_h_TruthReco_r_b, m_h_Truth_r_b,
                TString::Format("#splitline{Truth vertices}{matched to reco: %d}", int(m_h_TruthReco_r_b->GetEntries())), TString::Format("Truth vertices: %d", int(m_h_Truth_r_b->GetEntries())),
                TString("stack_vtxeff_r_b; Truth vertex |#bf{r}| [m]; Vertices / bin"), m_plotdir_vtxEfficiency+"num_denom_vtxeff_r_b");
    saveTEfficiency(m_h_TruthReco_r_b, m_h_Truth_r_b,
                    TString("vtxeff_r_b; Truth vertex |#bf{r}| [m]; Efficiency: n_{truth}^{reco matched} / n_{truth}"), m_plotdir_vtxEfficiency+"vtxeff_r_b");
    
    // distance form IP: endcap
    saveTHStack(m_h_TruthReco_r_e, m_h_Truth_r_e,
                TString::Format("#splitline{Truth vertices}{matched to reco: %d}", int(m_h_TruthReco_r_e->GetEntries())), TString::Format("Truth vertices: %d", int(m_h_Truth_r_e->GetEntries())),
                TString("stack_vtxeff_r_e; Truth vertex |#bf{r}| [m]; Vertices / bin"), m_plotdir_vtxEfficiency+"num_denom_vtxeff_r_e");
    saveTEfficiency(m_h_TruthReco_r_e, m_h_Truth_r_e,
                    TString("vtxeff_r_e; Truth vertex |#bf{r}| [m]; Efficiency: n_{truth}^{reco matched} / n_{truth}"), m_plotdir_vtxEfficiency+"vtxeff_r_e");


    // vertex fakerate
    // transverse distance: barrel
    TH1 *h_RecoNoTruth_Lxy_b = getUnmatchedHist(m_h_Reco_Lxy_b, m_h_RecoTruth_Lxy_b, TString("RecoNoTruth_b"));
    saveTHStack(h_RecoNoTruth_Lxy_b, m_h_Reco_Lxy_b,
                TString::Format("#splitline{Reco vertices}{not matched to truth: %d}", int(h_RecoNoTruth_Lxy_b->GetEntries())), TString::Format("Reco vertices: %d", int(m_h_Reco_Lxy_b->GetEntries())),
                TString("stack_vtxfrate_Lxy_b; Reco vertex L_{xy} [m]; Vertices / bin"), m_plotdir_vtxFakeRate+"num_denom_vtxfrate_Lxy_b");
    saveTEfficiency(h_RecoNoTruth_Lxy_b, m_h_Reco_Lxy_b,
                    TString("vtxfrate_Lxy_b; Reco vertex L_{xy} [m]; Fake rate: n_{reco}^{not truth matched} / n_{reco}"), m_plotdir_vtxFakeRate+"vtxfrate_Lxy_b");
    // longitudinal distance: endcap
    TH1 *h_RecoNoTruth_z_e = getUnmatchedHist(m_h_Reco_z_e, m_h_RecoTruth_z_e, TString("RecoNoTruth_e"));
    saveTHStack(h_RecoNoTruth_z_e, m_h_Reco_z_e,
                TString::Format("#splitline{Reco vertices}{not matched to truth: %d}", int(h_RecoNoTruth_z_e->GetEntries())), TString::Format("Reco vertices: %d", int(m_h_Reco_z_e->GetEntries())),
                TString("stack_vtxfrate_z_e; Reco vertex |z| [m]; Vertices / bin"), m_plotdir_vtxFakeRate+"num_denom_vtxfrate_z_e");
    saveTEfficiency(h_RecoNoTruth_z_e, m_h_Reco_z_e,
                    TString("vtxfrate_z_e; Reco vertex |z| [m]; Fake rate: n_{reco}^{not truth matched} / n_{reco}"), m_plotdir_vtxFakeRate+"vtxfrate_z_e");

    // eta-binned
    TH1 *h_RecoNoTruth_eta = getUnmatchedHist(m_h_Reco_eta, m_h_RecoTruth_eta, TString("RecoNoTruth_eta"));
    saveTHStack(h_RecoNoTruth_eta, m_h_Reco_eta,
                TString::Format("#splitline{Reco vertices}{not matched to truth: %d}", int(h_RecoNoTruth_eta->GetEntries())), TString::Format("Reco vertices: %d", int(m_h_Reco_eta->GetEntries())),
                TString("stack_vtxfrate_eta; Reco vertex #kern[-0.425]{#eta }; Vertices / bin"), m_plotdir_vtxFakeRate+"num_denom_vtxfrate_eta");
    saveTEfficiency(h_RecoNoTruth_eta, m_h_Reco_eta,
                    TString("vtxfrate_eta; Reco vertex #kern[-0.425]{#eta }; Fake rate: n_{reco}^{not truth matched} / n_{reco}"), m_plotdir_vtxFakeRate+"vtxfrate_eta");

    // distance form IP: barrel
    TH1 *h_RecoNoTruth_r_b = getUnmatchedHist(m_h_Reco_r_b, m_h_RecoTruth_r_b, TString("RecoNoTruth_r_b"));
    saveTHStack(h_RecoNoTruth_r_b, m_h_Reco_r_b,
                TString::Format("#splitline{Reco vertices}{not matched to truth: %d}", int(h_RecoNoTruth_r_b->GetEntries())), TString::Format("Reco vertices: %d", int(m_h_Reco_r_b->GetEntries())),
                TString("stack_vtxfrate_r_b; Reco vertex |#bf{r}| [m]; Vertices / bin"), m_plotdir_vtxFakeRate+"num_denom_vtxfrate_r_b");
    saveTEfficiency(h_RecoNoTruth_r_b, m_h_Reco_r_b,
                    TString("vtxfrate_r_b; Reco vertex |#bf{r}| [m]; Fake rate: n_{reco}^{not truth matched} / n_{reco}"), m_plotdir_vtxFakeRate+"vtxfrate_r_b");
    
    // distance form IP: endcaps
    TH1 *h_RecoNoTruth_r_e = getUnmatchedHist(m_h_Reco_r_e, m_h_RecoTruth_r_e, TString("RecoNoTruth_r_e"));
    saveTHStack(h_RecoNoTruth_r_e, m_h_Reco_r_e,
                TString::Format("#splitline{Reco vertices}{not matched to truth: %d}", int(h_RecoNoTruth_r_e->GetEntries())), TString::Format("Reco vertices: %d", int(m_h_Reco_r_e->GetEntries())),
                TString("stack_vtxfrate_r_e; Reco vertex |#bf{r}| [m]; Vertices / bin"), m_plotdir_vtxFakeRate+"num_denom_vtxfrate_r_e");
    saveTEfficiency(h_RecoNoTruth_r_e, m_h_Reco_r_e,
                    TString("vtxfrate_r_e; Reco vertex |#bf{r}| [m]; Fake rate: n_{reco}^{not truth matched} / n_{reco}"), m_plotdir_vtxFakeRate+"vtxfrate_r_e");
    
    
    m_output_file->Write();
    m_output_file->Delete("*;2"); // delete all nameclye 2 objects 
    m_output_file->Close();

    return;
}



// --- setup branches and declare plots --- //

void MSVtxPlotMaker::setupBranches(){
    // truth vertex variables
    m_tree->SetBranchAddress("llpVtx_X", &m_truthVtx_x);
    m_tree->SetBranchAddress("llpVtx_Y", &m_truthVtx_y);
    m_tree->SetBranchAddress("llpVtx_Z", &m_truthVtx_z);

    // reconstructed vertex variables
    m_tree->SetBranchAddress("msVtx_chi2", &m_msVtx_chi2);
    m_tree->SetBranchAddress("msVtx_Ntrklet", &m_msVtx_Ntrklet);
    m_tree->SetBranchAddress("msVtx_X", &m_msVtx_x);
    m_tree->SetBranchAddress("msVtx_Y", &m_msVtx_y);
    m_tree->SetBranchAddress("msVtx_Z", &m_msVtx_z);
    // MDT hits near the vertex: total and per layer
    m_tree->SetBranchAddress("msVtx_nMDT", &m_msVtx_nMDT);
    m_tree->SetBranchAddress("msVtx_nMDT_inwards", &m_msVtx_nMDT_inwards);
    m_tree->SetBranchAddress("msVtx_nMDT_I", &m_msVtx_nMDT_I);
    m_tree->SetBranchAddress("msVtx_nMDT_M", &m_msVtx_nMDT_M);
    m_tree->SetBranchAddress("msVtx_nMDT_O", &m_msVtx_nMDT_O);
    // RPC hits near the vertex: total and per layer
    m_tree->SetBranchAddress("msVtx_nRPC", &m_msVtx_nRPC);
    m_tree->SetBranchAddress("msVtx_nRPC_inwards", &m_msVtx_nRPC_inwards);
    m_tree->SetBranchAddress("msVtx_nRPC_I", &m_msVtx_nRPC_I);
    m_tree->SetBranchAddress("msVtx_nRPC_M", &m_msVtx_nRPC_M);
    m_tree->SetBranchAddress("msVtx_nRPC_O", &m_msVtx_nRPC_O);
    // TGC hits near the vertex: total and per layer
    m_tree->SetBranchAddress("msVtx_nTGC", &m_msVtx_nTGC);
    m_tree->SetBranchAddress("msVtx_nTGC_inwards", &m_msVtx_nTGC_inwards);
    m_tree->SetBranchAddress("msVtx_nTGC_I", &m_msVtx_nTGC_I);
    m_tree->SetBranchAddress("msVtx_nTGC_M", &m_msVtx_nTGC_M);
    m_tree->SetBranchAddress("msVtx_nTGC_O", &m_msVtx_nTGC_O);

    // input objects for the vertex reconstruction
    m_tree->SetBranchAddress("trklet_posX", &m_obj_x);
    m_tree->SetBranchAddress("trklet_posY", &m_obj_y);
    m_tree->SetBranchAddress("trklet_posZ", &m_obj_z);
    m_tree->SetBranchAddress("trklet_phi", &m_obj_phi);
    m_tree->SetBranchAddress("trklet_theta", &m_obj_theta);
    m_tree->SetBranchAddress("trklet_eta", &m_obj_eta);
    m_tree->SetBranchAddress("trklet_vtxLink", &m_obj_vtxLink);

    return;
}


void MSVtxPlotMaker::formatTGraphs(){
    // Set the formating options for TGraph and TMultiGraph 
    
    // Lxy-z map of truth vertices
    m_zLxy_truth->SetTitle("zLxy_truth; Truth vertex z [m]; Truth vertex L_{xy} [m]");
    m_zLxy_truth_b->SetMarkerColor(1); m_zLxy_truth_b->SetMarkerStyle(20); m_zLxy_truth_b->SetMarkerSize(0.5);
    m_zLxy_truth_e->SetMarkerColor(2); m_zLxy_truth_e->SetMarkerStyle(20); m_zLxy_truth_e->SetMarkerSize(0.5);
    m_zLxy_truth_out->SetMarkerColor(14); m_zLxy_truth_out->SetMarkerStyle(24); m_zLxy_truth_out->SetMarkerSize(0.5);
    // eta-phi map of truth vertices
    m_etaphi_truth->SetTitle("etaphi_truth; Truth vertex #eta; Truth vertex #phi [rad]");
    m_etaphi_truth_b->SetMarkerColor(1); m_etaphi_truth_b->SetMarkerStyle(20); m_etaphi_truth_b->SetMarkerSize(0.5);
    m_etaphi_truth_e->SetMarkerColor(2); m_etaphi_truth_e->SetMarkerStyle(20); m_etaphi_truth_e->SetMarkerSize(0.5);
    m_etaphi_truth_out->SetMarkerColor(14); m_etaphi_truth_out->SetMarkerStyle(24); m_etaphi_truth_out->SetMarkerSize(0.5);

    // Lxy-z map of reconstructed vertices
    m_zLxy->SetTitle("zLxy; Reco vertex z [m]; Reco vertex L_{xy} [m]");
    m_zLxy_b->SetMarkerColor(1); m_zLxy_b->SetMarkerStyle(20); m_zLxy_b->SetMarkerSize(0.5);
    m_zLxy_e->SetMarkerColor(2); m_zLxy_e->SetMarkerStyle(20); m_zLxy_e->SetMarkerSize(0.5);
    m_zLxy_out->SetMarkerColor(14); m_zLxy_out->SetMarkerStyle(24); m_zLxy_out->SetMarkerSize(0.5);
    // eta-phi map of reconstructed vertices
    m_etaphi->SetTitle("etaphi; Reco vertex #eta; Reco vertex #phi [rad]");
    m_etaphi_b->SetMarkerColor(1); m_etaphi_b->SetMarkerStyle(20); m_etaphi_b->SetMarkerSize(0.5);
    m_etaphi_e->SetMarkerColor(2); m_etaphi_e->SetMarkerStyle(20); m_etaphi_e->SetMarkerSize(0.5);
    m_etaphi_out->SetMarkerColor(14); m_etaphi_out->SetMarkerStyle(24); m_etaphi_out->SetMarkerSize(0.5);

    return;
}


// --- main filling functions --- //

void MSVtxPlotMaker::fillTruthVtxPlots(const std::vector<Amg::Vector3D> &truth_vertices){
    
    fillNvtxHists(truth_vertices, m_h_NVtx_truth);
    fillVtxPosMaps(truth_vertices, m_h_VtxPos_truth);
    fillVtxPosHists(truth_vertices, m_h_VtxPosHists_truth);

    return;
}


void MSVtxPlotMaker::fillReconstructionObjectsHists(){
    // fill histograms for the objects from which vertices can be reconstructed

    int Nobj_count = m_obj_phi->size();
    int Nobj_count_b{0}, Nobj_count_e{0};
    int NobjReco_count{0};
    int NobjReco_count_b{0}, NobjReco_count_e{0};

    for (int i=0; i<Nobj_count; ++i){
        double obj_phi = (*m_obj_phi)[i];
        double obj_eta = (*m_obj_eta)[i];
        bool used_in_reco = (*m_obj_vtxLink)[i] != -1;
        m_h_obj_eta->Fill(obj_eta);
        if (used_in_reco){
            ++NobjReco_count;
            m_h_objReco_eta->Fill(obj_eta);
        }

        if (inBarrel(obj_eta)){
            ++Nobj_count_b;
            m_h_obj_phi_b->Fill(obj_phi);
            if (used_in_reco){
                ++NobjReco_count_b;
                m_h_objReco_phi_b->Fill(obj_phi);
            }
        }
        if (inEndcaps(obj_eta)){
            ++Nobj_count_e;
            m_h_obj_phi_e->Fill(obj_phi);
            if (used_in_reco){
                ++NobjReco_count_e;
                m_h_objReco_phi_e->Fill(obj_phi);
            }
        }
    }
    m_h_Nobj->Fill(Nobj_count);
    m_h_Nobj_b->Fill(Nobj_count_b);
    m_h_Nobj_e->Fill(Nobj_count_e);
    m_h_NobjReco->Fill(NobjReco_count);
    m_h_NobjReco_b->Fill(NobjReco_count_b);
    m_h_NobjReco_e->Fill(NobjReco_count_e);

    return;
}


void MSVtxPlotMaker::fillRecoVtxPlots(const std::vector<Amg::Vector3D> &reco_vertices, const std::vector<std::vector<Amg::Vector3D>> &reco_constituentPos){

    fillNvtxHists(reco_vertices, m_h_NVtx);
    fillVtxPosMaps(reco_vertices, m_h_VtxPos);
    fillVtxPosHists(reco_vertices, m_h_VtxPosHists);

    for (unsigned int j=0; j<reco_vertices.size(); ++j){
        if (inBarrel(reco_vertices[j])){
            m_h_Nconsti_b->Fill((*m_msVtx_Ntrklet)[j]);
            fillChi2Hists((*m_msVtx_chi2)[j], (*m_msVtx_Ntrklet)[j], m_h_VtxChi2_b);
            fillAngularVtxConstiHists(reco_vertices[j], reco_constituentPos[j], m_h_AngularVtxConsti_b);
            fillVtxNhitsHists((*m_msVtx_nMDT)[j], (*m_msVtx_nMDT_inwards)[j], (*m_msVtx_nMDT_I)[j], (*m_msVtx_nMDT_M)[j], (*m_msVtx_nMDT_O)[j], m_h_Nhits_MDT_b);
            fillVtxNhitsHists((*m_msVtx_nRPC)[j], (*m_msVtx_nRPC_inwards)[j], (*m_msVtx_nRPC_I)[j], (*m_msVtx_nRPC_M)[j], (*m_msVtx_nRPC_O)[j], m_h_Nhits_RPC);
        }
        if (inEndcaps(reco_vertices[j])){
            m_h_Nconsti_e->Fill((*m_msVtx_Ntrklet)[j]);
            fillChi2Hists((*m_msVtx_chi2)[j], (*m_msVtx_Ntrklet)[j], m_h_VtxChi2_e);
            fillAngularVtxConstiHists(reco_vertices[j], reco_constituentPos[j], m_h_AngularVtxConsti_e);
            fillVtxNhitsHists((*m_msVtx_nMDT)[j], (*m_msVtx_nMDT_inwards)[j], (*m_msVtx_nMDT_I)[j], (*m_msVtx_nMDT_M)[j], (*m_msVtx_nMDT_O)[j], m_h_Nhits_MDT_e);
            fillVtxNhitsHists((*m_msVtx_nTGC)[j], (*m_msVtx_nTGC_inwards)[j], (*m_msVtx_nTGC_I)[j], (*m_msVtx_nTGC_M)[j], (*m_msVtx_nTGC_O)[j], m_h_Nhits_TGC);
        }
    }

    return;
}


void MSVtxPlotMaker::fillTruthComparisonHists(const std::vector<Amg::Vector3D> &reco_vertices, const std::vector<Amg::Vector3D> &truth_vertices){
    // fills the histograms detailing the differences between reconstructed and truth vertices
    for (const Amg::Vector3D &vtx : reco_vertices){
        if (!inFiducialVol(vtx)) continue;
        Amg::Vector3D truth_vtx = findBestMatch(vtx, truth_vertices); 
        if (!isValidMatch(truth_vtx)) continue;

        double dR = xAOD::P4Helpers::deltaR(vtx.eta(), vtx.phi(), truth_vtx.eta(), truth_vtx.phi()); // angular distance
        double d_theta = vtx.theta() - truth_vtx.theta();
        double d_phi = Amg::deltaPhi(vtx, truth_vtx);
        double d_Lxy = (vtx.perp() - truth_vtx.perp())/Gaudi::Units::cm; // distance in transverse plane
        double d_z = (vtx.z() - truth_vtx.z())/Gaudi::Units::cm; // in cm
        double d_phys = Amg::distance(vtx, truth_vtx)/Gaudi::Units::cm; // distance to the IP in cm

        if (inFiducialVolBarrel(vtx)) fillResidualHists(truth_vtx.eta(), dR, d_theta, d_phi, d_Lxy, d_z, d_phys, m_h_VtxResiduals_b);
        if (inFiducialVolEndcaps(vtx)) fillResidualHists(truth_vtx.eta(), dR, d_theta, d_phi, d_Lxy, d_z, d_phys, m_h_VtxResiduals_e);
    }

    return;
}


void MSVtxPlotMaker::fillEfficiency_NumeratorDenominatorHists(const std::vector<Amg::Vector3D> &vertices, const std::vector<Amg::Vector3D> &match_candidates,
                                                              std::unique_ptr<EffInputTH1> &denomHists, std::unique_ptr<EffInputTH1> &numHists){
    // Fills the transverse distance (barrel vertices only), longitudinal distance (endcaps vertices only), eta-binned, and distance to the IP binned histograms
    // for vertices passing the good vertex selection. These are the denominator histograms for efficiency calculations. 
    // If additionally, the vertex can be macthed to an element in match_candidates, the numerator histograms are filled. 
    
    for(const Amg::Vector3D &vtx : vertices){
        if (!isGoodVtx(vtx)) continue;
        fillVtxPosFiducialVolHists(vtx, denomHists);
        if (!hasMatch(vtx, match_candidates)) continue;
        fillVtxPosFiducialVolHists(vtx, numHists);
    }

    return;
}


// --- helper filling functions --- //

void MSVtxPlotMaker::fillNvtxHists(const std::vector<Amg::Vector3D> &vertices, std::unique_ptr<NVtxTH1> &hists){
    // Fills the number of vertices in the whole detector region and split into barrel and endcaps.
    hists->h_Nvtx->Fill(getNvtxDetectorRegion(vertices));
    hists->h_Nvtx_b->Fill(getNvtxBarrel(vertices));
    hists->h_Nvtx_e->Fill(getNvtxEndcaps(vertices));

    return;
}


void MSVtxPlotMaker::fillVtxPosMaps(const std::vector<Amg::Vector3D> &vertices, std::unique_ptr<VtxPosTGraph> &graphs){
    // Fills the barrel, endcaps, or outside the detector region z-Lxy and eta-phi maps with the vertex position. 
    for (const Amg::Vector3D &vtx : vertices){
        if (inBarrel(vtx)){
            graphs->zLxy_b->SetPoint(graphs->zLxy_b->GetN(), vtx.z()/Gaudi::Units::m, vtx.perp()/Gaudi::Units::m);
            graphs->etaphi_b->SetPoint(graphs->etaphi_b->GetN(), vtx.eta(), vtx.phi());
        }
        else if (inEndcaps(vtx)){
            graphs->zLxy_e->SetPoint(graphs->zLxy_e->GetN(), vtx.z()/Gaudi::Units::m, vtx.perp()/Gaudi::Units::m);
            graphs->etaphi_e->SetPoint(graphs->etaphi_e->GetN(), vtx.eta(), vtx.phi());
        }
        else {
            graphs->zLxy_out->SetPoint(graphs->zLxy_out->GetN(), vtx.z()/Gaudi::Units::m, vtx.perp()/Gaudi::Units::m);
            graphs->etaphi_out->SetPoint(graphs->etaphi_out->GetN(), vtx.eta(), vtx.phi());
        }
    }
    
    return;
}


void MSVtxPlotMaker::fillVtxPosHists(const std::vector<Amg::Vector3D> &vertices, std::unique_ptr<VtxPosTH> &hists){ 
    // Fills the transverse distance, longitudinal distance, eta-binned, phi-binned and distance to the IP -binned histograms
    // Requires the vertex to be within the detector region (inside the barrel or endcaps).

    for (const Amg::Vector3D &vtx : vertices){
        if (!inDetectorRegion(vtx)) continue;
        hists->h_zLxy->Fill(vtx.z()/Gaudi::Units::m, vtx.perp()/Gaudi::Units::m);
        hists->h_etaphi->Fill(vtx.eta(), vtx.phi());
        hists->h_distanceToIP->Fill(vtx.mag()/Gaudi::Units::m);
    }

    return;
}


void MSVtxPlotMaker::fillChi2Hists(double chi2, double NDoF, std::unique_ptr<Chi2TH1> &hists){
    // Fills histograms related to the chi2 value of the vertex fit.
    hists->h_chi2->Fill(chi2);
    hists->h_chi2nDoF->Fill(chi2/NDoF);
    hists->h_chi2prob->Fill( TMath::Prob(chi2, NDoF-1));

    return;
}


void MSVtxPlotMaker::fillAngularVtxConstiHists(const Amg::Vector3D &vtx, const std::vector<Amg::Vector3D> &consti, std::unique_ptr<AngularVtxConstiTH1> &hists){
    // Fills histograms with the angular differences between the vertex and its constituents.

    if (consti.size() == 0) return; // no constituents

    double dphi{-1}, deta{-1}, deltaR{-1};
    double dphi_max{0}, deta_max{0}, dR_max{0};

    for (const Amg::Vector3D &c : consti){
        dphi = xAOD::P4Helpers::deltaPhi(vtx.phi(), c.phi());
        deta = vtx.eta() - c.eta();
        deltaR = xAOD::P4Helpers::deltaR(vtx.eta(), vtx.phi(), c.eta(), c.phi());
        hists->h_dR->Fill(deltaR);
        if (std::abs(dphi) > std::abs(dphi_max)) dphi_max = dphi;
        if (std::abs(deta) > std::abs(deta_max)) deta_max = deta;
        if (deltaR > dR_max) dR_max = deltaR;  
    }

    hists->h_dphimax->Fill(dphi_max);
    hists->h_detamax->Fill(deta_max);
    hists->h_dRmax->Fill(dR_max);

    return; 
}


void MSVtxPlotMaker::fillVtxNhitsHists(double total, double inwards, double inner, double middle, double outer, std::unique_ptr<NHitsTH1> &hists){
    // Fills histograms with the number of hits associated to the vertex, hits inwards of the vertex and the ratios between hits in different muon spectrometer layers.

    if (total < 0) return; // no hits

    hists->h_total->Fill(total);
    hists->h_I->Fill(inner);
    hists->h_M->Fill(middle);
    hists->h_O->Fill(outer);
    hists->h_inwardsTotal->Fill(inwards/total);
    if (middle > 0) hists->h_IM->Fill(inner/middle);
    if (outer > 0) hists->h_IO->Fill(inner/outer);
    if (outer > 0) hists->h_MO->Fill(middle/outer);

    return;
}


void MSVtxPlotMaker::fillResidualHists(double eta, double dR, double d_theta, double d_phi, double d_Lxy, double d_z, double d_phys, std::unique_ptr<ResidualTH1> &hists){
    // Fills histograms with the differences between the reconstructed and the matched truth vertex.
    // The value of eta can be used to split the histograms into positive and negative eta regions.
    hists->h_delta_R->Fill(dR);
    hists->h_delta_theta->Fill(d_theta);
    hists->h_delta_phi->Fill(d_phi);
    hists->h_delta_Lxy->Fill(d_Lxy);
    hists->h_delta_z->Fill(d_z);
    hists->h_delta_phys->Fill(d_phys);

    if (eta >= 0){
        hists->h_delta_Lxy_posEta->Fill(d_Lxy);
        hists->h_delta_z_posEta->Fill(d_z);
    }
    else {
        hists->h_delta_Lxy_negEta->Fill(d_Lxy);
        hists->h_delta_z_negEta->Fill(d_z);
    }

    return;
}


void MSVtxPlotMaker::fillVtxPosFiducialVolHists(const Amg::Vector3D &vtx, std::unique_ptr<EffInputTH1> &hists){
    // Fills the transverse distance (barrel vertices only), longitudinal distance (endcaps vertices only), 
    // eta-binned, and distance to the IP binned histograms.
    if (inFiducialVolBarrel(vtx)){
        hists->h_Lxy->Fill(vtx.perp()/Gaudi::Units::m);
        hists->h_distanceToIP_b->Fill(vtx.mag()/Gaudi::Units::m);
    }
    if (inFiducialVolEndcaps(vtx)){
        hists->h_z->Fill(std::abs(vtx.z()/Gaudi::Units::m));
        hists->h_distanceToIP_e->Fill(vtx.mag()/Gaudi::Units::m);
    } 
    hists->h_eta->Fill(vtx.eta());

    return;
}


// --- plots savers --- //

void MSVtxPlotMaker::saveVtxPos(std::unique_ptr<VtxPosTH> &hists, const TString &plotdir){
    // Saves the passed TH2s and TH1.
    saveTH2(hists->h_zLxy, plotdir+"zLxy");
    saveTH2(hists->h_etaphi, plotdir+"etaphi");

    // project and save the 1D histograms
    const TString extra =  plotdir.Contains("truth") ? TString("_truth") : TString("");

    saveTH1(hists->h_zLxy->ProjectionX("z"+extra), plotdir+"z"+extra, "", "Vertices / bin");
    saveTH1(hists->h_zLxy->ProjectionY("Lxy"+extra), plotdir+"Lxy"+extra, "", "Vertices / bin");
    saveTH1(hists->h_etaphi->ProjectionX("eta"+extra), plotdir+"eta"+extra, "", "Vertices / bin");
    TH1* h_phi = hists->h_etaphi->ProjectionY("phi"+extra);
    h_phi->GetXaxis()->SetRangeUser(-TMath::Pi(), TMath::Pi()); // adjust range
    saveTH1(h_phi, plotdir+"phi"+extra, "", "Vertices / bin");
    saveTH1(hists->h_distanceToIP, plotdir+"distanceToIP");

    return;
}


void MSVtxPlotMaker::saveTGraph(TMultiGraph* zLxy, TMultiGraph* etaphi, std::unique_ptr<VtxPosTGraph> &graphs, const TString &plotdir){
    // Combines the TGraph objects to a the TMultiGraph and save it.
    zLxy->Add(graphs->zLxy_b, "P");
    zLxy->Add(graphs->zLxy_e, "P");
    zLxy->Add(graphs->zLxy_out, "P");
    zLxy->GetXaxis()->SetRangeUser(-16,16);
    zLxy->GetYaxis()->SetRangeUser(-1,12);
    zLxy->Draw("A");
    drawATLASlabel("Simulation Internal");
    TLegend* legend_zLxy = makeLegend(0.16, 0.02, 0.65, 0.07, 0.05);
    legend_zLxy->SetNColumns(3);
    legend_zLxy->AddEntry(graphs->zLxy_b, "Barrel", "p");
    legend_zLxy->AddEntry(graphs->zLxy_e, "Endcaps", "p");
    legend_zLxy->AddEntry(graphs->zLxy_out, "Outside", "p");
    legend_zLxy->Draw();
    m_c->SaveAs(plotdir+"map_zLxy.pdf");
    zLxy->Write();

    etaphi->Add(graphs->etaphi_b, "P");
    etaphi->Add(graphs->etaphi_e, "P");
    etaphi->Add(graphs->etaphi_out, "P");
    etaphi->GetXaxis()->SetRangeUser(-2.6,2.6);
    etaphi->GetYaxis()->SetRangeUser(-TMath::Pi()-0.25,TMath::Pi()+1);
    etaphi->Draw("A");
    drawATLASlabel("Simulation Internal");
    TLegend* legend_etaphi = makeLegend(0.16, 0.02, 0.65, 0.07, 0.05); 
    legend_etaphi->SetNColumns(3);
    legend_etaphi->AddEntry(graphs->etaphi_b, "Barrel", "p");
    legend_etaphi->AddEntry(graphs->etaphi_e, "Endcaps", "p");
    legend_etaphi->AddEntry(graphs->etaphi_out, "Outside", "p");
    legend_etaphi->Draw();
    m_c->SaveAs(plotdir+"map_etaphi.pdf");
    etaphi->Write();

    return;
}


void MSVtxPlotMaker::saveTH1(TH1* h, TString plotpath, const char* dectectorLabel, const char* new_ylabel, bool norm, bool logy){
    // Saves the histogram with a detector label annotation and optionally normalizes (giving the frequency probability for each bin) it. 
    // Also allows to set a new y axis label and plot with a log y scale
    // If normalisation or log scaling is applied, "_norm" or "_log" are appended to the histogram and file name

    double maxy_factor = 1.4; // assures sufficeint space of annotations

    if (norm){
        h->Scale(1. / h->Integral()); // Add the argument "width" to normalize by bin-count*width to get the approximate pdf 
        h->SetName(TString(h->GetName())+"_norm");
        plotpath = plotpath.Insert(plotpath.Last(*"."),"_norm");
    }

    if (logy){
        gPad->SetLogy();
        h->SetName(TString(h->GetName())+"_log");
        plotpath = plotpath.Insert(plotpath.Last(*"."),"_log");
        maxy_factor = 4;
    }

    if (new_ylabel){
        h->GetYaxis()->SetTitle(new_ylabel);
    }

    h->SetMaximum(h->GetMaximum()*maxy_factor);
    h->Draw("hist");
    drawDetectorRegionLabel(h->GetName(), dectectorLabel);
    drawATLASlabel("Simulation Internal");
    m_c->SaveAs(plotpath+".pdf");
    h->Write();

    // reset log scale for next plots
    gPad->SetLogy(0);

    return;
}


void MSVtxPlotMaker::saveTHStack(TH1* h1, TH1* h2, 
                                 const TString &h1_legend, const TString &h2_legend,
                                 const TString &title, const TString &plotpath,
                                 int color1, int color2){
    // save the passed TH1 as a THStack. 
    // The THStack title is required to have the format "name; x-axis label; y-axis label"  
    TObjArray *tx_stack = title.Tokenize(";");
    auto name = ((TObjString*)(tx_stack->At(0)))->String();
    THStack *h_stack = new THStack(name, name);
    h1->SetLineColor(color1);
    h2->SetLineColor(color2);
    h_stack->Add(h1);
    h_stack->Add(h2);
    // adjust maximum for sufficent space for annotations and draw
    double maxy = h2->GetMaximum()>h1->GetMaximum() ? h2->GetMaximum() : h1->GetMaximum();
    h_stack->SetMaximum(1.4*maxy);
    h_stack->Draw("nostack");
    h_stack->GetXaxis()->SetTitle(((TObjString*)(tx_stack->At(1)))->String());
    h_stack->GetYaxis()->SetTitle(((TObjString*)(tx_stack->At(2)))->String());
    // add annotations
    TLegend* legend = makeLegend();
    legend->AddEntry(h1, h1_legend, "l");
    legend->AddEntry(h2, h2_legend, "l");
    legend->Draw();
    drawDetectorRegionLabel(name);
    drawATLASlabel("Simulation Internal");

    m_c->SaveAs(plotpath+".pdf");
    h_stack->Write();
    
    return;
}


void MSVtxPlotMaker::saveTEfficiency(TH1* h_num, TH1* h_denom,
                                     const TString &title, const TString &plotpath){
    // computes a TEfficiency object from the passed numerator and denominator histogram and saves it

    TEfficiency* h_eff = new TEfficiency(*h_num, *h_denom);
    h_eff->SetTitle(title);
    TObjArray *tx_eff = title.Tokenize(";");
    h_eff->SetName(((TObjString*)(tx_eff->At(0)))->String());
    // set drawing style and draw
    h_eff->SetLineColor(1);
    h_eff->SetMarkerColor(1);
    h_eff->SetMarkerSize(0.5);
    h_eff->SetMarkerStyle(24);
    h_eff->Draw(); 
    gPad->Update();
    // adjust maximum for sufficient space for annotations and draw annotations
    TGraphAsymmErrors* g_eff = h_eff->GetPaintedGraph(); 
    double maxy = getMaxy(g_eff);
    g_eff->GetYaxis()->SetRangeUser(0, 1.4*maxy);
    g_eff->GetXaxis()->SetTitle(((TObjString*)(tx_eff->At(1)))->String());
    g_eff->GetYaxis()->SetTitle(((TObjString*)(tx_eff->At(2)))->String());
    // annotations
    if (TString(h_eff->GetName()).Contains("_Lxy_")) drawDetectorBoundaryLines("Lxy", 1.1*maxy);
    if (TString(h_eff->GetName()).Contains("_z_")) drawDetectorBoundaryLines("z", 1.1*maxy);
    drawDetectorRegionLabel(h_eff->GetName());
    drawATLASlabel("Simulation Internal");
    gPad->Update();

    m_c->SaveAs(plotpath+".pdf");
    h_eff->Write();

    return;
}


void MSVtxPlotMaker::saveTH2(TH2* h, const TString &plotpath){
    // adjust margins to fit the palette on the pad
    gPad->SetRightMargin(0.15);
    h->Draw("colz");
    drawATLASlabel("Simulation Internal");   
    m_c->SaveAs(plotpath+".pdf");
    gPad->SetRightMargin(0.05); // reset margin for next plots

    return;
}


// --- other helper functions --- //


void MSVtxPlotMaker::setColorPalette(TStyle *plotStyle){
    // define a custom colour palette
    Int_t palette[100];
    // define colours and compute the gradient between them. 
    // the i-th entry in the following arrays defines the rgb values
    Double_t Red[] = {0.0, 0.0, 1.0, 1.0};
    Double_t Green[] = {1.0, 1.0, 1.0, 0.0};
    Double_t Blue[] = {1.0, 0.0, 0.0, 0.0};
    Double_t Length[] = {0.0, 0.33, 0.66, 1.0};
    Int_t firstColorIdx = TColor::CreateGradientColorTable(std::size(Length), Length, Red, Green, Blue, std::size(palette)); 
    // fill the pallette with the gradient
    for (unsigned int i=0; i<std::size(palette); i++) palette[i] = firstColorIdx+i;

    plotStyle->SetPalette(std::size(palette), palette);
    // plotStyle->SetPalette(kBlueRedYellow); // more standard color palette
    plotStyle->SetNumberContours(100);

    return;
}


TH1* MSVtxPlotMaker::getUnmatchedHist(TH1* h_all, TH1* h_matched, const TString &name_unmatched){
    // takes the difference between the all vertices and the matched vertices to get the unmatched vertices
    TH1* h_unmatched = (TH1*)h_all->Clone(name_unmatched);
    h_unmatched->Add(h_matched, -1);
    return h_unmatched;
}
