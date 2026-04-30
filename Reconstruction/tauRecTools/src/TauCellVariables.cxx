/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TauCellVariables.h"
#include "tauRecTools/HelperFunctions.h"

#include "CaloUtils/CaloVertexedCell.h"

#include <vector>


TauCellVariables::TauCellVariables(const std::string& name) :
  TauRecToolBase(name) {}

StatusCode TauCellVariables::execute(xAOD::TauJet& pTau) const {

  double sumCellET = 0.;
  double sumCellET01 = 0;
  double sumCellET12 = 0.;
  double sumEMCellET = 0.;
  double sumHadCellET = 0.;

  int numCells = 0;
  std::bitset<200000> cellSeen;

  TLorentzVector tauAxis = tauRecTools::getTauAxis(pTau, m_doVertexCorrection);
  
  // loop over cells in all the clusters and calculate the variables
  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : pTau.vertexedClusters()){
    const xAOD::CaloCluster& cluster = vertexedCluster.clust();
    const CaloClusterCellLink* cellLinks = cluster.getCellLinks();
    if (cellLinks == nullptr) {
      ATH_MSG_DEBUG("NO Cell links found for cluster with pT " << cluster.pt());
      continue;
    }
    for (const CaloCell* cell : *cellLinks) {
      ++numCells;
     
      // cells could be used by more than one cluster, only count the cell one time 
      if (cellSeen.test(cell->caloDDE()->calo_hash())) {
        continue;
      }
      else {
        cellSeen.set(cell->caloDDE()->calo_hash());
      }
      
      // cell four momentum corrected to point at the required vertex
      double cellPhi = cell->phi();
      double cellEta = cell->eta();
      double cellET = cell->et();
      double cellEnergy = cell->energy();
      
      const xAOD::Vertex* vertex = pTau.vertex();
      if (m_doVertexCorrection && vertex!=nullptr) {
        CaloVertexedCell vxCell (*cell, vertex->position());
        cellPhi = vxCell.phi();
        cellEta = vxCell.eta();
        cellET = vxCell.et();
        cellEnergy = vxCell.energy();
      }

      TLorentzVector temp_cc_p4;
      temp_cc_p4.SetPtEtaPhiE(cellET, cellEta, cellPhi, cellEnergy);
      double dR = tauAxis.DeltaR(temp_cc_p4);
     
      if (dR < m_cellCone) {
        sumCellET += cellET;
       
        if (dR < 0.1) sumCellET01 += cellET;
        if (dR > 0.1 && dR < 0.2) sumCellET12 += cellET;

        CaloSampling::CaloSample calo = cell->caloDDE()->getSampling();

        // EM layer: PreSamplerB, PreSamplerE, EMB1, EME1, EMB2, EME2
        // Most energy of neutral particles are deposited in the first two EM laywers
        // The third layer is regarded as HAD layber
        if (isEMLayer(calo)) {
          sumEMCellET += cellET;
        } // end of EM cells
        else { 
            sumHadCellET += cellET;
        } // end of HAD cells
      } // end of dR <  m_cellCone
    } // end of loop over cells
  } // end of loop over clusters

  ATH_MSG_DEBUG(numCells << " cells in seed");
  
  pTau.setDetail(xAOD::TauJetParameters::numCells ,  static_cast<int>  (numCells));
  pTau.setDetail(xAOD::TauJetParameters::etEMAtEMScale , static_cast<float>( sumEMCellET ));
  pTau.setDetail(xAOD::TauJetParameters::etHadAtEMScale , static_cast<float>( sumHadCellET ));

  // take care of the variables with division
  // -- fraction of cell energy within [0,0.1] and [0.1,0.2]
  if (std::abs(sumCellET) > 1e-6) {
    pTau.setDetail(xAOD::TauJetParameters::centFrac , static_cast<float>( sumCellET01 / sumCellET ));
    pTau.setDetail(xAOD::TauJetParameters::isolFrac , static_cast<float>( sumCellET12 / sumCellET ));
  } 
  else {
    pTau.setDetail(xAOD::TauJetParameters::centFrac , static_cast<float>( 0.0 ));
    pTau.setDetail(xAOD::TauJetParameters::isolFrac , static_cast<float>( -1.0 ));
  }
 
  return StatusCode::SUCCESS;
}

#endif
