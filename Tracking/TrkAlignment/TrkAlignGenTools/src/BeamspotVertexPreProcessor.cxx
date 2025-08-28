/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkAlignGenTools/BeamspotVertexPreProcessor.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "TrkAlignEvent/AlignTrack.h"
#include "TrkAlignEvent/AlignVertex.h"
#include "TrkVertexOnTrack/VertexOnTrack.h"

#include "AthContainers/DataVector.h"
#include "GaudiKernel/SmartDataPtr.h"

//++ new one

#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"
#include "TrkParticleBase/TrackParticleBase.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrackSummary/TrackSummary.h"

#include <cmath>
#include <algorithm>
#include <limits>


namespace Trk {

//________________________________________________________________________
BeamspotVertexPreProcessor::BeamspotVertexPreProcessor(const std::string & type,
                                                       const std::string & name,
                                                       const IInterface  * parent)
  : AthAlgTool(type,name,parent)
  , m_trackTypeCounter(AlignTrack::NTrackTypes,0)
{
  declareInterface<IAlignTrackPreProcessor>(this);
}

//________________________________________________________________________
BeamspotVertexPreProcessor::~BeamspotVertexPreProcessor()
= default;

//________________________________________________________________________
StatusCode BeamspotVertexPreProcessor::initialize()
{
  ATH_MSG_INFO("BeamspotVertexPreProcessor::initialize()");

  // configure main track selector if requested
  if (!m_trkSelector.empty()) {
    if (m_trkSelector.retrieve().isFailure())
      ATH_MSG_ERROR("Failed to retrieve tool "<<m_trkSelector<<". No Track Selection will be done.");
    else
      ATH_MSG_INFO("Retrieved " << m_trkSelector);
  }

  if (m_refitTracks) {
    // configure main track fitter
    if(m_trackFitter.retrieve().isFailure()) {
      ATH_MSG_FATAL("Could not get " << m_trackFitter);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Retrieved " << m_trackFitter);

    // configure straight-line track fitter if requested
    if (!m_useSingleFitter) {
      if (m_SLTrackFitter.retrieve().isFailure()) {
        ATH_MSG_FATAL("Could not get " << m_SLTrackFitter);
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Retrieved " << m_SLTrackFitter);
    }

    // TrackToVertexIPEstimator
    if (m_trackToVertexIPEstimatorTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Can not retrieve TrackToVertexIPEstimator of type " << m_trackToVertexIPEstimatorTool.typeAndName());
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO ( "Retrieved TrackToVertexIPEstimator Tool " << m_trackToVertexIPEstimatorTool.typeAndName() );
    }

    // configure Atlas extrapolator
    if (m_extrapolator.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool "<<m_extrapolator);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Retrieved " << m_extrapolator);

    // configure beam-spot conditions service
    ATH_CHECK(m_beamSpotKey.initialize());

    ATH_CHECK(m_PVContainerName.initialize());

    // configure beam-spot track selector if requested
    if(m_doBSTrackSelection) {
      if(m_BSTrackSelector.empty()) {
        ATH_MSG_FATAL("Requested BeamSpot track selection but Track Selector not configured");
        return StatusCode::FAILURE;
      }
      if (m_BSTrackSelector.retrieve().isFailure()) {
	ATH_MSG_FATAL("Could not get " << m_BSTrackSelector);
	return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Retrieved " << m_BSTrackSelector);
    }

  }  // end of 'if (m_refitTracks)'

  else if (m_doBeamspotConstraint) {
    ATH_MSG_FATAL("Requested beam-spot constraint but RefitTracks is False.");
    return StatusCode::FAILURE;
  }

  if( m_doFullVertexConstraint && m_doPrimaryVertexConstraint ) {
    if ( m_alignModuleTool.retrieve().isFailure() ) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_alignModuleTool);
      return StatusCode::FAILURE;
    }
    else {
      ATH_MSG_INFO("Retrieved tool " << m_alignModuleTool);

      ATH_MSG_INFO("************************************************************************");
      ATH_MSG_INFO("*                                                                      *");
      ATH_MSG_INFO("* You have requested the Full Vertex Constraint option.                *");
      ATH_MSG_INFO("* It is your duty to assure that all detector elements                 *");
      ATH_MSG_INFO("* used for track fitting are also loaded in the alignment framework!!! *");
      ATH_MSG_INFO("*                                                                      *");
      ATH_MSG_INFO("* Also make sure the accurate track covariance matrix                  *");
      ATH_MSG_INFO("* is returned by the GlobalChi2Fitter!                                 *");
      ATH_MSG_INFO("*                                                                      *");
      ATH_MSG_INFO("************************************************************************");
    }

  }
  return StatusCode::SUCCESS;
}


bool CompareTwoTracks::operator()(VxTrackAtVertex vtxTrk){ // MD: took away deref*

  ITrackLink* trkLink = vtxTrk.trackOrParticleLink();
  LinkToTrackParticleBase* linkToTrackParticle = dynamic_cast<Trk::LinkToTrackParticleBase*>(trkLink);
  if(!linkToTrackParticle) return false;
  const TrackParticleBase* tpb = *(linkToTrackParticle->cptr());

  const Track* originalTrk = tpb->originalTrack();

  bool equal = false;
  // compare the addresses of these two tracks directly
  if(m_method.find("compareAddress") != std::string::npos){
     if (m_track == originalTrk) equal = true;
  }

  // compare the perigee parameters of these two tracks, should safer
  if(m_method.find("comparePerigee") != std::string::npos){
    const Trk::Perigee * measPer1 = m_track->perigeeParameters();
    const Trk::Perigee * measPer2 = originalTrk->perigeeParameters();
    if(! (measPer1 && measPer2 )) equal = false;
    else{
      float diff = std::abs(std::numeric_limits<float>::epsilon());
      if( ( std::abs(measPer1->parameters()[Trk::d0]     - measPer2->parameters()[Trk::d0])     > diff)
         || ( std::abs(measPer1->parameters()[Trk::z0]     - measPer2->parameters()[Trk::z0])     > diff)
         || ( std::abs(measPer1->parameters()[Trk::phi]    - measPer2->parameters()[Trk::phi])    > diff)
         || ( std::abs(measPer1->parameters()[Trk::theta]  - measPer2->parameters()[Trk::theta])  > diff)
         || ( std::abs(measPer1->parameters()[Trk::qOverP] - measPer2->parameters()[Trk::qOverP]) > diff))
              equal = false;
    }
  }
  return equal;
}



bool BeamspotVertexPreProcessor::selectVertices(const xAOD::Vertex * vtx) const {

    if(0 == vtx->vertexType()) {
      ATH_MSG_DEBUG("this primary vertex has been rejected as type dummy");
      return false;
    }
    if (vtx->numberDoF() <= 0){
      ATH_MSG_WARNING(" VERY STRANGE!!!, this primary vertex has been rejected as non-positive DoF "<< vtx->numberDoF() <<" the type of this vertex: "<<  vtx->vertexType() );
      return false;
    }
    if (static_cast<int>(vtx->vxTrackAtVertex().size()) < m_minTrksInVtx){
      ATH_MSG_DEBUG(" this primary vertex vxTrackAtVertex size:  "<< vtx->vxTrackAtVertex().size() );
      return false;
    }
    return true;
}


bool BeamspotVertexPreProcessor::selectUpdatedVertices(const xAOD::Vertex * vtx) const {

    if (vtx->numberDoF() <= 0){
      ATH_MSG_WARNING(" VERY STRANGE!!! , the updated vertex has been rejected as non-positive DoF: "<< vtx->numberDoF() <<" the type of this vertex:"<<  vtx->vertexType() );
      return false;
    }

    if (static_cast<int>(vtx->vxTrackAtVertex().size()) < m_minTrksInVtx){
      ATH_MSG_DEBUG(" the updated vertex has been rejected as vxTrackAtVertex size:  "<< vtx->vxTrackAtVertex().size() );
      return false;
    }

    if ((vtx->covariancePosition())(0,0)<=0  ||
        (vtx->covariancePosition())(1,1)<=0  ||
        (vtx->covariancePosition())(2,2)<=0){
      ATH_MSG_WARNING(" VERY STRANGE!!! , this updated vertex has been rejected as negative diagonal error matrix ");
      return false;
    }
    return true;
}


bool BeamspotVertexPreProcessor::isAssociatedToPV(const Trk::Track * track, const xAOD::VertexContainer* vertices)
{
  if(!vertices) return false;

  for (const xAOD::Vertex* vtx : *vertices) {
    if (vtx->vertexType() != 1) break;
    if (isAssociatedToVertex(track, vtx)) return true;
  }

  return false;
}


//____________________________________________________________________________
bool BeamspotVertexPreProcessor::isAssociatedToVertex(const Trk::Track * track, const xAOD::Vertex * vertex)
{
  if(!vertex) return false;

  std::vector<VxTrackAtVertex >  vertexTracks = vertex->vxTrackAtVertex();
  Trk::CompareTwoTracks thisCompare(track, "compareAddress");

  std::vector<VxTrackAtVertex >::const_iterator iVxTrackBegin = vertexTracks.begin();
  std::vector<VxTrackAtVertex >::const_iterator iVxTrackEnd   = vertexTracks.end();

  std::vector<VxTrackAtVertex>::const_iterator findResult = std::find_if(iVxTrackBegin, iVxTrackEnd, thisCompare);

  return findResult != iVxTrackEnd;
}


void BeamspotVertexPreProcessor::prepareAllTracksVector(){

  // do clean up firstly
  m_allTracksVector.clear();

  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::VertexContainer> vtxReadHandle(m_PVContainerName, ctx);

  for(const xAOD::Vertex* vtx : *vtxReadHandle){
    if(!selectVertices(vtx)) {
      ATH_MSG_DEBUG("this vertex did not pass the primary vertex selection...");
      continue;
    }
    if (vtx->vxTrackAtVertexAvailable()){
      std::vector<VxTrackAtVertex> vtxTracks = vtx->vxTrackAtVertex();
      m_allTracksVector.emplace_back(vtx, vtxTracks);
    }
    else {
      ATH_MSG_DEBUG("this vertex did not pass the vxTrackAtVertexAvailable() call...");
      continue;
    }
  }

  ATH_MSG_DEBUG("m_allTracksVector size: "<<m_allTracksVector.size());
}


const xAOD::Vertex* BeamspotVertexPreProcessor::findVertexCandidate(const Track* track) const {

  const xAOD::Vertex* findVxCandidate = nullptr;

  for(const auto& thisPair : m_allTracksVector){
    auto iVxTrackBegin  = thisPair.second.begin();
    auto iVxTrackEnd    = thisPair.second.end();
    Trk::CompareTwoTracks thisCompare(track, "compareAddress");

    auto findResult = std::find_if(iVxTrackBegin, iVxTrackEnd, thisCompare);

    if(findResult != iVxTrackEnd){
      ATH_MSG_DEBUG("the found VxTrackAtVertex: "<<*findResult);
      findVxCandidate      = thisPair.first;
      break;
    }
  }

  return findVxCandidate;
}


const VertexOnTrack* BeamspotVertexPreProcessor::provideVotFromVertex(const Track* track, const xAOD::Vertex* &vtx) const {

  const EventContext& ctx = Gaudi::Hive::currentContext();
  const VertexOnTrack * vot       = nullptr;
  const xAOD::Vertex* tmpVtx      = nullptr;
  const xAOD::Vertex* updatedVtx  = nullptr;

  const xAOD::Vertex* findVtx = findVertexCandidate(track);

  ATH_MSG_DEBUG("findVtx in provideVotFromVertex: "<<findVtx);

  if (!( nullptr==findVtx) ) {
    vtx    = findVtx;

    if( m_doFullVertexConstraint ) {
      updatedVtx = new xAOD::Vertex(*vtx);
    } else {
      tmpVtx = new xAOD::Vertex(*vtx);
      updatedVtx = m_trackToVertexIPEstimatorTool->getUnbiasedVertex(track->perigeeParameters(), vtx );
    }


    if(updatedVtx){

      if(!selectUpdatedVertices(updatedVtx))
        return vot;

      if( !m_doFullVertexConstraint )
        ATH_MSG_DEBUG(" updated Vertex by KalmanVertexUpdator: "<<updatedVtx);

      ///vertex as perigeeSurface
      Amg::Vector3D  globPos(updatedVtx->position()); //look
      const PerigeeSurface surface(globPos);
      const Perigee* perigee = nullptr;
      std::unique_ptr<const Trk::TrackParameters> tmp =
        m_extrapolator->extrapolateTrack(ctx, *track, surface);
      //pass ownership only if of correct type
      if (tmp && tmp->associatedSurface().type() == Trk::SurfaceType::Perigee) {
         perigee = static_cast<const Perigee*> (tmp.release());
      }
      if (!perigee) {
        const Perigee * trackPerigee = track->perigeeParameters();
        if ( trackPerigee && trackPerigee->associatedSurface() == surface )
          perigee = trackPerigee->clone();
      }
      //if the perigee is still nonsense ...
      if (not perigee){
        //clean up
        if (updatedVtx!= tmpVtx) delete updatedVtx;
        delete tmpVtx;
        //WARNING
        ATH_MSG_WARNING("Perigee is nullptr in "<<__FILE__<<":"<<__LINE__);
        //exit
        return vot;
      }

      // create the Jacobian matrix from Cartisian to Perigee
      AmgMatrix(2,3) Jacobian;
      Jacobian.setZero();
      //perigee is dereferenced here, must not be nullptr!
      double ptInv                               =  1./perigee->momentum().perp();
      Jacobian(0,0)                              = -ptInv*perigee->momentum().y();
      Jacobian(0,1)                              =  ptInv*perigee->momentum().x();
      Jacobian(1,2)                              =  1.0;

      ATH_MSG_DEBUG(" Jacobian matrix from Cartesian to Perigee: "<< Jacobian);

      AmgSymMatrix(3) vtxCov = updatedVtx->covariancePosition();
      vtxCov *= m_PVScalingFactor * m_PVScalingFactor;

      Amg::MatrixX errorMatrix;
      if( m_doFullVertexConstraint ) {
        AmgSymMatrix(3)  tmpCov;
        tmpCov.setZero();
        tmpCov(0,0) = 1.e-10 ;
        tmpCov(1,1) = 1.e-10;
        tmpCov(2,2) = 1.e-10;
        errorMatrix = Amg::MatrixX( tmpCov.similarity(Jacobian) );
      } else {
        errorMatrix = Amg::MatrixX( vtxCov.similarity(Jacobian) );
      }
      delete perigee;

      // in fact, in most of the normal situation, pointer tmpVtx and updatedVtx are the same. You can check the source code
      // But for safety, I would like to delete them seperately
      // sroe(2016.09.23): This would result in an illegal double delete, if they really point to the same thing!
      // http://stackoverflow.com/questions/9169774/what-happens-in-a-double-delete
      if (tmpVtx != updatedVtx){
        delete updatedVtx;
      }
      delete tmpVtx;
      tmpVtx=nullptr;
      updatedVtx=nullptr;

      LocalParameters localParams = Trk::LocalParameters(Amg::Vector2D(0,0));

      // VertexOnTrack Object
      vot = new VertexOnTrack(std::move(localParams), std::move(errorMatrix), surface);
      ATH_MSG_DEBUG("the VertexOnTrack created from vertex: "<<*vot);
    }
  }

  return vot;

}


const VertexOnTrack* BeamspotVertexPreProcessor::provideVotFromBeamspot(const Track* track) const{

  const EventContext& ctx = Gaudi::Hive::currentContext();
  const VertexOnTrack * vot = nullptr;
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
  Amg::Vector3D  bpos = beamSpotHandle->beamPos();
  ATH_MSG_DEBUG("beam spot: "<<bpos);
  float beamSpotX = bpos.x();
  float beamSpotY = bpos.y();
  float beamSpotZ = bpos.z();
  float beamTiltX = beamSpotHandle->beamTilt(0);
  float beamTiltY = beamSpotHandle->beamTilt(1);
  float beamSigmaX = m_BSScalingFactor * beamSpotHandle->beamSigma(0);
  float beamSigmaY = m_BSScalingFactor * beamSpotHandle->beamSigma(1);

  ATH_MSG_DEBUG("running refit with beam-spot");

  float z0 = track->perigeeParameters()->parameters()[Trk::z0];
  float beamX = beamSpotX + std::tan(beamTiltX) * (z0-beamSpotZ);
  float beamY = beamSpotY + std::tan(beamTiltY) * (z0-beamSpotZ);
  Amg::Vector3D  BSC(beamX, beamY, z0);
  ATH_MSG_DEBUG("constructing beam point (x,y,z) = ( "<<beamX<<" , "<<beamY<<" , "<<z0<<" )");
  std::optional<PerigeeSurface> surface = std::nullopt;
  Amg::MatrixX  errorMatrix;
  LocalParameters beamSpotParameters;

  // covariance matrix of the beam-spot
  AmgSymMatrix(2) beamSpotCov;
  beamSpotCov.setZero();
  beamSpotCov(0,0) = beamSigmaX * beamSigmaX;
  beamSpotCov(1,1) = beamSigmaY * beamSigmaY;

  if(m_constraintMode == 0) {

    const Amg::Vector3D&  globPos(BSC);
    surface.emplace(globPos);

    // create a measurement for the beamspot
    DefinedParameter Par0(0.,Trk::d0);
    beamSpotParameters = LocalParameters(Par0);

    // calculate perigee parameters wrt. beam-spot
    const Perigee* perigee = nullptr;
    std::unique_ptr<const Trk::TrackParameters> tmp =
      m_extrapolator->extrapolateTrack(ctx, *track, *surface);
    // pass ownership only if of correct type
    if (tmp && tmp->associatedSurface().type() == Trk::SurfaceType::Perigee) {
      perigee = static_cast<const Perigee*>(tmp.release());
    }

    if (!perigee) {
      const Perigee * trackPerigee = track->perigeeParameters();
      if ( trackPerigee && trackPerigee->associatedSurface() == *surface )
        perigee = trackPerigee->clone();
    }
    if (not perigee){
      ATH_MSG_WARNING("Perigee is nullptr in "<<__FILE__<<":"<<__LINE__);
      return vot;
    }

    Eigen::Matrix<double,1,2> jacobian;
    jacobian.setZero();
    //perigee is dereferenced here, must not be nullptr
    double ptInv   =  1./perigee->momentum().perp();
    jacobian(0,0) = -ptInv * perigee->momentum().y();
    jacobian(0,1) =  ptInv * perigee->momentum().x();

    errorMatrix = Amg::MatrixX( jacobian*(beamSpotCov*jacobian.transpose()));
    if( errorMatrix.cols() != 1  )
      ATH_MSG_FATAL("Similarity transpose done incorrectly");
    delete perigee;
  }
  if (surface){
    vot = new VertexOnTrack(std::move(beamSpotParameters),
                            std::move(errorMatrix),
                            *surface);
  } else {
    ATH_MSG_WARNING("surface is nullptr in "<<__FILE__<<":"<<__LINE__);
  }
  if (vot){
    ATH_MSG_DEBUG(" the VertexOnTrack objects created from BeamSpot are " << *vot);
  }

  return vot;
}


void BeamspotVertexPreProcessor::provideVtxBeamspot(const AlignVertex* b, AmgSymMatrix(3)* q, Amg::Vector3D* v) const {

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };
  Amg::Vector3D bpos = beamSpotHandle->beamPos();
  ATH_MSG_DEBUG("beam spot: "<<bpos);
  float beamSpotX = bpos.x();
  float beamSpotY = bpos.y();
  float beamSpotZ = bpos.z();
  float beamTiltX = beamSpotHandle->beamTilt(0);
  float beamTiltY = beamSpotHandle->beamTilt(1);
  float beamSigmaX = m_BSScalingFactor * beamSpotHandle->beamSigma(0);
  float beamSigmaY = m_BSScalingFactor * beamSpotHandle->beamSigma(1);
  float beamSigmaZ = m_BSScalingFactor * beamSpotHandle->beamSigma(2);

  float z0 = b->originalPosition()->z();
  (*v)(0) = beamSpotX + std::tan(beamTiltX) * (z0-beamSpotZ);
  (*v)(1) = beamSpotY + std::tan(beamTiltY) * (z0-beamSpotZ);
  (*v)(2) = beamSpotZ;
  (*q)(0,0) = beamSigmaX*beamSigmaX;
  (*q)(1,1) = beamSigmaY*beamSigmaY;
  (*q)(2,2) = beamSigmaZ*beamSigmaZ;

  ATH_MSG_DEBUG("VTX constraint point (x,y,z) = ( "<< (*v)[0] <<" , "<< (*v)[1] <<" , "<< (*v)[2] <<" )");
  ATH_MSG_DEBUG("VTX constraint size  (x,y,z) = ( "<< beamSigmaX <<" , "<< beamSigmaY <<" , "<< beamSigmaZ <<" )");
}

const Track*
BeamspotVertexPreProcessor::doConstraintRefit(
  ToolHandle<Trk::IGlobalTrackFitter>& fitter,
  const Track* track,
  const VertexOnTrack* vot,
  const ParticleHypothesis& particleHypothesis) const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const Track* newTrack = nullptr;

