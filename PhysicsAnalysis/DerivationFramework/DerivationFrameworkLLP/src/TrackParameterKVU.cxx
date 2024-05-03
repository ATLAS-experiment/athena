/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackParametersKVU.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkLLP/TrackParametersKVU.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include <StoreGate/WriteDecorHandle.h>
#include <vector>
#include <string>

// Constructor
DerivationFramework::TrackParametersKVU::TrackParametersKVU( const std::string& t,
							     const std::string& n,
							     const IInterface* p ) :
  base_class(t, n, p){}
 
// Athena initialize and finalize
StatusCode DerivationFramework::TrackParametersKVU::initialize()
{

  if (m_trackContainerKey.empty() || m_vertexContainerKey.empty()) {
    ATH_MSG_ERROR("No selection variables for the TrackParametersKVU tool!");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_vertexContainerKey.initialize());

  m_KVUphiKey = m_trackContainerKey.key() + m_KVUphiKey.key();
  m_KVUthetaKey = m_trackContainerKey.key() + m_KVUthetaKey.key();
  m_KVUd0Key = m_trackContainerKey.key() + m_KVUd0Key.key();
  m_KVUz0Key = m_trackContainerKey.key() + m_KVUz0Key.key();
  m_KVUqOverPKey = m_trackContainerKey.key() + m_KVUqOverPKey.key();
  m_KVUChi2Key = m_trackContainerKey.key() + m_KVUChi2Key.key();
  m_KVUusedPVKey = m_trackContainerKey.key() + m_KVUusedPVKey.key();
  m_KVUCovMatKey = m_trackContainerKey.key() + m_KVUCovMatKey.key();

  ATH_CHECK(m_KVUphiKey.initialize());
  ATH_CHECK(m_KVUthetaKey.initialize());
  ATH_CHECK(m_KVUd0Key.initialize());
  ATH_CHECK(m_KVUz0Key.initialize());
  ATH_CHECK(m_KVUqOverPKey.initialize());
  ATH_CHECK(m_KVUChi2Key.initialize());
  ATH_CHECK(m_KVUusedPVKey.initialize());
  ATH_CHECK(m_KVUCovMatKey.initialize());

  ATH_CHECK(m_vertexTrackUpdator.retrieve());
  ATH_CHECK(m_extrapolator.retrieve());
  ATH_CHECK(m_LinearizedTrackFactory.retrieve());
  ATH_CHECK(m_IPEstimator.retrieve());

  ATH_MSG_DEBUG("initialize() ...");
  ATH_MSG_DEBUG("Successfully retrived the TrackParametersKVU tool" );
  return StatusCode::SUCCESS;
}

