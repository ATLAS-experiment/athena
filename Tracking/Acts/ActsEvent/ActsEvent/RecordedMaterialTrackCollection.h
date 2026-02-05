/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_RECORDEDMATERIALTRACKCOLLECTION
#define ACTSTRK_RECORDEDMATERIALTRACKCOLLECTION

#include <Acts/Propagator/MaterialInteractor.hpp>
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <vector>

namespace ActsTrk {
  using RecordedMaterialTrackCollection = std::vector<Acts::RecordedMaterialTrack>;
}

CLASS_DEF( ActsTrk::RecordedMaterialTrackCollection , 1338004588 , 1 )


#endif