  if(vot){

    std::vector<const MeasurementBase *> measurementCollection;
    measurementCollection.push_back(vot);
    // add all other measurements
    const auto &measurements =  *(track->measurementsOnTrack());
    for(const MeasurementBase* meas : measurements)
      measurementCollection.push_back(meas);

    if( m_doFullVertexConstraint ) {
      // get track parameters at the vertex:
      const PerigeeSurface&         surface=vot->associatedSurface();
      ATH_MSG_DEBUG(" Track reference surface will be:  " << surface);
      const TrackParameters* parsATvertex=m_extrapolator->extrapolateTrack(ctx, *track, surface).release();

      ATH_MSG_DEBUG(" Track will be refitted at this surface  ");
      newTrack = (fitter->fit(ctx, measurementCollection,
                             *parsATvertex, m_runOutlierRemoval, particleHypothesis)).release();
      delete parsATvertex;
    } else {
      newTrack = (fitter->fit(ctx,
                             measurementCollection, *(track->trackParameters()->front()),
                             m_runOutlierRemoval, particleHypothesis)).release();
    }
  }

  return newTrack;
}

bool BeamspotVertexPreProcessor::doBeamspotConstraintTrackSelection(const Track* track) {

  const xAOD::VertexContainer* vertices = nullptr;
  const xAOD::Vertex* vertex = nullptr;
  bool haveVertex = false;

  // retrieve the primary vertex if needed
  if(m_doAssociatedToPVSelection) {

    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::VertexContainer> vtxReadHandle(m_PVContainerName, ctx);
    if(!vtxReadHandle.isValid()){
      ATH_MSG_ERROR("Cannot retrieve the \'"<<m_PVContainerName<<"\' vertex collection from StoreGate");
      m_doAssociatedToPVSelection = false;
    } else {
      vertices = vtxReadHandle.cptr();
      // if there is no vertex, we can't associate the tracks to it
      if(vertices) {
        ATH_MSG_DEBUG("Primary vertex collection for this event has "<<vertices->size()<<" vertices");
        if (vertices->size()<2){
          ATH_MSG_DEBUG("Only Dummy vertex present, no Primary vertices.");
        } else {
          vertex = (*vertices)[0];
          haveVertex = true;
        }
      }
      else
        ATH_MSG_DEBUG("Could not retrieve primary vertex collection from the StoreGate");
    }
  }


  if( ( m_doAssociatedToPVSelection && haveVertex && vertex && isAssociatedToPV(track,vertices) ) ||
      ( m_doBSTrackSelection        && m_BSTrackSelector->accept(*track) ) ){

    if (m_maxPt > 0 )
      {
	const Trk::Perigee* perigee = track->perigeeParameters();
	if (!perigee) {
	  ATH_MSG_DEBUG("NO perigee on this track");
	  return false;
	}
	const double qoverP = perigee->parameters()[Trk::qOverP] * 1000.;
	double pt = 0.;
	if (qoverP != 0 )
	  pt = std::abs(1.0/qoverP)*sin(perigee->parameters()[Trk::theta]);
	ATH_MSG_DEBUG( " pt  : "<< pt );
	if (pt > m_maxPt)
	  return false;
      } //maxPt selection

    ATH_MSG_DEBUG("this track passes the beamspot track selection, will do beamspot constraint on it ");
    return true;
  }
  else return false;
}


