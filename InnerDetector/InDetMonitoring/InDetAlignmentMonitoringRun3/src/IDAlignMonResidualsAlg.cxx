/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ***************************************************************************************
// IDAlignMonResidualsAlg.cxx
// AUTHORS: Beate Heinemann, Tobias Golling, Ben Cooper, John Alison
// Adapted to AthenaMT 2021-2022 by Per Johansson
// ***************************************************************************************

//main header
#include "IDAlignMonResidualsAlg.h"

#include "TMath.h"
#include <cmath>
#include <sstream>
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "InDetPrepRawData/SiCluster.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetPrepRawData/SCT_Cluster.h"

#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/ResidualPull.h"

#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/Layer.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/TrackCollection.h"

#include "TRT_ConditionsServices/ITRT_ConditionsSvc.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"


// *********************************************************************
// Public Methods
// *********************************************************************

IDAlignMonResidualsAlg::IDAlignMonResidualsAlg( const std::string & name, ISvcLocator* pSvcLocator ) :
  AthMonitorAlgorithm(name, pSvcLocator),
  m_trtcaldbTool("TRT_CalDbTool", this),
  m_trackSelection( "InDet::InDetTrackSelectionTool/TrackSelectionTool", this),
  m_extendedPlots(false)
{
  m_iUpdator = ToolHandle<Trk::IUpdator>("Trk::KalmanUpdator");
  m_propagator = ToolHandle<Trk::IPropagator>("Trk::RungeKuttaPropagator");
  m_residualPullCalculator = ToolHandle<Trk::IResidualPullCalculator>(
    "Trk::ResidualPullCalculator/ResidualPullCalculator");
  m_hitQualityTool = ToolHandle<IInDetAlignHitQualSelTool>("");

  declareProperty("CheckRate"                 , m_checkrate=1000);
  declareProperty("ITRT_CalDbTool"            , m_trtcaldbTool);
  declareProperty("iUpdator"                  , m_iUpdator);
  declareProperty("propagator"                , m_propagator);
  declareProperty("TrackSelectionTool"        , m_trackSelection);
  declareProperty("ResidualPullCalculatorTool", m_residualPullCalculator);
  declareProperty("HitQualityTool"            , m_hitQualityTool);
  declareProperty("Pixel_Manager"             , m_Pixel_Manager);
  declareProperty("SCT_Manager"               , m_SCT_Manager);
  declareProperty("ApplyTrackSelection"       , m_applyTrkSel = true);
}

//---------------------------------------------------------------------------------------

IDAlignMonResidualsAlg::~IDAlignMonResidualsAlg() {}

StatusCode IDAlignMonResidualsAlg::initialize()
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

  /** Get TRTCalDbTool */
  ATH_CHECK( m_trtcaldbTool.retrieve() );

  ATH_CHECK( m_tracksName.initialize() );
  ATH_CHECK( m_tracksKey.initialize() );

  m_pixResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualX", m_nSiBlayers);
  m_pixResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualY", m_nSiBlayers);
  m_pixPullX = Monitored::buildToolMap<int>(m_tools, "PixPullX", m_nSiBlayers);
  m_pixPullY = Monitored::buildToolMap<int>(m_tools, "PixPullY", m_nSiBlayers);
  m_pixResidualXvsEta = Monitored::buildToolMap<int>(m_tools, "PixResidualXvsEta", m_nSiBlayers);
  m_pixResidualYvsEta = Monitored::buildToolMap<int>(m_tools, "PixResidualYvsEta", m_nSiBlayers);
  m_pixResidualXvsPhi = Monitored::buildToolMap<int>(m_tools, "PixResidualXvsPhi", m_nSiBlayers);
  m_pixResidualYvsPhi = Monitored::buildToolMap<int>(m_tools, "PixResidualYvsPhi", m_nSiBlayers);
  m_pixECAResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualXECA", m_nPixEClayers);
  m_pixECAResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualYECA", m_nPixEClayers);
  m_pixECCResidualX = Monitored::buildToolMap<int>(m_tools, "PixResidualXECC", m_nPixEClayers);
  m_pixECCResidualY = Monitored::buildToolMap<int>(m_tools, "PixResidualYECC", m_nPixEClayers);
  m_sctResidualX = Monitored::buildToolMap<int>(m_tools, "SCTResidualX", m_nSiBlayers);
  m_sctPullX = Monitored::buildToolMap<int>(m_tools, "SCTPullX", m_nSiBlayers);
  m_sctResidualXvsEta = Monitored::buildToolMap<int>(m_tools, "SCTResidualXvsEta", m_nSiBlayers);
  m_sctResidualXvsPhi = Monitored::buildToolMap<int>(m_tools, "SCTResidualXvsPhi", m_nSiBlayers);
  m_trtBPredictedR = Monitored::buildToolMap<int>(m_tools, "TRTPredictedRB", m_nTRTBlayers);
  m_trtBMeasuredR = Monitored::buildToolMap<int>(m_tools, "TRTMeasuredRB", m_nTRTBlayers);
  m_trtBResidualR = Monitored::buildToolMap<int>(m_tools, "TRTResidualRB", m_nTRTBlayers);
  m_trtBPullR = Monitored::buildToolMap<int>(m_tools, "TRTPullRB", m_nTRTBlayers);
  m_trtBResidualRNoTube = Monitored::buildToolMap<int>(m_tools, "TRTResidualRNoTubeB", m_nTRTBlayers);
  m_trtBPullRNoTube = Monitored::buildToolMap<int>(m_tools, "TRTPullRNoTubeB", m_nTRTBlayers);
  m_trtBLR = Monitored::buildToolMap<int>(m_tools, "TRTLRB", m_nTRTBlayers);
  m_trtBResVsEta= Monitored::buildToolMap<std::vector<int>>(m_tools, "TRTResVsEtaB", m_nTRTBlayers,m_nTRTBlayers);
  m_trtBResVsPhiSec= Monitored::buildToolMap<std::vector<int>>(m_tools, "TRTResVsPhiSecB", m_nTRTBlayers,m_nTRTBlayers);
  m_trtBLRVsPhiSec= Monitored::buildToolMap<std::vector<int>>(m_tools, "TRTLRVsPhiSecB", m_nTRTBlayers,m_nTRTBlayers);
  m_trtECPredictedR = Monitored::buildToolMap<int>(m_tools, "TRTPredictedREC", m_nTRTEClayers);
  m_trtECMeasuredR = Monitored::buildToolMap<int>(m_tools, "TRTMeasuredREC", m_nTRTEClayers);
  m_trtECResidualR = Monitored::buildToolMap<int>(m_tools, "TRTResidualREC", m_nTRTEClayers);
  m_trtECPullR = Monitored::buildToolMap<int>(m_tools, "TRTPullREC", m_nTRTEClayers);
  m_trtECResidualRNoTube = Monitored::buildToolMap<int>(m_tools, "TRTResidualRNoTubeEC", m_nTRTEClayers);
  m_trtECPullRNoTube = Monitored::buildToolMap<int>(m_tools, "TRTPullRNoTubeEC", m_nTRTEClayers);
  m_trtECLR = Monitored::buildToolMap<int>(m_tools, "TRTLREC", m_nTRTEClayers);
  m_trtECResVsEta= Monitored::buildToolMap<int>(m_tools, "TRTResVsEtaEC", m_nTRTEClayers);
  m_trtECResVsPhiSec= Monitored::buildToolMap<int>(m_tools, "TRTResVsPhiEC", m_nTRTEClayers);
  m_trtECLRVsPhiSec= Monitored::buildToolMap<int>(m_tools, "TRTLRVsPhiEC", m_nTRTEClayers);

  ATH_MSG_DEBUG("initialize() -- completed --");
  return AthMonitorAlgorithm::initialize();
}


