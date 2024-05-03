/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKANALYSISCOLLECTIONS_H
#define INDETTRACKPERFMON_TRACKANALYSISCOLLECTIONS_H

/**
 * @file   TrackAnalysisCollections.h
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date   30 June 2023
 * @brief  Class to hold for each event collections needed in the TrkAnalsis 
 */

/// Athena includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandle.h"

/// EDM includes
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "ITrackMatchingLookup.h"

/// STD includes
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>


namespace IDTPM {

  class TrackAnalysisCollections : public AthMessaging {

  public:

    /// Enum for selection stages
    /// - FULL = full track collections, no selectrions
    /// - FS = Full-Scan track collections, after quality-based selection
    /// - InRoI = selected track collections inside the RoI
    enum Stage : size_t { FULL, FS, InRoI, NStages };

    /// Constructor 
    TrackAnalysisCollections( const std::string& anaTag );

    /// Destructor
    ~TrackAnalysisCollections() = default;

    /// = operator
    TrackAnalysisCollections& operator=( const TrackAnalysisCollections& ) = delete;

    /// initialize
    StatusCode initialize();

    /// --- Setter methods ---

    /// fill FULL collections and vectors
    StatusCode fillTruthPartContainer(
        const SG::ReadHandleKey<xAOD::TruthParticleContainer>& handleKey );

    StatusCode fillOfflTrackContainer(
        const SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey );

    StatusCode fillTrigTrackContainer(
        const SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey );

    /// fill TEST vectors
    StatusCode fillTestTruthVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillTestTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// fill REFERENCE vectors
    StatusCode fillRefTruthVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillRefTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// get truth/offline/trigger track vector (TEST or REFERENCE)
    StatusCode fillTruthPartVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillOfflTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillTrigTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// --- Utility  methods ---

    /// check if collection are empty
    bool empty( Stage stage = FULL );

    /// Clear vectors
    void clear( Stage stage = FULL );

    /// copy content of FS vectors to InRoI vectors
    void copyFS();

    /// print Information about tracks in the collection(s)
    std::string printInfo( Stage stage = FULL ) const;

    /// --- Getter methods ---

    /// get TrackAnalysis tag
    const std::string& anaTag() { return m_anaTag; }

    /// get full TEST containers
    const xAOD::TruthParticleContainer* testTruthContainer();
    const xAOD::TrackParticleContainer* testTrackContainer();

    /// get full REFERENCE containers
    const xAOD::TruthParticleContainer* refTruthContainer();
    const xAOD::TrackParticleContainer* refTrackContainer();

    /// get truth/offline/trigger track containers (TEST or REFERENCE)
    const xAOD::TruthParticleContainer* truthPartContainer() {
      return m_truthPartContainer; }
    const xAOD::TrackParticleContainer* offlTrackContainer() {
      return m_offlTrackContainer; }
    const xAOD::TrackParticleContainer* trigTrackContainer() {
      return m_trigTrackContainer; }

    /// get TEST track vectors
    const std::vector< const xAOD::TruthParticle* >& testTruthVec( Stage stage = FULL );
    const std::vector< const xAOD::TrackParticle* >& testTrackVec( Stage stage = FULL );

    /// get REFERENCE track vectors
    const std::vector< const xAOD::TruthParticle* >& refTruthVec( Stage stage = FULL );
    const std::vector< const xAOD::TrackParticle* >& refTrackVec( Stage stage = FULL );

    /// get truth/offline/trigger track vector (TEST or REFERENCE)
    const std::vector< const xAOD::TruthParticle* >& truthPartVec( Stage stage = FULL ) {
      return m_truthPartVec[ stage ]; }
    const std::vector< const xAOD::TrackParticle* >& offlTrackVec( Stage stage = FULL ) {
      return m_offlTrackVec[ stage ]; }
    const std::vector< const xAOD::TrackParticle* >& trigTrackVec( Stage stage = FULL ) {
      return m_trigTrackVec[ stage ]; }

    /// return matching information 
    ITrackMatchingLookup& matches() { return *m_matches; }

    /// print matching information
    std::string printMatchInfo();

    /// update chainRois map
    bool updateChainRois( const std::string& chainRoi, const std::string& roiStr );

  private:

    /// TrackAnalysis properties
    std::string m_anaTag;
    ITrackAnalysisDefinitionSvc* m_trkAnaDefSvc;

    /// --- Collections class variables ---
    /// Full collections
    const xAOD::TruthParticleContainer* m_truthPartContainer{nullptr};
    const xAOD::TrackParticleContainer* m_offlTrackContainer{nullptr};
    const xAOD::TrackParticleContainer* m_trigTrackContainer{nullptr};

    /// vectors of track/truth particles at different stages of the selection/workflow
    std::vector<std::vector< const xAOD::TruthParticle* >> m_truthPartVec{};
    std::vector<std::vector< const xAOD::TrackParticle* >> m_offlTrackVec{};
    std::vector<std::vector< const xAOD::TrackParticle* >> m_trigTrackVec{};

    /// null vectors
    std::vector< const xAOD::TrackParticle* > m_nullTrackVec{};
    std::vector< const xAOD::TruthParticle* > m_nullTruthVec{};

    /// Lookup table for test-reference matching
    std::unique_ptr< ITrackMatchingLookup > m_matches;

    /// map of chainRoiNames for caching
    typedef std::unordered_map< std::string, std::string > mapChainRoi_t;
    mapChainRoi_t m_chainRois{};
 
  }; // class TrackAnalysisCollections

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRACKANALYSISCOLLECTIONS_H
