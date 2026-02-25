/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Version of the BTagTrackAugmenterAlg but applied to ByVertex jets for the pileup effort
*/

#include "BTagging/BTagTrackAugmenterByVertexAlg.h"

#include "TrkSurfaces/PerigeeSurface.h"


namespace Analysis {

  BTagTrackAugmenterByVertexAlg::BTagTrackAugmenterByVertexAlg( const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {
      declareProperty("dzCut", m_dzCut=10);
    }

  StatusCode BTagTrackAugmenterByVertexAlg::initialize() {
    ATH_MSG_INFO( "Inizializing " << name() << "... " );

    if ( m_track_to_vx.retrieve().isFailure() ) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_track_to_vx);
      return StatusCode::FAILURE;
    }

    if ( m_extrapolator.retrieve().isFailure() ) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_extrapolator);
      return StatusCode::FAILURE;
    }
    
    // Initialize Container keys
    ATH_MSG_DEBUG( "Inizializing containers:"        );
    ATH_MSG_DEBUG( "    ** " << m_TrackContainerKey  );
    ATH_MSG_DEBUG( "    ** " << m_VertexContainerKey );

    ATH_CHECK( m_TrackContainerKey.initialize() );
    ATH_CHECK( m_VertexContainerKey.initialize() );

    // Prepare decorators
    m_dec_d0       = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_d0.key();
    m_dec_z0       = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_z0.key();
    m_dec_d0_sigma = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_d0_sigma.key();
    m_dec_z0_sigma = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_z0_sigma.key();

    m_dec_track_pos = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_track_pos.key();
    m_dec_track_mom = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_track_mom.key();

    m_dec_invalid = m_TrackContainerKey.key() + "." + m_prefix.value() + m_dec_invalid.key();
    m_trk_origin_vtx = m_TrackContainerKey.key() + "." + m_prefix.value() + m_trk_origin_vtx.key();
    m_trk_origin_vtx_idx = m_TrackContainerKey.key() + "." + m_prefix.value() + m_trk_origin_vtx_idx.key();

    // Initialize decorators
    ATH_MSG_DEBUG( "Inizializing decorators:"  );
    ATH_MSG_DEBUG( "    ** " << m_dec_d0       );
    ATH_MSG_DEBUG( "    ** " << m_dec_z0       );
    ATH_MSG_DEBUG( "    ** " << m_dec_d0_sigma );
    ATH_MSG_DEBUG( "    ** " << m_dec_z0_sigma );
    ATH_MSG_DEBUG( "    ** " << m_dec_track_pos );
    ATH_MSG_DEBUG( "    ** " << m_dec_track_mom );
    ATH_MSG_DEBUG( "    ** " << m_dec_invalid  );
    ATH_MSG_DEBUG( "    ** " << m_trk_origin_vtx  );
    ATH_MSG_DEBUG( "    ** " << m_trk_origin_vtx_idx  );
    

    CHECK( m_dec_d0.initialize() );
    CHECK( m_dec_z0.initialize() );
    CHECK( m_dec_d0_sigma.initialize() );
    CHECK( m_dec_z0_sigma.initialize() );
    CHECK( m_dec_track_pos.initialize() );
    CHECK( m_dec_track_mom.initialize() );
    CHECK( m_dec_invalid.initialize() );
    CHECK( m_trk_origin_vtx.initialize() );
    CHECK( m_trk_origin_vtx_idx.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode BTagTrackAugmenterByVertexAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );
  
    // ========================================================================================================================== 
    //    ** Retrieve Ingredients
    // ========================================================================================================================== 

    SG::ReadHandle< xAOD::VertexContainer > vertexContainerHandle = SG::makeHandle< xAOD::VertexContainer >( m_VertexContainerKey,ctx );
    CHECK( vertexContainerHandle.isValid() );
    const xAOD::VertexContainer *verteces = vertexContainerHandle.get();
   
    SG::ReadHandle< xAOD::TrackParticleContainer > trackContainerHandle = SG::makeHandle< xAOD::TrackParticleContainer >( m_TrackContainerKey,ctx);
    CHECK( trackContainerHandle.isValid() );
    const xAOD::TrackParticleContainer* tracks = trackContainerHandle.get();
    ATH_MSG_DEBUG( "Retrieved " << tracks->size() << " input tracks..." );


    // ========================================================================================================================== 
    //    ** Make Decorators (these are outputs)
    // ==========================================================================================================================

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float> > decor_d0(m_dec_d0, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float> > decor_z0(m_dec_z0, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float> > decor_d0_sigma(m_dec_d0_sigma, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float> > decor_z0_sigma(m_dec_z0_sigma, ctx);

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<std::vector<float>> > decor_track_pos(m_dec_track_pos, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<std::vector<float>> > decor_track_mom(m_dec_track_mom, ctx);

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<ElementLink<xAOD::VertexContainer>>> decor_TrkOriginVtx(m_trk_origin_vtx, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> decor_TrkOriginVtx_idx(m_trk_origin_vtx_idx, ctx);
    
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<char> > decor_invalid(
        m_dec_invalid, ctx);

    
    // ==========================================================================================================================
    //    ** Computation
    // ==========================================================================================================================
    
    // now decorate the tracks
    for (const xAOD::TrackParticle *track: *tracks) {
      //counter for the vertex index 
      int vtx_i=0;
      //create vectors to fill
      std::vector<ElementLink<xAOD::VertexContainer>> vertexLink;
      std::vector<float> d0;
      std::vector<float> z0SinTheta;
      std::vector<float> sigmad0;
      std::vector<float> sigmaz0SinTheta;
      std::vector<int> vertex_index;
      std::vector<std::vector<float>> track_out_vec_pos;
      std::vector<std::vector<float>> track_out_vec_mom;
      std::vector<char> invalid_vec;
      // loop through vertices
      for (const xAOD::Vertex *primary: *verteces) {
        // get IP and extra pars
        std::unique_ptr< const Trk::ImpactParametersAndSigma > ip( m_track_to_vx->estimate( track, primary) );
        Trk::PerigeeSurface primary_surface( primary->position() );
        std::unique_ptr< const Trk::TrackParameters > extrap_pars( m_extrapolator->extrapolate(ctx,
                                                                                            track->perigeeParameters(),
                                                                                            primary_surface ) );
        
        if ( ip ){
          // if vertex to track is below a certain cut, save corresponding IP params
          if ( std::fabs(ip->IPz0SinTheta) < m_dzCut){
            d0.push_back(ip->IPd0);
            z0SinTheta.push_back(ip->IPz0SinTheta);
            sigmad0.push_back(ip->sigmad0);
            sigmaz0SinTheta.push_back(ip->sigmaz0SinTheta);
            vertex_index.push_back(vtx_i);
            vertexLink.push_back(ElementLink<xAOD::VertexContainer>(*verteces, vtx_i));
            
          }else{
            d0.push_back(-99);
            z0SinTheta.push_back(-99);
            sigmad0.push_back(-99);
            sigmaz0SinTheta.push_back(-99);
            vertex_index.push_back(vtx_i);
            vertexLink.push_back(ElementLink<xAOD::VertexContainer>(*verteces, vtx_i));
          }
        }else {
          ATH_MSG_WARNING( "failed to estimate track impact parameter, using dummy values" );
          d0.push_back(-99);
          z0SinTheta.push_back(-99);
          sigmad0.push_back(-99);
          sigmaz0SinTheta.push_back(-99);
          vertex_index.push_back(vtx_i);
          vertexLink.push_back(ElementLink<xAOD::VertexContainer>(*verteces, vtx_i));
        }
        // some other parameters we have go get directly from the
        // extrapolator. This is more or less copied from:
        // https://goo.gl/iWLv5T
        if ( extrap_pars ) {
          const Amg::Vector3D& track_pos = extrap_pars->position();
          const Amg::Vector3D& vertex_pos = primary->position();
  
          const Amg::Vector3D position = track_pos - vertex_pos;
          const Amg::Vector3D momentum = extrap_pars->momentum();
  
          //Test output for cross checking output with stored values
          ATH_MSG_DEBUG( "vertex_pos (x,y,z)= (" << vertex_pos.x() << ", " << vertex_pos.y() << ", " << vertex_pos.z() << ")");
          ATH_MSG_DEBUG( "track_pos (x,y,z)= (" << track_pos.x() << ", " << track_pos.y() << ", " << track_pos.z() << ")");
          ATH_MSG_DEBUG( "track_displacement (x,y,z)= (" << position.x() << ", " << position.y() << ", " << position.z() << ")");
          ATH_MSG_DEBUG( "track_momentum (x,y,z)= (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")");
  
          std::vector< float > out_vec_pos( position.data(), position.data() + position.size() );
          std::vector< float > out_vec_mom( momentum.data(), momentum.data() + momentum.size() );
          
          track_out_vec_pos.push_back(out_vec_pos);
          track_out_vec_mom.push_back(out_vec_mom);
        }
        bool invalid = !(ip && extrap_pars);
        invalid_vec.push_back(invalid ? 1 : 0);
        
        vtx_i++;
      }
      decor_TrkOriginVtx(*track) = vertexLink;
      decor_TrkOriginVtx_idx(*track) = vertex_index;
      decor_d0(*track) = d0;
      decor_z0(*track) = z0SinTheta;
      decor_d0_sigma(*track) = sigmad0;
      decor_z0_sigma(*track) = sigmaz0SinTheta;
      decor_track_pos (*track) = track_out_vec_pos;
      decor_track_mom (*track) = track_out_vec_mom;
      decor_invalid(*track) = invalid_vec;
      
    }

    return StatusCode::SUCCESS;
  }
}


