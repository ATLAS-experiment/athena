/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MUON_MUON_TRACK_HELPERS_H
#define COLUMNAR_MUON_MUON_TRACK_HELPERS_H

#include "ColumnarTracking/TrackDef.h"
#include "ColumnarVariant/VariantAccessor.h"
#include "ColumnarVariant/VariantDef.h"
#include "ColumnarVariant/VariantLinkColumn.h"

namespace columnar
{
  using MuonTrackDef = columnar::VariantContainerId<columnar::ContainerId::track0,columnar::ContainerId::track0,columnar::ContainerId::track3,columnar::ContainerId::track1,columnar::ContainerId::track2>;
}

#endif