//---------------------------------------------------------------------------------------

StatusCode IDAlignMonResidualsAlg::fillHistograms( const EventContext& ctx ) const
{
  using namespace Monitored;

  ATH_MSG_DEBUG("fillHistograms() -- dealing with track collection: " << m_tracksName.key());

  // For histogram naming
  auto residualGroup = getGroup("Residuals");

  //counters
  bool hasBeenCalledThisEvent=false;
  float mu = 0.;
  int nTracks = 0;

  mu = lbAverageInteractionsPerCrossing(ctx);
  auto mu_m = Monitored::Scalar<float>("mu_m", 0.0);
  if (!hasBeenCalledThisEvent){
    mu = lbAverageInteractionsPerCrossing(ctx);
    mu_m = mu;
    hasBeenCalledThisEvent=true;
  }
  else
    mu = -999;

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
    if ( !trksItr || trksItr->perigeeParameters() == nullptr )
      {
	ATH_MSG_DEBUG( "InDetAlignmentMonitoringRun3: NULL track pointer in collection" );
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
    ATH_MSG_DEBUG(
      "** IDAlignMonResiduals::fillHistograms() ** track: " << nTracks << "  has " << trksItr->trackStateOnSurfaces()->size() <<
        " TrkSurfaces");


    int nHits =  0;//counts number of tsos from which we can define residual/pull
    int nTSOS = -1;//counts all TSOS on the track

    //looping over the hits
    for (const Trk::TrackStateOnSurface* tsos : *trksItr->trackStateOnSurfaces()) {

      ++nTSOS;

      //trackStateOnSurfaces is a vector of Trk::TrackStateOnSurface objects which contain information
      //on track at each (inner)detector surface it crosses eg hit used to fit track
      ATH_MSG_DEBUG(
      "** IDAlignMonResiduals::fillHistograms() ** track: " << nTracks << "  has " << trksItr->trackStateOnSurfaces()->size() <<
        " TrkSurfaces");

      if (tsos == nullptr) {
        ATH_MSG_DEBUG("     TSOS (hit) = " << nTSOS << " is NULL ");
        continue;
      }

      //skipping outliers
      ATH_MSG_DEBUG(
        " --> testing hit " << nTSOS << "/" << trksItr->trackStateOnSurfaces()->size() << " to be measurement type");
      if(!tsos->type(Trk::TrackStateOnSurface::Measurement)) {
	ATH_MSG_DEBUG("Skipping TSOS " << nTSOS << " because it is an outlier (or the first TSOS on the track)");
	continue;
      }

      const Trk::MeasurementBase* mesh =tsos->measurementOnTrack();
       ATH_MSG_DEBUG(
        " --> Defined hit measurementOnTrack() for hit: " << nTSOS << "/" << trksItr->trackStateOnSurfaces()->size() << " of track " <<
          nTracks);

       //Trk::RIO_OnTrack object contains information on the hit used to fit the track at this surface
       const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>(mesh);
       ATH_MSG_DEBUG(" --> Going to retrive the Trk::RIO_OnTrack for hit " << nTSOS);
       if (hit== nullptr) {
	//for some reason the first tsos has no associated hit - maybe because this contains the defining parameters?
	if (nHits >0) ATH_MSG_DEBUG("No hit associated with TSOS " << nTSOS);
	continue;
      }

      ATH_MSG_DEBUG(" --> Going to retrive the track parameters of this TSOS: " << nTSOS);
      const Trk::TrackParameters* trackParameter = tsos->trackParameters();
      if(trackParameter==nullptr) {
	//if no TrackParameters for TSOS we cannot define residuals
	ATH_MSG_DEBUG(" Skipping TSOS " << nTSOS << " because it does not have TrackParameters");
	continue;
      }

      const AmgSymMatrix(5)* TrackParCovariance = trackParameter ? trackParameter->covariance() : nullptr;

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
      int sctSide = 99;
      int   modEta = 9999;
      int   modPhi = 9999;

      const Identifier & hitId = hit->identify();
      ATH_MSG_DEBUG("Defining hit Identifier ");
      if (m_idHelper->is_trt(hitId)) detType = 2;
      else if (m_idHelper->is_sct(hitId)) detType = 1;
      else  detType = 0;


      //TRT hits: detType = 2
      if (detType == 2) {
        ATH_MSG_DEBUG("** IDAlignMonResidualsAlg::fillHistograms() ** Hit is from the TRT, finding residuals... ");
        bool isTubeHit = (mesh->localCovariance()(Trk::locX, Trk::locX) > 1.0);
        const Trk::TrackParameters* trackParameter = tsos->trackParameters();
        float hitR = hit->localParameters()[Trk::driftRadius];
        float trketa = tsos->trackParameters()->eta();
        float pullR = -9.9;

        const Identifier& id = m_trtID->layer_id(hitId);
        int barrel_ec = m_trtID->barrel_ec(id);
        int layer_or_wheel = m_trtID->layer_or_wheel(id);
        int phi_module = m_trtID->phi_module(id);

        //finding residuals
        if (!trackParameter) {
          ATH_MSG_WARNING("No TrackParameters associated with TRT TrkSurface " << nTSOS);
          continue;
        }
        ATH_MSG_DEBUG("Found Trk::TrackParameters for hit " << nTSOS << " --> TRT hit (detType= " << detType);

        //getting unbiased track parameters by removing the hit from the track and refitting
      	//std::unique_ptr <Trk::TrackParameters> trackParameterUnbiased;
      	auto trackParameterUnbiased = getUnbiasedTrackParameters(trksItr, tsos);

        if (!trackParameterUnbiased) {//updator can fail
          ATH_MSG_WARNING("Cannot define unbiased parameters for hit, skipping it.");
          continue;
        }
        ATH_MSG_DEBUG(" --> TRT UnBiased TrackParameters of hit " << nTSOS << " FOUND");

        float predictR = trackParameterUnbiased->parameters()[Trk::locR];

        const Trk::MeasurementBase* mesh = tsos->measurementOnTrack();
        std::optional<Trk::ResidualPull> residualPull =
            m_residualPullCalculator->residualPull(mesh,
                                                   trackParameterUnbiased.get(),
                                                   Trk::ResidualPull::Unbiased);

        if (residualPull) {
          pullR = residualPull->pull()[Trk::locR];
        } else {
          ATH_MSG_DEBUG(" no covariance of the track parameters given, can not calculate pull!");
        }

        //delete trackParameterUnbiased;

        float residualR = hitR - predictR;

        const InDet::TRT_DriftCircleOnTrack* trtCircle =
          dynamic_cast<const InDet::TRT_DriftCircleOnTrack*>(tsos->measurementOnTrack());

	if (trtCircle != nullptr) {
          ATH_MSG_DEBUG("** fillHistograms() ** Filling TRT HISTOS for hit/tsos " << nTSOS);

          ATH_MSG_DEBUG(" fillHistograms() ** filling TRT histos:"
                        << "  Barrel/EndCap: " << barrel_ec
                        << "  layer/wheel: " << layer_or_wheel
                        << "  phi: " << phi_module
                        << "  Residual: " << residualR);
          /** filling TRT histograms */
          fillTRTHistograms(barrel_ec
      	  		    , layer_or_wheel
      	  		    , phi_module
      	  		    , predictR
      	  		    , hitR
      	  		    , residualR
      	  		    , pullR
      	  		    , isTubeHit
      	  		    , trketa);
      	}
      }
      else {//have identified pixel or SCT hit
	ATH_MSG_DEBUG("** fillHistograms() ** Hit is pixel or SCT, type: " << detType);

	if(m_doHitQuality) {
	  ATH_MSG_DEBUG("applying hit quality cuts to Silicon hit...");

	  hit = m_hitQualityTool->getGoodHit(tsos);
	  if(hit==nullptr) {
	    ATH_MSG_DEBUG("hit failed quality cuts and is rejected.");
	    continue;
	  }
	  ATH_MSG_DEBUG("hit passed quality cuts");
	} else
	  ATH_MSG_DEBUG("hit quality cuts NOT APPLIED to Silicon hit.");

	//determining Si module physical position (can modify residual calculation eg. SCT endcaps)
	if (detType==0){//pixel
	  const Identifier& id = m_pixelID->wafer_id(hitId);
	  barrelEC  = m_pixelID -> barrel_ec(id);
	  layerDisk = m_pixelID -> layer_disk(id);
	  modEta = m_pixelID->eta_module(id);  //For the endcaps these are the rings
          modPhi = m_pixelID->phi_module(id);
	}
	else {//sct. Since detType == 0 or detType == 1 here
	  const Identifier& id = m_sctID->wafer_id(hitId);
	  barrelEC  = m_sctID->barrel_ec(id);
	  layerDisk = m_sctID->layer_disk(id);
	  modEta = m_sctID->eta_module(id);
          modPhi = m_sctID->phi_module(id);
	  sctSide = m_sctID->side(id);
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

	}else
	  ATH_MSG_DEBUG("No TrackParameters associated with Si TrkSurface "<< nTSOS << " - Hit is probably an outlier");
      }


      //--------------------------------------------
      //
      // Filling Residual Histograms for Pixel and SCT
      //
      //--------------------------------------------

      //Common for Pixel and SCT and other variables used
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
	  int ModEtaShift[4] = {0, 30, 53, 76};
          int ModPhiShift[4] = {0, 24, 56, 104};

	  //common Si plots
	  si_b_residualx_m = residualX;
	  fill(residualGroup, si_b_residualx_m);

	  layerDisk_si_m = layerDisk;
	  si_barrel_resX_m = residualX;
	  fill(residualGroup, layerDisk_si_m, si_barrel_resX_m);
	  si_barrel_resY_m = residualY;
	  fill(residualGroup, layerDisk_si_m, si_barrel_resY_m);
	  si_barrel_pullX_m = pullX;
	  fill(residualGroup, layerDisk_si_m, si_barrel_pullX_m);
	  si_barrel_pullY_m = pullY;
	  fill(residualGroup, layerDisk_si_m, si_barrel_pullY_m);

	  //Pixel Residual plots
	  auto pix_b_residualx_m = Monitored::Scalar<float>( "m_pix_b_residualx", residualX);
	  fill(residualGroup, pix_b_residualx_m);
	  auto pix_b_biased_residualx_m = Monitored::Scalar<float>( "m_pix_b_biased_residualx", biasedResidualX);
	  fill(residualGroup, pix_b_biased_residualx_m);
	  auto pix_b_residualy_m = Monitored::Scalar<float>( "m_pix_b_residualy", residualY);
	  fill(residualGroup, pix_b_residualy_m);
	  auto pix_b_biased_residualy_m = Monitored::Scalar<float>( "m_pix_b_biased_residualy", biasedResidualY);
	  fill(residualGroup, pix_b_biased_residualy_m);
	  auto pix_b_residualsx_m = Monitored::Scalar<float>("m_pix_residualsx", residualX);
	  fill(m_tools[m_pixResidualX[layerDisk]], pix_b_residualsx_m);
	  auto pix_b_residualsy_m = Monitored::Scalar<float>("m_pix_residualsy", residualY);
	  fill(m_tools[m_pixResidualY[layerDisk]], pix_b_residualsy_m);
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
	  auto modPhiShift_barrel_m = Monitored::Scalar<int>( "m_modPhiShift_barrel", modPhi + ModPhiShift[layerDisk] );
	  auto modEtaShift_barrel_m = Monitored::Scalar<int>( "m_modEtaShift_barrel", modEta + ModEtaShift[layerDisk] );
	  fill(residualGroup, modPhiShift_barrel_m, residualX_barrel_m);
	  fill(residualGroup, modPhiShift_barrel_m, residualY_barrel_m);
	  fill(residualGroup, modEtaShift_barrel_m, residualX_barrel_m);
	  fill(residualGroup, modEtaShift_barrel_m, residualY_barrel_m);

	}
	else if(barrelEC==2){//three Pixel endcap disks from 0-2
	  int ModPhiShift[3] = {0, 55, 110};

	  //Common Si plots
	  layerDisk_si_m = layerDisk;
	  si_eca_resX_m = residualX;
	  fill(residualGroup, layerDisk_si_m, si_eca_resX_m);
	  si_eca_resY_m = residualY;
	  fill(residualGroup, layerDisk_si_m, si_eca_resY_m);
	  si_eca_pullX_m = pullX;
	  fill(residualGroup, layerDisk_si_m, si_eca_pullX_m);
	  si_eca_pullY_m = pullY;
	  fill(residualGroup, layerDisk_si_m, si_eca_pullY_m);

	  //Pixel Residual plots
	  auto pix_eca_residualx_m = Monitored::Scalar<float>( "m_pix_eca_residualx", residualX);
	  fill(residualGroup, pix_eca_residualx_m);
	  auto pix_eca_residualy_m = Monitored::Scalar<float>( "m_pix_eca_residualy", residualY);
	  fill(residualGroup, pix_eca_residualy_m);
	  auto pix_eca_pullx_m = Monitored::Scalar<float>( "m_pix_eca_pullx", pullX);
	  fill(residualGroup, pix_eca_pullx_m);
	  auto pix_eca_pully_m = Monitored::Scalar<float>( "m_pix_eca_pully", pullY);
	  fill(residualGroup, pix_eca_pully_m);

	  //Residuals vs Eta and Phi
	  auto residualX_eca_m = Monitored::Scalar<float>( "m_residualX_eca", residualX );
	  auto residualY_eca_m = Monitored::Scalar<float>( "m_residualY_eca", residualY );
	  auto modPhiShift_eca_m = Monitored::Scalar<int>( "m_modPhiShift_eca", modPhi + ModPhiShift[layerDisk]);
	  fill(m_tools[m_pixECAResidualX[layerDisk]], modPhi_m, pix_eca_residualx_m);
	  fill(m_tools[m_pixECAResidualY[layerDisk]], modPhi_m, pix_eca_residualy_m);
	  fill(residualGroup, modPhiShift_eca_m, residualX_eca_m);
	  fill(residualGroup, modPhiShift_eca_m, residualY_eca_m);

	}
	else if(barrelEC==-2){
	  int ModPhiShift[3] = {0, 55, 110};

	  //Common Si plots
	  layerDisk_si_m = layerDisk;
	  si_ecc_resX_m = residualX;
	  fill(residualGroup, layerDisk_si_m, si_ecc_resX_m);
	  si_ecc_resY_m = residualY;
	  fill(residualGroup, layerDisk_si_m, si_ecc_resY_m);
	  si_ecc_pullX_m = pullX;
	  fill(residualGroup, layerDisk_si_m, si_ecc_pullX_m);
	  si_ecc_pullY_m = pullY;
	  fill(residualGroup, layerDisk_si_m, si_ecc_pullY_m);

	  //Pixel Residual plots
	  auto pix_ecc_residualx_m = Monitored::Scalar<float>( "m_pix_ecc_residualx", residualX);
	  fill(residualGroup, pix_ecc_residualx_m);
	  auto pix_ecc_residualy_m = Monitored::Scalar<float>( "m_pix_ecc_residualy", residualY);
	  fill(residualGroup, pix_ecc_residualy_m);
	  auto pix_ecc_pullx_m = Monitored::Scalar<float>( "m_pix_ecc_pullx", pullX);
	  fill(residualGroup, pix_ecc_pullx_m);
	  auto pix_ecc_pully_m = Monitored::Scalar<float>( "m_pix_ecc_pully", pullY);
	  fill(residualGroup, pix_ecc_pully_m);

	  //Residuals vs Eta and Phi
	  auto residualX_ecc_m = Monitored::Scalar<float>( "m_residualX_ecc", residualX);
	  auto residualY_ecc_m = Monitored::Scalar<float>( "m_residualY_ecc", residualY);
	  auto modPhiShift_ecc_m = Monitored::Scalar<int>( "m_modPhiShift_ecc", modPhi + ModPhiShift[layerDisk] );
	  fill(m_tools[m_pixECCResidualX[layerDisk]], modPhi_m, pix_ecc_residualx_m);
	  fill(m_tools[m_pixECCResidualY[layerDisk]], modPhi_m, pix_ecc_residualy_m);
	  fill(residualGroup, modPhiShift_ecc_m, residualX_ecc_m);
	  fill(residualGroup, modPhiShift_ecc_m, residualY_ecc_m);

	}
      }
      else if (detType==1) {//filling SCT histograms
	si_residualx_m = residualX;
	fill(residualGroup, si_residualx_m);

	ATH_MSG_DEBUG(" This is an SCT hit " << hitId << " - filling histograms");

	if(barrelEC==0){//filling SCT barrel histograms
	  int ModPhiShift[4] = {0, 42, 92, 150};
	  int ModEtaShift[4] = {0, 23, 46, 69};

	  //common Si plots
	  si_b_residualx_m = residualX;
	  fill(residualGroup, si_b_residualx_m);

	  layerDisk_si_m = 4 + 2 * layerDisk + sctSide;
	  si_barrel_resX_m = residualX;
	  fill(residualGroup, layerDisk_si_m, si_barrel_resX_m);
	  si_barrel_pullX_m = pullX;
	  fill(residualGroup, layerDisk_si_m, si_barrel_pullX_m);

	  //SCT Residual plots
	  auto sct_b_residualx_m = Monitored::Scalar<float>( "m_sct_b_residualx", residualX);
	  fill(residualGroup, sct_b_residualx_m);
	  auto sct_b_biased_residualx_m = Monitored::Scalar<float>( "m_sct_b_biased_residualx", biasedResidualX);
	  auto sct_b_residualsx_m = Monitored::Scalar<float>("m_sct_residualsx", residualX);
	  fill(m_tools[m_sctResidualX[layerDisk]], sct_b_residualsx_m);
	  auto sct_b_pullsx_m = Monitored::Scalar<float>("m_sct_pullsx", pullX);
	  fill(m_tools[m_sctPullX[layerDisk]], sct_b_pullsx_m);

	  //Residuals vs Eta and Phi
	  fill(m_tools[m_sctResidualXvsEta[layerDisk]], modEta_m, residualX_m);
	  fill(m_tools[m_sctResidualXvsPhi[layerDisk]], modPhi_m, residualX_m);

	  auto residualX_sct_barrel_m = Monitored::Scalar<float>( "m_residualX_sct_barrel", residualX);
	  auto modPhiShift_sct_barrel_m = Monitored::Scalar<int>( "m_modPhiShift_sct_barrel", modPhi + ModPhiShift[layerDisk] );
	  auto modEtaShift_sct_barrel_m = Monitored::Scalar<int>( "m_modEtaShift_sct_barrel", modEta + ModEtaShift[layerDisk] );
	  fill(residualGroup, modPhiShift_sct_barrel_m, residualX_sct_barrel_m);
	  fill(residualGroup, modEtaShift_sct_barrel_m, residualX_sct_barrel_m);


	} else if(barrelEC==2){//nine SCT endcap disks from 0-8
	  int Nmods = 52;
	  int gap_sct = 10;

	  //Common Si plots
	  layerDisk_si_m = 3 + 2 * layerDisk + sctSide;
	  si_eca_resX_m = residualX;
	  fill(residualGroup, layerDisk_si_m, si_eca_resX_m);
	  si_eca_pullX_m = pullX;
	  fill(residualGroup, layerDisk_si_m, si_eca_pullX_m);

	  //SCT Residual plots
	  auto sct_eca_residualx_m = Monitored::Scalar<float>( "m_sct_eca_residualx", residualX);
	  fill(residualGroup, sct_eca_residualx_m);
	  auto sct_eca_pullx_m = Monitored::Scalar<float>( "m_sct_eca_pullx", pullX);
	  fill(residualGroup, sct_eca_pullx_m);

	  //Residuals vs Eta and Phi
	  auto residualX_sct_eca_m = Monitored::Scalar<float>( "m_residualX_sct_eca", residualX);
	  auto modPhiShift_sct_eca_m = Monitored::Scalar<int>( "m_modPhiShift_sct_eca", modPhi + layerDisk * (gap_sct + Nmods) );
	  fill(residualGroup,  modPhiShift_sct_eca_m, residualX_sct_eca_m);

	} else if(barrelEC==-2){
	  int Nmods = 52;
	  int gap_sct = 10;

	  //Common Si plots
	  layerDisk_si_m = 3 + 2 * layerDisk + sctSide;
	  si_ecc_resX_m = residualX;
	  fill(residualGroup, layerDisk_si_m, si_ecc_resX_m);
	  si_ecc_pullX_m = pullX;
	  fill(residualGroup, layerDisk_si_m, si_ecc_pullX_m);

	  //SCT Residual plots
	  auto sct_ecc_residualx_m = Monitored::Scalar<float>( "m_sct_ecc_residualx", residualX);
	  fill(residualGroup, sct_ecc_residualx_m);
	  auto sct_ecc_pullx_m = Monitored::Scalar<float>( "m_sct_ecc_pullx", pullX);
	  fill(residualGroup, sct_ecc_pullx_m);

	  //Residuals vs Eta and Phi
	  auto residualX_sct_ecc_m = Monitored::Scalar<float>( "m_residualX_sct_ecc", residualX);
	  auto modPhiShift_sct_ecc_m = Monitored::Scalar<int>( "m_modPhiShift_sct_ecc", modPhi + layerDisk * (gap_sct + Nmods) );
	  fill(residualGroup,  modPhiShift_sct_ecc_m, residualX_sct_ecc_m);

	}
      }// end of SCT
      ++nHits;
      //++nHitsEvent;
    }//end of loop on track surfaces
  } // end of loop on tracks


  ATH_MSG_DEBUG("Number of tracks : "<< nTracks);

  return StatusCode::SUCCESS;
}

