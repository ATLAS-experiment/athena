/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file OfflineObjectDecorHelper.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 **/

/// local includes
#include "OfflineObjectDecorHelper.h"
#include "AthContainers/ConstAccessor.h"
#include "TrackParametersHelper.h"

namespace IDTPM {

  /// getLinkedElectron
  const xAOD::Electron* getLinkedElectron( const xAOD::TrackParticle& track,
                                           const std::string& quality ) {
    std::string decoName = "LinkedElectron_" + quality;
    return getLinkedObject< xAOD::ElectronContainer >( track, decoName );
  }


  /// getLinkedMuon
  const xAOD::Muon* getLinkedMuon( const xAOD::TrackParticle& track,
                                   const std::string& quality ) {
    std::string decoName = "LinkedMuon_" + quality;
    return getLinkedObject< xAOD::MuonContainer >( track, decoName );
  }


  /// getLinkedTau
  const xAOD::TauJet* getLinkedTau( const xAOD::TrackParticle& track,
                                    const int requiredNtracks,
                                    const std::string& type,
                                    const std::string& quality ) {
    std::string decoName = "LinkedTau" + type +
        std::to_string( requiredNtracks ) + "_" + quality;
    return getLinkedObject< xAOD::TauJetContainer >( track, decoName );
  }


  /// getLinkedJet
  const xAOD::Jet* getLinkedJet( const xAOD::TrackParticle& track,
                                 const std::string& quality ) {
    std::string decoName = "LinkedJet_" + quality;
    return getLinkedObject< xAOD::JetContainer >( track, decoName );
  }


  /// recompute d0 projecting in jet direction w.r.t. origin
  float getD0TrackInJet( const xAOD::TrackParticle& track,
                         const std::string& quality ) {
    const xAOD::Jet* jet = getLinkedJet( track, quality );
    if( not jet ) return -999.;

    //Amg::Vector3D nullPos( 0, 0, 0 );
    Amg::Vector3D jetDirection( jet->px(), jet->py(), jet->pz() );
    Amg::Vector3D unit = jetDirection.unit();

    float vs = std::sin( std::atan2( unit.y(), unit.x() ) - track.phi() ) * track.d0();
    float signD0 = vs>=0. ? 1. : -1.;
    return signD0 * std::fabs( track.d0() );
  }


  /// isUnlinkedTruth
  bool isUnlinkedTruth( const xAOD::TrackParticle& track ) {
    const xAOD::TruthParticle* truth = getLinkedObject< xAOD::TruthParticleContainer >(
        track, "truthParticleLink" );
    return ( truth == nullptr );
  }


  /// getTruthMatchProb
  float getTruthMatchProb( const xAOD::TrackParticle& track ) {
    static const SG::ConstAccessor< float > truthMatchProbabilityAcc( "truthMatchProbability" );
    float prob = truthMatchProbabilityAcc.withDefault( track, -1 );
    if( std::isnan(prob) ) return -1;
    return prob;
  }

  /// getLinkedTruth
  const xAOD::TruthParticle* getLinkedTruth( const xAOD::TrackParticle& track,
                                             const float truthProbCut ) {
    float prob = getTruthMatchProb( track );
    if( std::isnan(prob) ) return nullptr;
    if( prob <= truthProbCut ) return nullptr;

    return getLinkedObject< xAOD::TruthParticleContainer >(
        track, "truthParticleLink" );
  }

  template < typename PARTICLE >
  unsigned int getEtaBin (const PARTICLE& p, const std::vector<float>& etaBins)  {
    float absEta = std::abs(p.eta());
    absEta = std::clamp(absEta, etaBins.front(), etaBins.back());
    const auto pVal =  std::lower_bound(etaBins.begin(), etaBins.end(), absEta);
    const unsigned int bin = std::distance(etaBins.begin(), pVal) - 1;
    return bin;
  }
  template unsigned int getEtaBin < xAOD::TruthParticle >(const xAOD::TruthParticle& truth, const std::vector<float>& etaBins);
  template unsigned int getEtaBin < xAOD::TrackParticle >(const xAOD::TrackParticle& track, const std::vector<float>& etaBins);


  /// getVertexTracksAndWeights
  bool getVertexTracksAndWeights( const xAOD::Vertex& vtx,
                                  std::vector< const xAOD::TrackParticle* >& vtxTracks,
                                  std::vector< float >& vtxTrackWeights,
                                  const std::vector< const xAOD::TrackParticle* >& selTracks,
                                  bool useSelected )
  {
    bool success( true );

    /// clear output vectors
    vtxTracks.clear();
    vtxTrackWeights.clear();

    /// dummy vertices -> no associated tracks. return
    if( vtx.vertexType() == xAOD::VxType::NoVtx ) return success;

    /// getting associated tracks and track weights
    size_t nTracks = vtx.nTrackParticles();
    xAOD::Vertex::TrackParticleLinks_t elVec = vtx.trackParticleLinks();
    std::vector< float > wVec = vtx.trackWeights();

    /// check if sizes match
    if( not( nTracks == elVec.size() and nTracks == wVec.size() ) ) return false; // shouldn't happen

    /// Loop over associated tracks
    for( size_t it=0 ; it<nTracks ; it++ ) {
      /// skipping non-valid track ElementLinks
      if( not elVec[ it ].isValid() ) {
        success = false; // shouldn't happen
        continue;
      }

      const xAOD::TrackParticle* thisTrk = *elVec[ it ];
      float thisTrkW = wVec[ it ];

      /// if requested, skip if associated track is not in selected track vector
      if( useSelected and
          std::find( selTracks.begin(), selTracks.end(), thisTrk ) == selTracks.end() ) continue;

      /// Filling associated track and track weight vectors
      vtxTracks.push_back( thisTrk );
      vtxTrackWeights.push_back( thisTrkW );
    } // close associated tracks loop

    return success;
  }

} // namespace IDTPM
