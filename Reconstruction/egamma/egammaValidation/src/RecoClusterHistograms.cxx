/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoClusterHistograms.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloClusterCellLink.h"
#include "CaloEvent/CaloCell.h"

#include "TH3D.h"

using namespace egammaMonitoring;

StatusCode RecoClusterHistograms::initializePlots() {

  const char* fN = m_name.c_str();

  for (int il = 0; il < 4; il++) {
    TString hN = Form("hNcellsvseteta_Lr%i",il);
    m_histo3DMap[hN.Data()] = new TH3D(Form("%s_%s",fN,hN.Data()),"",20,0,200,25,0,2.5,60,0,60);
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+hN.Data(), m_histo3DMap[hN.Data()]));
    hN = Form("hEvseteta_Lr%i",il);
    m_histo3DMap[hN.Data()] = new TH3D(Form("%s_%s",fN,hN.Data()),"",20,0,200,25,0,2.5,50,0,200);
    ATH_CHECK(m_rootHistSvc->regHist(m_folder+hN.Data(), m_histo3DMap[hN.Data()]));
  }
  TString hN = "hNToposvseteta";
  m_histo3DMap[hN.Data()] = new TH3D(Form("%s_%s",fN,hN.Data()),"",20,0,200,25,0,2.5,10,0,10);
  ATH_CHECK(m_rootHistSvc->regHist(m_folder+hN.Data(), m_histo3DMap[hN.Data()]));

  return StatusCode::SUCCESS;
  
} // initializePlots

void RecoClusterHistograms::fill(const xAOD::Egamma& egamma) {

  const xAOD::CaloCluster *cluster = egamma.caloCluster();
  // This should be a real error...
  if (!cluster) {
    return;
  }
  
  const CaloClusterCellLink* cellLinks = cluster->getCellLinks();
  if (!cellLinks) {
    return;
  }

  double et = egamma.pt()*1e-3;
  double aeta = std::abs(egamma.eta());
  
  std::map<int, int > cells_per_layer;
  for (const CaloCell* cell : *cellLinks) {
    if (cell) {
      int layer = cell->caloDDE()->getLayer();
      cells_per_layer[layer]++;
    }
  }

  auto associatedTopoCluster = xAOD::EgammaHelpers::getAssociatedTopoClusters(cluster);
  m_histo3DMap["hNToposvseteta"]->Fill(et,aeta,associatedTopoCluster.size());

  for (int il = 0; il < 4; il++) {
    int nc = cells_per_layer.find(il) != cells_per_layer.end() ? cells_per_layer[il] : 0;
    m_histo3DMap[Form("hNcellsvseteta_Lr%i",il)]->Fill(et,aeta,nc);
    m_histo3DMap[Form("hEvseteta_Lr%i",il)]->Fill(et,aeta,cluster->energyBE(il)*1e-3);
  }
  
}
