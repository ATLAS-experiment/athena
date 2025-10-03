/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_TRIG_INDET_UTILS_H
#define TRIGINDETPATTRECOTOOLS_TRIG_INDET_UTILS_H

#include "TrkTrack/Track.h"
#include <vector>

#include "BeamSpotConditionsData/BeamSpotData.h"

struct trackInfo {
  int n_hits_pix = 0; int n_hits_sct = 0; int n_hits_inner = 0; int n_hits_innermost = 0;
  float ptGeV = 0; float a0beam = 0;float eta = 0; float phi0 = 0;
};

namespace FTF {
    bool isGoodTrackUTT(const Trk::Track* track, trackInfo& theTrackInfo, const float shift_x, const float shift_y, float trkcut_ptgev);
    void getBeamSpotShift(float& shift_x, float& shift_y, const InDet::BeamSpotData& beamSpotHandle);
}

typedef struct WeightedCoordinate{
  struct Comparator {
    bool operator()(const struct WeightedCoordinate& wc1, const struct WeightedCoordinate& wc2) {
      return (wc2.m_x > wc1.m_x);
    }
  };
public:
WeightedCoordinate(double x, double w) : m_x(x), m_w(w) {};
WeightedCoordinate(const WeightedCoordinate& wc) : m_x(wc.m_x), m_w(wc.m_w) {};
  double m_x, m_w;
private:
WeightedCoordinate() : m_x(0.0), m_w(0.0) {};
} WEIGHTED_COORDINATE;

typedef class CdfApproximator {
 public:
  CdfApproximator(){};
  double findMedian(std::vector<WEIGHTED_COORDINATE>&);
  double findModes(std::vector<WEIGHTED_COORDINATE>&, std::vector<double>&, int);
} CDF_APPROXIMATOR;

#endif