//__________________________________________________________________________
StatusCode  IDAlignMonResidualsAlg::getSiResiduals(const Trk::Track* track, const Trk::TrackStateOnSurface* tsos, bool unBias, double* results) const
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

  // for SCT modules the residual pull calculator only finds the (rotated) Rphi residual
  // for each of the SCT sides; residualPull->dimension()==1 always.

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


void IDAlignMonResidualsAlg::fillTRTHistograms(int barrel_ec, int layer_or_wheel, int phi_module, float predictR, float hitR, float residualR, float pullR, bool isTubeHit, float trketa) const {
  bool LRcorrect = (predictR * hitR > 0);

  //Need to correct the TRT residual on the C-side.
  if (barrel_ec == -1) {
    residualR *= -1;
  }

  if (barrel_ec == 1 || barrel_ec == -1)
    fillTRTBarrelHistograms(barrel_ec
			    , layer_or_wheel
			    , phi_module
			    , predictR
			    , hitR
			    , residualR
			    , pullR
			    , LRcorrect
			    , isTubeHit
			    , trketa);

  /** Filling EndCapA histograms */
  if (barrel_ec == 2 || barrel_ec == -2)
    fillTRTEndcapHistograms(barrel_ec
  			    , phi_module
  			    , predictR
  			    , hitR
  			    , residualR
  			    , pullR
  			    , LRcorrect
  			    , isTubeHit
  			    , trketa);

  return;
}

