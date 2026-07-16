/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

// **********************************************************************
// IDAlignMonPVBiases.cxx
// AUTHORS: Ambrosius  Vermeulen, Pierfrancesco Butti
// Adapted to new AthenaMT Monitoring 2022 by Per Johansson
// **********************************************************************

//main header
#include "IDAlignMonPVBiasesAlg.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"

#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "InDetPrepRawData/SiCluster.h"

#include "Particle/TrackParticle.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"

#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/LocalParameters.h"


#include "CLHEP/GenericFunctions/CumulativeChiSquare.hh"

#include "InDetAlignGenTools/IInDetAlignHitQualSelTool.h"

#include <cmath>
#include <memory>

// *********************************************************************
// Public Methods
// *********************************************************************

IDAlignMonPVBiasesAlg::IDAlignMonPVBiasesAlg(const std::string& name, ISvcLocator* pSvcLocator ) :
  AthMonitorAlgorithm(name, pSvcLocator){}

IDAlignMonPVBiasesAlg::~IDAlignMonPVBiasesAlg() { }

StatusCode IDAlignMonPVBiasesAlg::initialize() {

  ATH_CHECK( m_trackParticleName.initialize() );
  ATH_CHECK( m_vxContainerName.initialize(not m_vxContainerName.key().empty()));
  ATH_CHECK( m_trackToVertexIPEstimator.retrieve());
    
  return AthMonitorAlgorithm::initialize();
}


