/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingSmearMonAlg.h"
#include "AthContainers/ConstAccessor.h"


EFTrackingSmearMonAlg::EFTrackingSmearMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) 
: AthHistogramAlgorithm( name, pSvcLocator ){}



StatusCode EFTrackingSmearMonAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK( m_inputTrackParticleKey.initialize() );
  ATH_CHECK( m_inputTruthParticleKey.initialize() );
  ATH_CHECK( m_smearedTrackParticleKey.initialize() );
  ATH_CHECK( m_smearedTruthParticleKey.initialize() );
  
  
  return StatusCode::SUCCESS;
}

StatusCode EFTrackingSmearMonAlg::execute() {
  auto ctx = getContext() ;

  SG::ReadHandle<xAOD::TrackParticleContainer> inputTracks_handle( m_inputTrackParticleKey, ctx );
  const xAOD::TrackParticleContainer* inputTracks = inputTracks_handle.cptr();
  if (not inputTracks) {
     ATH_MSG_FATAL("Unable to retrieve input ID tacks");
     return StatusCode::FAILURE;
  }


  ATH_MSG_DEBUG ("Found "<<inputTracks->size()<< " input tracks");
  // trakc particles
  for ( const auto* trk : *inputTracks ) 
    {            
      // get Cov matrix of input track
      xAOD::ParametersCovMatrix_t trkcov = trk->definingParametersCovMatrix();  
      auto trkcovvec = trk->definingParametersCovMatrixVec(); 
      if (trk->pt() == 0.) continue;      
      ATH_MSG_DEBUG ("Track: "
                      <<" curv=" << 1./trk->pt()
                      <<" phi="  << trk->phi()
                      <<" eta="  << trk->eta()
                      <<" d0="   << trk->d0()
                      <<" z0="   << trk->z0()
                      <<" pT="   << trk->pt()
                      <<" cov_d0=" << trkcov(Trk::d0,Trk::d0)
                      <<" cov_z0=" << trkcov(Trk::z0,Trk::z0)
                      <<" sigma_d0="  << std::sqrt(std::abs(trkcov(Trk::d0,Trk::d0)))
                      <<" sigma_z0="  << std::sqrt(std::abs(trkcov(Trk::z0,Trk::z0))) );      
    }
    
    ///////////////////
  SG::ReadHandle<xAOD::TrackParticleContainer> smearedTracks_handle( m_smearedTrackParticleKey, ctx );
  const xAOD::TrackParticleContainer* smearedTracks = smearedTracks_handle.cptr();
  if (not smearedTracks) {
     ATH_MSG_FATAL("Unable to retrieve smeared ID tacks");
     return StatusCode::FAILURE;
  }


  ATH_MSG_DEBUG ("Found "<<smearedTracks->size()<< " smeared tracks");
  // trakc particles
  for ( const auto* trk : *smearedTracks ) 
    {            
      // get Cov matrix of input track
      xAOD::ParametersCovMatrix_t trkcov = trk->definingParametersCovMatrix();  
      auto trkcovvec = trk->definingParametersCovMatrixVec(); 
      if (trk->pt() == 0.) continue;          
      ATH_MSG_DEBUG ("Smeared Track: "
                      <<" curv=" << 1./trk->pt()
                      <<" phi="  << trk->phi()
                      <<" eta="  << trk->eta()
                      <<" d0="   << trk->d0()
                      <<" z0="   << trk->z0()
                      <<" pT="   << trk->pt()
                      <<" cov_d0=" << trkcov(Trk::d0,Trk::d0)
                      <<" cov_z0=" << trkcov(Trk::z0,Trk::z0)
                      <<" sigma_d0="  << std::sqrt(std::abs(trkcov(Trk::d0,Trk::d0)))
                      <<" sigma_z0="  << std::sqrt(std::abs(trkcov(Trk::z0,Trk::z0))) );      
    }

    ///////////////////////////
    //truth
    SG::ReadHandle<xAOD::TruthParticleContainer> inputTruth_handle( m_inputTruthParticleKey, ctx );
    const xAOD::TruthParticleContainer* inputTruth = inputTruth_handle.cptr();
    if (not inputTruth) {
        ATH_MSG_FATAL("Unable to retrieve input truth particle");
        return StatusCode::FAILURE;
    }

    static const SG::ConstAccessor<float> ptAcc("pt");
    static const SG::ConstAccessor<float> d0Acc("d0");
    static const SG::ConstAccessor<float> z0Acc("z0");

    ATH_MSG_DEBUG ("Found "<<inputTruth->size()<< " input truth particles");
    for ( const auto* part : *inputTruth ) 
    {          
      if (part->pt() == 0.) continue;
      ATH_MSG_DEBUG ("===> Truth : "       
                      <<" curv=" << 1./part->pt()
                      <<" phi="  << part->phi()
                      <<" eta="  << part->eta()
                      <<" d0="   << d0Acc(*part)
                      <<" z0="   << z0Acc(*part)
                      <<" pT="   << part->pt()
                      <<" PDGID=" << part->pdgId()
                      <<" status=" << part->status()                                        
                      ); 
      if (part->parent(0)) ATH_MSG_DEBUG (" parent pdgId=" << part->parent(0)->pdgId()); 
    }

    ///////////////////////////
    
    SG::ReadHandle<xAOD::TruthParticleContainer> smearedTruth_handle( m_smearedTruthParticleKey, ctx );
    const xAOD::TruthParticleContainer* smearedTruth = smearedTruth_handle.cptr();
    if (not smearedTruth) {
        ATH_MSG_FATAL("Unable to retrieve input truth particle");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG ("Found "<<smearedTruth->size()<< " smeared truth particles");
    for ( const auto* part : *smearedTruth ) 
    {          
      if (part->pt() == 0.) continue;   
      ATH_MSG_DEBUG ("===> Truth : "       
                      <<" curv=" << 1./ptAcc(*part)
                      <<" phi="  << part->phi()
                      <<" eta="  << part->eta()
                      <<" d0="   << d0Acc(*part)
                      <<" z0="   << z0Acc(*part)
                      <<" pT="   << ptAcc(*part)
                      <<" PDGID=" << part->pdgId()
                      <<" status=" << part->status()                                        
                      ); 
      if (part->parent(0)) ATH_MSG_DEBUG (" parent pdgId=" << part->parent(0)->pdgId()); 
    }

  return StatusCode::SUCCESS;
}
