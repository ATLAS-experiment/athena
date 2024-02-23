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
/// TODO - To be included in next MRs
//#include "InDetTrackPerfMon/TrackMatchAccessHelper.h"

/// STD includes
#include <string>
#include <vector>
#include <sstream>


namespace IDTPM {

  class TrackAnalysisCollections : public AthMessaging {

  public:

    /// Enum for selection stages
    /// - FULL = full track collections, no selectrions
    /// - FS = Full-Scan track collections, after quality-based selection
    /// - InRoI = selected track collections inside the RoI
    enum Stage : size_t { FULL, FS, InRoI, NStages };

    /// Constructor 
    TrackAnalysisCollections( std::string anaTag );

    /// Destructor
    ~TrackAnalysisCollections() = default;

    /// = operator
    TrackAnalysisCollections& operator=( const TrackAnalysisCollections& ) = delete;

    /// load the TrkAnalysisDefinition service
    StatusCode loadTrkAnaDefSvc();

    /// --- Setter methods ---

    /// fill FULL collections and vectors
    StatusCode fillTruthTrackContainer(
      SG::ReadHandleKey<xAOD::TruthParticleContainer>& handleKey );

    StatusCode fillOfflTrackContainer(
      SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey );

    StatusCode fillTrigTrackContainer(
      SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey );

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
    StatusCode fillTruthTrackVec(
        const std::vector< const xAOD::TruthParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillOfflTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    StatusCode fillTrigTrackVec(
        const std::vector< const xAOD::TrackParticle* >& vec,
        Stage stage = FULL );

    /// set the chainRoiName
    void setChainRoiName( std::string chainRoiName ) { 
      m_chainRoiName = chainRoiName; 
    }

    /// --- Utility  methods ---

    /// check if collection are empty
    bool empty( Stage stage = FULL );

    /// Clear vectors
    void clear( Stage stage = FULL );

    /// copy content of FS vectors to InRoI vectors
    void copyFS();

    /// print Information about tracks in the collection(s)
    std::string printInfo( Stage stage = FULL ) const;

    /// TODO - to be included in later MRs
    /// return matching information 
    //IDTPM::TrackMatchAccessHelper matches();

    /// print matching information
    //std::string printMatchInfo();

    /// --- Getter methods ---

    /// get TrackAnalysis tag
    std::string anaTag() { return m_anaTag; }

    /// get full TEST containers
    const xAOD::TruthParticleContainer* testTruthContainer();
    const xAOD::TrackParticleContainer* testTrackContainer();

    /// get full REFERENCE containers
    const xAOD::TruthParticleContainer* refTruthContainer();
    const xAOD::TrackParticleContainer* refTrackContainer();

    /// get truth/offline/trigger track containers (TEST or REFERENCE)
    const xAOD::TruthParticleContainer* truthTrackContainer() {
      return m_truthTrackContainer; }
    const xAOD::TrackParticleContainer* offlTrackContainer() {
      return m_offlTrackContainer; }
    const xAOD::TrackParticleContainer* trigTrackContainer() {
      return m_trigTrackContainer; }

    /// get TEST track vectors
    std::vector< const xAOD::TruthParticle* > testTruthVec( Stage stage = FULL );
    std::vector< const xAOD::TrackParticle* > testTrackVec( Stage stage = FULL );

    /// get REFERENCE track vectors
    std::vector< const xAOD::TruthParticle* > refTruthVec( Stage stage = FULL );
    std::vector< const xAOD::TrackParticle* > refTrackVec( Stage stage = FULL );

    /// get truth/offline/trigger track vector (TEST or REFERENCE)
    std::vector< const xAOD::TruthParticle* > truthTrackVec( Stage stage = FULL ) {
      return m_truthTrackVec[ stage ]; }
    std::vector< const xAOD::TrackParticle* > offlTrackVec( Stage stage = FULL ) {
      return m_offlTrackVec[ stage ]; }
    std::vector< const xAOD::TrackParticle* > trigTrackVec( Stage stage = FULL ) {
      return m_trigTrackVec[ stage ]; }

  private:

    /// --- Collections class variables ---
    /// Full collections
    const xAOD::TruthParticleContainer* m_truthTrackContainer{nullptr};
    const xAOD::TrackParticleContainer* m_offlTrackContainer{nullptr};
    const xAOD::TrackParticleContainer* m_trigTrackContainer{nullptr};

    /// vectors of track/truth particles at different stages of the selection/workflow
    std::vector<std::vector< const xAOD::TruthParticle* >> m_truthTrackVec{};
    std::vector<std::vector< const xAOD::TrackParticle* >> m_offlTrackVec{};
    std::vector<std::vector< const xAOD::TrackParticle* >> m_trigTrackVec{};

    /// --- Other class variables ---
    std::string m_anaTag;
    std::string m_chainRoiName;
    ITrackAnalysisDefinitionSvc* m_trkAnaDefSvc;
 
  }; // class TrackAnalysisCollections

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRACKANALYSISCOLLECTIONS_H
