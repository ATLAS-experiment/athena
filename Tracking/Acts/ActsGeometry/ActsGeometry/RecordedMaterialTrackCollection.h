/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_RECORDEDMATERIALTRACKCOLLECTION_H
#define ACTSGEOMETRY_RECORDEDMATERIALTRACKCOLLECTION_H

#include <Acts/Propagator/MaterialInteractor.hpp>
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <vector>

namespace ActsTrk {
  using RecordedMaterialTrackCollection = std::vector<Acts::RecordedMaterialTrack>;
}

CLASS_DEF( ActsTrk::RecordedMaterialTrackCollection , 1338004588 , 1 )


#endif
