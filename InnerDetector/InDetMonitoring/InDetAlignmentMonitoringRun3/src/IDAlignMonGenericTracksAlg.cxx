/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// AlignmentMonAlg.cxx
// AUTHORS: Beate Heinemann, Tobias Golling
// Adapted to AthenaMT by Per Johansson 2021
// **********************************************************************

//main header
#include "IDAlignMonGenericTracksAlg.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"

#include "TrkTrack/TrackCollection.h"
#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "InDetPrepRawData/SiCluster.h"

#include "Particle/TrackParticle.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"

#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/LocalParameters.h"

#include "CLHEP/GenericFunctions/CumulativeChiSquare.hh"

#include "InDetAlignGenTools/IInDetAlignHitQualSelTool.h"

#include <cmath>

// *********************************************************************
// Public Methods
// *********************************************************************

IDAlignMonGenericTracksAlg::IDAlignMonGenericTracksAlg( const std::string & name, ISvcLocator* pSvcLocator ) :
   AthMonitorAlgorithm(name, pSvcLocator),
   m_idHelper(nullptr),
   m_pixelID(nullptr),
   m_sctID(nullptr),
   m_trtID(nullptr),
   m_d0Range(2.0),
   m_d0BsRange(0.5),
   m_z0Range(250.0),
   m_etaRange(3.0),
   m_NTracksRange(200),
   m_barrelEta(0.8), //Tracks between -0.8 & 0.8 are considered as Barrel Tracks, otherwise are End-Caps
   m_trackSelection( "InDet::InDetTrackSelectionTool/TrackSelectionTool", this),
   m_hitQualityTool("")
{
  declareProperty("Pixel_Manager"        , m_Pixel_Manager);
  declareProperty("SCT_Manager"          , m_SCT_Manager);
  declareProperty("TRT_Manager"          , m_TRT_Manager);
  declareProperty("TrackSelectionTool"   , m_trackSelection);
  declareProperty("HitQualityTool"       , m_hitQualityTool);
  declareProperty("useExtendedPlots"     , m_extendedPlots = false);
  declareProperty("d0Range"              , m_d0Range);
  declareProperty("d0BsRange"            , m_d0BsRange);
  declareProperty("z0Range"              , m_z0Range);
  declareProperty("etaRange"             , m_etaRange);
  declareProperty("pTRange"              , m_pTRange);
  declareProperty("NTracksRange"         , m_NTracksRange);
  declareProperty("doIP"                 , m_doIP = false);
  declareProperty("ApplyTrackSelection"  , m_applyTrkSel = true);
}


IDAlignMonGenericTracksAlg::~IDAlignMonGenericTracksAlg() { }