AlignTrack* BeamspotVertexPreProcessor::doTrackRefit(const Track* track) {

  AlignTrack * alignTrack = nullptr;
  const Track* newTrack = nullptr;
  const VertexOnTrack* vot = nullptr;
  const xAOD::Vertex*    vtx = nullptr;
  AlignTrack::AlignTrackType type = AlignTrack::Unknown;
  // configuration of the material effects needed for track fitter
  ParticleHypothesis particleHypothesis = Trk::ParticleSwitcher::particle[m_particleNumber];

  // initialization the GX2 track fitter
  ToolHandle<Trk::IGlobalTrackFitter> fitter = m_trackFitter;
  if (!m_useSingleFitter && AlignTrack::isSLTrack(track) )
    fitter = m_SLTrackFitter;

  IGlobalTrackFitter::AlignmentCache alignCache;

  ATH_MSG_DEBUG( "doTrackRefit ** START ** ");

  if(m_doPrimaryVertexConstraint){
    vot = provideVotFromVertex(track, vtx);
    if( !vot )  ATH_MSG_INFO( "VoT not found for this track! ");
    if( !vtx )  ATH_MSG_INFO( "VTX pointer not found for this track! ");
    if(vot){
      newTrack = doConstraintRefit(fitter, track, vot, particleHypothesis);
      type = AlignTrack::VertexConstrained;
      // this track failed the PV constraint reift
      if (!newTrack)  {
        ++m_nFailedPVRefits;
        ATH_MSG_DEBUG("VertexConstraint track refit failed! ");
      }
    }
  }

  if( !newTrack && m_doBeamspotConstraint && doBeamspotConstraintTrackSelection(track) ){
      vot = provideVotFromBeamspot(track);
      if(vot){
        newTrack = doConstraintRefit(fitter, track, vot, particleHypothesis);
        type = AlignTrack::BeamspotConstrained;
        // this track failed the BS constraint refit
        if (!newTrack)  {
          ++m_nFailedBSRefits;
          ATH_MSG_DEBUG("BSConstraint track refit failed! ");
        }
      }
  }


  //Refit to get full fitter covariance matrix
  // @TODO This is a little inefficienct and should
  // be addressed when the alignment code is made MT safe
  if(newTrack){
    Trk::Track* tmpTrk =  fitter->alignmentFit(alignCache,*newTrack,m_runOutlierRemoval,particleHypothesis);
    delete newTrack;
    newTrack = tmpTrk;
    if(!tmpTrk){
      if(type == AlignTrack::VertexConstrained)
      {
        ++m_nFailedPVRefits;
        ATH_MSG_DEBUG("VertexConstraint track refit2 failed! ");
      }else if(type == AlignTrack::BeamspotConstrained)
      {
        ++m_nFailedPVRefits;
        ATH_MSG_DEBUG("BSConstraint track refit2 failed! ");
      }
    }
  }

  if(!newTrack && m_doNormalRefit){
      newTrack = fitter->alignmentFit(alignCache,*track,m_runOutlierRemoval,particleHypothesis);
      type = AlignTrack::NormalRefitted;
      // this track failed the normal refit
      if (!newTrack)   {
        ++m_nFailedNormalRefits;
        ATH_MSG_DEBUG("Normal track refit failed! ");
      }
  }





  if(newTrack) {
    alignTrack = new AlignTrack(*newTrack);
    // set original track pointer
    alignTrack->setOriginalTrack(track);
    // set the refit type
    alignTrack->setType(type);


    if (msgLvl(MSG::DEBUG) || msgLvl(MSG::VERBOSE)) {
      ATH_MSG_DEBUG("before refit: "<< *track);
      if (msgLvl(MSG::VERBOSE)) AlignTrack::dumpLessTrackInfo(*track,msg(MSG::DEBUG));

      ATH_MSG_DEBUG("after refit: "<< *newTrack);
      if (msgLvl(MSG::VERBOSE))   AlignTrack::dumpLessTrackInfo(*newTrack,msg(MSG::DEBUG));
    }

    if(AlignTrack::VertexConstrained == type || AlignTrack::BeamspotConstrained == type) alignTrack->doFindPerigee();

    if (m_storeFitMatrices) {
      alignTrack->setFullCovarianceMatrix(alignCache.m_fullCovarianceMatrix);
      alignTrack->setDerivativeMatrix(alignCache.m_derivMatrix);
    }
    delete newTrack;

    if( m_doFullVertexConstraint && vtx!=nullptr && type == AlignTrack::VertexConstrained ){
    // try to log the track-vertex association in the AlignVertex object:
      bool ifound=false;
      for (AlignVertex* ivtx : m_AlignVertices) {
        if( (ivtx->originalVertex())==vtx ) {
          ifound = true;
        }
      }
      if( !ifound ) {
        AlignVertex*  avtx=new AlignVertex(vtx);
        ATH_MSG_DEBUG(" New AlignVertex has ben created.");

        // Beam Spot constraint on the vertex:
        if( m_doBeamspotConstraint && (xAOD::VxType::PriVtx == vtx->vertexType() || xAOD::VxType::PileUp == vtx->vertexType()) && vtx->vxTrackAtVertex().size()>4 ) {     // a beam line verex
          ATH_MSG_DEBUG(" The Beam Spot constraint will be added to the vertex.." );
          AmgSymMatrix(3)     qtemp;
          AmgVector(3)        vtemp;
          provideVtxBeamspot(avtx, &qtemp, &vtemp);
          (qtemp)(2,2) = 1000000.0;                  // disable Z constraint
          avtx->setConstraint( &qtemp, &vtemp);
        }

        m_AlignVertices.push_back(avtx);
      }
    }
     // increment counters
     ++m_trackTypeCounter[type];
     ++m_nTracks;

  }
  // garbage collection:
  if(vot)  delete vot;

  ATH_MSG_DEBUG( "doTrackRefit ** COMPLETED ** ");
  return alignTrack;
}




