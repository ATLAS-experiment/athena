/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ClusterHistograms.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloClusterCellLink.h"
#include "CaloEvent/CaloCell.h"

#include "TH2D.h"
#include "TProfile.h"

using namespace egammaMonitoring;

StatusCode ClusterHistograms::initializePlots() {

  const char* fN = m_name.c_str();
  
  for (int is = 0; is < 4; is++) {
    TString pNm = Form("number_cells_vs_eta_in_layer_%i_profile",is);
    TString pN  = Form("%s_number_cells_vs_eta_in_layer_%i_profile",fN,is);
    profileMap[pNm.Data()] = new TProfile(pN.Data(), "Number of cells;truth #eta", 90,-4.5,4.5, 0, 100);
    pN = Form("%s%s",m_folder.c_str(),pNm.Data());
    ATH_CHECK(m_rootHistSvc->regHist(pN.Data(), profileMap[pNm.Data()]));

    pNm = Form("number_cells_vs_e_in_layer_%i_profile",is);
    pN  = Form("%s_number_cells_vs_e_in_layer_%i_profile",fN,is);
    profileMap[pNm.Data()] = new TProfile(pN.Data(), "Number of cells;truth E [GeV]", 60, 0, 300, 0, 100);
    pN = Form("%s%s",m_folder.c_str(),pNm.Data());
    ATH_CHECK(m_rootHistSvc->regHist(pN.Data(), profileMap[pNm.Data()]));
  }

  // tempo, for checks
  for (int is = 0; is < 24; is++) {
    TString pNm = Form("number_cells_vs_eta_in_sampling_%i_profile",is);
    TString pN = Form("%s_number_cells_vs_eta_in_sampling_%i_profile",fN,is);
    profileMap[pNm.Data()] = new TProfile(pN.Data(), "Number of cells;truth #eta", 90,-4.5,4.5, 0,100);
    pN = Form("%s%s",m_folder.c_str(),pNm.Data());
    ATH_CHECK(m_rootHistSvc->regHist(pN.Data(), profileMap[pNm.Data()]));
  }

  profileMap["Eraw_Etruth_vs_Etruth_profile"] = new TProfile(Form("%s_Eraw_Etruth_vs_Etruth_profile",fN), ";E^{truth};E^{raw}/E^{truth}", 100, 0., 200., 0.5, 1.5);
  profileMap["Eraw_Etruth_vs_eta_profile"]    = new TProfile(Form("%s_Eraw_Etruth_vs_eta_profile",fN),    ";truth #eta;E^{raw}/E^{truth}", 90, -4.5, 4.5, 0.5, 1.5);

  profileMap["number_topocluster_vs_e_profile"]   = new TProfile(Form("%s_number_topocluster_vs_e_profile",fN),  "Number of topocluster;truth E [GeV]", 60, 0, 300, -0.5, 14.5);
  profileMap["number_topocluster_vs_eta_profile"] = new TProfile(Form("%s_number_topocluster_vs_eta_profile",fN), "Number of topocluster;truth #eta", 90,-4.5,4.5, -0.5, 14.5);

  // do not undertstand this histo...
  profileMap["number_cell_in_layer"]    = new TProfile(Form("%s_number_cell_in_layer",fN), ";Number of cells;Layer", 100, 0, 200,0, 4);
  histo2DMap["mu_energy_resolution_2D"] = new TH2D(Form("%s_mu_energy_resolution_2D",fN), ";<#mu>; Energy Resolution", 5, 0, 80, 20, -1, 1);

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"Eraw_Etruth_vs_Etruth_profile", profileMap["Eraw_Etruth_vs_Etruth_profile"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"Eraw_Etruth_vs_eta_profile", profileMap["Eraw_Etruth_vs_eta_profile"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"number_topocluster_vs_e_profile", profileMap["number_topocluster_vs_e_profile"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"number_topocluster_vs_eta_profile", profileMap["number_topocluster_vs_eta_profile"]));

  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"number_cell_in_layer", profileMap["number_cell_in_layer"]));
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+"mu_energy_resolution_2D", histo2DMap["mu_energy_resolution_2D"]));

  return StatusCode::SUCCESS;
  
} // initializePlots

void ClusterHistograms::fill(const xAOD::Egamma& egamma) {
  fill(egamma,0.);
}

void ClusterHistograms::fill(const xAOD::Egamma& egamma, float mu = 0) {

  const xAOD::CaloCluster *cluster = egamma.caloCluster();
  
  const xAOD::TruthParticle *truth_egamma = xAOD::TruthHelpers::getTruthParticle(egamma);

  if ( !truth_egamma ) return;

  const auto Ereco = cluster->rawE();
  const auto Etruth = truth_egamma->e();
  const auto Eres = (Ereco - Etruth)/Etruth;

  profileMap["Eraw_Etruth_vs_Etruth_profile"]->Fill(Etruth/1000,Ereco/Etruth);
  profileMap["Eraw_Etruth_vs_eta_profile"]->Fill(truth_egamma->eta(),Ereco/Etruth);
  histo2DMap["mu_energy_resolution_2D"]->Fill(mu,Eres);

  const CaloClusterCellLink* cellLinks = cluster->getCellLinks();
  
  if ( !cellLinks ) return;

  std::map<int, int > cells_per_layer;
  std::map<int, int > cells_per_sam;

  for (const CaloCell* cell : *cellLinks) {
    if (cell) {
      int layer = cell->caloDDE()->getLayer();
      cells_per_layer[layer]++;
      int sam = cell->caloDDE()->getSampling();
      /*
      std::cout << m_name << " sam = " << sam << " layer = " << layer
		<< " eta cell raw = " << cell->caloDDE()->eta_raw() //<< " eta part = " << truth_egamma->eta()
		<< " IW " << cell->caloDDE()->is_lar_em_endcap_inner() << " OW " << cell->caloDDE()->is_lar_em_endcap_outer() << std::endl;
      */
      cells_per_sam[sam]++;
    }
  }

  auto associatedTopoCluster = xAOD::EgammaHelpers::getAssociatedTopoClusters(cluster);
  profileMap["number_topocluster_vs_e_profile"]->Fill(Etruth/1000, associatedTopoCluster.size());
  profileMap["number_topocluster_vs_eta_profile"]->Fill(truth_egamma->eta(), associatedTopoCluster.size());

  for (auto const& x : cells_per_layer) {
   profileMap["number_cell_in_layer"]->Fill(x.second,x.first);
   profileMap["number_cells_vs_e_in_layer_"+std::to_string(x.first)+"_profile"]->Fill(Etruth/1000, x.second);
   profileMap["number_cells_vs_eta_in_layer_"+std::to_string(x.first)+"_profile"]->Fill(truth_egamma->eta(), x.second);
  }
  for (auto const& x : cells_per_sam) {
    profileMap[Form("number_cells_vs_eta_in_sampling_%i_profile",x.first)]->Fill(truth_egamma->eta(), x.second);
  }
  
}
