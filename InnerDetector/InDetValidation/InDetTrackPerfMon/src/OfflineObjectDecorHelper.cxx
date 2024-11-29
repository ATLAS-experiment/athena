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

  /// isFake
  bool isFakeTruth( const xAOD::TrackParticle& track, const float truthProbCut, const bool unlinkedAsFakes )
  {
    /// if fakes include unlinked, return true if isUnlinked is true
    if( unlinkedAsFakes and isUnlinkedTruth( track ) ) return true;
    float prob = getTruthMatchProb( track );
    /// returns true if truthMatchProbability deco isn't available or
    /// if the truth matching probability is below theshold
    return ( prob < truthProbCut );
  }

  /// isReconstructable
  bool isReconstructable( const xAOD::TruthParticle& truth, const std::vector<unsigned int>& minSilHits, const std::vector<float>& etaBins)
  {
    // Get eta bin
    float absEta = std::abs(truth.eta());
    absEta = std::clamp(absEta, etaBins.front(), etaBins.back());
    const auto pVal =  std::lower_bound(etaBins.begin(), etaBins.end(), absEta);
    const unsigned int bin = std::distance(etaBins.begin(), pVal) - 1;
    return ( nSiHits(truth) >= minSilHits.at( bin ) );
  }

} // namespace IDTPM
