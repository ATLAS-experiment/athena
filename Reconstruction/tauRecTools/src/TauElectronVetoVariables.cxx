/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TauElectronVetoVariables.h"

#include "CaloUtils/CaloVertexedCell.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <array>

#include "TVector2.h"

using Gaudi::Units::GeV;

TauElectronVetoVariables::TauElectronVetoVariables(const std::string &name) :
TauRecToolBase(name) {}

StatusCode TauElectronVetoVariables::initialize() {

  return StatusCode::SUCCESS;
}


StatusCode TauElectronVetoVariables::executeTool(xAOD::TauJet& pTau,
						 const EventContext& /*ctx*/) const {
    if (pTau.nTracks() < 1) {
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("in execute()");
    float detPhiTrk = 0.;
    float detEtaTrk = 0.;
    float sumETCellsLAr = 0.;
    float eta0cut = 0.075;
    float eta1cut = 0.0475;
    float eta2cut = 0.075;
    float eta3cut = 1.5;
    float phi0cut = 0.3;
    float phi1cut = 0.3;
    float phi2cut = 0.075;
    float phi3cut = 0.075;
    const CaloCell *pCell;

    //---------------------------------------------------------------------
    // Retrieve eta, phi impact point of leading track at calorimeter layers EM 0,1,2,3
    //---------------------------------------------------------------------
    constexpr size_t numberOfEM_Layers{4};
    constexpr double invalidCoordinate{-11111.};
    constexpr double invalidCoordinateThreshold{-11110.}; 
    std::array<double, numberOfEM_Layers> extrapolatedEta{};
    std::array<double, numberOfEM_Layers> extrapolatedPhi{};
    extrapolatedEta.fill(invalidCoordinate);
    extrapolatedPhi.fill(invalidCoordinate);

    static const SG::ConstAccessor<std::array<double,numberOfEM_Layers>> acc_extrapolLeadTrackEta("extrapolLeadTrackEta");
    extrapolatedEta = acc_extrapolLeadTrackEta(pTau);

    static const SG::ConstAccessor<std::array<double,numberOfEM_Layers>> acc_extrapolLeadTrackPhi("extrapolLeadTrackPhi");
    extrapolatedPhi = acc_extrapolLeadTrackPhi(pTau); 
     
    for (size_t i = 0; i < numberOfEM_Layers; ++i) {
      if ( extrapolatedEta[i] < invalidCoordinateThreshold || extrapolatedPhi[i] < invalidCoordinateThreshold ){
        ATH_MSG_DEBUG("extrapolation of leading track to calo surfaces failed for sampling : " << i );
        return StatusCode::SUCCESS;
      }
    }
    
    // Loop through jets, get links to clusters
    //Local variable cellSeen uses 25000 bytes of stack space
    //coverity[STACK_USE]
    std::bitset<200000> cellSeen{};
    const std::unordered_map<int, int> samplingLookup{
      {4,0}, {5,1}, {6,2}, {7,3}, {8,12},
      {15, 12}, {16,13}, {17,14}, {18,12}, {19, 13}, {20,14}
    };
    const auto notFound{samplingLookup.end()};
    
    std::vector<xAOD::CaloVertexedTopoCluster> vertexedClusterList = pTau.vertexedClusters();
    for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : vertexedClusterList){
      
      const xAOD::CaloCluster& cluster = vertexedCluster.clust();
      auto cell_links = cluster.getCellLinks();
      if (cell_links == nullptr) {
        ATH_MSG_DEBUG("NO Cell links found for cluster with pT " << cluster.pt());
        continue;
      }
      CaloClusterCellLink::const_iterator pCellIter  = cluster.getCellLinks()->begin();
      CaloClusterCellLink::const_iterator pCellIterE = cluster.getCellLinks()->end();
      for (; pCellIter != pCellIterE; ++pCellIter) {
	double cellEta{}, cellPhi{}, cellET{};
        pCell = *pCellIter;
        if (cellSeen.test(pCell->caloDDE()->calo_hash())) continue;
        else cellSeen.set(pCell->caloDDE()->calo_hash());
        if (m_doVertexCorrection && pTau.vertex()!=nullptr) {
          CaloVertexedCell vxCell (*pCell, pTau.vertex()->position());
          cellPhi = vxCell.phi();
          cellEta = vxCell.eta();
          cellET = vxCell.et();
        } else {
          cellPhi = pCell->phi();
          cellEta = pCell->eta();
          cellET = pCell->et();
        }
        int sampling = pCell->caloDDE()->getSampling();
        if (const auto & pElement {samplingLookup.find(sampling)};pElement != notFound){
          sampling = pElement->second;
        }
        int i = 2;
        if (sampling < 4) i = sampling;
        if (sampling == 12 || sampling == 13 || sampling == 14) i = 3;
        detPhiTrk = TVector2::Phi_mpi_pi(cellPhi-extrapolatedPhi[i]);
        detEtaTrk = std::abs( cellEta - extrapolatedEta[i] );
        if ((sampling == 0 && detEtaTrk < eta0cut && detPhiTrk < phi0cut) ||
                (sampling == 1 && detEtaTrk < eta1cut && detPhiTrk < phi1cut) ||
                (sampling == 2 && detEtaTrk < eta2cut && detPhiTrk < phi2cut) ||
                (sampling == 3 && detEtaTrk < eta3cut && detPhiTrk < phi3cut)) {
            sumETCellsLAr += cellET;
        }
      } //end cell loop
    }// end jet constituent loop

    pTau.setDetail(xAOD::TauJetParameters::sumEMCellEtOverLeadTrkPt , static_cast<float>( ( sumETCellsLAr / ( pTau.track(0)->pt() ) ) ) );
    return StatusCode::SUCCESS;
}

#endif