//Filling barrel histograms
void IDAlignMonResidualsAlg::fillTRTBarrelHistograms(int barrel_ec, int layer_or_wheel, int phi_module, float predictR, float hitR, float residualR, float pullR, bool LRcorrect, bool isTubeHit, float trketa) const {

  //Loop over the barrel sides
  for (unsigned int side = 0; side < 3; ++side) {
    bool doFill = false;
    if (!side) doFill = true;
    else if (side == 1 && barrel_ec == 1) doFill = true;
    else if (side == 2 && barrel_ec == -1) doFill = true;

    if (!doFill) continue;

    auto trt_b_PredictedR_m = Monitored::Scalar<float>( "m_trt_b_PredictedR", predictR);
    fill(m_tools[m_trtBPredictedR[side]], trt_b_PredictedR_m);
    auto trt_b_MeasuredR_m = Monitored::Scalar<float>( "m_trt_b_MeasuredR", hitR);
    fill(m_tools[m_trtBMeasuredR[side]], trt_b_MeasuredR_m);
    auto trt_b_residualR_m = Monitored::Scalar<float>( "m_trt_b_residualR", residualR);
    fill(m_tools[m_trtBResidualR[side]], trt_b_residualR_m);
    auto trt_b_pullR_m = Monitored::Scalar<float>( "m_trt_b_pullR", pullR);
    fill(m_tools[m_trtBPullR[side]], trt_b_pullR_m);

    if (!isTubeHit) {
      auto trt_b_residualR_notube_m = Monitored::Scalar<float>( "m_trt_b_residualR_notube", residualR);
      fill(m_tools[m_trtBResidualRNoTube[side]], trt_b_residualR_notube_m);
      auto trt_b_pullR_notube_m = Monitored::Scalar<float>( "m_trt_b_pullR_notube", pullR);
      fill(m_tools[m_trtBPullRNoTube[side]], trt_b_pullR_notube_m);
    }

    auto trt_b_lr_m = Monitored::Scalar<float>( "m_trt_b_lr", 0.0);
    if (LRcorrect && !isTubeHit)  trt_b_lr_m = 0.5;
    if (LRcorrect && isTubeHit)   trt_b_lr_m = 1.5;
    if (!LRcorrect && !isTubeHit) trt_b_lr_m = 2.5;
    if (!LRcorrect && isTubeHit)  trt_b_lr_m = 3.5;
    fill(m_tools[m_trtBLR[side]], trt_b_lr_m);

    auto trt_b_aveResVsTrackEta_m = Monitored::Scalar<float>( "m_trt_b_aveResVsTrackEta", trketa);
    auto trt_b_PhiSec_m = Monitored::Scalar<float>( "m_trt_b_PhiSec", phi_module);
    auto trt_b_lrVsPhiSec_m = Monitored::Scalar<float>( "m_trt_b_lrVsPhiSec", LRcorrect);
    fill(m_tools[m_trtBResVsEta[side][layer_or_wheel]], trt_b_aveResVsTrackEta_m, trt_b_residualR_m);
    fill(m_tools[m_trtBResVsPhiSec[side][layer_or_wheel]], trt_b_PhiSec_m, trt_b_residualR_m);
    trt_b_lrVsPhiSec_m = LRcorrect;
    fill(m_tools[m_trtBLRVsPhiSec[side][layer_or_wheel]], trt_b_PhiSec_m, trt_b_lrVsPhiSec_m);

  }//Over sides

  return;
}//fillTRTBarrelHistograms

