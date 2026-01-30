/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_INDETTRACKPERFMONTOOL_H
#define INDETTRACKPERFMON_INDETTRACKPERFMONTOOL_H

/**
 * @file InDetTrackPerfMonTool.h
 * header file for class of same name
 * @author marco aparo
 * @date 16 February 2023
**/

/// gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// Athena includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

/// EDM includes
#include "xAODCore/BaseContainer.h"
#include "xAODCore/AuxContainerBase.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackPerfMon/RoiSelectionTool.h"
#include "InDetTrackPerfMon/TrackRoiSelectionTool.h"
#include "InDetTrackPerfMon/VertexRoiSelectionTool.h"
#include "ITrackSelectionTool.h"
#include "IVertexSelectionTool.h"
#include "ITrackMatchingTool.h"
#include "InDetTrackPerfMon/TrackAnalysisPlotsMgr.h"
#include "InDetTrackPerfMon/TrackAnalysisInfoWriteTool.h"

/// STL includes
#include <string>
#include <vector>


class InDetTrackPerfMonTool : public ManagedMonitorToolBase {

public :

    /// Constructor with parameters
    InDetTrackPerfMonTool( const std::string& type, const std::string& name, const IInterface* parent );

    /// Destructor
    virtual ~InDetTrackPerfMonTool();

    virtual StatusCode initialize();
    virtual StatusCode bookHistograms();
    virtual StatusCode fillHistograms();
    virtual StatusCode procHistograms();

private :

    /// prevent default construction
    InDetTrackPerfMonTool();

    /// retrieve all collections and load them into trkAnaCollections object
    StatusCode loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls );

    /// Offline TrackParticleContainer name
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_offlineTrkParticleName {
        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };

    /// Trigger TrackParticleContainer name
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_triggerTrkParticleName {
        this, "TriggerTrkParticleContainerName", "", "Name of container of trigger tracks" };

    /// TruthParticle container name
    SG::ReadHandleKey< xAOD::TruthParticleContainer > m_truthParticleName {
        this, "TruthParticleContainerName",  "TruthParticles", "Name of container of TruthParticles" };

    /// TruthEvent container name
    SG::ReadHandleKey< xAOD::TruthEventContainer > m_truthEventName {
        this, "TruthEvents", "TruthEvents", "Name of the truth events container probably either TruthEvent || TruthEvents" };

    /// TruthPileupEvent container name
    SG::ReadHandleKey< xAOD::TruthPileupEventContainer > m_truthPileUpEventName {
        this, "TruthPileupEvents", "TruthPileupEvents", "Name of the truth pileup events container probably TruthPileupEvent(s)" };

    /// EventInfo container name
    SG::ReadHandleKey< xAOD::EventInfo > m_eventInfoContainerName {
        this, "EventInfoContainerName", "EventInfo", "event info" };

    /// Offline Primary vertex container name
    SG::ReadHandleKey< xAOD::VertexContainer > m_offlineVertexContainerName {
        this, "OfflineVertexContainerName", "PrimaryVertices", "" };

    /// Trigger Primary vertex container name
    SG::ReadHandleKey< xAOD::VertexContainer > m_triggerVertexContainerName {
        this, "TriggerVertexContainerName", "", "" };

    /// Truth vertex container name
    SG::ReadHandleKey< xAOD::TruthVertexContainer > m_truthVertexContainerName {
        this, "TruthVertexContainerName",  "TruthVertices", "" };

    /// WriteHandle for trkAnaInfo for reprocessing
    SG::WriteHandleKey< xAOD::BaseContainer > m_trkAnaInfoKey {
        this, "TrkAnaInfoKey", "TrackAnalysisInfo", "Dedicated TrackAnalysis Info written out" };

    /// -----------------------------
    /// --------- Sub-Tools ---------
    /// -----------------------------

    ToolHandle< IDTPM::ITrackSelectionTool > m_trackQualitySelectionTool {
        this, "TrackQualitySelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool", "Wrapper-tool to perform general quality-based track(truth) selection" };

    ToolHandle< IDTPM::IVertexSelectionTool > m_vertexQualitySelectionTool {
        this, "VertexQualitySelectionTool", "IDTPM::InDetTrackPerfMon/IVertexSelectionTool", "Wrapper-tool to perform general quality-based (truth) vertex selection" };

    ToolHandle< IDTPM::RoiSelectionTool > m_roiSelectionTool {
        this, "RoiSelectionTool", "IDTPM::InDetTrackPerfMon/RoiSelectionTool", "Tool to retrieve and select RoIs" };

    ToolHandle< IDTPM::TrackRoiSelectionTool > m_trackRoiSelectionTool {
        this, "TrackRoiSelectionTool", "IDTPM::InDetTrackPerfMon/TrackRoiSelectionTool", "Tool to select track within a RoI" };

    ToolHandle< IDTPM::VertexRoiSelectionTool > m_vertexRoiSelectionTool {
        this, "VertexRoiSelectionTool", "IDTPM::InDetTrackPerfMon/VertexRoiSelectionTool", "Tool to select vertices within a RoI" };

    ToolHandle< IDTPM::ITrackMatchingTool > m_trackMatchingTool {
        this, "TrackMatchingTool", "IDTPM::InDetTrackPerfMon/ITrackMatchingTool", "Tool to match test to reference tracks and viceversa" };

    ToolHandle< IDTPM::TrackAnalysisInfoWriteTool > m_trkAnaInfoWriteTool {
        this, "TrackAnalysisInfoWriteTool", "IDTPM::InDetTrackPerfMon/TrackAnalysisInfoWriteTool", "Tool to write TrackAnalysisInfo to StoreGate" };

    StringProperty m_anaTag{ this, "AnaTag", "", "Track analysis tag" }; 

    BooleanProperty m_doMatch{ this, "doMatch", false, "Enable TrackMatchingTool" };

    BooleanProperty m_writeOut{ this, "writeOut", false, "Write TrkAnaInfo Collection to AOD_IDTPM" };

    /// TrackAnalysisDefinitionSvc
    SmartIF<ITrackAnalysisDefinitionSvc> m_trkAnaDefSvc;

    /// plots
    std::vector< std::unique_ptr< IDTPM::TrackAnalysisPlotsMgr > >  m_trkAnaPlotsMgrVec;
};

#endif
