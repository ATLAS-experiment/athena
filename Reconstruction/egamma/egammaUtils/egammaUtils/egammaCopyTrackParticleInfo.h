/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "xAODTracking/TrackParticle.h"
namespace egammaCopyTrackParticleInfo {

struct ToCopy {
  bool isRefitted = true;
  bool doTruth = false;
  bool doPix = false;
  bool doSCT = false;
  bool doTRT = false;
  bool doHGTD = false;
};

void
copy(xAOD::TrackParticle& created,
     const xAOD::TrackParticle& original,
     const egammaCopyTrackParticleInfo::ToCopy& toCopy);

}  // namespace egammaCopyTrackParticleInfo
