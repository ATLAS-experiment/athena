/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TauElectronVetoVariables.h"

#include "CaloUtils/CaloVertexedCell.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"

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
  ATH_CHECK( m_caloExtensionTool.retrieve() );
  if (!m_ParticleCacheKey.key().empty()) {
    ATH_CHECK(m_ParticleCacheKey.initialize());
  } else {
    m_useOldCalo = true;
  }
  return StatusCode::SUCCESS;
}



StatusCode TauElectronVetoVariables::execute(xAOD::TauJet& pTau) const {
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
    int trackIndex = -1;

    //---------------------------------------------------------------------
    // Calculate eta, phi impact point of leading track at calorimeter layers EM 0,1,2,3
    //---------------------------------------------------------------------
    Trk::TrackParametersIdHelper parsIdHelper;
    constexpr size_t numberOfEM_Layers{4};
    constexpr double invalidCoordinate{-11111.};
    constexpr double invalidCoordinateThreshold{-11110.}; 
    std::array<double, numberOfEM_Layers> extrapolatedEta{};
    std::array<double, numberOfEM_Layers> extrapolatedPhi{};
    extrapolatedEta.fill(invalidCoordinate);
    extrapolatedPhi.fill(invalidCoordinate);
    
    /*get the CaloExtension object*/
    const Trk::CaloExtension * caloExtension = nullptr;
    std::unique_ptr<Trk::CaloExtension> uniqueExtension ;
    const xAOD::TrackParticle *orgTrack = pTau.track(0)->track();
    trackIndex = orgTrack->index();
    if (m_useOldCalo) {
      /* If CaloExtensionBuilder is unavailable, use the calo extension tool */
      ATH_MSG_VERBOSE("Using the CaloExtensionTool");
      uniqueExtension = m_caloExtensionTool->caloExtension(
        Gaudi::Hive::currentContext(), *orgTrack);
      caloExtension = uniqueExtension.get();
    } else {
      /*get the CaloExtension object*/
      ATH_MSG_VERBOSE("Using the CaloExtensionBuilder Cache");
      SG::ReadHandle<CaloExtensionCollection>  particleCache {m_ParticleCacheKey};
      caloExtension = (*particleCache)[trackIndex];
      ATH_MSG_VERBOSE("Getting element " << trackIndex << " from the particleCache");
      if( not caloExtension ){
        ATH_MSG_VERBOSE("Cache does not contain a calo extension -> Calculating with the a CaloExtensionTool" );
        uniqueExtension = m_caloExtensionTool->caloExtension(
          Gaudi::Hive::currentContext(), *orgTrack);
        caloExtension = uniqueExtension.get();
      }
    }
    if( not caloExtension){
      ATH_MSG_DEBUG("extrapolation of leading track to calo surfaces failed  : caloExtension is nullptr" );
      return StatusCode::RECOVERABLE;
    }
    const std::vector<Trk::CurvilinearParameters>& clParametersVector = caloExtension->caloLayerIntersections();
    if(clParametersVector.empty() ){
      ATH_MSG_DEBUG("extrapolation of leading track to calo surfaces failed  : caloLayerIntersection is empty" );
      return StatusCode::RECOVERABLE;
    }
    // loop over calo layers
    for( const Trk::CurvilinearParameters& cur : clParametersVector ){
      // only use entry layer
      if( !parsIdHelper.isEntryToVolume(cur.cIdentifier()) ) continue;
      CaloSampling::CaloSample sample = parsIdHelper.caloSample(cur.cIdentifier());
      int index = -1;
      if( sample == CaloSampling::PreSamplerE || sample == CaloSampling::PreSamplerB ) index = 0;
      else if( sample == CaloSampling::EME1 || sample == CaloSampling::EMB1 )          index = 1;
      else if( sample == CaloSampling::EME2 || sample == CaloSampling::EMB2 )          index = 2;
      else if( sample == CaloSampling::EME3 || sample == CaloSampling::EMB3 )          index = 3;
      if( index < 0 ) continue;
      extrapolatedEta[index] = cur.position().eta();
      extrapolatedPhi[index] = cur.position().phi();
    }
    for (size_t i = 0; i < numberOfEM_Layers; ++i) {
      if ( extrapolatedEta[i] < invalidCoordinateThreshold || extrapolatedPhi[i] < invalidCoordinateThreshold ){
        ATH_MSG_DEBUG("extrapolation of leading track to calo surfaces failed for sampling : " << i );
        return StatusCode::SUCCESS;
      }
    }
    
    // Loop through jets, get links to clusters
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
