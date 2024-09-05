/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_TRIG_TRACK_SEED_GENERATOR_ITK_H
#define TRIGINDETPATTRECOTOOLS_TRIG_TRACK_SEED_GENERATOR_ITK_H

#include<vector>
#include<algorithm>

#include "TrigInDetPattRecoTools/TrigCombinatorialSettings.h"

class TrigSiSpacePointBase;
class TrigInDetTriplet;
class TrigFTF_GNN_DataStorage;
class IRoiDescriptor;

struct GNN_TrigTracklet {
public:
GNN_TrigTracklet(std::vector<const TrigSiSpacePointBase*>& vSP, std::vector<TrigInDetTriplet>& tbuf) : m_track(vSP), m_seeds(tbuf) {};
  ~GNN_TrigTracklet() {};
  
  std::vector<const TrigSiSpacePointBase*> m_track;
  std::vector<TrigInDetTriplet> m_seeds;
};

typedef class TrigTrackSeedGeneratorITk {

 public:

  TrigTrackSeedGeneratorITk(const TrigCombinatorialSettings&);
  ~TrigTrackSeedGeneratorITk(); 

  TrigTrackSeedGeneratorITk(const TrigTrackSeedGeneratorITk&) = delete;
  TrigTrackSeedGeneratorITk& operator=(const TrigTrackSeedGeneratorITk&) = delete;

  void loadSpacePoints(const std::vector<TrigSiSpacePointBase>&);
  void createSeeds(const IRoiDescriptor*);
  void createSeedsZv();
  void getSeeds(std::vector<TrigInDetTriplet>&);
  void getTracklets(const IRoiDescriptor*, std::vector<GNN_TrigTracklet>&, bool);

private:

  void runGNN_TrackFinder(const IRoiDescriptor*, std::vector<GNN_TrigTracklet>&, bool);
  
  TrigFTF_GNN_DataStorage* m_storage;

  const TrigCombinatorialSettings& m_settings;
  float m_phiSliceWidth;
  float m_minDeltaRadius, m_maxDeltaRadius;

  float m_maxCurv;

  std::vector<TrigInDetTriplet> m_triplets;

} TRIG_TRACK_SEED_GENERATOR_ITK;



#endif 