//____________________________________________________________________________
DataVector<Track> * BeamspotVertexPreProcessor::processTrackCollection(const DataVector<Track> * tracks)
{
  ATH_MSG_DEBUG("BeamspotVertexPreProcessor::processTrackCollection()");

  if( !tracks || (tracks->empty()) )
    return nullptr;

  // Clear the AlignVertex container (will destruct the objects it owns as well!)
  m_AlignVertices.clear();

  if(m_doPrimaryVertexConstraint)
    prepareAllTracksVector();

  // the output collection of AlignTracks
  // we define it as collection of Tracks but fill AlignTracks inside
  DataVector<Track> * newTrks = new DataVector<Track>;

  int index(0);
  // loop over tracks
  ATH_MSG_DEBUG( "Starting loop on input track collection: "<<index);
  for (const auto* track : *tracks){
    ++index;
    ATH_MSG_DEBUG("Processing track "<<index);
    AlignTrack * alignTrack = nullptr;
    if (not track) continue;

    // check whether the track passes the basic selection
    if (m_doTrkSelection) {
      ATH_MSG_DEBUG( "Testing track selection on track: "<<index);
      if ((not m_trkSelector.empty()) and (not m_trkSelector->accept(*track))) continue;
    } // appliying track selection

    if(m_refitTracks){
      ATH_MSG_DEBUG( "Refitting track: "<<index );
      alignTrack = doTrackRefit(track);

      // 2nd track check after refit
      if(alignTrack && !m_trkSelector.empty()) {
	// refitted track loses the summary information, restoring it here
	alignTrack->setTrackSummary( std::make_unique<Trk::TrackSummary> (*track->trackSummary()) );
	// do not check for FullVertex tracks:
        if( !(alignTrack->getVtx()) ) {
	  if( m_doTrkSelection && !m_trkSelector->accept(*alignTrack))
	    continue;
	}
      }
      else {
        ATH_MSG_DEBUG( "Refit of track " << index << " ended with no alignTrack" );
      }
    } else {
      ATH_MSG_DEBUG( "No Track refit for track " << index << " --> building new aligntrack");
      alignTrack = new AlignTrack(*track);
      alignTrack->setOriginalTrack(track);
      alignTrack->setType(AlignTrack::Original);
    }
    // add the new align track to the collection
    if (alignTrack)  newTrks->push_back(alignTrack);
  } // end of loop over tracks

  ATH_MSG_INFO( "Processing of input track collection completed (size: " << tracks->size() << "). Size of the alignTrack collection: " << newTrks->size() );
  // delete the collection if it's empty
  if (newTrks->empty()) {
    delete newTrks;
    return nullptr;
  }

  return newTrks;
}

