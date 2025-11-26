/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
  Decorates the vertex constrained track parameters to the track container.
  The vertex fit is performed with a track and the nearest vertex.
  The updated the perigee paramters, chi2 of the vertex fit, and covariance matrix of the perigee parameters are decorated, but d0 and z0 should be 0.
  The nDoF of the vertex fit is always 2. (5 track parameters + 3 vertex parameters - 3 track parameters - 3 vertex parameters)

  user specifies...

  TrackParticleContainerName: track particle container.
  VertexContainerName:        vertex container.
*/

#ifndef DERIVATIONFRAMEWORK_TRACKPARAMETERSATPV_H
#define DERIVATIONFRAMEWORK_TRACKPARAMETERSATPV_H

#include<string>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"


#include "VxVertex/VxContainer.h"
#include "VxVertex/VxCandidate.h"
#include "VxVertex/VxTrackAtVertex.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/Track.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "xAODTracking/VertexContainer.h"
#include "TrkVertexFitterInterfaces/IVertexTrackUpdator.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkVertexFitterInterfaces/IVertexLinearizedTrackFactory.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"

namespace DerivationFramework {


  /** @class TrackParametersKVU

      the code used in this implementation is kindly stolen from:
      atlasoff:: ISF/ISF_Core/ISF_Tools

      @author James Catmore -at- cern.ch
  */
  class TrackParametersKVU : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    // Athena algtool's Hooks
    virtual StatusCode initialize() override;

    /** Check that the current event passes this filter */
    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_trackContainerKey{
      this, "TrackParticleContainerName", "InDetDisappearingTrackParticles"};
    SG::ReadHandleKey< xAOD::VertexContainer > m_vertexContainerKey{
      this, "VertexContainerName", "PrimaryVertices"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUphiKey{
      this, "KVUphiKey", m_trackContainerKey, "KVUphi"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUthetaKey{
      this, "KVUthetaKey", m_trackContainerKey, "KVUtheta"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUd0Key{
      this, "KVUd0Key", m_trackContainerKey, "KVUd0"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUz0Key{
      this, "KVUz0Key", m_trackContainerKey, "KVUz0"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUqOverPKey{
      this, "KVUqOverPKey", m_trackContainerKey, "KVUqOverP"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUChi2Key{
      this, "KVUChi2Key", m_trackContainerKey, "KVUChi2"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUusedPVKey{
      this, "KVUusedPVKey", m_trackContainerKey, "KVUusedPV"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_KVUCovMatKey{
      this, "KVUCovMatKey", m_trackContainerKey, "KVUCovMat"};

    ToolHandle< Trk::IVertexTrackUpdator > m_vertexTrackUpdator {
      this, "VertexTrackUpdator", "Trk::KalmanVertexTrackUpdator"};
    ToolHandle< Trk::IExtrapolator >  m_extrapolator {
      this, "TrackExtrapolator", "Trk::Extrapolator/AtlasExtrapolator"};
    ToolHandle< Trk::IVertexLinearizedTrackFactory > m_LinearizedTrackFactory {
      this, "LinearizedTrackFactory", "Trk::FullLinearizedTrackFactory/FullLinearizedTrackFactory"};
    ToolHandle< Trk::ITrackToVertexIPEstimator> m_IPEstimator {
      this, "IPEstimator", "Trk::TrackToVertexIPEstimator/TrackToVertexIPEstimator"};
  };

}

#endif
