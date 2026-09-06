/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CUDAMagFieldProviderTool.h"

#include "traccc/cuda/utils/make_magnetic_field.hpp"

namespace ActsTrk {

StatusCode CUDAMagFieldProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing CUDAMagFieldProviderTool.");

  if (m_magFieldStorage.value() == "global_memory") {
    m_storage = traccc::cuda::magnetic_field_storage::global_memory;
  } else if (m_magFieldStorage.value() == "texture_memory") {
    m_storage = traccc::cuda::magnetic_field_storage::texture_memory;
  } else {
    ATH_MSG_FATAL("Unknown MagFieldStorage value.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


traccc::magnetic_field
CUDAMagFieldProviderTool::getDeviceMagneticField(
    traccc::magnetic_field const& host_bfield) const
{
    return traccc::cuda::make_magnetic_field(host_bfield, m_storage);
}

} // namespace ActsTrk