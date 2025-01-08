/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKTRUTHSELECTOR_H
#define TRACKTRUTHSELECTOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "TrkTruthData/DetailedTrackTruthCollection.h"
#include "TrkTruthData/TrackTruthCollection.h"

class TrackTruthSelector: public AthAlgorithm {
public:
  TrackTruthSelector(const std::string &name,ISvcLocator *pSvcLocator);
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  
private:

   
  // DetailedTrackTruthCollection input name
  SG::ReadHandleKey<DetailedTrackTruthCollection> m_detailedTrackTruthName
  {this, "DetailedTrackTruthName", "DetailedTrackTruth"};

  // TrackTruthCollection output name
  SG::WriteHandleKey<TrackTruthCollection> m_outputName
    {this, "OutputName", "TrackTruthNew"};

  // Subdetector weights
  std::vector<double> m_subDetWeights = std::vector<double>(SubDetHitStatistics::NUM_SUBDETECTORS, 1.);

  DoubleProperty m_weightPixel{this, "WeightPixel", 1.};
  DoubleProperty m_weightSCT{this, "WeightSCT", 1.};
  DoubleProperty m_weightTRT{this, "WeightTRT", 1.};
  DoubleProperty m_weightMDT{this, "WeightMDT", 1.};
  DoubleProperty m_weightRPC{this, "WeightRPC", 1.};
  DoubleProperty m_weightTGC{this, "WeightTGC", 1.};
  DoubleProperty m_weightCSC{this, "WeightCSC", 1.};
  DoubleProperty m_weightsTGC{this, "WeightsTGC", 0.25};
  DoubleProperty m_weightMM{this, "WeightMM", 0.25};

  void fillOutput(TrackTruthCollection *out, const DetailedTrackTruthCollection *in);
  double getProbability(const DetailedTrackTruth& dt) const;
};

#endif/*TRACKTRUTHSELECTOR_H*/
