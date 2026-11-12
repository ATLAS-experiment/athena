/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCMEASTOPIXELINDEX_H
#define ACTSGPUEVENT_TRACCCMEASTOPIXELINDEX_H

#include "AthenaKernel/CLASS_DEF.h"
#include <vecmem/containers/data/vector_buffer.hpp>

namespace ActsTrk {
/// Device buffer, one entry per traccc measurement: the index of the
/// measurement among the pixel measurements (= pixel cluster / pixel SP index), or 0xFFFFFFFF for strip measurements.
using TracccMeasToPixelIndex = vecmem::data::vector_buffer<unsigned int>;
}

CLASS_DEF( vecmem::data::vector_buffer<unsigned int>, 159233072, 1 )

#endif // ACTSGPUEVENT_TRACCCMEASTOPIXELINDEX_H