//____________________________________________________________________________
void BeamspotVertexPreProcessor::accumulateVTX(AlignTrack* alignTrack) {

  if( !m_doFullVertexConstraint ) return;

  AlignVertex*  alignVertex = alignTrack->getVtx();

  ATH_MSG_DEBUG( " In accumulateVTX ");
  if( !alignVertex ) {
    ATH_MSG_DEBUG( "This alignTrack is not associated to any vertex -> return. ");
    return;
  }

  // get pointers so we can reuse them if they're valid
  const Amg::MatrixX           * ptrWeights   = alignTrack->weightMatrix();
  const Amg::MatrixX           * ptrWeightsFD = alignTrack->weightMatrixFirstDeriv();
  const Amg::VectorX           * ptrResiduals = alignTrack->residualVector();
  const std::vector<AlignModuleDerivatives> * ptrDerivs    = alignTrack->derivatives();

  // check if pointers are valid
  if (!ptrWeights || !ptrWeightsFD || !ptrResiduals || !ptrDerivs) {
    ATH_MSG_ERROR("something missing from alignTrack!");
    if (!ptrWeights)   ATH_MSG_ERROR("no weights!");
    if (!ptrWeightsFD) ATH_MSG_ERROR("no weights for first deriv!");
    if (!ptrResiduals) ATH_MSG_ERROR("no residuals!");
    if (!ptrDerivs)    ATH_MSG_ERROR("no derivatives!");
    return;
  }

  // get vectors
  const Amg::VectorX&                  residuals     = *ptrResiduals;
  std::vector<AlignModuleDerivatives>  derivatives   = *ptrDerivs;

  // get weight matrices
  const Amg::MatrixX&          weights           = *ptrWeights;
  const Amg::MatrixX&          weightsFirstDeriv = *ptrWeightsFD;
  ATH_MSG_VERBOSE("weights="<<weights);
  ATH_MSG_VERBOSE("weightsFirstDeriv="<<weightsFirstDeriv);

  // get all alignPars and all derivatives
  ATH_MSG_DEBUG("accumulateVTX: The derivative vector size is  " << derivatives.size() );

  std::vector<const Amg::VectorX*> allDerivatives[3];
  Amg::VectorX  VTXDerivatives[3];
  const int    WSize(weights.cols());
  Amg::MatrixX WF(3,WSize);
  std::vector<AlignModuleVertexDerivatives> derivX;

  for (const auto& deriv : derivatives) {
    // get AlignModule
    const AlignModule* module=deriv.first;

    // get alignment parameters
    if( module ) {
      Amg::MatrixX   F(3,WSize);
      const std::vector<Amg::VectorX>& deriv_vec = deriv.second;
      ATH_MSG_VERBOSE( "accumulateVTX: The deriv_vec size is  " << deriv_vec.size() );
      DataVector<AlignPar>* alignPars = m_alignModuleTool->getAlignPars(module);
      int nModPars = alignPars->size();
      if ((nModPars+3) != std::ssize(deriv_vec)) {
        ATH_MSG_ERROR("accumulateVTX: Derivatives w.r.t. the vertex seem to be missing");
        return;
      }
      for (int i=0;i<3;i++) {
        allDerivatives[i].push_back(&deriv_vec[nModPars+i]);
        for (int j=0;j<WSize;j++) {
          F(i,j) = deriv_vec[nModPars+i][j];
        }
      }

      // prepare the X object in the AlignVertex:
      WF += F * weights;

    } else {
      ATH_MSG_ERROR("accumulateVTX: Derivatives do not have a valid pointer to the module.");
      return;
    }
  }


  // second loop to fill the X object:
  for (const auto& deriv : derivatives) {
    // get AlignModule
    const AlignModule* module=deriv.first;

    // get alignment parameters
    if( module ) {
      const std::vector<Amg::VectorX>& deriv_vec = deriv.second;
      std::vector<Amg::VectorX> drdaWF;
      ATH_MSG_DEBUG("accumulateVTX: The deriv_vec size is  "
                    << deriv_vec.size());
      DataVector<AlignPar>* alignPars = m_alignModuleTool->getAlignPars(module);
      int nModPars = alignPars->size();
      if ((nModPars + 3) != std::ssize(deriv_vec)) {
        ATH_MSG_ERROR(
            "accumulateVTX: Derivatives w.r.t. the vertex seem to be missing");
        return;
      }
      drdaWF.reserve(nModPars);
      for (int i = 0; i < nModPars; i++) {
        drdaWF.emplace_back(2.0 * WF * deriv_vec[i]);
      }
      ATH_MSG_DEBUG("accumulateVTX: derivX incremented by:  " << drdaWF);
      // now add contribution from this track to the X object:
      derivX.emplace_back(module,std::move(drdaWF));

    } else {
      ATH_MSG_ERROR("accumulateVTX: Derivatives do not have a valid pointer to the module.");
      return;
    }
  }

  // prepare derivatives w.r.t. the vertex position:
  int nmodules = allDerivatives[0].size();
  ATH_MSG_DEBUG("accumulateVTX: allDerivatives size is  " << nmodules);
  for( int ii=0; ii<3; ++ii ) {
    VTXDerivatives[ii] = (*(allDerivatives[ii])[0]);
    for( int jj=1; jj<nmodules; ++jj ) {
      VTXDerivatives[ii] += (*(allDerivatives[ii])[jj]);
    }
  }

  AmgVector(3)     vtxV;
  AmgSymMatrix(3)  vtxM;

  Amg::VectorX RHM= weightsFirstDeriv * residuals;
  ATH_MSG_DEBUG("RHM: "<<RHM);

  for (int ipar=0;ipar<3;ipar++) {

    // calculate first derivative
    Amg::MatrixX derivativesT = (VTXDerivatives[ipar]).transpose();
    ATH_MSG_DEBUG("derivativesT (size "<<derivativesT.cols()<<"): "<<derivativesT);

    Amg::MatrixX  tempV = (2.* derivativesT * RHM);
    vtxV[ipar] = tempV(0,0);

    for (int jpar=ipar;jpar<3;jpar++) {

      // calculate second derivatives
      Amg::MatrixX RHM2 = weights * (VTXDerivatives[jpar]);

      Amg::MatrixX  tempM = (2.* derivativesT * RHM2);
      vtxM(ipar,jpar) = tempM(0,0);

    }

  }

  // increment the vtx algebra objects:

  alignVertex->incrementVector(vtxV);
  alignVertex->incrementMatrix(vtxM);
  //   ATH_MSG_DEBUG("accumulateVTX: derivX size = "<< derivX->size());
  alignVertex->addDerivatives(&derivX);

}


