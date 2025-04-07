/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGING_TRACK_AUGMENTER_BY_VERTEX_ALG_HH
#define BTAGGING_TRACK_AUGMENTER_BY_VERTEX_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "StoreGate/WriteDecorHandle.h"

namespace Analysis {


  class BTagTrackAugmenterByVertexAlg: public AthReentrantAlgorithm {
  public:
    BTagTrackAugmenterByVertexAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    StatusCode initialize() override final;
    StatusCode execute(const EventContext& ctx) const override final;

  private:
    ToolHandle< Trk::ITrackToVertexIPEstimator > m_track_to_vx {this,"TrackToVertexIPEstimator","Trk::TrackToVertexIPEstimator",""};
    ToolHandle< Trk::IExtrapolator >  m_extrapolator {this,"Extrapolator","Trk::Extrapolator",""};
    float m_dzCut;
    // Input Containers
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackContainerKey {this,"TrackContainer","InDetTrackParticles","Key for the input track collection"};
    SG::ReadHandleKey< xAOD::VertexContainer > m_VertexContainerKey {this,"PrimaryVertexContainer","PrimaryVertices","Key for the input vertex collection"};

    // Decorators for tracks
    // Decorator keys will be modified at run-time to conform to the correct container name
    // For the run-time update to work, the decoration key name properties must start with a period (".")
    Gaudi::Property< std::string > m_prefix{this,"prefix","btagIp_",""};

    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_d0 {this, "ByVertex_d0", "ByVertex_d0", "d0 of tracks"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_z0 {this, "ByVertex_z0SinTheta", "ByVertex_z0SinTheta", "z0SinTheta of tracks"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_d0_sigma {this, "ByVertex_d0Uncertainty", "ByVertex_d0Uncertainty", "d0Uncertainty of tracks"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_z0_sigma {this, "ByVertex_z0SinThetaUncertainty", "ByVertex_z0SinThetaUncertainty", "z0SinThetaUncertainty of tracks"};

    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_track_pos {this, "ByVertex_trackDisplacement","ByVertex_trackDisplacement","trackDisplacement of tracks" };
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_track_mom {this, "ByVertex_trackMomentum","ByVertex_trackMomentum","trackMomentum of tracks" };

    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trk_origin_vtx {this, "ByVertex_TrkOriginVertex", "ByVertex_TrkOriginVtx", "origin vertex of track"}; 
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trk_origin_vtx_idx {this, "ByVertex_TrkOriginVertex_idx", "ByVertex_TrkOriginVtx_idx", "origin vertex of track index"};

    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_invalid {
      this, "ByVertex_invalidIp", "ByVertex_invalidIp", "flag for invalid impact parameter"
    };
  };

}

#endif
