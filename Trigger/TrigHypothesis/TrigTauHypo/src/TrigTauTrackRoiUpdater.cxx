/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>

#include "TrigTauTrackRoiUpdater.h"

#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "CxxUtils/phihelper.h"

#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrackSummary/TrackSummary.h"

#include "PathResolver/PathResolver.h"
#include "tauRecTools/HelperFunctions.h"

TrigTauTrackRoiUpdater::TrigTauTrackRoiUpdater(const std::string & name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

TrigTauTrackRoiUpdater::~TrigTauTrackRoiUpdater()
{
}

StatusCode TrigTauTrackRoiUpdater::initialize()
{
  ATH_MSG_DEBUG( "Initializing " << name() );
  ATH_MSG_DEBUG( "z0HalfWidth  " << m_z0HalfWidth );
  ATH_MSG_DEBUG( "etaHalfWidth " << m_etaHalfWidth );
  ATH_MSG_DEBUG( "phiHalfWidth " << m_phiHalfWidth );
  ATH_MSG_DEBUG( "nHitPix      " << m_nHitPix );
  ATH_MSG_DEBUG( "nSiHoles     " << m_nSiHoles );

  if(m_z0HalfWidth<0. || m_etaHalfWidth<0. || m_phiHalfWidth<0.) {
    ATH_MSG_ERROR( "Incorrect parameters." );
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "Initialising HandleKeys" );
  CHECK( m_roIInputKey.initialize() );
  CHECK( m_tracksKey.initialize() );
  CHECK( m_roIOutputKey.initialize() );
  CHECK( m_tauJetKey.initialize(SG::AllowEmpty) );

  return StatusCode::SUCCESS;
}

StatusCode TrigTauTrackRoiUpdater::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG( "Running "<< name() );

  // Retrieve Input TrackCollection
  SG::ReadHandle< TrackCollection > TrackCollectionHandle = SG::makeHandle( m_tracksKey,ctx );
  CHECK( TrackCollectionHandle.isValid() );
  const TrackCollection *foundTracks = TrackCollectionHandle.get();

  if(foundTracks == nullptr) {
    ATH_MSG_ERROR( "No track container found, the track ROI updater should not be scheduled" );
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG( "Found " << foundTracks->size() << " fast tracks, updating the RoI" );

  const Trk::Track *leadTrack = nullptr;
  const Trk::Perigee *trackPer = nullptr;
  const Trk::TrackSummary* summary = nullptr;
  double trkPtMax = 0.;
  
  // use the highest-pt track satisfying quality cuts is used
  // if no track is found, the input ROI is used
  for (const Trk::Track* track : *foundTracks) {
    trackPer = track->perigeeParameters();
    summary = track->trackSummary();
    if(summary==nullptr) {
      ATH_MSG_WARNING( "track summary not available in RoI updater " << name() );
      continue;
    }
    float trackPt = trackPer->pT();
    if ( trackPt > trkPtMax ) {
      int nPix  = summary->get(Trk::numberOfPixelHits);
      if(nPix<0) nPix=0;
      if(nPix < m_nHitPix) {
        ATH_MSG_DEBUG( "Track rejected because of nHitPix " << nPix << " < " << m_nHitPix );
	continue;
      }
      int nPixHole = summary->get(Trk::numberOfPixelHoles);
      if (nPixHole < 0) nPixHole = 0;
      int nSCTHole = summary->get(Trk::numberOfSCTHoles);
      if (nSCTHole < 0) nSCTHole = 0;
      if((nPixHole + nSCTHole) > m_nSiHoles) {
        ATH_MSG_DEBUG( "Track rejected because of nSiHoles " << nPixHole+nSCTHole << " > " << m_nSiHoles );
	continue;
      }
      leadTrack = track;
      trkPtMax = trackPt;
    }
  }

  // get RoI descriptor
  SG::ReadHandle< TrigRoiDescriptorCollection > roisHandle = SG::makeHandle( m_roIInputKey, ctx );
  ATH_MSG_DEBUG("Size of roisHandle: "<<roisHandle->size());
  const TrigRoiDescriptor *roiDescriptor = roisHandle->at(0);

  // if a leading track is found, update all the ROI parameters (eta, etaMinus, etaPlus, phi, phiMinus, phiPlus, zed, zedMinus, zedPlus)
  // else, only update the eta/phi width (etaMinus, etaPlus, phiMinus, phiPlus)
  double zed = roiDescriptor->zed();
  double zedMinus = roiDescriptor->zedMinus();
  double zedPlus = roiDescriptor->zedPlus();
  double eta = roiDescriptor->eta();
  double phi = roiDescriptor->phi();  
  if( leadTrack ) {
    zed = leadTrack->perigeeParameters()->parameters()[Trk::z0];
    zedMinus = zed - m_z0HalfWidth;
    zedPlus = zed + m_z0HalfWidth;    
    eta = leadTrack->perigeeParameters()->eta();
    phi = leadTrack->perigeeParameters()->parameters()[Trk::phi0];
  }
  double etaMinus = eta - m_etaHalfWidth; 
  double etaPlus  = eta + m_etaHalfWidth;
  double phiMinus = CxxUtils::wrapToPi( phi - m_phiHalfWidth ); 
  double phiPlus  = CxxUtils::wrapToPi( phi + m_phiHalfWidth ); 

  // Prepare the new RoI
  TrigRoiDescriptor *outRoi = new TrigRoiDescriptor(roiDescriptor->roiWord(), roiDescriptor->l1Id(), roiDescriptor->roiId(),
						    eta, etaMinus, etaPlus,
						    phi, phiMinus, phiPlus,
						    zed, zedMinus, zedPlus);
  
  ATH_MSG_DEBUG("Input RoI " << *roiDescriptor);
  ATH_MSG_DEBUG("Output RoI " << *outRoi);

  auto roICollection = std::make_unique<TrigRoiDescriptorCollection>();
  roICollection->push_back(outRoi);

  // Save Outputs
  ATH_MSG_DEBUG( "Saving RoIs to be used as input to Fast Tracking -- TO BE CHANGED -- ::: " << m_roIOutputKey.key() );
  SG::WriteHandle< TrigRoiDescriptorCollection > outputRoiHandle = SG::makeHandle( m_roIOutputKey,ctx );
  CHECK( outputRoiHandle.record( std::move( roICollection ) ) );

  return StatusCode::SUCCESS;
}