// Augmentation
StatusCode DerivationFramework::TrackParametersKVU::addBranches() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // --- Get the tracks
  //const xAOD::TrackParticleContainer* tracks = nullptr;
  SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_trackContainerKey,ctx);
  ATH_CHECK( tracks.isValid() );

  //-- for each track, update track params with vtx considered as extra measurement (choose the closest vtx)
  if(tracks->size() !=0) {
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUphi(m_KVUphiKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUtheta(m_KVUthetaKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUd0(m_KVUd0Key, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUz0(m_KVUz0Key, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUqOverP(m_KVUqOverPKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUChi2(m_KVUChi2Key, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decoratorKVUusedPV(m_KVUusedPVKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> decoratorKVUCovMat(m_KVUCovMatKey, ctx);

    for (const auto& track : *tracks) {
      if(track){
	const Trk::TrackParameters *tPerigee = &(track->perigeeParameters());
	// --- list of new variables that will decorate the track
	std::unique_ptr<AmgSymMatrix(5)> updateTrackCov = nullptr;
	float updatephi        = -999;
	float updatetheta      = -999;
	float updated0         = -999;
	float updatez0         = -999;
	float updateqOverP     = -999;
	float updateChi2       = -999; 
        bool usedPrimaryVertex = false;
	std::vector<float> vec;

	std::unique_ptr<const Trk::TrackParameters> trackParams = nullptr;
	float minIP = 1000.;
	//--- retrieve closest vertex to track
	const xAOD::Vertex* closestVertex = nullptr;
	int nVtx = 0;
	SG::ReadHandle<xAOD::VertexContainer> vxContainer(m_vertexContainerKey,ctx);
	if (vxContainer.isValid()) {
	  for (const xAOD::Vertex* vtx: *vxContainer) {
	    if ( (vtx->vertexType() == xAOD::VxType::PriVtx) ||
		 (vtx->vertexType() == xAOD::VxType::PileUp) ) {
	      Amg::Vector3D  vtxPos(vtx->position());
	      auto vtxSurface = std::make_unique<Trk::PerigeeSurface>(vtxPos);
	      trackParams = m_extrapolator->extrapolate(ctx,*tPerigee,*vtxSurface);
	      std::unique_ptr<const Trk::ImpactParametersAndSigma> iPandSigma = nullptr;
	      iPandSigma = m_IPEstimator->estimate(trackParams.get(), vtx);
	      if(sqrt(iPandSigma->IPd0*iPandSigma->IPd0+iPandSigma->IPz0*iPandSigma->IPz0) < minIP){
		minIP = sqrt(iPandSigma->IPd0*iPandSigma->IPd0+iPandSigma->IPz0*iPandSigma->IPz0);
		closestVertex = vtx;
	      }
	      ATH_MSG_VERBOSE("n vtx pos(x, y, z) IPd0 IPz0 : " << nVtx << " " << vtx->position().x() << " "<<  vtx->position().y() << " "<<  vtx->position().z() << " " << iPandSigma->IPd0 << " " << iPandSigma->IPz0);
	      ATH_MSG_VERBOSE("                   d0,Delta_z0-TrackParticle:" << track->d0() <<", "<< track->z0() - vtx->z() + track->vz());
	      nVtx++;
	    }
	  }
	}else{
	  ATH_MSG_ERROR ("Couldn't retrieve vxContainer  with key: " << m_vertexContainerKey.key() );
	  return StatusCode::FAILURE;
	} // --- end retrieve closest vertex to track

	
	// update the track params with vtx info after linearization of track around it
	if(closestVertex){
	  ATH_MSG_VERBOSE("Vertex selected position (x y z): " << closestVertex->position().x()<<" "<<closestVertex->position().y()<<" "<<closestVertex->position().z());
	  ATH_MSG_VERBOSE("                type: " << closestVertex->vertexType());
	  ATH_MSG_VERBOSE("                nParticles: " << closestVertex->nTrackParticles());  
	  ATH_MSG_VERBOSE("                quality chi2: " << closestVertex->chiSquared() << " " << closestVertex->numberDoF());
	  Amg::Vector3D  globPos(closestVertex->position());
	  auto recVtx = std::make_unique<xAOD::Vertex>();
	  recVtx->makePrivateStore(*closestVertex);
	  auto surface = std::make_unique<const Trk::PerigeeSurface>(globPos);
	  trackParams = m_extrapolator->extrapolate(ctx,*tPerigee,*surface);
	  auto linearTrack = std::make_unique<Trk::VxTrackAtVertex>(0., nullptr, nullptr, trackParams.get(), nullptr);
	  if(linearTrack){
	    ATH_MSG_VERBOSE("Linearizing track");
	    m_LinearizedTrackFactory->linearize(*linearTrack,globPos);
	    ATH_MSG_VERBOSE("Updating linearized track parameters after vertex fit. Track weight = " << linearTrack->weight());
	    m_vertexTrackUpdator->update((*linearTrack), (*recVtx));
	    ATH_MSG_VERBOSE ("track info after vertex track updator !"<<*linearTrack );
	    if(linearTrack->perigeeAtVertex()) {
	      //retrieve & store updated track param qOverP,d0, z0,theta,phi after KVU
	      updateqOverP = linearTrack->perigeeAtVertex()->parameters()[Trk::qOverP];
	      updated0 = linearTrack->perigeeAtVertex()->parameters()[Trk::d0];
	      updatez0 = linearTrack->perigeeAtVertex()->parameters()[Trk::z0];
	      updatephi = linearTrack->perigeeAtVertex()->parameters()[Trk::phi0];
	      updatetheta = linearTrack->perigeeAtVertex()->parameters()[Trk::theta];
	      //retrieve & store updated track cov matrix plus save the KVU Chi2 
	      updateTrackCov = std::make_unique<AmgSymMatrix(5)>(*linearTrack->perigeeAtVertex()->covariance());
	      updateChi2 = linearTrack->trackQuality().chiSquared();
	      // store whether this used the PV or not
	      usedPrimaryVertex = closestVertex->vertexType()==xAOD::VxType::PriVtx;
	    }
	  }
	}// --- end if closest vertex
	
	//decorate tracks with new updated track parameters:
	decoratorKVUqOverP(*track) = updateqOverP;
	decoratorKVUd0(*track)     = updated0;
	decoratorKVUz0(*track)     = updatez0;
	decoratorKVUphi(*track)    = updatephi;
	decoratorKVUtheta(*track)  = updatetheta;
	decoratorKVUChi2(*track)   = updateChi2;
        decoratorKVUusedPV(*track) = usedPrimaryVertex;
	if (updateTrackCov){
	  Amg::compress(*updateTrackCov, vec);
	}else{
	  vec.assign(5, 0.0);
	}
	decoratorKVUCovMat(*track) =  vec;
	ATH_MSG_VERBOSE("track updated.");
      } // --- end if(track)
    } // --- end loop tracks
    ATH_MSG_VERBOSE("All tracks updated.");
  }
  
  return StatusCode::SUCCESS;
}
