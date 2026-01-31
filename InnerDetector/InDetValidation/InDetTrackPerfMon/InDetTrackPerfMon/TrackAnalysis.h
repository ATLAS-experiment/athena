// enacs: this is -*- c++ -*- 
/**
 **   @file    TrackAnalysis.h                                                                                        
 **
 **   @author sutt 
 **   @date   Mon 17 Nov 2025 21:09:16 GMT 
 **
 **   $Id: TrackAnalysis.cxx, v0.0   Mon 17 Nov 2025 20:50:16 GMT sutt $
 ** 
 **   Copyright (C) 2025 sutt (sutt@cern.ch)
 **   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef INDETTRACKPERFMON_TRACKANALYSIS_H
#define INDETTRACKPERFMON_TRACKANALYSIS_H

  
/// gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// Athena includes
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

/// EDM includes
#include "xAODCore/BaseContainer.h"
#include "xAODCore/AuxContainerBase.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackPerfMon/RoiSelectionTool.h"
#include "InDetTrackPerfMon/TrackRoiSelectionTool.h"
#include "InDetTrackPerfMon/VertexRoiSelectionTool.h"
#include "InDetTrackPerfMon/ITrackSelectionTool.h"
#include "IVertexSelectionTool.h"
#include "InDetTrackPerfMon/ITrackMatchingTool.h"
#include "InDetTrackPerfMon/TrackAnalysisPlotsMgr.h"
#include "InDetTrackPerfMon/TrackAnalysisInfoWriteTool.h"

#include "InDetTrackPerfMon/TrackCollections.h"

/// STL includes
#include <string>
#include <vector>

namespace IDTPM { 

class TrackAnalysis : public AthAlgTool {

public :

  /// Constructor with parameters
  TrackAnalysis( const std::string& type, const std::string& name, const IInterface* parent );
  
  /// Destructor
  virtual ~TrackAnalysis();
  
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms();
  virtual StatusCode procHistograms();
  
  void execute();
  
  /// can't clone a gaudi algorithm ...
  //    TrackAnalysis* clone() const { return new TrackAnalysis(*this); }
  
  void setTDT( ToolHandle<Trig::TrigDecisionTool>& tdt ) { m_tdt = tdt; }
  
private :

  /// prevent default construction
  TrackAnalysis();

  /// TrackAnalysisDefinitionSvc
  //    SmartIF<ITrackAnalysisDefinitionSvc> m_trkAnaDefSvc;

  
  /// retrieve all collections && load them into trkAnaCollections object
  template<typename T, typename S=T>
  void loadCollections( TrackCollections<T,S>& trackCollections );

  /// retrieve all collections && load them into trkAnaCollections object
  StatusCode loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls );


  
  ToolHandle< IDTPM::ITrackSelectionTool > m_trackQualitySelectionTool { this, "TrackQualitySelectionTool", "", "" };

  //   "IDTPM::InDetTrackPerfMon/ITrackSelectionTool", "Wrapper-tool to perform track selection" };
  
  ToolHandle< IDTPM::IVertexSelectionTool > m_vertexQualitySelectionTool { this, "VertexQualitySelectionTool",
    "IDTPM::InDetTrackPerfMon/IVertexSelectionTool", "Wrapper-tool to perform general quality-based (truth) vertex selection" };
  
  ToolHandle< IDTPM::RoiSelectionTool > m_roiSelectionTool { this, "RoiSelectionTool", "", "roi selection tool" };
    //    "IDTPM::InDetTrackPerfMon/RoiSelectionTool", "Tool to retrieve && select RoIs" };
  
  
  
  /// we can't have all these ReadHandles because we don't know how many we will
  /// actually need until we configure the class, ie if we use Trigger tracks,
  /// && trigger vertices, this is ! how we access them - we have to do it through
  /// the TrigDecisionTool, but for now, we can give it a go, in terms of knowing the
  /// track collection names when we create all the toole - we don't actually need to
  /// use ReadHAndleKeys. But there are better ways to do it, than even this ...

  StringProperty m_offlineTracks { this, "OfflineTracks", "InDetTrackParticles", "offline track collection" };
  StringProperty m_triggerTracks { this, "TriggerTracks", "", "trigger track collection" };
  
  /// Offline TrackParticleContainer name
  //    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_offlineTrkParticleName {
  //        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };
  
  /// Trigger TrackParticleContainer name
  //    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_triggerTrkParticleName {
  //     this, "TriggerTrkParticleContainerName", "", "Name of container of trigger tracks" };
  
  /// TruthParticle container name
  // SG::ReadHandleKey< xAOD::TruthParticleContainer > m_truthParticleName {
  //      this, "TruthParticleContainerName",  "TruthParticles", "Name of container of TruthParticles" };

  /// TruthEvent container name
  //  SG::ReadHandleKey< xAOD::TruthEventContainer > m_truthEventName {
  //    this, "TruthEvents", "TruthEvents", "Name of the truth events container probably either TruthEvent || TruthEvents" };

    /// TruthPileupEvent container name
    //SG::ReadHandleKey< xAOD::TruthPileupEventContainer > m_truthPileUpEventName {
    //    this, "TruthPileupEvents", "TruthPileupEvents", "Name of the truth pileup events container probably TruthPileupEvent(s)" };

    /// EventInfo container name
  // SG::ReadHandleKey< xAOD::EventInfo > m_eventInfoContainerName {
  //    this, "EventInfoContainerName", "EventInfo", "event info" };

    /// Offline Primary vertex container name
    //SG::ReadHandleKey< xAOD::VertexContainer > m_offlineVertexContainerName {
  //  this, "OfflineVertexContainerName", "PrimaryVertices", "" };

    /// Trigger Primary vertex container name
    //SG::ReadHandleKey< xAOD::VertexContainer > m_triggerVertexContainerName {
  //  this, "TriggerVertexContainerName", "", "" };

    /// Truth vertex container name
  //   SG::ReadHandleKey< xAOD::TruthVertexContainer > m_truthVertexContainerName {
  //     this, "TruthVertexContainerName",  "TruthVertices", "" };

    /// WriteHandle for trkAnaInfo for reprocessing
  //  SG::WriteHandleKey< xAOD::BaseContainer > m_trkAnaInfoKey {
  //    this, "TrkAnaInfoKey", "TrackAnalysisInfo", "Dedicated TrackAnalysis Info written out" };

    /// -----------------------------
    /// --------- Sub-Tools ---------
    /// -----------------------------

    // ToolHandle< IDTPM::ITrackSelectionTool > m_trackQualitySelectionTool {
  //  this, "TrackQualitySelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool", "Wrapper-tool to perform general quality-based track(truth) selection" };

  //    ToolHandle< IDTPM::IVertexSelectionTool > m_vertexQualitySelectionTool {
  //    this, "VertexQualitySelectionTool", "IDTPM::InDetTrackPerfMon/IVertexSelectionTool", "Wrapper-tool to perform general quality-based (truth) vertex selection" };

  //    ToolHandle< IDTPM::RoiSelectionTool > m_roiSelectionTool {
  //       this, "RoiSelectionTool", "IDTPM::InDetTrackPerfMon/RoiSelectionTool", "Tool to retrieve && select RoIs" };

  ToolHandle< IDTPM::TrackRoiSelectionTool > m_trackRoiSelectionTool {
         this, "TrackRoiSelectionTool", "IDTPM::InDetTrackPerfMon/TrackRoiSelectionTool", "Tool to select track within a RoI" };

//  ToolHandle< IDTPM::VertexRoiSelectionTool > m_vertexRoiSelectionTool {
  //    this, "VertexRoiSelectionTool", "IDTPM::InDetTrackPerfMon/VertexRoiSelectionTool", "Tool to select vertices within a RoI" };

  ToolHandle< IDTPM::ITrackMatchingTool > m_trackMatchingTool { this, "TrackMatchingTool",
    "IDTPM::InDetTrackPerfMon/ITrackMatchingTool", "Tool to match test to reference tracks && viceversa" };

  //    ToolHandle< IDTPM::TrackAnalysisInfoWriteTool > m_trkAnaInfoWriteTool {
  //    this, "TrackAnalysisInfoWriteTool", "IDTPM::InDetTrackPerfMon/TrackAnalysisInfoWriteTool", "Tool to write TrackAnalysisInfo to StoreGate" };

  //  StringProperty m_anaTag{ this, "AnaTag", "", "Track analysis tag" }; 

  /// don;t understand what this is for, if we have a ITrackMatchingTool 
  //    BooleanProperty m_doMatch{ this, "doMatch", false, "Enable TrackMatchingTool" };

  //    BooleanProperty m_writeOut{ this, "writeOut", false, "Write TrkAnaInfo Collection to AOD_IDTPM" };


  ToolHandle<Trig::TrigDecisionTool> m_tdt; // { this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", "TDT" };
  
  ToolHandle<GenericMonitoringTool> m_tool { this, "montool", "", "histogram montool" };

  //  StringProperty m_trigger { this, "trigger", "", "actual trigger chain to select" }; 

  //  StringProperty m_refTracksCfg { this, "RefTracks", "", "reference tracks" }; 

  StringProperty m_trigger { this, "trigger", "", "trigger chain" };
  
  std::string m_refTracks;
  std::string m_testTracks;
  std::string m_rois;
  std::string m_vertices;
  std::string m_leg;
  std::string m_extra;
  
};

}

#endif  // INDETTRACKPERFMON_TRACKANALYSIS_H
