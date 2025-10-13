/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>

#include "TrigTauTrackRoiUpdater.h"

#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "CxxUtils/phihelper.h"

#include "xAODTracking/TrackParticle.h"

TrigTauTrackRoiUpdater::TrigTauTrackRoiUpdater(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{

}


TrigTauTrackRoiUpdater::~TrigTauTrackRoiUpdater()
{

}


StatusCode TrigTauTrackRoiUpdater::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() );
    ATH_MSG_DEBUG("z0HalfWidth: " << m_z0HalfWidth );
    ATH_MSG_DEBUG("etaHalfWidth: " << m_etaHalfWidth );
    ATH_MSG_DEBUG("phiHalfWidth: " << m_phiHalfWidth );
    ATH_MSG_DEBUG("nHitPix: " << m_nHitPix );
    ATH_MSG_DEBUG("nSiHoles: " << m_nSiHoles );
    
    if(m_z0HalfWidth < 0 || m_etaHalfWidth < 0 || m_phiHalfWidth < 0) {
        ATH_MSG_ERROR("Incorrect parameters");
        return StatusCode::FAILURE;
    }
    
    ATH_MSG_DEBUG("Initialising HandleKeys");
    ATH_CHECK(m_roIInputKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());
    ATH_CHECK(m_roIOutputKey.initialize());
    
    return StatusCode::SUCCESS;
}


StatusCode TrigTauTrackRoiUpdater::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Running " << name());
 
    //---------------------------------------------------------------
    // Prepare I/O
    //---------------------------------------------------------------

    // Prepare output RoI container
    std::unique_ptr<TrigRoiDescriptorCollection> roiCollection = std::make_unique<TrigRoiDescriptorCollection>();
    SG::WriteHandle<TrigRoiDescriptorCollection> outputRoIHandle = SG::makeHandle(m_roIOutputKey, ctx);
    ATH_CHECK(outputRoIHandle.record(std::move(roiCollection)));


    // Retrieve Input TrackCollection
    SG::ReadHandle<xAOD::TrackParticleContainer> TrackCollectionHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(TrackCollectionHandle.isValid());
    const xAOD::TrackParticleContainer* foundTracks = TrackCollectionHandle.get();

    if(!foundTracks) {
      ATH_MSG_ERROR("No track container found, the Track RoI updater should not be scheduled");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Found " << foundTracks->size() << " FTF tracks, updating the RoI");

    
    // Retrieve input RoI descriptor
    SG::ReadHandle<TrigRoiDescriptorCollection> roisHandle = SG::makeHandle(m_roIInputKey, ctx);
    ATH_MSG_DEBUG("Size of roisHandle: " << roisHandle->size());
    const TrigRoiDescriptor* roiDescriptor = roisHandle->at(0); // We only have one RoI in the handle


    // Fill local variables for RoI reference position
    float eta = roiDescriptor->eta();
    float phi = roiDescriptor->phi();  

    float zed = roiDescriptor->zed();
    float zedMinus = roiDescriptor->zedMinus();
    float zedPlus = roiDescriptor->zedPlus();



    //---------------------------------------------------------------
    // Find leading track
    //---------------------------------------------------------------

    const xAOD::TrackParticle* leadTrack = nullptr;
    float trkPtMax = 0;
    
    // Use the highest-pt track satisfying quality cuts
    // If no track is found, the input ROI is used
    for(const xAOD::TrackParticle* track : *foundTracks) {

        float trackPt = track->pt();
        if(trackPt > trkPtMax) {
	  uint8_t nPix{}, nPixHoles{}, nSCTHoles{}, summaryVal{};
	  nPix = track->summaryValue(summaryVal, xAOD::numberOfPixelHits) ? summaryVal : 0;
	  if(nPix < m_nHitPix) {
	    ATH_MSG_DEBUG("Track rejected because nHitPix " << static_cast<int>(nPix) << " < " << m_nHitPix);
	    continue;
	  }

	  nPixHoles = track->summaryValue(summaryVal, xAOD::numberOfPixelHoles) ? summaryVal : 0;
	  nSCTHoles = track->summaryValue(summaryVal, xAOD::numberOfSCTHoles) ? summaryVal : 0;
	  if((nPixHoles + nSCTHoles) > m_nSiHoles) {
	    ATH_MSG_DEBUG("Track rejected because nSiHoles " << static_cast<int>(nPixHoles + nSCTHoles) << " > " << m_nSiHoles);
	    continue;
	  }

	  leadTrack = track;
	  trkPtMax = trackPt;
	  ATH_MSG_VERBOSE("pTmax = " << trkPtMax);
        }
    }



    //---------------------------------------------------------------
    // Update the RoI
    //---------------------------------------------------------------

    // If a leading track is found, update all the ROI position and size parameters;
    // else, only update the eta/phi width (etaMinus, etaPlus, phiMinus, phiPlus)
    if(leadTrack) {
      zed = leadTrack->z0() + leadTrack->vz();
      ATH_MSG_DEBUG("Track z0 " << leadTrack->z0() <<" vz: " << leadTrack->vz() << " zed:" << zed);
      zedMinus = zed - m_z0HalfWidth;
      zedPlus = zed + m_z0HalfWidth;
      eta = leadTrack->eta();
      phi = leadTrack->phi0();
    }

    float etaMinus = eta - m_etaHalfWidth;
    float etaPlus  = eta + m_etaHalfWidth;
    float phiMinus = CxxUtils::wrapToPi(phi - m_phiHalfWidth);
    float phiPlus  = CxxUtils::wrapToPi(phi + m_phiHalfWidth);


    // Create the new RoI
    outputRoIHandle->push_back(std::make_unique<TrigRoiDescriptor>(
        roiDescriptor->roiWord(), roiDescriptor->l1Id(), roiDescriptor->roiId(),
      	eta, etaMinus, etaPlus,
      	phi, phiMinus, phiPlus,
      	zed, zedMinus, zedPlus
    ));
    

    ATH_MSG_DEBUG("Input RoI: " << *roiDescriptor);
    ATH_MSG_DEBUG("Output RoI: " << *outputRoIHandle->back());

    return StatusCode::SUCCESS;
}