StatusCode IDAlignMonGenericTracksAlg::initialize()
{
  StatusCode sc;  

  ATH_MSG_DEBUG("Initialize -- START --");
  //ID Helper
  ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));
  
  m_pixelID = nullptr;
  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_MSG_DEBUG("Initialized PixelIDHelper");

  m_sctID = nullptr;
  ATH_CHECK(detStore()->retrieve(m_sctID, "SCT_ID"));
  ATH_MSG_DEBUG("Initialized SCTIDHelper");

  m_trtID = nullptr;
  ATH_CHECK(detStore()->retrieve(m_trtID, "TRT_ID"));
  ATH_MSG_DEBUG("Initialized TRTIDHelper");

  ATH_CHECK(m_trackSelection.retrieve());
  ATH_MSG_DEBUG("Retrieved tool " << m_trackSelection);

  if (m_hitQualityTool.empty()) {
    ATH_MSG_DEBUG("No hit quality tool configured - not hit quality cuts will be imposed");
    m_doHitQuality = false;
  } else if (m_hitQualityTool.retrieve().isFailure()) {
    ATH_MSG_WARNING("Could not retrieve " << m_hitQualityTool << " (to apply hit quality cuts to Si hits) ");
    m_doHitQuality = false;
  } else {
    ATH_MSG_DEBUG("Hit quality tool setup - hit quality cuts will be applied to Si hits");
    m_doHitQuality = true;
  }
  
  if (m_doIP) {
    ATH_CHECK (m_trackToVertexIPEstimator.retrieve());
  }else {
    m_trackToVertexIPEstimator.disable();
  }
  
  if ( m_beamSpotKey.initialize().isFailure() ) {
    ATH_MSG_WARNING("Failed to retrieve beamspot service " << m_beamSpotKey << " - will use nominal beamspot at (0,0,0)");
    m_hasBeamCondSvc = false;
  } 
  else {
    m_hasBeamCondSvc = true;
    ATH_MSG_DEBUG("Retrieved service " << m_beamSpotKey);
  }
  
  ATH_CHECK(m_VxPrimContainerName.initialize(not m_VxPrimContainerName.key().empty()));
  ATH_CHECK(m_tracksName.initialize());
  ATH_CHECK(m_tracksKey.initialize());

  // Building Tool Maps for the Hit Maps 
  m_measurements_vs_Eta_Phi_pix_b = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_pix_b", m_nSiBlayers);
  m_measurements_vs_Eta_Phi_pix_ec = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_pix_ec", 2);
  m_measurements_vs_Eta_Phi_sct_b_s0 = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_sct_b_s0", m_nSiBlayers);
  m_measurements_vs_Eta_Phi_sct_b_s1 = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_sct_b_s1", m_nSiBlayers);
  m_measurements_vs_Eta_Phi_sct_eca_s0 = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_sct_eca_s0", m_nSCTEClayers);
  m_measurements_vs_Eta_Phi_sct_eca_s1 = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_sct_eca_s1", m_nSCTEClayers);
  m_measurements_vs_Eta_Phi_sct_ecc_s0 = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_sct_ecc_s0", m_nSCTEClayers);
  m_measurements_vs_Eta_Phi_sct_ecc_s1 = Monitored::buildToolMap<int>(m_tools, "measurements_vs_Eta_Phi_sct_ecc_s1", m_nSCTEClayers); 

  ATH_MSG_DEBUG("Initialize -- completed --");
  return AthMonitorAlgorithm::initialize();
}

StatusCode IDAlignMonGenericTracksAlg::fillHistograms( const EventContext& ctx ) const {
  using namespace Monitored;
 
  // For histogram naming
  auto genericTrackGroup = getGroup("IDA_Tracks");

  //counters
  int ntrkMax=0;
  float xv=-999;
  float yv=-999;
  float zv=-999;
  int nTracks=0;
  int ngTracks=0;
  
  ATH_MSG_DEBUG ("IDAlignMonGenericTracksAlg::fillHistograms ** START ** call for track collection: " << m_tracksName.key());

  //get tracks
  auto trks = SG::makeHandle(m_tracksName, ctx);
  // check for tracks
  if (not trks.isValid()) {
    ATH_MSG_DEBUG ("IDAlignMonGenericTracksAlg::fillHistograms() --" << m_tracksName.key() << " could not be retrieved");
    return StatusCode::RECOVERABLE;
  }else {
    ATH_MSG_DEBUG("IDAlignMonGenericTracksAlg: Track container " << trks.name() <<" is found.");
  }

  //retrieving vertices
  auto handle_vxContainer = SG::makeHandle(m_VxPrimContainerName, ctx);
  // if m_doIP
  const xAOD::Vertex* pvtx = nullptr;
    
  if (!handle_vxContainer.isPresent()) {
    ATH_MSG_DEBUG ("InDetGlobalPrimaryVertexMonAlg: StoreGate doesn't contain primary vertex container with key "+m_VxPrimContainerName.key());
    return StatusCode::SUCCESS;
  }
  if (!handle_vxContainer.isValid()) {
    ATH_MSG_ERROR ("InDetGlobalPrimaryVertexMonAlg: Could not retrieve primary vertex container with key "+m_VxPrimContainerName.key());
    return StatusCode::RECOVERABLE;
  }

  const auto *vertexContainer = handle_vxContainer.cptr();
  for(const auto vtx : *vertexContainer) {
    if ( !vtx ) continue;
    if ( !vtx->vxTrackAtVertexAvailable() ) continue;
    
    const std::vector< Trk::VxTrackAtVertex >& theTrackAtVertex = vtx->vxTrackAtVertex();
    int numTracksPerVertex = theTrackAtVertex.size();
    ATH_MSG_DEBUG("Size of TrackAtVertex: " << numTracksPerVertex);
    if (numTracksPerVertex>ntrkMax) {
      ntrkMax=numTracksPerVertex;
      xv=vtx->position()[0];
      yv=vtx->position()[1];
      zv=vtx->position()[2];
    }
  }

  if (xv==-999 || yv==-999 || zv==-999) {
    ATH_MSG_DEBUG("No vertex found => setting it to 0");
    xv=0;yv=0;zv=0;
  }

 
  std::map<const xAOD::TrackParticle*, const xAOD::Vertex*> trackVertexMapTP;
  if (m_doIP) fillVertexInformation(trackVertexMapTP, ctx);
  
  float beamSpotX = 0.;
  float beamSpotY = 0.;
  float beamSpotZ = 0.;
  float beamTiltX = 0.;
  float beamTiltY = 0.;
 
  if (m_hasBeamCondSvc) {
    auto beamSpotHandle = SG::ReadCondHandle(m_beamSpotKey, ctx);
    Amg::Vector3D bpos = beamSpotHandle->beamPos();
    beamSpotX = bpos.x();
    beamSpotY = bpos.y();
    beamSpotZ = bpos.z();
    beamTiltX = beamSpotHandle->beamTilt(0);
    beamTiltY = beamSpotHandle->beamTilt(1);
    ATH_MSG_DEBUG ("Beamspot: x0 = " << beamSpotX << ", y0 = " << beamSpotY << ", z0 = " << beamSpotZ << ", tiltX = " << beamTiltX << ", tiltY = " << beamTiltY);
  }
  
  // Get EventInfo
  int lb       = GetEventInfo(ctx)->lumiBlock();
  auto lb_m    = Monitored::Scalar<int>( "m_lb", lb );
  int run      = GetEventInfo(ctx)->runNumber();
  auto run_m   = Monitored::Scalar<int>( "m_run", run );
  int event    = GetEventInfo(ctx)->eventNumber();
  auto event_m = Monitored::Scalar<int>( "m_event", event );
  float mu     = lbAverageInteractionsPerCrossing(ctx);
  auto mu_m    = Monitored::Scalar<float>("m_mu", mu);

  if (m_extendedPlots) {
    //Fill BeamSpot Position histos
    auto beamSpotX_m = Monitored::Scalar<float>( "m_beamSpotX", beamSpotX );
    auto beamSpotY_m = Monitored::Scalar<float>( "m_beamSpotY", beamSpotY );
    auto beamSpotZ_m = Monitored::Scalar<float>( "m_beamSpotZ", beamSpotZ );
    auto beamTiltX_m = Monitored::Scalar<float>( "m_beamTiltX", beamTiltX );
    auto beamTiltY_m = Monitored::Scalar<float>( "m_beamTiltY", beamTiltY );
    fill(genericTrackGroup, beamSpotX_m, beamSpotY_m, beamSpotZ_m, lb_m);

    // interactions per beam crossing
    fill(genericTrackGroup, mu_m);
  }
  
  if (m_doIP) {
    auto handle_vxContainer = SG::makeHandle(m_VxPrimContainerName, ctx);
    
    if (!handle_vxContainer.isPresent()) {
      ATH_MSG_DEBUG ("InDetGlobalPrimaryVertexMonAlg: StoreGate doesn't contain primary vertex container with key "+m_VxPrimContainerName.key());
      return StatusCode::SUCCESS;
    }
    if (!handle_vxContainer.isValid()) {
      ATH_MSG_ERROR ("InDetGlobalPrimaryVertexMonAlg: Could not retrieve primary vertex container with key "+m_VxPrimContainerName.key());
      return StatusCode::FAILURE;
    }
    
    const auto *vertexContainer = handle_vxContainer.cptr();
    
    xAOD::VertexContainer::const_iterator vxI = vertexContainer->begin();
    xAOD::VertexContainer::const_iterator vxE = vertexContainer->end();
    for (; vxI != vxE; ++vxI) {
      if ((*vxI)->type() == 1) {
	pvtx = (*vxI);
      }
    }
  }
  
  //
  // Start loop on tracks
  //

  ATH_MSG_DEBUG ("Start loop on tracks. Number of tracks " << trks->size());
  for (const Trk::Track* trksItr: *trks) {

    // Found track?!
    if ( !trksItr || trksItr->perigeeParameters() == nullptr )
      {
	ATH_MSG_DEBUG( "InDetAlignmentMonitoringRun3: NULL track pointer in collection" );
	continue;
      }

    // Select tracks
    if ( m_applyTrkSel and !m_trackSelection->accept( *trksItr) )
      continue; // track selection applied and failed 

    nTracks++;  
    
    float chisquared     = 0.;
    int DoF              = 0;
    float chi2oDoF       = -999;
    float trkd0          = -999;
    float Err_d0         = -999;
    float trkz0          = -999;
    float Err_z0         = -999;
    float trkphi         = -999;
    float Err_phi        = -999;
    float trktheta       = -999;
    float Err_theta      = -999;
    float Err_eta        = -999;
    float trketa         = -999;
    float qOverP         = -999;
    float Err_qOverP     = -999;
    float Err_Pt         = -999;
    float trkpt          = -999;
    float trkP           = -999;
    float charge         = 0;
    float trkd0c         = -999;
    float beamX          = 0;
    float beamY          = 0;
    float d0bscorr       = -999;
    int   layerDisk  = 99;
    int   sctSide = 99;
    int   modEta = 9999;
    int   modPhi = 9999;
 
    // get fit quality and chi2 probability of track
    const Trk::FitQuality* fitQual = trksItr->fitQuality();
    
    const Trk::Perigee* measPer = trksItr->perigeeParameters();
    const AmgSymMatrix(5)* covariance = measPer ? measPer->covariance() : nullptr;

    std::unique_ptr<Trk::ImpactParametersAndSigma> myIPandSigma=nullptr;


    if (m_doIP){

      //Get unbiased impact parameter
      if (pvtx) myIPandSigma = m_trackToVertexIPEstimator->estimate(trksItr->perigeeParameters(), pvtx, true);
    } 
    
    if (covariance == nullptr) {
      ATH_MSG_WARNING("No measured perigee parameters assigned to the track"); 
    }
    else{  
      AmgVector(5) perigeeParams = measPer->parameters(); 
      trkd0        = perigeeParams[Trk::d0];  
      trkz0        = perigeeParams[Trk::z0];    
      trkphi       = perigeeParams[Trk::phi0];  
      trktheta     = perigeeParams[Trk::theta];
      trketa       = measPer->eta(); 
      qOverP       = perigeeParams[Trk::qOverP]*1000.;  
      if(qOverP) trkP = 1/qOverP;
      trkpt        = measPer->pT()/1000.; 
      Err_d0       = Amg::error(*measPer->covariance(), Trk::d0); 
      Err_z0       = Amg::error(*measPer->covariance(), Trk::z0);
      Err_phi      = Amg::error(*measPer->covariance(), Trk::phi0);
      Err_theta    = Amg::error(*measPer->covariance(), Trk::theta);
      Err_eta      = Err_theta / sin(trktheta);
      Err_qOverP   = Amg::error(*measPer->covariance(), Trk::qOverP) * 1000;
      Err_Pt       = sin(trktheta) * Err_qOverP / pow(qOverP, 2);
      if (qOverP < 0) charge = -1;
      else charge=+1; 

      // correct the track d0 for the vertex position
      // would rather corrected for the beamline but could not find beamline
      trkd0c=trkd0-(yv*cos(trkphi)-xv*sin(trkphi));
      ATH_MSG_DEBUG("trkd0, trkd0c: " << trkd0 << ", " << trkd0c);

      // correct the track parameters for the beamspot position
      beamX = beamSpotX + tan(beamTiltX) * (trkz0-beamSpotZ);
      beamY = beamSpotY + tan(beamTiltY) * (trkz0-beamSpotZ);
      d0bscorr = trkd0 - ( -sin(trkphi)*beamX + cos(trkphi)*beamY );
    }    

    if (fitQual==nullptr) {
      ATH_MSG_WARNING("No fit quality assigned to the track"); 
    }       
    
    chisquared = (fitQual) ? fitQual->chiSquared() : -1.;
    DoF        = (fitQual) ? fitQual->numberDoF() : -1;
    if(DoF>0) chi2oDoF = chisquared/(float)DoF;

    if (trkphi<0) trkphi+=2*M_PI;
    
    ngTracks++;    
    ATH_MSG_DEBUG(nTracks << " is a good track!");  

    // fill lb histogram for each accepted track 
    auto lb_track_m = Monitored::Scalar<int>( "m_lb_track", lb );
    fill(genericTrackGroup, lb_track_m);	

    int nhpixB=0, nhpixECA=0, nhpixECC=0, nhsctB=0, nhsctECA=0, nhsctECC=0, nhtrtB=0, nhtrtECA=0, nhtrtECC=0;

    // loop over all hits on track
    ATH_MSG_VERBOSE ("  starting to loop over TSOS: " << trksItr->trackStateOnSurfaces()->size());
    for (const Trk::TrackStateOnSurface* tsos : *trksItr->trackStateOnSurfaces()) {
      //check that we have track parameters defined for the surface (pointer is not null)
      if(!(tsos->trackParameters())) {
	      ATH_MSG_DEBUG(" hit skipped because no associated track parameters");
        continue;
      }
      
      Identifier surfaceID;
      const Trk::MeasurementBase* mesb=tsos->measurementOnTrack();
      // hits, outliers
      if (mesb != nullptr && mesb->associatedSurface().associatedDetectorElement()!=nullptr) surfaceID = mesb->associatedSurface().associatedDetectorElement()->identify();

      // holes, perigee 
      else continue; 

      if ( tsos->type(Trk::TrackStateOnSurface::Measurement) ){   
	      //hit quality cuts for Si hits if tool is configured - default is NO CUTS
        if (m_idHelper->is_pixel(surfaceID) || m_idHelper->is_sct(surfaceID)) {
          if (m_doHitQuality) {
            ATH_MSG_DEBUG("applying hit quality cuts to Silicon hit...");
	    
            const Trk::RIO_OnTrack* hit = m_hitQualityTool->getGoodHit(tsos);
            if (hit == nullptr) {
              ATH_MSG_DEBUG("hit failed quality cuts and is rejected.");
              continue;
            } else {
              ATH_MSG_DEBUG("hit passed quality cuts");
            }
          } else {
            ATH_MSG_VERBOSE("hit quality cuts NOT APPLIED to Silicon hit.");
          }
        } // hit is Pixel or SCT
	
        // --- pixel hit count
        if (m_idHelper->is_pixel(surfaceID)){
          if(m_pixelID->barrel_ec(surfaceID)      ==  0){
            nhpixB++;
          }
          else if(m_pixelID->barrel_ec(surfaceID) ==  2)  nhpixECA++;
          else if(m_pixelID->barrel_ec(surfaceID) == -2) nhpixECC++;
        }
        // --- sct hit count
        else if (m_idHelper->is_sct(surfaceID)){
          if(m_sctID->barrel_ec(surfaceID)      ==  0){
            nhsctB++;
          }
          else if(m_sctID->barrel_ec(surfaceID) ==  2) nhsctECA++;
          else if(m_sctID->barrel_ec(surfaceID) == -2) nhsctECC++;
        }
        // --- trt hit count
        if (m_idHelper->is_trt(surfaceID)){
          int barrel_ec      = m_trtID->barrel_ec(surfaceID);
          if(barrel_ec == 1 || barrel_ec == -1 ) {
            nhtrtB++;
          }
          else if(barrel_ec ==  2){
            nhtrtECA++;
          }else if(barrel_ec == -2){
            nhtrtECC++;
          }
        }
        // filling hit maps 
        if (m_idHelper->is_pixel(surfaceID)){
          layerDisk = m_pixelID -> layer_disk(surfaceID);
          modEta = m_pixelID->eta_module(surfaceID); 
          modPhi = m_pixelID->phi_module(surfaceID);
          auto modEta_m = Monitored::Scalar<int>( "m_modEta", modEta );
          auto modPhi_m = Monitored::Scalar<int>( "m_modPhi", modPhi );    
          auto layerDisk_m = Monitored::Scalar<float>("m_layerDisk", layerDisk);
          
          if(m_pixelID->barrel_ec(surfaceID)      ==  0){ //pixel barrel hit
            fill(m_tools[m_measurements_vs_Eta_Phi_pix_b[layerDisk]], modEta_m, modPhi_m);
          }
          else if(m_pixelID->barrel_ec(surfaceID) ==  2){ //pixel endcap A hit
            fill(m_tools[m_measurements_vs_Eta_Phi_pix_ec[0]], layerDisk_m, modPhi_m);
          }
          else if(m_sctID->barrel_ec(surfaceID) == -2){ //pixel endcap C hit
            fill(m_tools[m_measurements_vs_Eta_Phi_pix_ec[1]], layerDisk_m, modPhi_m);
          }
        } 
        else if (m_idHelper->is_sct(surfaceID)){ 
          layerDisk = m_sctID->layer_disk(surfaceID);
          modEta = m_sctID->eta_module(surfaceID); 
          modPhi = m_sctID->phi_module(surfaceID);
          sctSide = m_sctID->side(surfaceID);
          auto modEta_m = Monitored::Scalar<int>( "m_modEta", modEta );
          auto modPhi_m = Monitored::Scalar<int>( "m_modPhi", modPhi );    
          auto layerDisk_m = Monitored::Scalar<float>("m_layerDisk", layerDisk);

          if(m_sctID->barrel_ec(surfaceID)      ==  0){ //SCT barrel hit
            if (sctSide == 0) {
              fill(m_tools[m_measurements_vs_Eta_Phi_sct_b_s0[layerDisk]], modEta_m, modPhi_m);
            } else {
              fill(m_tools[m_measurements_vs_Eta_Phi_sct_b_s1[layerDisk]], modEta_m, modPhi_m);
            }
          }
          else if(m_sctID->barrel_ec(surfaceID) ==  2){ //SCT endcap A hit
            if (sctSide == 0) {
              fill(m_tools[m_measurements_vs_Eta_Phi_sct_eca_s0[layerDisk]], modEta_m, modPhi_m);
            } else {
              fill(m_tools[m_measurements_vs_Eta_Phi_sct_eca_s1[layerDisk]], modEta_m, modPhi_m);
            }
          }
          else if(m_sctID->barrel_ec(surfaceID) == -2){ //SCT endcap C hit
            if (sctSide == 0) {
              fill(m_tools[m_measurements_vs_Eta_Phi_sct_ecc_s0[layerDisk]], modEta_m, modPhi_m);
            } else {
              fill(m_tools[m_measurements_vs_Eta_Phi_sct_ecc_s1[layerDisk]], modEta_m, modPhi_m);
            }
          }
        }

      } 
    }

    int nhpix= nhpixB +nhpixECA + nhpixECC;
    int nhsct= nhsctB +nhsctECA + nhsctECC;
    int nhtrt= nhtrtB +nhtrtECA + nhtrtECC;
    int nhits= nhpix+ nhsct+ nhtrt;

    auto nhits_per_track_m = Monitored::Scalar<float>( "m_nhits_per_track", nhits );
    fill(genericTrackGroup, nhits_per_track_m);
    //Pixel hits
    auto npixelhits_per_track_m = Monitored::Scalar<float>( "m_npixelhits_per_track", nhpix );
    auto npixelhits_per_track_barrel_m = Monitored::Scalar<float>( "m_npixelhits_per_track_barrel", nhpixB );
    fill(genericTrackGroup, npixelhits_per_track_barrel_m);
    auto npixelhits_per_track_eca_m = Monitored::Scalar<float>( "m_npixelhits_per_track_eca", nhpixECA );
    fill(genericTrackGroup, npixelhits_per_track_eca_m);
    auto npixelhits_per_track_ecc_m = Monitored::Scalar<float>( "m_npixelhits_per_track_ecc", nhpixECC );
    fill(genericTrackGroup, npixelhits_per_track_ecc_m);
    //SCT hits
    auto nscthits_per_track_m = Monitored::Scalar<float>( "m_nscthits_per_track", nhsct );
    auto nscthits_per_track_barrel_m = Monitored::Scalar<float>( "m_nscthits_per_track_barrel", nhsctB );
    fill(genericTrackGroup, nscthits_per_track_barrel_m);
    auto nscthits_per_track_eca_m = Monitored::Scalar<float>( "m_nscthits_per_track_eca", nhsctECA );
    fill(genericTrackGroup, nscthits_per_track_eca_m);
    auto nscthits_per_track_ecc_m = Monitored::Scalar<float>( "m_nscthits_per_track_ecc", nhsctECC );
    fill(genericTrackGroup, nscthits_per_track_ecc_m);
    //TRT hits
    auto ntrthits_per_track_m = Monitored::Scalar<float>( "m_ntrthits_per_track", nhtrt );
    auto ntrthits_per_track_barrel_m = Monitored::Scalar<float>( "m_ntrthits_per_track_barrel", nhtrtB );
    fill(genericTrackGroup, ntrthits_per_track_barrel_m);
    auto ntrthits_per_track_eca_m = Monitored::Scalar<float>( "m_ntrthits_per_track_eca", nhtrtECA );
    fill(genericTrackGroup, ntrthits_per_track_eca_m);
    auto ntrthits_per_track_ecc_m = Monitored::Scalar<float>( "m_ntrthits_per_track_ecc", nhtrtECC );
    fill(genericTrackGroup, ntrthits_per_track_ecc_m);

    auto chi2oDoF_m = Monitored::Scalar<float>( "m_chi2oDoF", chi2oDoF );
    fill(genericTrackGroup, chi2oDoF_m);
    auto eta_m = Monitored::Scalar<float>( "m_eta", trketa );
    auto errEta_m = Monitored::Scalar<float>( "m_errEta", Err_eta );
    fill(genericTrackGroup, errEta_m);

    // Eta for positive and negative tracks
    auto isTrkPositive = Monitored::Scalar<float>( "isTrkPositive", charge > 0 ? 1 : 0 );
    auto isTrkNegative = Monitored::Scalar<float>( "isTrkNegative", charge > 0 ? 0 : 1 );
  
    // z0
    auto z0_m = Monitored::Scalar<float>( "m_z0", trkz0 );
    auto errZ0_m = Monitored::Scalar<float>( "m_errZ0", Err_z0 );
    auto z0_bscorr_m = Monitored::Scalar<float>( "m_z0_bscorr", trkz0-beamSpotZ );
    float z0sintheta = trkz0*(sin(trktheta));
    auto z0sintheta_m = Monitored::Scalar<float>( "m_z0sintheta", z0sintheta );
    fill(genericTrackGroup, z0_m, errZ0_m, z0_bscorr_m, z0sintheta_m);

    //d0
    auto d0_m = Monitored::Scalar<float>( "m_d0", trkd0 );
    fill(genericTrackGroup, d0_m);
    auto errD0_m = Monitored::Scalar<float>( "m_errD0", Err_d0 );
    fill(genericTrackGroup, errD0_m);
    auto d0_bscorr_m = Monitored::Scalar<float>( "m_d0_bscorr", d0bscorr );

    // Phi
    auto phi_m = Monitored::Scalar<float>( "m_phi", trkphi );
    auto errPhi_m = Monitored::Scalar<float>( "m_errPhi", Err_phi );

    //d0 vs phi in barrel, End-Cap A, End-cap C
    auto isTrackBarrel = Monitored::Scalar<float>( "isTrackBarrel", fabs(trketa) < m_barrelEta ? 1 : 0 );
    auto isTrackECA = Monitored::Scalar<float>( "isTrackECA", trketa > m_barrelEta ? 1 : 0 );
    auto isTrackECC = Monitored::Scalar<float>( "isTrackECC", trketa < - m_barrelEta ? 1 : 0 );

    //pT and p
    float pT = charge*trkpt;
    auto pT_m = Monitored::Scalar<float>( "m_pT", pT );
    auto errPt_m = Monitored::Scalar<float>( "m_errPt", Err_Pt );
    auto pTRes_m = Monitored::Scalar<float>( "m_pTRes", std::fabs(Err_qOverP / qOverP) );

    //d0 (BS) vs Eta, vs Phi (Phi, Barrel, EndCap A, EndCap C), vs pT // Eta vs Npixhits_per_track, SCT, TRT // Eta for positive and negative tracks 
    fill(genericTrackGroup, npixelhits_per_track_m, nscthits_per_track_m, ntrthits_per_track_m, eta_m, isTrkPositive, isTrkNegative, d0_bscorr_m, phi_m, isTrackBarrel, isTrackECA, isTrackECC, errPhi_m, pT_m, errPt_m, pTRes_m);
    
    auto p_m = Monitored::Scalar<float>( "m_p", trkP );
    fill(genericTrackGroup, p_m);
  } //
  // end of loop on trks
  //

  // histo with the count of used(good) tracks
  auto ngTracks_m = Monitored::Scalar<float>( "m_ngTracks", ngTracks );
  fill(genericTrackGroup, ngTracks_m);
  
  ATH_MSG_DEBUG("Histogram filling completed for #good_tracks: " << ngTracks);

  return StatusCode::SUCCESS;
}

