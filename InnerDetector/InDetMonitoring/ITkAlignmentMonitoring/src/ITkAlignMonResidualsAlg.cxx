/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ***************************************************************************************
// ITkAlignMonResidualsAlg.cxx
// AUTHORS: Beate Heinemann, Tobias Golling, Ben Cooper, John Alison
// Adapted to AthenaMT 2021-2022 by Per Johansson
// ***************************************************************************************

//main header
#include "ITkAlignMonResidualsAlg.h"

#include "TMath.h"
#include <cmath>
#include <sstream>
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"  // NB: the ITk strip identifier helper is still class SCT_ID

#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/Layer.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/TrackCollection.h"


// *********************************************************************
// Public Methods
// *********************************************************************

ITkAlignMonResidualsAlg::ITkAlignMonResidualsAlg( const std::string & name, ISvcLocator* pSvcLocator ) :
  AthMonitorAlgorithm(name, pSvcLocator),
  m_iUpdator ("Trk::KalmanUpdator"),
  m_propagator ("Trk::RungeKuttaPropagator"),
  m_residualPullCalculator( "Trk::ResidualPullCalculator/ResidualPullCalculator"),
  m_trackSelection( "InDet::InDetTrackSelectionTool/TrackSelectionTool", this),
  m_hitQualityTool(""){
  declareProperty("CheckRate"                 , m_checkrate=1000);
  declareProperty("iUpdator"                  , m_iUpdator);
  declareProperty("propagator"                , m_propagator);
  declareProperty("TrackSelectionTool"        , m_trackSelection);
  declareProperty("ResidualPullCalculatorTool", m_residualPullCalculator);
  declareProperty("HitQualityTool"            , m_hitQualityTool);
  declareProperty("ApplyTrackSelection"       , m_applyTrkSel = true);
  declareProperty("NPixelBarrelLayers"        , m_nPixBlayers);
  declareProperty("NStripBarrelLayers"          , m_nStripBlayers);
  declareProperty("NPixelEndcapLayers"        , m_nPixEClayers);
  declareProperty("NStripEndcapLayers"          , m_nStripEClayers);
  declareProperty("PixelBarrelModEtaShift"    , m_pixBModEtaShift);
  declareProperty("PixelBarrelModPhiShift"    , m_pixBModPhiShift);
  declareProperty("PixelEndcapModPhiShift"    , m_pixECModPhiShift);
  declareProperty("StripBarrelModEtaShift"      , m_stripBModEtaShift);
  declareProperty("StripBarrelModPhiShift"      , m_stripBModPhiShift);
  declareProperty("StripEndcapNmods"            , m_stripECNmods);
  declareProperty("StripEndcapModGap"           , m_stripECGap);
}

//---------------------------------------------------------------------------------------

ITkAlignMonResidualsAlg::~ITkAlignMonResidualsAlg() {}

StatusCode ITkAlignMonResidualsAlg::initialize()
{
  //initialize tools and services
  ATH_MSG_DEBUG("Calling initialize() to setup tools/services");
  StatusCode sc = setupTools();
  if (sc.isFailure()) {
    ATH_MSG_WARNING("Failed to initialize tools/services!");
    return StatusCode::SUCCESS;
  }
  else
    ATH_MSG_DEBUG("Successfully initialized tools/services");
  
  ATH_CHECK( m_tracksName.initialize() );
  ATH_CHECK( m_tracksKey.initialize() );

  m_pixResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualX", m_nPixBlayers);
  m_pixResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "PixResidualX_2DProf", m_nPixBlayers);
  m_pixResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualY", m_nPixBlayers);
  m_pixResidualY_2DProf = Monitored::buildToolMap<int>(m_tools, "PixResidualY_2DProf", m_nPixBlayers);
  m_pixPullX = Monitored::buildToolMap<int>(m_tools, "PixPullX", m_nPixBlayers);
  m_pixPullY = Monitored::buildToolMap<int>(m_tools, "PixPullY", m_nPixBlayers);
  m_pixResidualXvsEta = Monitored::buildToolMap<int>(m_tools, "PixResidualXvsEta", m_nPixBlayers);
  m_pixResidualYvsEta = Monitored::buildToolMap<int>(m_tools, "PixResidualYvsEta", m_nPixBlayers);
  m_pixResidualXvsPhi = Monitored::buildToolMap<int>(m_tools, "PixResidualXvsPhi", m_nPixBlayers);
  m_pixResidualYvsPhi = Monitored::buildToolMap<int>(m_tools, "PixResidualYvsPhi", m_nPixBlayers);
  m_pixECAResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualXECA", m_nPixEClayers);
  m_pixECAResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualYECA", m_nPixEClayers);
  m_pixECResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "PixResidualXEC_2DProf", 2);
  m_pixECResidualY_2DProf = Monitored::buildToolMap<int>(m_tools, "PixResidualYEC_2DProf", 2);
  m_pixECCResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualXECC", m_nPixEClayers);
  m_pixECCResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualYECC", m_nPixEClayers);
  m_stripResidualX = Monitored::buildToolMap<int>(m_tools, "StripResidualX", m_nStripBlayers);
  m_stripResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripResidualX_2DProf", m_nStripBlayers);
  m_strip_s0_ResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "Strip_s0_ResidualX_2DProf", m_nStripBlayers);
  m_strip_s1_ResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "Strip_s1_ResidualX_2DProf", m_nStripBlayers);
  m_stripECAResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripECAResidualX_2DProf", m_nStripEClayers);
  m_stripECA_s0_ResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripECA_s0_ResidualX_2DProf", m_nStripEClayers);
  m_stripECA_s1_ResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripECA_s1_ResidualX_2DProf", m_nStripEClayers);
  m_stripECCResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripECCResidualX_2DProf", m_nStripEClayers);
  m_stripECC_s0_ResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripECC_s0_ResidualX_2DProf", m_nStripEClayers);
  m_stripECC_s1_ResidualX_2DProf = Monitored::buildToolMap<int>(m_tools, "StripECC_s1_ResidualX_2DProf", m_nStripEClayers);
  m_stripPullX = Monitored::buildToolMap<int>(m_tools, "StripPullX", m_nStripBlayers);
  m_stripResidualXvsEta = Monitored::buildToolMap<int>(m_tools, "StripResidualXvsEta", m_nStripBlayers);
  m_stripResidualXvsPhi = Monitored::buildToolMap<int>(m_tools, "StripResidualXvsPhi", m_nStripBlayers);

  if ((int)m_pixBModEtaShift.size() < m_nPixBlayers ||
      (int)m_pixBModPhiShift.size() < m_nPixBlayers ||
      (int)m_pixECModPhiShift.size() < m_nPixEClayers ||
      (int)m_stripBModEtaShift.size() < m_nStripBlayers ||
      (int)m_stripBModPhiShift.size() < m_nStripBlayers) {
    ATH_MSG_ERROR("Module shift vectors are shorter than the configured layer counts");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("initialize() -- completed --");
  return AthMonitorAlgorithm::initialize();
}


