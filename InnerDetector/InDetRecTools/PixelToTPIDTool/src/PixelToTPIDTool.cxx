/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelToTPIDTool/PixelToTPIDTool.h"

InDet::PixelToTPIDTool::PixelToTPIDTool(const std::string& t, const std::string& n, const IInterface*  p )
  :AthAlgTool(t,n,p),
   m_IBLParameterSvc("IBLParameterSvc",n),
   m_pixelid(nullptr)
{
  declareInterface<IPixelToTPIDTool>(this);
}

InDet::PixelToTPIDTool::~PixelToTPIDTool() = default;

StatusCode InDet::PixelToTPIDTool::initialize() {

  ATH_CHECK(AthAlgTool::initialize());

  if(m_equalizeClusterMeasurements) {
    ATH_MSG_INFO("Will equalize individual cluster dE/dx measurements and return the truncated mean.");
  }
  
  ATH_CHECK(m_eventInfoKey.initialize());

  /// FIXME TODO: Check if running on data or simulation

  /// For now, cannot equalize dE/dx measurements in simulation. 
  if(m_isMC && m_equalizeClusterMeasurements) {
    ATH_MSG_ERROR("Requested to equalize the dE/dx, but this is not yet supported for MC.");
    ATH_MSG_ERROR("Eventually, can apply scale factors to \"undo\" the radiation modeling in MC23.");
    ATH_MSG_ERROR("Or equalize the MC to the data reference run.");        
    return StatusCode::FAILURE;
  }

  ATH_CHECK(detStore()->retrieve(m_pixelid,"PixelID"));

  if (m_IBLParameterSvc.retrieve().isFailure()) {
    ATH_MSG_FATAL("Could not retrieve IBLParameterSvc");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_moduleDataKey.initialize());

  //ATH_CHECK(m_dedxKey.initialize());

  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode InDet::PixelToTPIDTool::finalize()
{
  StatusCode sc = AthAlgTool::finalize();
  return sc;
}


//============================================================================================

/// Will return the dE/dx, and will update nUsedHits (the divisor in the truncated mean) and nUsedIBLOverflowHits.
/// Whether this is the raw or equalized dE/dx will be determined by the tool properties.
float InDet::PixelToTPIDTool::dEdx(const EventContext& ctx,
                            const Trk::Track& track,
                            int& nUsedHits,
                            int& nUsedIBLOverflowHits) const
{

  /// passed by ref, so will update here.  
  nUsedHits=0; // divisor in the truncated mean calculation.
  nUsedIBLOverflowHits=0; // number of IBL hits in overflow.
    
  /// Get pixel clusters in this simple struct to abstract away the two EDMs.
  std::vector<PixelDEdx::PixelClusterStruct> clusters;

  /// Second value keeps track if the cluster is in IBL and has at least an overflow hit
  std::multimap<float,int> dEdxMap;

  // Check for track states:
  const Trk::TrackStates* recoTrackStates = track.trackStateOnSurfaces();
  if (recoTrackStates) {
    Trk::TrackStates::const_iterator tsosIter    = recoTrackStates->begin();
    Trk::TrackStates::const_iterator tsosIterEnd = recoTrackStates->end();

    // Loop over track states on surfaces (i.e. generalized hits):
    for (; tsosIter != tsosIterEnd; ++tsosIter) {
      const Trk::MeasurementBase *measurement = (*tsosIter)->measurementOnTrack();
      if (measurement && !(*tsosIter)->type(Trk::TrackStateOnSurface::Outlier)) {
        if (!(*tsosIter)->trackParameters()) {
          msg(MSG::WARNING) << "No track parameters available for a state of type measurement, returning -1" << endmsg;
          msg(MSG::WARNING) << "Don't run this tool on slimmed tracks!" << endmsg;
          return -1;
        }

        const InDet::PixelClusterOnTrack* pixclus = nullptr;
        if (measurement->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
          const Trk::RIO_OnTrack* tmpRio = static_cast<const Trk::RIO_OnTrack*>(measurement);
          if (tmpRio->rioType(Trk::RIO_OnTrackType::PixelCluster)) {
            pixclus = static_cast<const InDet::PixelClusterOnTrack*>(tmpRio);
          }
        }
        if (pixclus) {

          /// Build PixelClusterStruct to abstract away the EDMs.
          PixelDEdx::PixelClusterStruct cluster;            
          cluster.locx=pixclus->localParameters()[Trk::locX];
          cluster.locy=pixclus->localParameters()[Trk::locY];
          cluster.bec=m_pixelid->barrel_ec(pixclus->identify());
          cluster.layer=m_pixelid->layer_disk(pixclus->identify());
          cluster.eta_module=m_pixelid->eta_module(pixclus->identify());//check eta module to select thickness
            
          float dotProd = (*tsosIter)->trackParameters()->momentum().dot( (*tsosIter)->trackParameters()->associatedSurface().normal() );
          cluster.cosalpha = fabs(dotProd / (*tsosIter)->trackParameters()->momentum().mag());
          cluster.charge = pixclus->prepRawData()->totalCharge();

          /// keep track if this is an ibl cluster with overflow
          bool iblOverflow = false;
          if ((m_IBLParameterSvc->containsIBL()) and (cluster.bec==0) and (cluster.layer==0)) { // check if IBL
              
            //loop over ToT and check if anyone is overflow (ToT==14) check for IBL cluster overflow
            int overflowIBLToT = SG::ReadCondHandle<PixelChargeCalibCondData>(m_moduleDataKey, ctx)->getFEI4OverflowToT();
            const std::vector<int>& ToTs = pixclus->prepRawData()->totList();
              
            for (int pixToT : ToTs) {
              if (pixToT >= overflowIBLToT) {
                //overflow pixel hit -- flag cluster
                iblOverflow = true;
                break; //no need to check other hits of this cluster
              }
            }// end
            cluster.isIBL = true;
            cluster.iblOverflow = iblOverflow;
          }

          /// If good measurement, update cluster raw cluster dE/dx, passdEdxCutsLoose, and passdEdxCutsTight.
          /// Also, increment nUsedIBLOverflowHits
          /// If bad measurement, keep default negative value for dE/dx, don't increment.
          getClusterdEdx(cluster, nUsedIBLOverflowHits);

          /// Check if good measurement.
          if (cluster.dEdx < 0.0) { continue; }

          /// Apply cluster-level equalization.
          /// Read conditions database.
          if(m_equalizeClusterMeasurements){

            /// Insert code from Rebeccas Hicks' QT task here (ATLIDTRKCP-579).
            /// Pulls dE/dx equalization SF from conditons database.
            float SF = 1.;

            /// Apply scale factor and store
            cluster.dEdxEq = cluster.dEdx * SF;

          }

          /// Only pushing back clusters with good dE/dx measurements!
          clusters.push_back(cluster);

        } // pixclus iterator
      } // end if measurement found and not outlier. 
    } // tsos iterator
  } // end if reco track states found.
  
  /// Always calculate raw truncated mean & update number of hits used in truncated mean.
  float averagedEdx = 0;
  float sigmadEdx = 0;
  PixelDEdx::getdEdxMetrics(clusters, averagedEdx, sigmadEdx, nUsedHits);
    
  /// Calculate equalized truncated mean.
  if(m_equalizeClusterMeasurements) {
    int nUsedHitsEq=0; // need separate counter or will double count if calculating both raw and equalized dE/dx
    float averagedEdxEq = 0;
    float sigmadEdxEq = 0;
    PixelDEdx::getdEdxMetrics(clusters, averagedEdxEq, sigmadEdxEq, nUsedHitsEq, true);

    /// Sanity check that nUsedHits and nUsedHitsEq are the same.
    if (nUsedHitsEq != nUsedHits) {
      ATH_MSG_ERROR("The numberOfUsedHitsdEdx calculated for the raw ("<< nUsedHits <<") and equalized ("<< nUsedHitsEq <<") dE/dx differ!  Should not happen!");
    }
      
    /// Return equalized truncated mean dE/dx
    return averagedEdxEq;
  }

  /// Return raw truncated mean dE/dx
  return averagedEdx;

}