const xAOD::Vertex* IDAlignMonGenericTracksAlg::findAssociatedVertexTP(const std::map<const xAOD::TrackParticle*, const xAOD::Vertex*>& trackVertexMapTP, const xAOD::TrackParticle *track) const
{

  std::map<const xAOD::TrackParticle*, const xAOD::Vertex* >::const_iterator tpVx = trackVertexMapTP.find( track);

  if (tpVx == trackVertexMapTP.end() ){
    ATH_MSG_VERBOSE("Did not find the vertex. Returning 0");
    return nullptr;
  } 
  return (*tpVx).second;

}


const Trk::Track* IDAlignMonGenericTracksAlg::getTrkTrack(const Trk::VxTrackAtVertex *trkAtVx)
{

  //find the link to the TrackParticleBase
  const Trk::ITrackLink* trkLink = trkAtVx->trackOrParticleLink();
  const Trk::TrackParticleBase* trkPB(nullptr);
  if(nullptr!= trkLink){
    const Trk::LinkToTrackParticleBase* linktrkPB = dynamic_cast<const Trk::LinkToTrackParticleBase *>(trkLink);
    if(nullptr!= linktrkPB){
      if(linktrkPB->isValid()) trkPB = linktrkPB->cachedElement();
    }//end of dynamic_cast check
  }//end of ITrackLink existance check

  //cast to TrackParticle
  if(trkPB){
    if ( trkPB->trackElementLink()->isValid() ) {      
      // retrieve and refit original track
      const Trk::Track* trktrk = trkPB->originalTrack();
      return trktrk;
    } 
  } 
  return nullptr;
}



