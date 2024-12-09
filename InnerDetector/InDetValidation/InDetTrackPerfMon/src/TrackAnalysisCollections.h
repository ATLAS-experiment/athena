/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthPileupEventContainer.h"
#include "xAODTracking/VertexContainer.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "ITrackMatchingLookup.h"

/// STD includes
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <utility>


namespace IDTPM {

  class TrackAnalysisCollections : public AthMessaging {

  public:

    /// Enum for selection stages
    /// - FULL = full track collections, no selectrions
    /// - FS = Full-Scan track collections, after quality-based selection
    /// - InRoI = selected track collections inside the RoI
    enum Stage : size_t { FULL, FS, InRoI, NStages };

    /// Enum for counting objects at various stages 
    enum Counter : size_t { ALL, SELECTED, INROI, MATCHED, NCOUNTERS };

    /// Constructor 
    TrackAnalysisCollections( const std::string& anaTag );

    /// Destructor
    ~TrackAnalysisCollections() = default;

    /// = operator
    TrackAnalysisCollections& operator=( const TrackAnalysisCollections& ) = delete;

    /// initialize
    StatusCode initialize();

    /// --- Setter methods ---

    /// fill event info
    StatusCode fillEventInfo(
        const SG::ReadHandleKey<xAOD::EventInfo>& eventInfoHandleKey,
        const SG::ReadHandleKey< xAOD::TruthEventContainer >& truthEventHandleKey,
        const SG::ReadHandleKey< xAOD::TruthPileupEventContainer >& truthPUEventHandleKey );

    /// fill FULL track collections and vectors
    StatusCode fillTruthPartContainer(
        const SG::ReadHandleKey< xAOD::TruthParticleContainer >& truthPartHandleKey );

    StatusCode fillOfflTrackContainer(
        const SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey );

    StatusCode fillTrigTrackContainer(
        const SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey );

    /// fill TEST track vectors
    StatusCode fillTestTruthVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillTestTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// fill REFERENCE track vectors
    StatusCode fillRefTruthVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillRefTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// fill truth/offline/trigger track vector (TEST or REFERENCE)
    StatusCode fillTruthPartVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillOfflTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillTrigTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// fill FULL vertex collections and vectors
    StatusCode fillTruthVertexContainer(
        const SG::ReadHandleKey< xAOD::TruthVertexContainer >& truthVertexHandleKey );

    StatusCode fillOfflVertexContainer(
        const SG::ReadHandleKey< xAOD::VertexContainer >& handleKey );

    StatusCode fillTrigVertexContainer(
        const SG::ReadHandleKey< xAOD::VertexContainer >& handleKey );

    /// fill TEST vertex vectors
    StatusCode fillTestTruthVertexVec(
        const std::vector< const xAOD::TruthVertex* >& vec,
        Stage stage = FULL );

    StatusCode fillTestRecoVertexVec(
        const std::vector< const xAOD::Vertex* >& vec,
        Stage stage = FULL );

    /// fill REFERENCE vertex vectors
    StatusCode fillRefTruthVertexVec(
        const std::vector< const xAOD::TruthVertex* >& vec,
        Stage stage = FULL );

    StatusCode fillRefRecoVertexVec(
        const std::vector< const xAOD::Vertex* >& vec,
        Stage stage = FULL );

    /// fill truth/offline/trigger vertex vector (TEST or REFERENCE)
    StatusCode fillTruthVertexVec(
        const std::vector< const xAOD::TruthVertex* >& vec,
        Stage stage = FULL );

    StatusCode fillOfflVertexVec(
        const std::vector< const xAOD::Vertex* >& vec,
        Stage stage = FULL );

    StatusCode fillTrigVertexVec(
        const std::vector< const xAOD::Vertex* >& vec,
        Stage stage = FULL );

    /// --- Utility  methods ---

    /// check if collection are empty
    bool empty( Stage stage = FULL );

    /// Clear vectors
    void clear( Stage stage = FULL );

    /// copy content of FS vectors to InRoI vectors
    void copyFS();

    /// print information about tracks in the collection(s)
    std::string printInfo( Stage stage = FULL, bool printVertex = true ) const;

    /// print information about vertices in the collection(s)
    std::string printVertexInfo( Stage stage = FULL ) const;

    /// --- Getter methods ---

    /// get TrackAnalysis tag
    const std::string& anaTag() { return m_anaTag; }

    /// get event info
    const xAOD::EventInfo* eventInfo() { return m_eventInfo; }
    const xAOD::TruthEventContainer* truthEventContainer() { return m_truthEventContainer; }
    const xAOD::TruthPileupEventContainer* truthPileupEventContainer() { return m_truthPUEventContainer; }

    /// get full TEST track containers
    const xAOD::TruthParticleContainer* testTruthContainer();
    const xAOD::TrackParticleContainer* testTrackContainer();

    /// get full REFERENCE track containers
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