void IDAlignMonResidualsAlg::fillTRTEndcapHistograms(int barrel_ec, int phi_module, float predictR, float hitR, float residualR, float pullR, bool LRcorrect, bool isTubeHit, float trketa) const {
  for (unsigned int endcap = 0; endcap < 2; ++endcap) {
    bool doFill = false;
    if (!endcap && barrel_ec == 2) doFill = true;
    else if (endcap && barrel_ec == -2) doFill = true;

    if (!doFill) continue;

    auto trt_ec_PredictedR_m = Monitored::Scalar<float>( "m_trt_ec_PredictedR", predictR);
    fill(m_tools[m_trtECPredictedR[endcap]], trt_ec_PredictedR_m);
    auto trt_ec_MeasuredR_m = Monitored::Scalar<float>( "m_trt_ec_MeasuredR", hitR);
    fill(m_tools[m_trtECMeasuredR[endcap]], trt_ec_MeasuredR_m);
    auto trt_ec_residualR_m = Monitored::Scalar<float>( "m_trt_ec_residualR", residualR);
    fill(m_tools[m_trtECResidualR[endcap]], trt_ec_residualR_m);
    auto trt_ec_pullR_m = Monitored::Scalar<float>( "m_trt_ec_pullR", pullR);
    fill(m_tools[m_trtECPullR[endcap]], trt_ec_pullR_m);

    if (!isTubeHit) {
      auto trt_ec_pullR_notube_m = Monitored::Scalar<float>( "m_trt_ec_pullR_notube", pullR);
      fill(m_tools[m_trtECPullRNoTube[endcap]], trt_ec_pullR_notube_m);
      auto trt_ec_residualR_notube_m = Monitored::Scalar<float>( "m_trt_ec_residualR_notube", residualR);
      fill(m_tools[m_trtECResidualRNoTube[endcap]], trt_ec_residualR_notube_m);
    }

    auto trt_ec_lr_m = Monitored::Scalar<float>( "m_trt_ec_lr", 0.0);
    if (LRcorrect && !isTubeHit)       trt_ec_lr_m = 0.5;
    else if (LRcorrect && isTubeHit)   trt_ec_lr_m = 1.5;
    else if (!LRcorrect && !isTubeHit) trt_ec_lr_m = 2.5;
    else if (!LRcorrect && isTubeHit)  trt_ec_lr_m = 3.5;
    fill(m_tools[m_trtECLR[endcap]], trt_ec_lr_m);

    auto trt_ec_aveResVsTrackEta_m = Monitored::Scalar<float>( "m_trt_ec_aveResVsTrackEta", trketa);
    fill(m_tools[m_trtECResVsEta[endcap]], trt_ec_aveResVsTrackEta_m, trt_ec_residualR_m);

    auto trt_ec_phi_m = Monitored::Scalar<float>( "m_trt_ec_phi", phi_module);
    fill(m_tools[m_trtECResVsPhiSec[endcap]], trt_ec_phi_m, trt_ec_residualR_m);
    auto trt_ec_lrVsPhiSec_m = Monitored::Scalar<float>( "m_trt_ec_lrVsPhiSec", LRcorrect);
    fill(m_tools[m_trtECLRVsPhiSec[endcap]], trt_ec_phi_m, trt_ec_lrVsPhiSec_m);
  }

  return;
}