StatusCode IDAlignMonPVBiasesAlg::fillHistograms( const EventContext& ctx ) const {
  using namespace Monitored;

  // For histogram naming
  auto pvGroup = getGroup("PVBiases");

  /******************************************************************
  ** Retrieve Trackparticles
  ******************************************************************/
  auto trackParticles = SG::makeHandle(m_trackParticleName, ctx);

  if ( !(trackParticles.isValid()) ) {
    ATH_MSG_ERROR("IDAlignMonPVBiasesAlg: Track container "<< m_trackParticleName.key() << " could not be found.");
    return StatusCode::RECOVERABLE;
  } else {
    ATH_MSG_DEBUG("IDAlignMonPVBiasesAlg: Track container "<< trackParticles.name() <<" is found.");
  }

  // Retrieve Vertices
  auto vertices = SG::makeHandle(m_vxContainerName, ctx);

  if (!vertices.isPresent()) {
    ATH_MSG_DEBUG ("IDAlignPVBiasesAlg: StoreGate doesn't contain primary vertex container with key "+m_vxContainerName.key());
    return StatusCode::SUCCESS;
  }
  if (not vertices.isValid()) {
    ATH_MSG_WARNING("IDAlignPVBiasesAlg: Failed to retrieve vertex container with key " << m_vxContainerName.key());
    return StatusCode::RECOVERABLE;
  }


  /******************************************************************
  ** Trackparticle Loop
  *******************************************************************/
  for (const auto& trackPart: *trackParticles) {
    if ( !trackPart )
      {
	ATH_MSG_DEBUG( "InDetAlignPVBiasesAlg: NULL track pointer in collection" );
	continue;
      }

    const xAOD::Vertex* foundVertex { nullptr };
    for (const auto* const vx : *vertices) {
      for (const auto& tpLink : vx->trackParticleLinks()) {
        if (*tpLink == trackPart) {
          foundVertex = vx;
          break;
        }
        if (foundVertex) break;
      }
    }
    // require having vertex
    if (!foundVertex) continue;
    // require associated with primary vertex
    if (foundVertex->vertexType() != 1) continue;
    // require at least 10 tracks associated
    if (foundVertex->nTrackParticles() < 10) continue;

    std::unique_ptr<const Trk::ImpactParametersAndSigma> myIPandSigma(m_trackToVertexIPEstimator->estimate(trackPart, foundVertex, true));

    // require d0_pv to be smaller than 4
    if(myIPandSigma.get()==nullptr || std::abs(myIPandSigma->IPd0) > 4.0) continue;
 
    double charge = trackPart->charge();

    /******************************************************************
    ** Fill Histograms
    *******************************************************************/
    double pt = trackPart->pt() * 0.001;
 
    if (pt > 0.4 && pt < 0.6) {

      if (charge == 1){
	auto phi_46p_m = Monitored::Scalar<float>( "m_Phi_46p", trackPart->phi() );
	auto eta_46p_m = Monitored::Scalar<float>( "m_Eta_46p", trackPart->eta() );
	auto d0_46p_m = Monitored::Scalar<float>( "m_d0_46p", myIPandSigma->IPd0 );
	fill(pvGroup, phi_46p_m, d0_46p_m);
	fill(pvGroup, eta_46p_m, d0_46p_m);
      }
      else if (charge == -1){
	auto phi_46n_m = Monitored::Scalar<float>( "m_Phi_46n", trackPart->phi() );
	auto eta_46n_m = Monitored::Scalar<float>( "m_Eta_46n", trackPart->eta() );
	auto d0_46n_m = Monitored::Scalar<float>( "m_d0_46n", myIPandSigma->IPd0 );
	fill(pvGroup, phi_46n_m, d0_46n_m);
	fill(pvGroup, eta_46n_m, d0_46n_m);
      }
    }

    if (pt > 0.6 && pt < 1) {
      if (charge == 1){
	auto phi_61p_m = Monitored::Scalar<float>( "m_Phi_61p", trackPart->phi() );
	auto eta_61p_m = Monitored::Scalar<float>( "m_Eta_61p", trackPart->eta() );
	auto d0_61p_m = Monitored::Scalar<float>( "m_d0_61p", myIPandSigma->IPd0 );
      	fill(pvGroup, phi_61p_m, d0_61p_m);
	fill(pvGroup, eta_61p_m, d0_61p_m);
      }
    
      else if (charge == -1){
	auto phi_61n_m = Monitored::Scalar<float>( "m_Phi_61n", trackPart->phi() );
	auto eta_61n_m = Monitored::Scalar<float>( "m_Eta_61n", trackPart->eta() );
	auto d0_61n_m = Monitored::Scalar<float>( "m_d0_61n", myIPandSigma->IPd0 );
	fill(pvGroup, phi_61n_m, d0_61n_m);
	fill(pvGroup, eta_61n_m, d0_61n_m);
      }
    }

    if (pt > 1 && pt < 2) {
      if (charge == 1){
	auto phi_12p_m = Monitored::Scalar<float>( "m_Phi_12p", trackPart->phi() );
	auto eta_12p_m = Monitored::Scalar<float>( "m_Eta_12p", trackPart->eta() );
	auto d0_12p_m = Monitored::Scalar<float>( "m_d0_12p", myIPandSigma->IPd0 );
	fill(pvGroup, phi_12p_m, d0_12p_m);
	fill(pvGroup, eta_12p_m, d0_12p_m);
      }
      else if (charge == -1){
	auto phi_12n_m = Monitored::Scalar<float>( "m_Phi_12n", trackPart->phi() );
	auto eta_12n_m = Monitored::Scalar<float>( "m_Eta_12n", trackPart->eta() );
	auto d0_12n_m = Monitored::Scalar<float>( "m_d0_12n", myIPandSigma->IPd0 );
	fill(pvGroup, phi_12n_m, d0_12n_m);
	fill(pvGroup, eta_12n_m, d0_12n_m);
      }
    }
    
    if (pt > 2 && pt < 5) {
      if (charge == 1){
	auto phi_25p_m = Monitored::Scalar<float>( "m_Phi_25p", trackPart->phi() );
	auto eta_25p_m = Monitored::Scalar<float>( "m_Eta_25p", trackPart->eta() );
	auto d0_25p_m = Monitored::Scalar<float>( "m_d0_25p", myIPandSigma->IPd0 );
	fill(pvGroup, phi_25p_m, d0_25p_m);
	fill(pvGroup, eta_25p_m, d0_25p_m);
      }
      else if (charge == -1){
	auto phi_25n_m = Monitored::Scalar<float>( "m_Phi_25n", trackPart->phi() );
	auto eta_25n_m = Monitored::Scalar<float>( "m_Eta_25n", trackPart->eta() );
	auto d0_25n_m = Monitored::Scalar<float>( "m_d0_25n", myIPandSigma->IPd0 );
	fill(pvGroup, phi_25n_m, d0_25n_m);
	fill(pvGroup, eta_25n_m, d0_25n_m);
      }
    }
    
    if (pt > 5 && pt < 10) {
      if (charge == 1){
	auto phi_510p_m = Monitored::Scalar<float>( "m_Phi_510p", trackPart->phi() );
	auto eta_510p_m = Monitored::Scalar<float>( "m_Eta_510p", trackPart->eta() );
	auto d0_510p_m = Monitored::Scalar<float>( "m_d0_510p", myIPandSigma->IPd0 );
	fill(pvGroup, phi_510p_m, d0_510p_m);
	fill(pvGroup, eta_510p_m, d0_510p_m);
      }
      else if (charge == -1){
	auto phi_510n_m = Monitored::Scalar<float>( "m_Phi_510n", trackPart->phi() );
	auto eta_510n_m = Monitored::Scalar<float>( "m_Eta_510n", trackPart->eta() );
	auto d0_510n_m = Monitored::Scalar<float>( "m_d0_510n", myIPandSigma->IPd0 );
	fill(pvGroup, phi_510n_m, d0_510n_m);
	fill(pvGroup, eta_510n_m, d0_510n_m);
      }
    }

    if (pt > 10) {
     if (charge == 1){
	auto phi_g10p_m = Monitored::Scalar<float>( "m_Phi_g10p", trackPart->phi() );
	auto eta_g10p_m = Monitored::Scalar<float>( "m_Eta_g10p", trackPart->eta() );
	auto d0_g10p_m = Monitored::Scalar<float>( "m_d0_g10p", myIPandSigma->IPd0 );
	fill(pvGroup, phi_g10p_m, d0_g10p_m);
	fill(pvGroup, eta_g10p_m, d0_g10p_m);
      }
     else if (charge == -1){
	auto phi_g10n_m = Monitored::Scalar<float>( "m_Phi_g10n", trackPart->phi() );
	auto eta_g10n_m = Monitored::Scalar<float>( "m_Eta_g10n", trackPart->eta() );
	auto d0_g10n_m = Monitored::Scalar<float>( "m_d0_g10n", myIPandSigma->IPd0 );
	fill(pvGroup, phi_g10n_m, d0_g10n_m);
	fill(pvGroup, eta_g10n_m, d0_g10n_m);
      }
    }

  } // End of track selection loop

  return StatusCode::SUCCESS;
}

