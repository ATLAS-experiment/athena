/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_OFFLINEOBJECTDECORHELPER_H
#define INDETTRACKPERFMON_OFFLINEOBJECTDECORHELPER_H

/**
 * @file OfflineObjectDecorHelper.h
 * @brief Utility methods to access
 *        offline object decorations
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// xAOD includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODJet/JetContainer.h"

#include "TrackParametersHelper.h"


/// STL includes
#include <string>


namespace IDTPM {

  /// Templated method to check if a track is not linked to an object
  template < typename container_t >
  bool isUnlinkedObject( const xAOD::TrackParticle& track,
                         const std::string& decoName ) {
    using elementLink_t = ElementLink< container_t >;
    const SG::ConstAccessor< elementLink_t > acc( decoName );
    return ( not acc.isAvailable( track ) );
  }

  /// Templated method to retrieve object linked to a track
  template < typename container_t >
  typename container_t::const_value_type getLinkedObject(
      const xAOD::TrackParticle& track,
      const std::string& decoName )
  {
    using elementLink_t = ElementLink< container_t >;

    if( isUnlinkedObject< container_t >( track, decoName ) ) return nullptr;

    const SG::ConstAccessor< elementLink_t > acc( decoName );
    elementLink_t eleLink = acc( track );

    if( not eleLink.isValid() ) return nullptr;

    return *eleLink;
  }

  /// Non-templated methods
  /// For offline electrons
  const xAOD::Electron* getLinkedElectron( const xAOD::TrackParticle& track,
                                           const std::string& quality="All" );

  /// For offline muons
  const xAOD::Muon* getLinkedMuon( const xAOD::TrackParticle& track,
                                   const std::string& quality="All" );

  /// For offline hadronic taus
  const xAOD::TauJet* getLinkedTau( const xAOD::TrackParticle& track,
                                    const int requiredNtracks,
                                    const std::string& type="RNN",
                                    const std::string& quality="" );

  /// For jets
  const xAOD::Jet* getLinkedJet( const xAOD::TrackParticle& track,
                                 const std::string& quality="DRtruthJet" );

  float getD0TrackInJet( const xAOD::TrackParticle& track,
                         const std::string& quality="DRtruthJet" );

  inline const xAOD::Jet* getLinkedJet( const xAOD::TruthParticle&,
                         const std::string& ="" ) { return nullptr; }; // dummy - to avoid compilation errors

  inline float getD0TrackInJet( const xAOD::TruthParticle&,
                         const std::string& ="" ) { return -999.; }; // dummy - to avoid compilation errors

  /// For truth particles
  bool isUnlinkedTruth( const xAOD::TrackParticle& track );
  inline bool isUnlinkedTruth( const xAOD::TruthParticle& ) { return false; }; // dummy - to avoid compilation errors

  float getTruthMatchProb( const xAOD::TrackParticle& track );
  inline float getTruthMatchProb( const xAOD::TruthParticle& ) { return -1.; }; // dummy - to avoid compilation errors

  const xAOD::TruthParticle* getLinkedTruth( const xAOD::TrackParticle& track,
                                             const float truthProbCut=0. );
  inline const xAOD::TruthParticle* getLinkedTruth(
      const xAOD::TruthParticle&, const float ) { return nullptr; }; // dummy - to avoid compilation errors

  template< typename PARTICLE >
  unsigned int getEtaBin( const PARTICLE& p,
                                const std::vector< float >& etaBins );

  template< typename PARTICLE >
  inline bool nHitsSelVec( const PARTICLE& p,
                           const std::vector< unsigned int >& minHits,
                           const std::vector< float >& etaBins) {return nSiHits(p) >= minHits.at( getEtaBin( p, etaBins ) );};

  template bool nHitsSelVec < xAOD::TruthParticle >( const xAOD::TruthParticle& truth,
                                                     const std::vector< unsigned int >& minHits,
                                                     const std::vector< float >& etaBins );
  template bool nHitsSelVec < xAOD::TrackParticle >( const xAOD::TrackParticle& track,
                                                     const std::vector< unsigned int >& minHits,
                                                     const std::vector< float >& etaBins );

  inline bool minPtSelVec( const xAOD::TrackParticle& track,
                           const std::vector< float >& minPt,
                           const std::vector< float >& etaBins) { return pT(track) >= minPt.at( getEtaBin( track, etaBins ) );};
  inline bool minPtSelVec( const xAOD::TruthParticle& ,
                           const std::vector< float >& ,
                           const std::vector< float >& ) { return false; }; // dummy - to avoid compilation errors;

  inline bool maxD0SelVec( const xAOD::TrackParticle& track,
                           const std::vector< float >& maxD0,
                           const std::vector< float >& etaBins) { return std::fabs( d0(track) ) <= maxD0.at( getEtaBin( track, etaBins ) );};
  inline bool maxD0SelVec( const xAOD::TruthParticle& ,
                           const std::vector< float >& ,
                           const std::vector< float >& ) { return false; }; // dummy - to avoid compilation errors;

  inline bool maxZ0SelVec( const xAOD::TrackParticle& track,
                           const std::vector< float >& maxZ0,
                           const std::vector< float >& etaBins) { return std::fabs( z0(track) ) <= maxZ0.at( getEtaBin( track, etaBins ) );};
  inline bool maxZ0SelVec( const xAOD::TruthParticle& ,
                           const std::vector< float >& ,
                           const std::vector< float >& ) { return false; }; // dummy - to avoid compilation errors;

  /// For vertices (truth and reco)
  /// get vertex-associated tracks and their weights
  bool getVertexTracksAndWeights( const xAOD::Vertex& vtx,
                                  std::vector< const xAOD::TrackParticle* >& vtxTracks,
                                  std::vector< float >& vtxTrackWeights,
                                  const std::vector< const xAOD::TrackParticle* >& selTracks = {},
                                  bool useSelected = false );
  inline bool getVertexTracksAndWeights( const xAOD::TruthVertex&,
                                         std::vector< const xAOD::TruthParticle* >&,
                                         std::vector< float >&,
                                         const std::vector< const xAOD::TruthParticle* >& /*selTracks*/ = {},
                                         bool /*useSelected*/ = false ) { return true; } // TODO: Change to get (outgoing?) truth particles

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_OFFLINEOBJECTDECORHELPER_H
