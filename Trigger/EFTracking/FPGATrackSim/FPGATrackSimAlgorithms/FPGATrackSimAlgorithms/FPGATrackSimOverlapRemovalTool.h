/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
* @file FPGATrackSimOverlapRemovalTool.h
* @author Zhaoyuan.Cui@cern.ch
* @date Dec.4, 2020
* @brief Overlap removal tool for FPGATrackSimTrack.
*/

#ifndef FPGATrackSimOVERLAPREMOVALTOOL_H
#define FPGATrackSimOVERLAPREMOVALTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimHough/FPGATrackSimHoughFunctions.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include <string>
#include <vector>
#include <ostream>

/**
* @class FPGATrackSimOverlapRemovalTool
* @brief Remove (mark) duplicate tracks
* This tool takes FPGATrackSimTrack as input and mark their status of passing/failing the overlap removal criteria.
*/
class FPGATrackSimOverlapRemovalTool: public AthAlgTool {
public:

  FPGATrackSimOverlapRemovalTool (const std::string&, const std::string&, const IInterface*);
  FPGATrackSimOverlapRemovalTool() = delete;

  StatusCode initialize() override;

  StatusCode runOverlapRemoval(std::vector<FPGATrackSimRoad>& roads);

  // Original Overlap Removal function
  // Compare chi2 and common number of hits
  StatusCode runOverlapRemoval(std::vector<FPGATrackSimTrack>& tracks);

  void setDoSecondStage(bool v) { m_do2ndStage = v; }
  bool getDoSecondStage() const { return m_do2ndStage; }
  ORAlgo getAlgorithm() const {return m_algo;}
  //  Find the one track from the duplicate group with the minium Chi2 and maximum # of hits

  StatusCode removeOverlapping(FPGATrackSimTrack & track1, FPGATrackSimTrack & track2);
  static bool compareTrackQuality(const FPGATrackSimTrack & track1, const FPGATrackSimTrack & track2);

private:


  Gaudi::Property <bool> m_do2ndStage { this, "DoSecondStage", false, "Do second stage of not?"};
  Gaudi::Property <int> m_NumOfHitPerGrouping { this, "NumOfHitPerGrouping", 5, "Number of hits pre grouping"};
  Gaudi::Property <float> m_minChi2 { this, "MinChi2", 40, "Minimum chi2 requirement for tracks being considered in OR"};
  Gaudi::Property <bool> m_roadSliceOR { this, "roadSliceOR", true, "Do slice-wise overlap removal for roads (Hough-only)"};
  Gaudi::Property <int> m_localMaxWindowSize { this, "localMaxWindowSize", 0, "Only create roads that are a local maximum within this window size. Set this to 0 to turn off local max filtering"};
  Gaudi::Property <unsigned> m_imageSize_x { this, "nBins_x", 0, "number of bins in, eg, phi_track"};
  Gaudi::Property <unsigned> m_imageSize_y { this, "nBins_y", 0, "number of bins in, eg, q/pT"};
  Gaudi::Property <std::string> m_algorithm { this, "ORAlgo", "Normal", "Overlap removal algorithm"};
  Gaudi::Property <bool> m_doFastOR { this, "doFastOR", false, "Use fast overlap removal algorithm instead of default"};
  Gaudi::Property <bool> m_compareAllHits {this, "compareAllHits", true, "Compare all hits to all other hits when comparing two tracks"};
  Gaudi::Property <bool> m_useV2OR {this, "useV2OR", true, "Use v2 overlap removal algorithm (worst-to-best sorting)"};
  ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool", "", "Monitoring tool"};

  ORAlgo m_algo{ORAlgo::Normal};       //  Internal ORAlgo enum for faster compare


  StatusCode runOverlapRemoval_fast(std::vector<FPGATrackSimTrack>& tracks);
  
  // V2 overlap removal algorithm with sorted worst-to-best approach
  StatusCode runOverlapRemoval_v2(std::vector<FPGATrackSimTrack>& tracks);
  
  // Compare track quality for v2 algorithm (sorts worst-to-best)
  static bool compareTrackQuality_v2(const FPGATrackSimTrack& track1, const FPGATrackSimTrack& track2);
  
  // Helper for v2: count real hits in a track
  static int countRealHits_v2(const FPGATrackSimTrack& track);
  
  // Helper for v2: count overlapping hits between two tracks
  int countOverlappingHits_v2(const FPGATrackSimTrack& track1, const FPGATrackSimTrack& track2) const;

};

#endif // FPGATrackSimOverlapRemovalTool_h
