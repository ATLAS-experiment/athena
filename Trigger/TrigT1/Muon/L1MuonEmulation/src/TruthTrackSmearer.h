/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MUONEMULATION_TRUTHTRACKSMEARER_H
#define L1MUONEMULATION_TRUTHTRACKSMEARER_H

#include <memory>
#include <array>

#include "AthenaKernel/RNGWrapper.h"

#include "L1MuonTrack.h"

namespace L1Muon {

class TruthTrackSmearer {
 public:
  TruthTrackSmearer(ATHRNG::RNGWrapper* rndWrapper);
  ~TruthTrackSmearer() = default;

  TruthTrackSmearer(const TruthTrackSmearer & other) = delete;
  TruthTrackSmearer & operator = (const TruthTrackSmearer & other) = delete;

  double effFunc(double pt) const;

  bool emulateL1MuonTrack(double curv, float eta, float phi, L1MuonTrack& otrack) const;

 private:
  ATHRNG::RNGWrapper* m_rngWrapper;
  std::array<float, 2> m_efficiencyMap;   // [pt]
};

}   // end of namespace

#endif
