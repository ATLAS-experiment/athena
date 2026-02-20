/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimOPTIONALEVENTINFO_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimOPTIONALEVENTINFO_H

#include <TObject.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"

class FPGATrackSimOptionalEventInfo {

public:

  FPGATrackSimOptionalEventInfo() {};
  virtual ~FPGATrackSimOptionalEventInfo();

  void reset() const;

  // Offline Clusters
  const std::vector<FPGATrackSimCluster>& getOfflineClusters() const { return m_OfflineClusters; }
  size_t nOfflineClusters() const { return m_OfflineClusters.size(); }
  void addOfflineCluster(const FPGATrackSimCluster& c) const { m_OfflineClusters.push_back(c); }

  // Offline Tracks
  const std::vector<FPGATrackSimOfflineTrack>& getOfflineTracks() const { return m_OfflineTracks; }
  size_t nOfflineTracks() const { return m_OfflineTracks.size(); }
  void addOfflineTrack(const FPGATrackSimOfflineTrack& t) const { m_OfflineTracks.push_back(t); };

  // Truth Tracks
  const std::vector<FPGATrackSimTruthTrack>& getTruthTracks() const { return m_TruthTracks; }
  size_t nTruthTracks() const { return m_TruthTracks.size(); }
  void addTruthTrack(const FPGATrackSimTruthTrack& t) const { m_TruthTracks.push_back(t); }


  //reserve sizes
  void reserveOfflineClusters(size_t size) const { m_OfflineClusters.reserve(size); }
  void reserveOfflineTracks(size_t size) const { m_OfflineTracks.reserve(size); }
  void reserveTruthTracks(size_t size) const { m_TruthTracks.reserve(size); }


private:

  // Mutable members required for ROOT I/O operations within const execute() method.
  // - Call chain: FPGATrackSimDataPrepAlg::execute() const 
  //                   --> FPGATrackSimOutputHeaderTool::writeData() const
  //                       --> FPGATrackSimLogicalEventInputHeader::reset() const
  // - Execute creates local copies, moves data to ROOT objects under m_rootWriteMutex, then resets
  // - Thread-safety: local objects are per-thread, persistent ROOT objects protected by mutex

  mutable std::vector<FPGATrackSimCluster>       m_OfflineClusters ATLAS_THREAD_SAFE;
  mutable std::vector<FPGATrackSimOfflineTrack>  m_OfflineTracks ATLAS_THREAD_SAFE;
  mutable std::vector<FPGATrackSimTruthTrack>    m_TruthTracks ATLAS_THREAD_SAFE;


  ClassDefNV(FPGATrackSimOptionalEventInfo, 3)
};

std::ostream& operator<<(std::ostream&, const FPGATrackSimOptionalEventInfo&);
#endif
