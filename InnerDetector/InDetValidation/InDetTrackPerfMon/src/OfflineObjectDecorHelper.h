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
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

/// STL includes
#include <string>


namespace IDTPM {

  /// Templated method to check if a track is not linked to an object
  template < typename container_t >
  bool isUnlinkedObject( const xAOD::TrackParticle& track,
                         const std::string& decoName ) {
    using elementLink_t = ElementLink< container_t >;
    return ( not track.isAvailable< elementLink_t >( decoName ) );
  }

  /// Templated method to retrieve object linked to a track
  template < typename container_t >
  typename container_t::const_value_type getLinkedObject(
      const xAOD::TrackParticle& track,
      const std::string& decoName )
  {
    using elementLink_t = ElementLink< container_t >;

    if( isUnlinkedObject< container_t >( track, decoName ) ) return nullptr;

    elementLink_t eleLink = track.auxdata< elementLink_t >( decoName );

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

  /// For truth particles
  bool isUnlinkedTruth( const xAOD::TrackParticle& track );

  float getLinkedTruthMatchProb( const xAOD::TrackParticle& track );

  const xAOD::TruthParticle* getLinkedTruth( const xAOD::TrackParticle& track,
                                             const float truthProbCut=0. );

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_OFFLINEOBJECTDECORHELPER_H
