/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_MEASTOPIXELINDEX_H
#define ACTSGPUEVENTCNV_MEASTOPIXELINDEX_H

#include "GaudiKernel/StatusCode.h"

#include <traccc/edm/measurement_collection.hpp>
#include <vecmem/containers/data/vector_buffer.hpp>
#include <vecmem/memory/memory_resource.hpp>
#include <vecmem/utils/copy.hpp>

#include <cuda_runtime_api.h>

#include <memory>

namespace ActsTrk {

/// Marker for "not a pixel measurement" (same value as kDeviceSeedInvalidIndex)
inline constexpr unsigned int kInvalidPixelIndex = 0xFFFFFFFFu;

/// Instead of building the map every time on the seed converter
/// it is smarter (as pointed out by Neza and Fabrice) to only build the map when needed.
/// Otherwise, just use the one built by the device measurement converter.
/// So... build (on device) the map: traccc measurement index -> pixel index:
///   pixel measurement: its index among the pixel measurements, in collection
///                      order (exclusive scan of the pixel flags)
///   strip measurement: kInvalidPixelIndex (bc no strip seeds for now)
/// The pixel index equals the pixel cluster / pixel spacepoint index of the
/// measurement converters, which keep collection order (i <-> i).
/// The output buffer is always allocated, also for nb_measurements == 0.
StatusCode buildMeasToPixelIndexOnGPU(
    cudaStream_t stream
  , vecmem::memory_resource & mr
  , vecmem::copy const & copy
  , unsigned int nb_measurements
  , const traccc::edm::measurement_collection::const_view & measurements
  , std::unique_ptr<vecmem::data::vector_buffer<unsigned int>> & output
  );

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_MEASTOPIXELINDEX_H