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


  /// isUnlinkedTruth
  bool isUnlinkedTruth( const xAOD::TrackParticle& track ) {
    return isUnlinkedObject< xAOD::TruthParticleContainer >(
        track, "truthParticleLink" );
  }


  /// getTruthMatchProb
  float getTruthMatchProb( const xAOD::TrackParticle& track ) {
    static const SG::ConstAccessor< float > truthMatchProbabilityAcc( "truthMatchProbability" );
    return truthMatchProbabilityAcc.withDefault( track, -1 );
  }


  /// getLinkedTruth
  const xAOD::TruthParticle* getLinkedTruth( const xAOD::TrackParticle& track,
                                             const float truthProbCut ) {
    float prob = getTruthMatchProb( track );
    if( prob < truthProbCut ) return nullptr;

    return getLinkedObject< xAOD::TruthParticleContainer >(
        track, "truthParticleLink" );
  }

  /// isFake
  bool isFakeTruth( const xAOD::TrackParticle& track, const float truthProbCut )
  {
    /// Truth-unlinked tracks should not be counted as fake
    if( isUnlinkedTruth( track ) ) return false;
    float prob = getTruthMatchProb( track );
    return ( prob < truthProbCut );
  }

} // namespace IDTPM