//---------------------------------------------------------------------------------------
std::unique_ptr <Trk::TrackParameters> IDAlignMonResidualsAlg::getUnbiasedTrackParameters(const Trk::Track* trkPnt, const Trk::TrackStateOnSurface* tsos) const
{

  std::unique_ptr <Trk::TrackParameters> TrackParams{};
  std::unique_ptr <Trk::TrackParameters> UnbiasedTrackParams{};
  std::unique_ptr <Trk::TrackParameters> PropagatedTrackParams{};
  std::unique_ptr <Trk::TrackParameters> OtherSideUnbiasedTrackParams{};

  //controls if the SCT residuals will be 'truly' unbiased - removing also the opposite side hit.
  bool trueUnbiased = true;

  Identifier surfaceID;


  ATH_MSG_VERBOSE("original track parameters: " << *(tsos->trackParameters()) );
  ATH_MSG_VERBOSE("Trying to unbias track parameters.");

  const Trk::RIO_OnTrack* hitOnTrack = dynamic_cast <const Trk::RIO_OnTrack*>(tsos->measurementOnTrack());

  if (hitOnTrack != nullptr) surfaceID = hitOnTrack->identify();


  // if SCT Hit and TrueUnbiased then remove other side hit first
  if (surfaceID.is_valid() && trueUnbiased && m_idHelper->is_sct(surfaceID)) {  //there's no TrueUnbiased for non-SCT (pixel) hits)
    ATH_MSG_VERBOSE("Entering True Unbiased loop.");

    // check if other module side was also hit and try to remove other hit as well
    const Trk::TrackStateOnSurface* OtherModuleSideHit(nullptr);
    const Identifier waferID = m_sctID->wafer_id(surfaceID);
    const IdentifierHash waferHash = m_sctID->wafer_hash(waferID);
    IdentifierHash otherSideHash;
    m_sctID->get_other_side(waferHash, otherSideHash);
    const Identifier OtherModuleSideID = m_sctID->wafer_id(otherSideHash);

    for (const Trk::TrackStateOnSurface* TempTsos : *trkPnt->trackStateOnSurfaces()) {

      const Trk::RIO_OnTrack* TempHitOnTrack = dynamic_cast <const Trk::RIO_OnTrack*>(TempTsos->measurementOnTrack());
      if (TempHitOnTrack != nullptr) {
	if (m_sctID->wafer_id(TempHitOnTrack->identify()) == OtherModuleSideID) {
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

  // if propagation failed or no TrueUnbiased or no SCT then use original TrackParams
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
StatusCode IDAlignMonResidualsAlg::setupTools()
{
  //initializing tools

  ATH_MSG_DEBUG("In setupTools()");

  StatusCode sc;
  //Get the PIX manager from the detector store
  ATH_CHECK(detStore()->retrieve(m_PIX_Mgr,m_Pixel_Manager));
  ATH_MSG_DEBUG("Initialized PixelManager");

  //Get the SCT manager from the detector store
  ATH_CHECK(detStore()->retrieve(m_SCT_Mgr, m_SCT_Manager));
  ATH_MSG_DEBUG("Initialized SCTManager");

  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_MSG_DEBUG("Initialized PixelIDHelper");

  ATH_CHECK(detStore()->retrieve(m_sctID, "SCT_ID"));
  ATH_MSG_DEBUG("Initialized SCTIDHelper");

  ATH_CHECK(detStore()->retrieve(m_trtID, "TRT_ID"));
  ATH_MSG_DEBUG("Initialized TRTIDHelper");

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
bool IDAlignMonResidualsAlg::trackRequiresRefit(const Trk::Track* track) const
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