//____________________________________________________________________________
void BeamspotVertexPreProcessor::solveVTX() {

  if( m_doFullVertexConstraint ){
    ATH_MSG_DEBUG("In solveVTX. Number of vertices = " << m_AlignVertices.size() );
    for (AlignVertex* ivtx : m_AlignVertices) {
      if( ivtx->Ntracks()>1 ) {
        ivtx->fitVertex();
       } else {
	ATH_MSG_WARNING("This vertex contains " << ivtx->Ntracks() << " tracks. No solution possible.");
       }

       ATH_MSG_DEBUG( "This vertex contains " << ivtx->Ntracks() << " tracks.");
       if( msgLvl(MSG::DEBUG) )  ivtx->dump(msg(MSG::DEBUG));
    }
  }
}

//____________________________________________________________________________
void BeamspotVertexPreProcessor::printSummary()
{
  if(m_logStream) {

    *m_logStream<<"*************************************************************"<<std::endl;
    *m_logStream<<"******        BeamspotVertexPreProcessor summary       ******"<<std::endl;
    *m_logStream<<"*"<<std::endl;
    *m_logStream<<"* number of created AlignTracks :  "<<m_nTracks<<std::endl;
    if(m_nTracks>0) {
      *m_logStream<<"* --------------------------------------------"<<std::endl;
      for(int i=0; i<AlignTrack::NTrackTypes; ++i) {
        if(m_trackTypeCounter[i]>0)
          *m_logStream<<"*           "<<(AlignTrack::AlignTrackType)i<<":  "<<m_trackTypeCounter[i]<<std::endl;
        }
    }
    *m_logStream<<"*"<<std::endl;
    *m_logStream<<"* number of failed normal refits :              " << m_nFailedNormalRefits << std::endl;
    *m_logStream<<"* number of failed refits with primary vertex : " << m_nFailedPVRefits     << std::endl;
    *m_logStream<<"* number of failed refits with beam-spot :      " << m_nFailedBSRefits     << std::endl;
    *m_logStream<<"*"<<std::endl;
  }
}

//____________________________________________________________________________
StatusCode BeamspotVertexPreProcessor::finalize()
{
  ATH_MSG_INFO("BeamspotVertexPreProcessor::finalize()");

  return StatusCode::SUCCESS;
}

//____________________________________________________________________________
}