    /// get counts vectors for TEST/REFERENCE track vectors at all stages
    std::vector< size_t > testTrackCounts();
    std::vector< size_t > refTrackCounts();

    /// get full TEST vertex containers
    const xAOD::TruthVertexContainer* testTruthVertexContainer();
    const xAOD::VertexContainer*      testRecoVertexContainer();

    /// get full REFERENCE vertex containers
    const xAOD::TruthVertexContainer* refTruthVertexContainer();
    const xAOD::VertexContainer*      refRecoVertexContainer();

    /// get truth/offline/trigger vertex containers (TEST or REFERENCE)
    const xAOD::TruthVertexContainer* truthVertexContainer() {
      return m_truthVertexContainer; }
    const xAOD::VertexContainer* offlVertexContainer() {
      return m_offlVertexContainer; }
    const xAOD::VertexContainer* trigVertexContainer() {
      return m_trigVertexContainer; }

    /// get TEST vertex vectors
    const std::vector< const xAOD::TruthVertex* >&  testTruthVertexVec( Stage stage = FULL );
    const std::vector< const xAOD::Vertex* >&       testRecoVertexVec( Stage stage = FULL );

    /// get REFERENCE vertex vectors
    const std::vector< const xAOD::TruthVertex* >&  refTruthVertexVec( Stage stage = FULL );
    const std::vector< const xAOD::Vertex* >&       refRecoVertexVec( Stage stage = FULL );

    /// get truth/offline/trigger vertex vector (TEST or REFERENCE)
    const std::vector< const xAOD::TruthVertex* >& truthVertexVec( Stage stage = FULL ) {
      return m_truthVertexVec[ stage ]; }
    const std::vector< const xAOD::Vertex* >& offlVertexVec( Stage stage = FULL ) {
      return m_offlVertexVec[ stage ]; }
    const std::vector< const xAOD::Vertex* >& trigVertexVec( Stage stage = FULL ) {
      return m_trigVertexVec[ stage ]; }

    /// get counts vectors for TEST/REFERENCE vertex vectors at all stages
    std::vector< size_t > testVertexCounts();
    std::vector< size_t > refVertexCounts();

    /// get track matching information 
    ITrackMatchingLookup& matches() { return *m_matches; }

    /// print track matching information
    std::string printMatchInfo();

    /// update chainRois map
    bool updateChainRois( const std::string& chainRoi, const std::string& roiStr );

  private:

    /// TrackAnalysis properties
    std::string m_anaTag;
    SmartIF< ITrackAnalysisDefinitionSvc > m_trkAnaDefSvc;

    /// --- Collections class variables ---
    /// EventInfo, TruthEvent, and TruthPUEvent
    const xAOD::EventInfo* m_eventInfo{nullptr};
    const xAOD::TruthEventContainer* m_truthEventContainer{nullptr};
    const xAOD::TruthPileupEventContainer* m_truthPUEventContainer{nullptr};

    /// Full track collections
    const xAOD::TruthParticleContainer* m_truthPartContainer{nullptr};
    const xAOD::TrackParticleContainer* m_offlTrackContainer{nullptr};
    const xAOD::TrackParticleContainer* m_trigTrackContainer{nullptr};

    /// vectors of track/truth particles at different stages of the selection/workflow
    std::vector<std::vector< const xAOD::TruthParticle* >> m_truthPartVec{};
    std::vector<std::vector< const xAOD::TrackParticle* >> m_offlTrackVec{};
    std::vector<std::vector< const xAOD::TrackParticle* >> m_trigTrackVec{};

    /// Full vertex collections
    const xAOD::TruthVertexContainer* m_truthVertexContainer{nullptr};
    const xAOD::VertexContainer*      m_offlVertexContainer{nullptr};
    const xAOD::VertexContainer*      m_trigVertexContainer{nullptr};

    /// vectors of reco/truth vertices at different stages of the selection/workflow
    std::vector<std::vector< const xAOD::TruthVertex* >>  m_truthVertexVec{};
    std::vector<std::vector< const xAOD::Vertex* >>       m_offlVertexVec{};
    std::vector<std::vector< const xAOD::Vertex* >>       m_trigVertexVec{};

    /// null vectors
    std::vector< const xAOD::TrackParticle* > m_nullTrackVec{};
    std::vector< const xAOD::TruthParticle* > m_nullTruthVec{};
    std::vector< const xAOD::TruthVertex* >   m_nullTruthVertVec{};
    std::vector< const xAOD::Vertex* >        m_nullRecoVertVec{};

    /// Lookup table for test-reference matching
    std::unique_ptr< ITrackMatchingLookup > m_matches;

    /// map of chainRoiNames for caching
    typedef std::unordered_map< std::string, std::string > mapChainRoi_t;
    mapChainRoi_t m_chainRois{};
 
  }; // class TrackAnalysisCollections

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRACKANALYSISCOLLECTIONS_H
