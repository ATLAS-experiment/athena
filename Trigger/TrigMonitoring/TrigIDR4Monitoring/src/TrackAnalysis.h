// enacs: this is -*- c++ -*- 
/**
 **   @file    TrackAnalysis.h                                                                                        
 **
 **   @author  sutt 
 **   @date    Mon 17 Nov 2025 21:09:16 GMT 
 **
 **   $Id: TrackAnalysis.h, v0.0   Mon 17 Nov 2025 20:50:16 GMT sutt $
 ** 
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef IDTPM22_TRACKANALYSIS_H
#define IDTPM22_TRACKANALYSIS_H

  
/// gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

/// Athena includes
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include "MsgMacros.h"


#include "ITrackSelector.h"
#include "TrackView.h"
#include "TrackAnalysisStore.h"

#include "TrackViewMatchingTool.h"

#include "AnalysisR4.h"
#include "RoiSelector.h"

#include "ConfiguredValueUtils.h"
#include "TagNProbe.h"

#include "AncestorFinder.h"

/// STL includes
#include <string>
#include <vector>


namespace IDTPM2 { 

class TrackAnalysis : public AthAlgTool {

public :

  /// Constructor with parameters
  TrackAnalysis( const std::string& type, const std::string& name, const IInterface* parent );

  TrackAnalysis() = delete;
  
  /// Destructor
  virtual ~TrackAnalysis();
  
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms( const EventContext& context ) const;
  virtual StatusCode procHistograms() { return StatusCode::SUCCESS; } /// finalise histograms in stop()
  virtual StatusCode proc();

  
  virtual StatusCode finalize() { return procHistograms(); } /// finalise histograms in stop()
  // virtual StatusCode finalize() { return proc(); }

  bool execute( const EventContext& context ) const;

  virtual StatusCode start() override { return StatusCode::SUCCESS; }

  virtual StatusCode  stop() override { return proc(); }

  
  /// assign the TrigDecisionTool - we get this passed in from
  /// the top level algorithm so don't need to mess about 
  /// retrieving it ourselves
  void setTDT( ToolHandle<Trig::TrigDecisionTool>& tdt ) { m_tdt = tdt; }

  const ToolHandle<Trig::TrigDecisionTool>& getTDT() const { return m_tdt; }

  ToolHandle<GenericMonitoringTool>&       monTool()       { return  m_tool; }
  const ToolHandle<GenericMonitoringTool>& monTool() const { return  m_tool; }

  bool hasTagNProbe() const { return m_tnp==0; }

  bool isInitialized() const { return m_initialized; }
  
  const std::string& refTracks()     const { return m_refTracks;     } 
  const std::string& refTracks_tag() const { return m_refTracks_tag; } 

protected:
  
  ToolHandle<Trig::TrigDecisionTool> m_tdt; // configure this in the constructor 

  /// histogram montool - books the histograms in the python

  ToolHandle<GenericMonitoringTool> m_tool { this, "montool", "", "histogram montool" };

  /// and the lightweight tool to actually fill them - could join this with the above montool,
  /// but as usual with athena code, that would make things a lot more complicated, whilst
  /// simultaneously being a lot less flexible
  std::unique_ptr<AnalysisR4> m_analysis;

  Gaudi::Property<std::vector<std::string>>  m_fillers { this, "Fillers", {}, "List of histogram filler tools" };  
  
  /// tools 
  
  ToolHandle<IDTPM2::ITrackSelector> m_refSelector  { this, "ReferenceSelector", "", "" };
  ToolHandle<IDTPM2::ITrackSelector> m_testSelector { this, "TestSelector", "", "" };

  /// need to define a proper interface class 
  ToolHandle<TrackViewMatchingTool> m_trackMatchingTool {
    this, "TrackMatchingTool", "IDTPM2::TrackViewMatchingTool", "Tool to match test to reference tracks and viceversa" };

  /// read handles, but ...
  
  /// EventInfo Container name
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoName { this, "EventInfo", "EventInfo", "EventInfo name" };

  /// properties

  Gaudi::Property<std::string> m_reference         { this, "Reference",         "InDetTrackParticles", "reference collection"  };

  Gaudi::Property<std::string> m_referenceVertices { this, "ReferenceVertices", "PrimaryVertices",     "reference vertex collection" };

  Gaudi::Property<std::string> m_tagChain          { this, "TagChain",     "",  "tag leg for tag n probe analysis" };

  Gaudi::Property<std::string> m_reference_tag     { this, "ReferenceTag", "",  "reference tag collection for Tag and Probe"  };

  Gaudi::Property<ConfiguredValue<int>> m_refPdgId     { this, "ReferencePdgId",    {}, "reference PDG ID (for truth particle tag and probe)"  };
  Gaudi::Property<ConfiguredValue<int>> m_refPdgId_tag { this, "ReferencePdgIdTag", {}, "reference PDG ID (for truth particle tag and probe)"  };

  
  Gaudi::Property<float> m_minTagMass     { this, "MinTagMass",  70,  "Minimum invariant mass for Tag and Probe"  };
  Gaudi::Property<float> m_maxTagMass     { this, "MaxTagMass", 110,  "Maximum invariant mass for Tag and Probe"  };
  Gaudi::Property<float> m_minTagPT       { this, "MinTagPT",    15,  "Minimum tag PT for Tag and Probe"          };

  
  /// and sensible internal variables we can set from the configuration ...

  bool m_initialized = false;
  
  std::string m_triggerchain;
  std::string m_refTracks;
  std::string m_testTracks;
  std::string m_rois;
  std::string m_vertices;
  //  std::string m_leg;
  int         m_leg = -1;
  std::string m_extra;
  
  std::string m_refVertices;

  std::string m_triggerchain_tag;
  std::string m_refTracks_tag;
  std::string m_roi_tag;
  //  std::string m_leg_tag;
  int         m_leg_tag = -1;
  std::string m_extra_tag;

  std::unique_ptr<TagNProbe> m_tnp = 0;

  Gaudi::Property<bool>  m_useDecorations { this, "useDecorations",  false,  "Use athena decorations or not" };
  
  SG::ReadDecorHandleKeyArray<xAOD::EventInfo> m_beamKeys
    {this, "BeamSpotDecoKeys", {"EventInfo.beamPosX", "EventInfo.beamPosY", "EventInfo.beamPosZ"}, "Beamspot positions"};

  std::unique_ptr<IPredicate<TrackView>> m_tnpPredicate = 0;
  
};

}

#endif  // INDETTRACKPERFMON2_TRACKANALYSIS_H
