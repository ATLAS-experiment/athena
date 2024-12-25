/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*  TrackParticleCaloCellDecorator.cxx    */

#include "DerivationFrameworkLLP/TrackParticleCaloCellDecorator.h"
#include <StoreGate/WriteDecorHandle.h>
#include <vector>
#include <string>
#include "TileEvent/TileCell.h"

namespace DerivationFramework {

  TrackParticleCaloCellDecorator::TrackParticleCaloCellDecorator( const std::string& t,
								  const std::string& n,
								  const IInterface* p) :
    base_class(t, n, p){}

  StatusCode TrackParticleCaloCellDecorator::initialize() {

    if ( m_sgName.empty() ) {
      ATH_MSG_WARNING("No decoration prefix name provided for the output of TrackParticleCaloCellDecorator!");
      return StatusCode::SUCCESS;
    }
    if ( m_trackContainerKey.empty() ) {
      ATH_MSG_WARNING("No TrackParticle collection provided for TrackParticleCaloCellDecorator!");
      return StatusCode::SUCCESS;
    }

    m_decCellEtaKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellEtaKey.key();
    m_decCellPhiKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellPhiKey.key();
    m_decCellRKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellRKey.key();
    m_decCelldEtaKey = m_trackContainerKey.key() + "." + m_sgName + m_decCelldEtaKey.key();
    m_decCelldPhiKey = m_trackContainerKey.key() + "." + m_sgName + m_decCelldPhiKey.key();
    m_decCelldRKey = m_trackContainerKey.key() + "." + m_sgName + m_decCelldRKey.key();
    m_decCellXKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellXKey.key();
    m_decCellYKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellYKey.key();
    m_decCellZKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellZKey.key();
    m_decCelldXKey = m_trackContainerKey.key() + "." + m_sgName + m_decCelldXKey.key();
    m_decCelldYKey = m_trackContainerKey.key() + "." + m_sgName + m_decCelldYKey.key();
    m_decCelldZKey = m_trackContainerKey.key() + "." + m_sgName + m_decCelldZKey.key();
    m_decCellTKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellTKey.key();
    m_decCellEKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellEKey.key();
    m_decCellIDKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellIDKey.key();
    m_decCellSamplingKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellSamplingKey.key();
    m_decCellQualityKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellQualityKey.key();
    m_decCellProvenanceKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellProvenanceKey.key();
    m_decCellGainKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellGainKey.key();
    m_decCellEneDiffKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellEneDiffKey.key();
    m_decCellTimeDiffKey = m_trackContainerKey.key() + "." + m_sgName + m_decCellTimeDiffKey.key();

    m_trackContainerKey = m_trackContainerKey.key() + "ClusterAssociations";
    ATH_CHECK(m_trackContainerKey.initialize());

    ATH_CHECK(m_decCellEtaKey.initialize());
    ATH_CHECK(m_decCellPhiKey.initialize());
    ATH_CHECK(m_decCellRKey.initialize());
    ATH_CHECK(m_decCelldEtaKey.initialize());
    ATH_CHECK(m_decCelldPhiKey.initialize());
    ATH_CHECK(m_decCelldRKey.initialize());
    ATH_CHECK(m_decCellXKey.initialize());
    ATH_CHECK(m_decCellYKey.initialize());
    ATH_CHECK(m_decCellZKey.initialize());
    ATH_CHECK(m_decCelldXKey.initialize());
    ATH_CHECK(m_decCelldYKey.initialize());
    ATH_CHECK(m_decCelldZKey.initialize());
    ATH_CHECK(m_decCellTKey.initialize());
    ATH_CHECK(m_decCellEKey.initialize());
    ATH_CHECK(m_decCellIDKey.initialize());
    ATH_CHECK(m_decCellSamplingKey.initialize());
    ATH_CHECK(m_decCellQualityKey.initialize());
    ATH_CHECK(m_decCellProvenanceKey.initialize());
    ATH_CHECK(m_decCellGainKey.initialize());
    ATH_CHECK(m_decCellEneDiffKey.initialize());
    ATH_CHECK(m_decCellTimeDiffKey.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode TrackParticleCaloCellDecorator::addBranches() const  {

    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::TrackParticleClusterAssociationContainer> clusterAssociations(m_trackContainerKey,ctx);
    ATH_CHECK( clusterAssociations.isValid() );

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellEta(m_decCellEtaKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellPhi(m_decCellPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellR(m_decCellRKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCelldEta(m_decCelldEtaKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCelldPhi(m_decCelldPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCelldR(m_decCelldRKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellX(m_decCellXKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellY(m_decCellYKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellZ(m_decCellZKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCelldX(m_decCelldXKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCelldY(m_decCelldYKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCelldZ(m_decCelldZKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellT(m_decCellTKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellE(m_decCellEKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<unsigned int>> decCellID(m_decCellIDKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decCellSampling(m_decCellSamplingKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decCellQuality(m_decCellQualityKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decCellProvenance(m_decCellProvenanceKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decCellGain(m_decCellGainKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellEneDiff(m_decCellEneDiffKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decCellTimeDiff(m_decCellTimeDiffKey, ctx);

    for(const auto clusterAssociation : *clusterAssociations) {
      std::vector<float> trackCellEta(0, 0.);
      std::vector<float> trackCellPhi(0, 0.);
      std::vector<float> trackCellR(0, 0.);
      std::vector<float> trackCelldEta(0, 0.);
      std::vector<float> trackCelldPhi(0, 0.);
      std::vector<float> trackCelldR(0, 0.);
      std::vector<float> trackCellX(0, 0.);
      std::vector<float> trackCellY(0, 0.);
      std::vector<float> trackCellZ(0, 0.);
      std::vector<float> trackCelldX(0, 0.);
      std::vector<float> trackCelldY(0, 0.);
      std::vector<float> trackCelldZ(0, 0.);
      std::vector<float> trackCellT(0, 0.);
      std::vector<float> trackCellE(0, 0.);
      std::vector<unsigned int> trackCellID(0, 0);
      std::vector<int> trackCellSampling(0, 0);
      std::vector<int> trackCellQuality(0, 0);
      std::vector<int> trackCellProvenance(0, 0);
      std::vector<int> trackCellGain(0, 0);
      std::vector<float> trackCellEneDiff(0, 0);
      std::vector<float> trackCellTimeDiff(0, 0);

      const xAOD::TrackParticle* track = nullptr;
      if (clusterAssociation->trackParticleLink().isValid()) {
        track = *(clusterAssociation->trackParticleLink());
      } else {
	ATH_MSG_DEBUG ("trackParticleLink is not valid! " );
	continue;
      }

      for (const auto& cluster : clusterAssociation->caloClusterLinks()) {
	const CaloClusterCellLink* cellLinks = (*cluster)->getCellLinks();

	trackCellEta.clear();
	trackCellPhi.clear();
	trackCellR.clear();
	trackCelldEta.clear();
	trackCelldPhi.clear();
	trackCelldR.clear();
	trackCellX.clear();
	trackCellY.clear();
	trackCellZ.clear();
	trackCelldX.clear();
	trackCelldY.clear();
	trackCelldZ.clear();
	trackCellT.clear();
	trackCellE.clear();
	trackCellID.clear();
	trackCellSampling.clear();
	trackCellQuality.clear();
	trackCellProvenance.clear();
	trackCellGain.clear();
	trackCellEneDiff.clear();
	trackCellTimeDiff.clear();

	if ( !cellLinks ) {
	  ATH_MSG_ERROR ("Unable to get cell links!");
	  continue;
	}

	for(const auto cell : *cellLinks) {
	  const CaloDetDescrElement *caloDDE = cell->caloDDE();
	  int sampling = -1;
	  if ( caloDDE ) {
	    sampling = caloDDE->getSampling();
	    trackCellEta.emplace_back(caloDDE->eta());
	    trackCellPhi.emplace_back(caloDDE->phi());
	    trackCellR.emplace_back(caloDDE->r());
	    trackCelldEta.emplace_back(caloDDE->deta());
	    trackCelldPhi.emplace_back(caloDDE->dphi());
	    trackCelldR.emplace_back(caloDDE->dr());
	    trackCellX.emplace_back(caloDDE->x());
	    trackCellY.emplace_back(caloDDE->y());
	    trackCellZ.emplace_back(caloDDE->z());
	    trackCelldX.emplace_back(caloDDE->dx());
	    trackCelldY.emplace_back(caloDDE->dy());
	    trackCelldZ.emplace_back(caloDDE->dz());
	    trackCellT.emplace_back(cell->time());
	    trackCellE.emplace_back(cell->e());
	    Identifier32 IdOfCell = cell->ID().get_identifier32();
	    trackCellID.emplace_back(IdOfCell.get_compact());
	    trackCellSampling.emplace_back(sampling);
	    trackCellQuality.emplace_back(cell->quality());
	    trackCellProvenance.emplace_back(cell->provenance());
	    trackCellGain.emplace_back(cell->gain());
            if (caloDDE->is_tile()) {
              trackCellEneDiff.emplace_back(static_cast<const TileCell*>(cell)->eneDiff());
              trackCellTimeDiff.emplace_back(static_cast<const TileCell*>(cell)->timeDiff());
            }
            else {
              trackCellEneDiff.emplace_back(0);
              trackCellTimeDiff.emplace_back(0);
            }
	  } // if ( caloDDE )	    
	} // for( cell )	  
      } // for( cluster )
      
      decCellEta(*track) = trackCellEta;
      decCellPhi(*track) = trackCellPhi;
      decCellR(*track) = trackCellR;
      decCelldEta(*track) = trackCelldEta;
      decCelldPhi(*track) = trackCelldPhi;
      decCelldR(*track) = trackCelldR;
      decCellX(*track) = trackCellX;
      decCellY(*track) = trackCellY;
      decCellZ(*track) = trackCellZ;
      decCelldX(*track) = trackCelldX;
      decCelldY(*track) = trackCelldY;
      decCelldZ(*track) = trackCelldZ;
      decCellT(*track) = trackCellT;
      decCellE(*track) = trackCellE;
      decCellID(*track) = trackCellID;
      decCellSampling(*track) = trackCellSampling;
      decCellQuality(*track) = trackCellQuality;
      decCellProvenance(*track) = trackCellProvenance;
      decCellGain(*track) = trackCellGain;
      decCellEneDiff(*track) = trackCellEneDiff;
      decCellTimeDiff(*track) = trackCellTimeDiff;
    }


    return StatusCode::SUCCESS;
  }
}
