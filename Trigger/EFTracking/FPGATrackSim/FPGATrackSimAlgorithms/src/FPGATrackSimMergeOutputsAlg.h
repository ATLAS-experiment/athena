/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef FPGATrackSim_MERGEOUTPUTSALG_H
#define FPGATrackSim_MERGEOUTPUTSALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
//#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
//#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
//#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"

#include <vector>

class TFile;
class TTree;
class FPGATrackSimTrack;
class FPGATrackSimOverlapRemovalTool;
class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimLogicalEventOutputHeader;

class FPGATrackSimMergeOutputsAlg : public AthAlgorithm {
public:
  FPGATrackSimMergeOutputsAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimMergeOutputsAlg () {};
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:

   const static unsigned N=1280;
  
  Gaudi::Property<std::vector<std::string>> m_inpaths {this, "InFileNames", {"."}, "input file paths"};
  Gaudi::Property<bool> m_SortTracks {this, "SortTracks", false, "If true, sort tracks before finalizing merging, based on track quality"};
  std::vector<TFile*> m_files; // vector of pointers to files
  std::vector<std::vector<TTree*>> m_trees; // vector of ttrees, first vector over the input files, second over the regions  
  std::vector<std::vector<FPGATrackSimLogicalEventOutputHeader*>> m_eventOutputHeaders; //vector of output headers from above trees
  
  FPGATrackSimLogicalEventInputHeader *m_dataprep = nullptr; // data prep header, we only need one!
  TTree *m_dataprep_tree = nullptr; // data prep ttree, we only need one!

  // output track collection
  SG::WriteHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTracks","FPGATracks","FPGATrackSim Tracks key"};
  SG::WriteHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey{this, "FPGATrackSimHitKey","FPGAHits", "FPGATrackSim Hits key"};
  ToolHandle<FPGATrackSimOverlapRemovalTool> m_overlapRemovalTool {this, "OverlapRemoval", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool_Last", "Last inter-region overlap removal tool"};

  Gaudi::Property<unsigned> m_evtlooptree {this, "SkipWritingEvents", 0,  "Set to the number of events you want to skip at the start"};
  
  double m_evtloop = 0; // for counting, make a double because will be for division
  unsigned long m_alltracks = 0;
  unsigned long m_tracksPassOR = 0;
};

#endif // FPGATrackSim_MERGEOUTPUTSALG_H
