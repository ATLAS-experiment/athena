/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
/// TODO - To be included in later MRs
//#include "AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "../src/TrackAnalysisCollections.h"
#include "../src/RoiSelectionTool.h"
#include "InDetTrackPerfMon/ITrackSelectionTool.h"
/// TODO - To be included in later MRs
//#include "InDetTrackPerfMon/ITrackMatchingTool.h"
//#include "InDetTrackPerfMon/TrackAnalysisPlotsMgr.h"

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

    /// reatrieve all collections and load them into trkAnaCollections object
    StatusCode loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls );

    /// Offline TrackParticleContainer's name
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_offlineTrkParticleName{
        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };

    /// Trigger TrackParticleContainer's name
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_triggerTrkParticleName{
        this, "TriggerTrkParticleContainerName", "HLT_IDTrack_Electron_IDTrig", "Name of container of trigger tracks" };

    /// TruthParticle container's name
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleName{
        this, "TruthParticleContainerName",  "TruthParticles", "Name of container of TruthParticles" };

    /// Offline Primary vertex container's name
    //SG::ReadHandleKey<xAOD::VertexContainer> m_offlineVertexContainerName{
    //    this, "VertexContainerName", "PrimaryVertices", "offline vertices" };

    /// Truth vertex container's name
    //SG::ReadHandleKey<xAOD::TruthVertexContainer> m_truthVertexContainerName{
    //    this, "TruthVertexContainerName",  "TruthVertices", "truth vertices" };

    /// EventInfo container name
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoContainerName{
        this, "EventInfoContainerName", "EventInfo", "event info" };

    /// TODO - To be included in later MRs
    //SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventName{
    //    this, "TruthEvents", "TruthEvents", "Name of the truth events container probably either TruthEvent or TruthEvents" };

    //SG::ReadHandleKey<xAOD::TruthPileupEventContainer> m_truthPileUpEventName{
    //    this, "TruthPileupEvents", "TruthPileupEvents", "Name of the truth pileup events container probably TruthPileupEvent(s)" };

    PublicToolHandle< Trig::TrigDecisionTool > m_trigDecTool{
        this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", "" };

    ToolHandle< IDTPM::ITrackSelectionTool > m_trackQualitySelectionTool{
        this, "TrackQualitySelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool", "Wrapper-tool to perform general quality-based track(truth) selection" };

    ToolHandle< IDTPM::RoiSelectionTool > m_roiSelectionTool{
        this, "RoiSelectionTool", "IDTPM::InDetTrackPerfMon/RoiSelectionTool", "Tool to retrieve and select RoIs" };

    ToolHandle< IDTPM::ITrackSelectionTool > m_trackRoiSelectionTool{
        this, "TrackRoiSelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool", "Tool to select track within a RoI" };

    /// TODO - To be included in later MRs
    //ToolHandle< IDTPM::ITrackMatchingTool > m_trackMatchingTool{ this, "TrackMatchingTool", "IDTPM::InDetTrackPerfMon/ITrackMatchingTool", "Tool to match test to reference tracks and viceversa" };

    /// Properties to fine-tune the tool behaviour
    StringProperty m_dirName{
        this, "DirName", "InDetTrackPerfMonPlots/", "Top level directory to write histograms into" };

    StringProperty m_anaTag{ this, "AnaTag", "", "Track analysis tag" }; 

    /// TrackAnalysisDefinitionSvc
    ITrackAnalysisDefinitionSvc* m_trkAnaDefSvc;

    /// histograms
    /// TODO - To be included in later MRs
    //std::vector< std::unique_ptr<TrackAnalysisPlotsMgr> > m_trkAnaPlotsMgrVec;
};

#endif