//---------------------------------------------------------------------------------------

StatusCode ITkAlignMonResidualsAlg::fillHistograms( const EventContext& ctx ) const
{
  using namespace Monitored;
  
  ATH_MSG_DEBUG("fillHistograms() -- dealing with track collection: " << m_tracksName.key());
  
  // For histogram naming
  const auto & residualGroup = getGroup("Residuals");
  
  //counters
  float mu = 0.;
  int nTracks = 0;
  //recreates original behaviour...but...
  //calls this every time
  mu = lbAverageInteractionsPerCrossing(ctx);
  auto mu_m = Monitored::Scalar<float>("mu_m", 0.0);
  mu_m = mu;
  
  if (m_extendedPlots){
    fill("residualGroup", mu_m);
  }
  
  // Retrieving tracks
  auto tracks = SG::makeHandle(m_tracksName, ctx);
  if (not tracks.isValid()) {
    ATH_MSG_ERROR(m_tracksName.key() << " could not be retrieved");
    return StatusCode::RECOVERABLE;
  }
  
  //looping over tracks
  ATH_MSG_DEBUG ("IDAlignMonResidual: Start loop on tracks. Number of tracks " << tracks->size());
  for (const Trk::Track* trksItr: *tracks) {
    
    // Found track?!
    if ( !trksItr || trksItr->perigeeParameters() == nullptr ) {
      ATH_MSG_DEBUG( "ITkAlignmentMonitoring: NULL track pointer in collection" );
      continue;
    }
    
    // Select tracks
    if ( m_applyTrkSel and !m_trackSelection->accept(*trksItr) )
      continue;
    
    nTracks++;
    
    //check that all TSOS of track have track parameters defined (required to compute residuals/pulls)
    if(trackRequiresRefit(trksItr)){
      ATH_MSG_DEBUG("Not all TSOS contain track parameters - will be missing residuals/pulls");
    }
    else
      ATH_MSG_DEBUG("All TSOS of track " << nTracks << "/" << tracks->size() << " contain track parameters - Good!");
    
    //trackStateOnSurfaces is a vector of Trk::TrackStateOnSurface objects which contain information
    //on track at each (inner)detector surface it crosses eg hit used to fit track
    ATH_MSG_DEBUG( "** ITkAlignMonResiduals::fillHistograms() ** track: " << nTracks << "  has " << trksItr->trackStateOnSurfaces()->size() << " TrkSurfaces");
    
    int nHits =  0; //counts number of tsos from which we can define residual/pull
    int nTSOS = -1; //counts all TSOS on the track

    //looping over the hits of the track
    for (const Trk::TrackStateOnSurface* tsos : *trksItr->trackStateOnSurfaces()) {
      
      ++nTSOS;
      
      if (tsos == nullptr) {
        ATH_MSG_DEBUG("     TSOS (hit) = " << nTSOS << " is NULL ");
        continue;
      }
      
      //skipping outliers
      ATH_MSG_DEBUG(" --> testing if hit " << nTSOS << "/" << trksItr->trackStateOnSurfaces()->size() << " is a track measurement");
      if(!tsos->type(Trk::TrackStateOnSurface::Measurement)) {
	ATH_MSG_DEBUG("Skipping TSOS " << nTSOS << " because it is an outlier (or the first TSOS on the track)");
	continue;
      }
      
      const Trk::MeasurementBase* mesh =tsos->measurementOnTrack();
      ATH_MSG_DEBUG(" --> Defined hit measurementOnTrack() for hit: " << nTSOS << "/" << trksItr->trackStateOnSurfaces()->size() << " of track " << nTracks);
      
      //Trk::RIO_OnTrack object contains information on the hit used to fit the track at this surface
      const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>(mesh);
      ATH_MSG_DEBUG(" --> Going to retrieve the Trk::RIO_OnTrack for hit " << nTSOS);
      if (hit== nullptr) {
	//for some reason the first tsos has no associated hit - maybe because this contains the defining parameters?
	if (nHits >0) ATH_MSG_DEBUG("No hit associated with TSOS " << nTSOS);
	continue;
      }
      
      ATH_MSG_DEBUG(" --> Going to retrieve the track parameters of this TSOS: " << nTSOS);
      const Trk::TrackParameters* trackParameter = tsos->trackParameters();
      if(trackParameter==nullptr) {
        //if no TrackParameters for TSOS we cannot define residuals
        ATH_MSG_DEBUG(" Skipping TSOS " << nTSOS << " because it does not have TrackParameters");
        continue;
      }
      //trackParameter cannot be nullptr here
      const AmgSymMatrix(5)* TrackParCovariance = trackParameter->covariance();
      
      if(TrackParCovariance==nullptr) {
        //if no MeasuredTrackParameters the hit will not have associated convariance error matrix and will not
        //be able to define a pull or unbiased residual (errors needed for propagation)
        ATH_MSG_DEBUG("Skipping TSOS " << nTSOS << " because does not have MeasuredTrackParameters");
        continue;
      }
      
      ATH_MSG_DEBUG(
		    " --> going to define residuals and everything of TSOS  #" << nTSOS << "/" <<
		    trksItr->trackStateOnSurfaces()->size());
      
      float residualX = 9999.0;
      float residualY = 9999.0;
      float pullX = 9999.0;
      float pullY = 9999.0;
      float biasedResidualX = 9999.0;
      float biasedResidualY = 9999.0;
      int   detType    = 99;
      int   barrelEC   = 99;
      int   layerDisk  = 99;
      int   stripSide = 99;
      int   modEta = 9999;
      int   modPhi = 9999;
      
      const Identifier & hitId = hit->identify();
      if (m_idHelper->is_trt(hitId)) detType = 2;
      else if (m_idHelper->is_sct(hitId)) detType = 1;
      else if (m_idHelper->is_pixel(hitId)) detType = 0;
      else  detType = 99;
      
      //hits with detType = 0 are no Inner Detector hits -> skip
      if ( detType == 99) {
        ATH_MSG_DEBUG(" --> Hit " << nTSOS << " with detector type " << detType << " is not an Inner Detector hit -> skip this hit");
	continue;
      }
      
      //ITk has no TRT: any is_trt identifier here would indicate a misconfiguration
      if (detType == 2) {
        ATH_MSG_WARNING("TRT hit found on ITk geometry?! Skipping hit " << nTSOS);
        continue;
      }

      { //have identified a PIXEL or Strip hit
	if(m_doHitQuality) {
          ATH_MSG_DEBUG("applying hit quality cuts to Silicon hit...");
          hit = m_hitQualityTool->getGoodHit(tsos);
          if(hit==nullptr) {
            ATH_MSG_DEBUG("hit failed quality cuts and is rejected.");
            continue;
          }
          ATH_MSG_DEBUG("hit passed quality cuts");
        }
	else ATH_MSG_DEBUG("hit quality cuts NOT APPLIED to Silicon hit.");
	
        //determining Si module physical position (can modify residual calculation eg. Strip endcaps)
        if (detType==0){//pixel
          const Identifier& id = m_pixelID->wafer_id(hitId);
          barrelEC  = m_pixelID -> barrel_ec(id);
          layerDisk = m_pixelID -> layer_disk(id);
          modEta = m_pixelID->eta_module(id);  //For the endcaps these are the rings
          modPhi = m_pixelID->phi_module(id);
        }
        else {//strip. Since detType == 0 or detType == 1 here
          const Identifier& id = m_stripID->wafer_id(hitId);
          barrelEC  = m_stripID->barrel_ec(id);
          layerDisk = m_stripID->layer_disk(id);
          modEta = m_stripID->eta_module(id);
          modPhi = m_stripID->phi_module(id);
          stripSide = m_stripID->side(id);
        }
	
        //finding residuals
        if(trackParameter){//should always have TrackParameters since we now skip tracks with no MeasuredTrackParameters
	  
          ATH_MSG_DEBUG("Found Trk::TrackParameters " << trackParameter);
          
          double unbiasedResXY[4] = {9999.0,9999.0,9999.0,9999.0};
          double biasedResXY[4] = {9999.0,9999.0,9999.0,9999.0};
          
          //finding unbiased single residuals
          StatusCode sc;
          sc = getSiResiduals(trksItr,tsos,true,unbiasedResXY);
          if (sc.isFailure()) {
            ATH_MSG_DEBUG("Problem in determining unbiased residuals! Hit is skipped.");
            auto detType_m   = Monitored::Scalar<int>( "m_detType", detType);
            fill(residualGroup, detType_m);
            continue;
          }
          else
            ATH_MSG_DEBUG("unbiased residuals found ok");
          
          residualX = (float)unbiasedResXY[0];
          residualY = (float)unbiasedResXY[1];
          pullX     = (float)unbiasedResXY[2];
          pullY     = (float)unbiasedResXY[3];
          
          //finding biased single residuals (for interest)
          sc = getSiResiduals(trksItr,tsos,false,biasedResXY);
          if (sc.isFailure()) {
            ATH_MSG_DEBUG("Problem in determining biased residuals! Hit is skipped.");
            continue;
          }
          else
            ATH_MSG_DEBUG("biased residuals found ok");
          
          biasedResidualX = (float)biasedResXY[0];
          biasedResidualY = (float)biasedResXY[1];
          
        }
        else {
          ATH_MSG_DEBUG("No TrackParameters associated with Si TrkSurface "<< nTSOS << " - Hit is probably an outlier");
        }
      }//end-Pixel and Strip hits
      
      //--------------------------------------------
      //
      // Filling Residual Histograms for Pixel and Strip
      //
      //--------------------------------------------

      //Common for Pixel and Strip and other variables used
      auto si_residualx_m = Monitored::Scalar<float>( "m_si_residualx", 0.0);
      auto si_b_residualx_m = Monitored::Scalar<float>( "m_si_b_residualx", 0.0);
      auto si_barrel_resX_m = Monitored::Scalar<float>( "m_si_barrel_resX", 0.0);
      auto si_barrel_resY_m = Monitored::Scalar<float>( "m_si_barrel_resY", 0.0);
      auto si_barrel_pullX_m = Monitored::Scalar<float>( "m_si_barrel_pullX", 0.0);
      auto si_barrel_pullY_m = Monitored::Scalar<float>( "m_si_barrel_pullY", 0.0);
      auto si_eca_resX_m = Monitored::Scalar<float>( "m_si_eca_resX", 0.0);
      auto si_eca_resY_m = Monitored::Scalar<float>( "m_si_eca_resY", 0.0);
      auto si_eca_pullX_m = Monitored::Scalar<float>( "m_si_eca_pullX", 0.0);
      auto si_eca_pullY_m = Monitored::Scalar<float>( "m_si_eca_pullY", 0.0);
      auto si_ecc_resX_m = Monitored::Scalar<float>( "m_si_ecc_resX", 0.0);
      auto si_ecc_resY_m = Monitored::Scalar<float>( "m_si_ecc_resY", 0.0);
      auto si_ecc_pullX_m = Monitored::Scalar<float>( "m_si_ecc_pullX", 0.0);
      auto si_ecc_pullY_m = Monitored::Scalar<float>( "m_si_ecc_pullY", 0.0);
      auto residualX_m = Monitored::Scalar<float>( "m_residualX", residualX);
      auto residualY_m = Monitored::Scalar<float>( "m_residualY", residualY);
      auto modEta_m = Monitored::Scalar<int>( "m_modEta", modEta );
      auto modPhi_m = Monitored::Scalar<int>( "m_modPhi", modPhi );
      int lb       = GetEventInfo(ctx)->lumiBlock();
      auto lb_m    = Monitored::Scalar<int>( "m_lb", lb );
      auto layerDisk_m = Monitored::Scalar<float>("m_layerDisk", layerDisk);
      auto layerDisk_si_m = Monitored::Scalar<float>("m_layerDisk_si", 0);

      if (detType==0) {//filling pixel histograms
	ATH_MSG_DEBUG(" This is a PIXEL hit " << hitId  << " - filling histograms");
	
	si_residualx_m = residualX;
	fill(residualGroup, si_residualx_m);
	
	if(barrelEC==0){//filling pixel barrel histograms
	  if (layerDisk >= m_nPixBlayers) {
	    ATH_MSG_WARNING("Pixel barrel hit with layer " << layerDisk << " beyond configured NPixelBarrelLayers=" << m_nPixBlayers << " - skipping");
	    continue;
	  }

          //common Si plots
          si_b_residualx_m = residualX;
          fill(residualGroup, si_b_residualx_m);
          
          layerDisk_si_m = layerDisk;
          si_barrel_resX_m = residualX;
          si_barrel_resY_m = residualY;
          si_barrel_pullX_m = pullX;
          si_barrel_pullY_m = pullY;
          fill(residualGroup, layerDisk_si_m, si_barrel_resX_m, si_barrel_resY_m, si_barrel_pullX_m, si_barrel_pullY_m);
          
          //Pixel Residual plots
          auto pix_b_residualx_m = Monitored::Scalar<float>( "m_pix_b_residualx", residualX);
          auto pix_b_biased_residualx_m = Monitored::Scalar<float>( "m_pix_b_biased_residualx", biasedResidualX);
          auto pix_b_residualy_m = Monitored::Scalar<float>( "m_pix_b_residualy", residualY);
          auto pix_b_biased_residualy_m = Monitored::Scalar<float>( "m_pix_b_biased_residualy", biasedResidualY);
          fill(residualGroup, pix_b_residualx_m, pix_b_biased_residualx_m, pix_b_residualy_m, pix_b_biased_residualy_m);
          auto pix_b_residualsx_m = Monitored::Scalar<float>("m_pix_residualsx", residualX);
          fill(m_tools[m_pixResidualX[layerDisk]], pix_b_residualsx_m);
          fill(m_tools[m_pixResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, pix_b_residualsx_m);
          auto pix_b_residualsy_m = Monitored::Scalar<float>("m_pix_residualsy", residualY);
          fill(m_tools[m_pixResidualY[layerDisk]], pix_b_residualsy_m);
          fill(m_tools[m_pixResidualY_2DProf[layerDisk]], modEta_m, modPhi_m, pix_b_residualsy_m);
          auto pix_b_pullsx_m = Monitored::Scalar<float>("m_pix_pullsx", pullX);
          fill(m_tools[m_pixPullX[layerDisk]], pix_b_pullsx_m);
          auto pix_b_pullsy_m = Monitored::Scalar<float>("m_pix_pullsy", pullY);
          fill(m_tools[m_pixPullY[layerDisk]], pix_b_pullsy_m);

          //Residuals vs Eta and Phi
          fill(m_tools[m_pixResidualXvsEta[layerDisk]], modEta_m, residualX_m );
          fill(m_tools[m_pixResidualYvsEta[layerDisk]], modEta_m, residualY_m );
          fill(m_tools[m_pixResidualXvsPhi[layerDisk]], modPhi_m, residualX_m );
          fill(m_tools[m_pixResidualYvsPhi[layerDisk]], modPhi_m, residualY_m );
          
          auto residualX_barrel_m = Monitored::Scalar<float>( "m_residualX_barrel", residualX);
          auto residualY_barrel_m = Monitored::Scalar<float>( "m_residualY_barrel", residualY);
          auto modPhiShift_barrel_m = Monitored::Scalar<int>( "m_modPhiShift_barrel", modPhi + m_pixBModPhiShift[layerDisk] );
          auto modEtaShift_barrel_m = Monitored::Scalar<int>( "m_modEtaShift_barrel", modEta + m_pixBModEtaShift[layerDisk] );
          fill(residualGroup, modPhiShift_barrel_m, residualX_barrel_m, residualY_barrel_m);
          fill(residualGroup, modEtaShift_barrel_m, residualX_barrel_m, residualY_barrel_m);
	}
	else if(barrelEC==2){//Pixel endcap disks (three for Run 1-3 ID, nine for ITk)
	  if (layerDisk >= m_nPixEClayers) {
	    ATH_MSG_WARNING("Pixel endcap hit with disk " << layerDisk << " beyond configured NPixelEndcapLayers=" << m_nPixEClayers << " - skipping");
	    continue;
	  }

         //Common Si plots
          layerDisk_si_m = layerDisk;
          si_eca_resX_m = residualX;
          si_eca_resY_m = residualY;
          si_eca_pullX_m = pullX;
          si_eca_pullY_m = pullY;
          fill(residualGroup, layerDisk_si_m, si_eca_resX_m, si_eca_resY_m, si_eca_pullX_m, si_eca_pullY_m);
          
          //Pixel Residual plots
          auto pix_eca_residualx_m = Monitored::Scalar<float>( "m_pix_eca_residualx", residualX);
          auto pix_ec_residualx_m = Monitored::Scalar<float>( "m_pix_ec_residualx", residualX);
          fill(m_tools[m_pixECResidualX_2DProf[0]], layerDisk_m , modPhi_m, pix_ec_residualx_m);
          auto pix_eca_residualy_m = Monitored::Scalar<float>( "m_pix_eca_residualy", residualY);
          auto pix_ec_residualy_m = Monitored::Scalar<float>( "m_pix_ec_residualy", residualY);
          fill(residualGroup, pix_eca_residualx_m, pix_eca_residualy_m);
          fill(m_tools[m_pixECResidualY_2DProf[0]], layerDisk_m, modPhi_m, pix_ec_residualy_m);
          auto pix_eca_pullx_m = Monitored::Scalar<float>( "m_pix_eca_pullx", pullX);
          auto pix_eca_pully_m = Monitored::Scalar<float>( "m_pix_eca_pully", pullY);
          fill(residualGroup, pix_eca_pullx_m, pix_eca_pully_m);

          //Residuals vs Eta and Phi
          auto residualX_eca_m = Monitored::Scalar<float>( "m_residualX_eca", residualX );
          auto residualY_eca_m = Monitored::Scalar<float>( "m_residualY_eca", residualY );
          auto modPhiShift_eca_m = Monitored::Scalar<int>( "m_modPhiShift_eca", modPhi + m_pixECModPhiShift[layerDisk]);
          fill(m_tools[m_pixECAResidualX[layerDisk]], modPhi_m, pix_eca_residualx_m);
          fill(m_tools[m_pixECAResidualY[layerDisk]], modPhi_m, pix_eca_residualy_m);
          fill(residualGroup, modPhiShift_eca_m, residualX_eca_m, residualY_eca_m);
        }
	else if(barrelEC==-2){
	  if (layerDisk >= m_nPixEClayers) {
	    ATH_MSG_WARNING("Pixel endcap hit with disk " << layerDisk << " beyond configured NPixelEndcapLayers=" << m_nPixEClayers << " - skipping");
	    continue;
	  }

          //Common Si plots
          layerDisk_si_m = layerDisk;
          si_ecc_resX_m = residualX;
          si_ecc_resY_m = residualY;
          si_ecc_pullX_m = pullX;
          si_ecc_pullY_m = pullY;
          fill(residualGroup, layerDisk_si_m, si_ecc_resX_m, si_ecc_resY_m, si_ecc_pullX_m, si_ecc_pullY_m);
          
          //Pixel Residual plots
          auto pix_ecc_residualx_m = Monitored::Scalar<float>( "m_pix_ecc_residualx", residualX);
          auto pix_ec_residualx_m = Monitored::Scalar<float>( "m_pix_ec_residualx", residualX);
          fill(m_tools[m_pixECResidualX_2DProf[1]], layerDisk_m , modPhi_m, pix_ec_residualx_m);
          auto pix_ecc_residualy_m = Monitored::Scalar<float>( "m_pix_ecc_residualy", residualY);
          auto pix_ec_residualy_m = Monitored::Scalar<float>( "m_pix_ec_residualy", residualY);
          fill(residualGroup, pix_ecc_residualx_m, pix_ecc_residualy_m);
          fill(m_tools[m_pixECResidualY_2DProf[1]], layerDisk_m, modPhi_m, pix_ec_residualy_m);
          auto pix_ecc_pullx_m = Monitored::Scalar<float>( "m_pix_ecc_pullx", pullX);
          auto pix_ecc_pully_m = Monitored::Scalar<float>( "m_pix_ecc_pully", pullY);
          fill(residualGroup, pix_ecc_pullx_m, pix_ecc_pully_m);
          
          //Residuals vs Eta and Phi
          auto residualX_ecc_m = Monitored::Scalar<float>( "m_residualX_ecc", residualX);
          auto residualY_ecc_m = Monitored::Scalar<float>( "m_residualY_ecc", residualY);
          auto modPhiShift_ecc_m = Monitored::Scalar<int>( "m_modPhiShift_ecc", modPhi + m_pixECModPhiShift[layerDisk] );
          fill(m_tools[m_pixECCResidualX[layerDisk]], modPhi_m, pix_ecc_residualx_m);
          fill(m_tools[m_pixECCResidualY[layerDisk]], modPhi_m, pix_ecc_residualy_m);
          fill(residualGroup, modPhiShift_ecc_m, residualX_ecc_m, residualY_ecc_m);
	}
      }
      else if (detType==1) {//filling Strip histograms
        si_residualx_m = residualX;
        fill(residualGroup, si_residualx_m);
        
        ATH_MSG_DEBUG(" This is a Strip hit " << hitId << " - filling histograms");
        
        if(barrelEC==0){//filling Strip barrel histograms
          if (layerDisk >= m_nStripBlayers) {
            ATH_MSG_WARNING("Strip barrel hit with layer " << layerDisk << " beyond configured NStripBarrelLayers=" << m_nStripBlayers << " - skipping");
            continue;
          }

          //common Si plots
          si_b_residualx_m = residualX;
          fill(residualGroup, si_b_residualx_m);

          layerDisk_si_m = m_nPixBlayers + 2 * layerDisk + stripSide;
          si_barrel_resX_m = residualX;
          si_barrel_pullX_m = pullX;
          fill(residualGroup, layerDisk_si_m, si_barrel_resX_m, si_barrel_pullX_m);
          
          //Strip Residual plots
          auto strip_b_residualx_m = Monitored::Scalar<float>( "m_strip_b_residualx", residualX);
          fill(residualGroup, strip_b_residualx_m);
          auto strip_b_biased_residualx_m = Monitored::Scalar<float>( "m_strip_b_biased_residualx", biasedResidualX);
          auto strip_b_residualsx_m = Monitored::Scalar<float>("m_strip_residualsx", residualX);
          fill(m_tools[m_stripResidualX[layerDisk]], strip_b_residualsx_m);
          fill(m_tools[m_stripResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_b_residualsx_m);
          auto strip_b_pullsx_m = Monitored::Scalar<float>("m_strip_pullsx", pullX);
          fill(m_tools[m_stripPullX[layerDisk]], strip_b_pullsx_m);
          if (stripSide == 0) {
            fill(m_tools[m_strip_s0_ResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_b_residualsx_m);
          } else {
            fill(m_tools[m_strip_s1_ResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_b_residualsx_m);
          }

          //Residuals vs Eta and Phi
          fill(m_tools[m_stripResidualXvsEta[layerDisk]], modEta_m, residualX_m);
          fill(m_tools[m_stripResidualXvsPhi[layerDisk]], modPhi_m, residualX_m);
          
          auto residualX_strip_barrel_m = Monitored::Scalar<float>( "m_residualX_strip_barrel", residualX);
          auto modPhiShift_strip_barrel_m = Monitored::Scalar<int>( "m_modPhiShift_strip_barrel", modPhi + m_stripBModPhiShift[layerDisk] );
          auto modEtaShift_strip_barrel_m = Monitored::Scalar<int>( "m_modEtaShift_strip_barrel", modEta + m_stripBModEtaShift[layerDisk] );
          fill(residualGroup, modPhiShift_strip_barrel_m, modEtaShift_strip_barrel_m, residualX_strip_barrel_m);
        } // end Strip barrel
        
        else if(barrelEC==2){//Strip endcap disks (nine for Run 1-3 ID, six for ITk)
          if (layerDisk >= m_nStripEClayers) {
            ATH_MSG_WARNING("Strip endcap hit with disk " << layerDisk << " beyond configured NStripEndcapLayers=" << m_nStripEClayers << " - skipping");
            continue;
          }

          //Common Si plots
          layerDisk_si_m = m_nPixEClayers + 2 * layerDisk + stripSide;
          si_eca_resX_m = residualX;
          si_eca_pullX_m = pullX;
          fill(residualGroup, layerDisk_si_m, si_eca_resX_m, si_eca_pullX_m);
          
          //Strip Residual plots
          auto strip_eca_residualx_m = Monitored::Scalar<float>( "m_strip_eca_residualx", residualX);
          fill(m_tools[m_stripECAResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_eca_residualx_m);
          auto strip_eca_pullx_m = Monitored::Scalar<float>( "m_strip_eca_pullx", pullX);
          fill(residualGroup, strip_eca_residualx_m, strip_eca_pullx_m);
          if (stripSide == 0) {
            fill(m_tools[m_stripECA_s0_ResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_eca_residualx_m);
          } else {
            fill(m_tools[m_stripECA_s1_ResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_eca_residualx_m);
          }
          
          //Residuals vs Eta and Phi
          auto residualX_strip_eca_m = Monitored::Scalar<float>( "m_residualX_strip_eca", residualX);
          auto modPhiShift_strip_eca_m = Monitored::Scalar<int>( "m_modPhiShift_strip_eca", modPhi + layerDisk * (m_stripECGap + m_stripECNmods) );
          fill(residualGroup,  modPhiShift_strip_eca_m, residualX_strip_eca_m);
        } // end Strip end-cap A
        
        else if(barrelEC==-2){//start Strip end-cap C
          if (layerDisk >= m_nStripEClayers) {
            ATH_MSG_WARNING("Strip endcap hit with disk " << layerDisk << " beyond configured NStripEndcapLayers=" << m_nStripEClayers << " - skipping");
            continue;
          }

          //Common Si plots
          layerDisk_si_m = m_nPixEClayers + 2 * layerDisk + stripSide;
          si_ecc_resX_m = residualX;
          si_ecc_pullX_m = pullX;
          fill(residualGroup, layerDisk_si_m, si_ecc_resX_m, si_ecc_pullX_m);
          
          //Strip Residual plots
          auto strip_ecc_residualx_m = Monitored::Scalar<float>( "m_strip_ecc_residualx", residualX);
          fill(m_tools[m_stripECCResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_ecc_residualx_m);
          auto strip_ecc_pullx_m = Monitored::Scalar<float>( "m_strip_ecc_pullx", pullX);
          fill(residualGroup, strip_ecc_residualx_m, strip_ecc_pullx_m);
          if (stripSide == 0) {
            fill(m_tools[m_stripECC_s0_ResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_ecc_residualx_m);
          } else {
            fill(m_tools[m_stripECC_s1_ResidualX_2DProf[layerDisk]], modEta_m, modPhi_m, strip_ecc_residualx_m);
          }
          
          //Residuals vs Eta and Phi
          auto residualX_strip_ecc_m = Monitored::Scalar<float>( "m_residualX_strip_ecc", residualX);
          auto modPhiShift_strip_ecc_m = Monitored::Scalar<int>( "m_modPhiShift_strip_ecc", modPhi + layerDisk * (m_stripECGap + m_stripECNmods) );
          fill(residualGroup,  modPhiShift_strip_ecc_m, residualX_strip_ecc_m);
        } // end Strip end-cap C
      }// end of Strip
      ++nHits;
      //++nHitsEvent;
    }//end of loop on track surfaces
  } // end of loop on tracks
  
  ATH_MSG_DEBUG("Number of tracks : "<< nTracks);
  
  return StatusCode::SUCCESS;
}

//__________________________________________________________________________
StatusCode  ITkAlignMonResidualsAlg::getSiResiduals(const Trk::Track* track, const Trk::TrackStateOnSurface* tsos, bool unBias, double* results) const
{
  if (!m_doPulls) return StatusCode::FAILURE;
  
  StatusCode sc = StatusCode::SUCCESS;
  
  double residualX = -9999.0;
  double residualY = -9999.0;
  double pullX = -9999.0;
  double pullY = -9999.0;
  
  //extract the hit object from the tsos
  const Trk::MeasurementBase* mesh =tsos->measurementOnTrack();
  const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>(mesh);
  
  //get the unbiased track parameters (can fail if no MeasuredTrackParameters exists)
  std::unique_ptr <Trk::TrackParameters> trackParameterUnbiased{};
  if(unBias) trackParameterUnbiased = getUnbiasedTrackParameters(track,tsos);
  
  //updator can fail in defining unbiased parameters, in which case we use biased
  std::unique_ptr <Trk::TrackParameters> trackParameterForResiduals{};
  if(trackParameterUnbiased){
    trackParameterForResiduals = std:: move(trackParameterUnbiased);
  }
  else {
    //use the original biased track parameters
    std::unique_ptr <Trk::TrackParameters> uTrkPtr = tsos->trackParameters()->uniqueClone();
    trackParameterForResiduals = std::move(uTrkPtr);
  }
  
  if (!m_residualPullCalculator.empty()) {
    
    if (hit && trackParameterForResiduals) {
      
      ATH_MSG_DEBUG(" got hit and track parameters ");
      
      std::optional<Trk::ResidualPull> residualPull = std::nullopt;
      if(unBias) residualPull = m_residualPullCalculator->residualPull(mesh, trackParameterForResiduals.get(), Trk::ResidualPull::Unbiased);
      else residualPull = m_residualPullCalculator->residualPull(mesh, trackParameterForResiduals.get(), Trk::ResidualPull::Biased);
      
      ATH_MSG_DEBUG(" got hit and track parameters...done ");
      if (residualPull) {
	
	ATH_MSG_DEBUG(" got residual pull object");
	residualX = residualPull->residual()[Trk::loc1];
	if(residualPull->isPullValid()) pullX = residualPull->pull()[Trk::loc1];
	else {
	  ATH_MSG_DEBUG("ResidualPullCalculator finds invalid X Pull!!!");
	  sc = StatusCode::FAILURE;
	}
	
	if (residualPull->dimension() >= 2){
	  
	  ATH_MSG_DEBUG(" residualPull dim >= 2");
	  residualY = residualPull->residual()[Trk::loc2];
	  
	  ATH_MSG_DEBUG(" residual Y = " << residualY);
	  if(residualPull->isPullValid()) pullY = residualPull->pull()[Trk::loc2];
	  else {
	    ATH_MSG_DEBUG("ResidualPullCalculator finds invalid Y Pull!!!");
	    sc = StatusCode::FAILURE;
	  }
	}
      }
      else {
	ATH_MSG_DEBUG("ResidualPullCalculator failed!");
	sc = StatusCode::FAILURE;
      }
    }
  }
  
  // for Strip modules the residual pull calculator only finds the (rotated) Rphi residual
  // for each of the Strip sides; residualPull->dimension()==1 always.
  
  //std::pair <double, double> result(residualX, residualY);
  results[0] = residualX;
  results[1] = residualY;
  results[2] = pullX;
  results[3] = pullY;
  
  if(pullX!=pullX || pullY!=pullY){
    ATH_MSG_DEBUG("ResidualPullCalculator finds Pull=NAN!!!");
    sc = StatusCode::FAILURE;
  }
  
  return sc;
  
}


//---------------------------------------------------------------------------------------
std::unique_ptr <Trk::TrackParameters> ITkAlignMonResidualsAlg::getUnbiasedTrackParameters(const Trk::Track* trkPnt, const Trk::TrackStateOnSurface* tsos) const
{
  
  std::unique_ptr <Trk::TrackParameters> TrackParams{};
  std::unique_ptr <Trk::TrackParameters> UnbiasedTrackParams{};
  std::unique_ptr <Trk::TrackParameters> PropagatedTrackParams{};
  std::unique_ptr <Trk::TrackParameters> OtherSideUnbiasedTrackParams{};
  
  //controls if the Strip residuals will be 'truly' unbiased - removing also the opposite side hit.
  bool trueUnbiased = true;
  
  Identifier surfaceID;
  
  
  ATH_MSG_VERBOSE("original track parameters: " << *(tsos->trackParameters()) );
  ATH_MSG_VERBOSE("Trying to unbias track parameters.");
  
  const Trk::RIO_OnTrack* hitOnTrack = dynamic_cast <const Trk::RIO_OnTrack*>(tsos->measurementOnTrack());
  
  if (hitOnTrack != nullptr) surfaceID = hitOnTrack->identify();
  
  
  // if Strip Hit and TrueUnbiased then remove other side hit first
  if (surfaceID.is_valid() && trueUnbiased && m_idHelper->is_sct(surfaceID)) {  //there's no TrueUnbiased for non-Strip (pixel) hits)
    ATH_MSG_VERBOSE("Entering True Unbiased loop.");
    
    // check if other module side was also hit and try to remove other hit as well
    const Trk::TrackStateOnSurface* OtherModuleSideHit(nullptr);
    const Identifier waferID = m_stripID->wafer_id(surfaceID);
    const IdentifierHash waferHash = m_stripID->wafer_hash(waferID);
    IdentifierHash otherSideHash;
    m_stripID->get_other_side(waferHash, otherSideHash);
    const Identifier OtherModuleSideID = m_stripID->wafer_id(otherSideHash);
    
    for (const Trk::TrackStateOnSurface* TempTsos : *trkPnt->trackStateOnSurfaces()) {
      
      const Trk::RIO_OnTrack* TempHitOnTrack = dynamic_cast <const Trk::RIO_OnTrack*>(TempTsos->measurementOnTrack());
      if (TempHitOnTrack != nullptr) {
	if (m_stripID->wafer_id(TempHitOnTrack->identify()) == OtherModuleSideID) {
	  ATH_MSG_VERBOSE("True unbiased residual. Removing OtherModuleSide Hit " << m_idHelper->show_to_string(OtherModuleSideID,nullptr,'/') );
	  OtherModuleSideHit = TempTsos;
	}
      }
    }
    
    if (OtherModuleSideHit) {
      const Trk::TrackParameters* OMSHmeasuredTrackParameter = OtherModuleSideHit->trackParameters();
      const AmgSymMatrix(5)* OMSHmeasuredTrackParameterCov = OMSHmeasuredTrackParameter ? OMSHmeasuredTrackParameter->covariance() : nullptr;
      
      // check that the hit on the other module side has measuredtrackparameters, otherwise it cannot be removed from the track
      if (OMSHmeasuredTrackParameterCov) {
	ATH_MSG_VERBOSE("OtherSideTrackParameters: " << *(OtherModuleSideHit->trackParameters()) );
	OtherSideUnbiasedTrackParams = m_iUpdator->removeFromState(*(OtherModuleSideHit->trackParameters()),
	 							   OtherModuleSideHit->measurementOnTrack()->localParameters(),
	 							   OtherModuleSideHit->measurementOnTrack()->localCovariance());
	
	if (OtherSideUnbiasedTrackParams) {
	  ATH_MSG_VERBOSE("Unbiased OtherSideTrackParameters: " << *OtherSideUnbiasedTrackParams);
	  
	  
	  const Trk::Surface* TempSurface = &(OtherModuleSideHit->measurementOnTrack()->associatedSurface());
	  
	  const Trk::MagneticFieldProperties* TempField = nullptr;
	  if (TempSurface)
	    {
	      ATH_MSG_VERBOSE("After OtherSide surface call. Surface exists");
	      if (TempSurface->associatedLayer())
		{
		  ATH_MSG_VERBOSE("TempSurface->associatedLayer() exists");
		  if(TempSurface->associatedLayer()->enclosingTrackingVolume())
		    {
		      ATH_MSG_VERBOSE("TempSurface->associatedLayer()->enclosingTrackingVolume exists");
		      
		      TempField = dynamic_cast <const Trk::MagneticFieldProperties*>(TempSurface->associatedLayer()->enclosingTrackingVolume());
		      ATH_MSG_VERBOSE("After MagneticFieldProperties cast");
		      ATH_MSG_VERBOSE("Before other side unbiased propagation");
		      
		      if (TempSurface->associatedLayer() && TempField) PropagatedTrackParams = m_propagator->propagate(
														       Gaudi::Hive::currentContext(),
														       *OtherSideUnbiasedTrackParams,
														       tsos->measurementOnTrack()->associatedSurface(),
														       Trk::anyDirection, false,
														       *TempField,
														       Trk::nonInteracting);
		      
		    } else {
		    ATH_MSG_VERBOSE("TempSurface->associatedLayer()->enclosingTrackingVolume does not exist");
		  }
		} else {
		ATH_MSG_VERBOSE("TempSurface->associatedLayer() does not exist");
	      }
	    } else {
	    ATH_MSG_VERBOSE("After OtherSide surface call. Surface does not exist");
	  }
	  
	  ATH_MSG_VERBOSE("After other side unbiased propagation");
	  if (PropagatedTrackParams) {
	    ATH_MSG_VERBOSE("Propagated Track Parameters: " << *PropagatedTrackParams);
	  } else {
	    ATH_MSG_DEBUG("Propagation of unbiased OtherSideParameters failed");
	  }
	} else {
	  ATH_MSG_DEBUG("RemoveFromState did not work for OtherSideParameters");
	}
      } else {
	ATH_MSG_VERBOSE("No OtherModuleSideHit Measured Track Parameters found");
      }
    } else {
      ATH_MSG_VERBOSE("No OtherModuleSideHit found");
    }
  }
  
  // if propagation failed or no TrueUnbiased or no Strip then use original TrackParams
  if (!PropagatedTrackParams) {
    std::unique_ptr <Trk::TrackParameters> uTrkPtr = tsos->trackParameters()->uniqueClone();
    PropagatedTrackParams = std::move(uTrkPtr);
  }
  
  UnbiasedTrackParams =
    m_iUpdator
    ->removeFromState(*PropagatedTrackParams,
		      tsos->measurementOnTrack()->localParameters(),
		      tsos->measurementOnTrack()->localCovariance());
  
  if (UnbiasedTrackParams) {
    if(surfaceID.is_valid() ) ATH_MSG_VERBOSE("Unbiased residual. Removing original Hit " << m_idHelper->show_to_string(surfaceID,nullptr,'/') );
    ATH_MSG_VERBOSE("Unbiased Trackparameters: " << *UnbiasedTrackParams);
    
    TrackParams = std::move(UnbiasedTrackParams);
    
  } else { // Unbiasing went awry.
    ATH_MSG_WARNING("RemoveFromState did not work, using original TrackParameters");
    
    std::unique_ptr <Trk::TrackParameters> uTrkPtr = tsos->trackParameters()->uniqueClone();
    TrackParams = std::move(uTrkPtr);
  }
  
  return TrackParams;
  
}

//---------------------------------------------------------------------------------------
StatusCode ITkAlignMonResidualsAlg::setupTools()
{
  //initializing tools
  
  ATH_MSG_DEBUG("In setupTools()");
  
  StatusCode sc;
  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_MSG_DEBUG("Initialized PixelIDHelper");

  ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));
  ATH_MSG_DEBUG("Initialized StripIDHelper");

  //ID Helper
  ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));
  
  ATH_CHECK(m_iUpdator.retrieve());
  ATH_MSG_DEBUG("Retrieved iUpdator tool " << m_iUpdator);

  if (m_propagator.retrieve().isFailure()) {
    ATH_MSG_WARNING("Can not retrieve Propagator tool of type " << m_propagator.typeAndName());
    return StatusCode::FAILURE;
  } else ATH_MSG_INFO("Retrieved tool " << m_propagator.typeAndName());
  
  if (m_trackSelection.retrieve().isFailure()) {
    ATH_MSG_WARNING("Can not retrieve TrackSelection tool of type " << m_trackSelection.typeAndName());
    return StatusCode::FAILURE;
  } else ATH_MSG_INFO("Retrieved tool " << m_trackSelection.typeAndName());;
  
  if (m_residualPullCalculator.empty()) {
    ATH_MSG_DEBUG("No residual/pull calculator for general hit residuals configured.");
    ATH_MSG_DEBUG("It is recommended to give R/P calculators to the det-specific tool handle lists then.");
    m_doPulls = false;
    ATH_CHECK(m_residualPullCalculator.retrieve( DisableTool{!m_doPulls} ));
  } else if (m_residualPullCalculator.retrieve().isFailure()) {
    ATH_MSG_WARNING("Could not retrieve "<< m_residualPullCalculator << " (to calculate residuals and pulls) ");
    m_doPulls = false;
    
  } else {
    ATH_MSG_DEBUG("Generic hit residuals&pulls will be calculated in one or both available local coordinates");
    m_doPulls = true;
  }
  
  if (m_hitQualityTool.empty()) {
    ATH_MSG_DEBUG("No hit quality tool configured - not hit quality cuts will be imposed");
    m_doHitQuality = false;
    ATH_CHECK(m_hitQualityTool.retrieve( DisableTool{!m_doHitQuality} ));
  } else if (m_hitQualityTool.retrieve().isFailure()) {
    ATH_MSG_WARNING("Could not retrieve " << m_hitQualityTool << " to apply hit quality cuts to Si hits");
    m_doHitQuality = false;
  } else {
    ATH_MSG_DEBUG("Hit quality tool setup - hit quality cuts will be applied to Si hits");
    m_doHitQuality = true;
  }
  
  
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
bool ITkAlignMonResidualsAlg::trackRequiresRefit(const Trk::Track* track) const
{
  
  // Checks to see if any of the measurements on track do not have track parameters associated
  // (as happens for certain track collections in e.g. ESD)
  // If this is the case we cannot define residuals and track needs to be refitted (return true)
  
  bool refitTrack = false;
  
  int nHits = 0;
  int nHitsNoParams = 0;
  
  ATH_MSG_DEBUG("Testing track to see if requires refit...");
  
  for (const Trk::TrackStateOnSurface* tsos : *track->trackStateOnSurfaces()) {
    
    if(tsos == nullptr) continue;
    
    //skipping outliers
    if(!tsos->type(Trk::TrackStateOnSurface::Measurement)) continue;
    
    const Trk::MeasurementBase* mesh =tsos->measurementOnTrack();
    if (mesh==nullptr) continue;
    const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>(mesh);
    if (hit==nullptr) continue;
    
    ++nHits;
    
    const Trk::TrackParameters* trackParameter = tsos->trackParameters();
    if(trackParameter==nullptr) ++nHitsNoParams; //if no TrackParameters for TSOS we cannot define residuals
    
  }
  
  ATH_MSG_DEBUG("Total nhits on track (excluding outliers) = " << nHits << ", nhits without trackparameters = " << nHitsNoParams);
  
  if(nHitsNoParams>0) {
    refitTrack = true;
    ATH_MSG_DEBUG("Track Requires refit to get residuals!!!");
  }
  
  return refitTrack;
}

//--------------------------------------------------------------------------------------------