bool IDAlignMonGenericTracksAlg::fillVertexInformation(std::map<const xAOD::TrackParticle*, const xAOD::Vertex*>& trackVertexMapTP, const EventContext& ctx ) const
{
  ATH_MSG_DEBUG("Generic Tracks: fillVertexInformation(): Checking ");
  trackVertexMapTP.clear();

  // retrieving vertices
  auto handle_vxContainer = SG::makeHandle(m_VxPrimContainerName, ctx);
  
  if (!handle_vxContainer.isPresent()) {
    ATH_MSG_DEBUG ("InDetGlobalPrimaryVertexMonAlg: StoreGate doesn't contain primary vertex container with key "+m_VxPrimContainerName.key());
    return false;
  }
  if (!handle_vxContainer.isValid()) {
    ATH_MSG_ERROR ("InDetGlobalPrimaryVertexMonAlg: Could not retrieve primary vertex container with key "+m_VxPrimContainerName.key());
    return false;
  }

  const auto *vertexContainer = handle_vxContainer.cptr();
    
  for(const auto vtx : *vertexContainer) {
      auto tpLinks = vtx->trackParticleLinks();
      ATH_MSG_DEBUG("tpLinks size " << tpLinks.size());

      if (tpLinks.size() > 4 ) {
	for(const auto& link: tpLinks) {
	  const xAOD::TrackParticle *TP = *link;
          if(TP) {
            trackVertexMapTP.insert( std::make_pair( TP, vtx )  );
          }
        }
      }
    }

    return true;
}
