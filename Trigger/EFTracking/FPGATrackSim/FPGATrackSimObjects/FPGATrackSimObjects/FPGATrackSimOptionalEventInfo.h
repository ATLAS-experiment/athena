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

class FPGATrackSimOptionalEventInfo : public  TObject {

public:

  FPGATrackSimOptionalEventInfo() {};
  virtual ~FPGATrackSimOptionalEventInfo();

  void reset();

  // Offline Clusters
  const std::vector<FPGATrackSimCluster>& getOfflineClusters() const { return m_OfflineClusters; }
  size_t nOfflineClusters() const { return m_OfflineClusters.size(); }
  void addOfflineCluster(const FPGATrackSimCluster& c) { m_OfflineClusters.push_back(c); }

  // Offline Tracks
  const std::vector<FPGATrackSimOfflineTrack>& getOfflineTracks() const { return m_OfflineTracks; }
  size_t nOfflineTracks() const { return m_OfflineTracks.size(); }
  void addOfflineTrack(const FPGATrackSimOfflineTrack& t) { m_OfflineTracks.push_back(t); };

  // Truth Tracks
  const std::vector<FPGATrackSimTruthTrack>& getTruthTracks() const { return m_TruthTracks; }
  size_t nTruthTracks() const { return m_TruthTracks.size(); }
  void addTruthTrack(const FPGATrackSimTruthTrack& t) { m_TruthTracks.push_back(t); };


  //reserve sizes
  void reserveOfflineClusters(size_t size) { m_OfflineClusters.reserve(size); }
  void reserveOfflineTracks(size_t size) { m_OfflineTracks.reserve(size); }
  void reserveTruthTracks(size_t size) { m_TruthTracks.reserve(size); }


private:

  std::vector<FPGATrackSimCluster>       m_OfflineClusters;
  std::vector<FPGATrackSimOfflineTrack>  m_OfflineTracks;
  std::vector<FPGATrackSimTruthTrack>    m_TruthTracks;


  ClassDef(FPGATrackSimOptionalEventInfo, 2)
};

std::ostream& operator<<(std::ostream&, const FPGATrackSimOptionalEventInfo&);
#endif
