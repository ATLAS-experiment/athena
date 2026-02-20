/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PHASEII_PIXELRAWDATACONTAINERMT_H
#define PHASEII_PIXELRAWDATACONTAINERMT_H

#include "PhaseIIPixelRawDataContainer.h"
#include "PhaseIIInDetRawDataContainerMT.h"

/// @brief A container derived from PhaseIIPixelRawDataContainer which extends the container by a dynamic container list.
/// This container adds the means to provide one container per ROI for concurrent filling, but provides a
/// single registry with one entry per module.
using PhaseIIPixelRawDataContainerMT = PhaseII::IndexedRangesMT<PhaseII::PixelRawDataContainer>;

CLASS_DEF(PhaseIIPixelRawDataContainerMT, 1179197217, 1)

#include "AthenaKernel/BaseInfo.h"
SG_BASE(PhaseIIPixelRawDataContainerMT,PhaseIIPixelRawDataContainer);

#endif
