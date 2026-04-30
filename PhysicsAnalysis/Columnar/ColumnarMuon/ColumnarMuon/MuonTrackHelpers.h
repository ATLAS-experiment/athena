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
  using MuonTrackDef = columnar::VariantContainerId<columnar::Track0Def,columnar::Track0Def,columnar::Track3Def,columnar::Track1Def,columnar::Track2Def>;
}

#endif
