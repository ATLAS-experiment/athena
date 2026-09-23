/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaServices/AthEnvironmentSvc.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2026
 * @brief Environment settings, including Eigen cache sizes.
 */


#include "AthEnvironmentSvc.h"
#include <Eigen/Core>


/**
 * @brief Gaudi initialize method.
 */
StatusCode AthEnvironmentSvc::initialize()
{
  if (m_eigenL1CacheSize > 0 || m_eigenL2CacheSize > 0 || m_eigenL3CacheSize > 0) {
    ATH_MSG_VERBOSE("Setting Eigen cache sizes to L1: {} L2: {} L3: {}",
                    m_eigenL1CacheSize.value(), m_eigenL2CacheSize.value(), m_eigenL3CacheSize.value());
    Eigen::setCpuCacheSizes (m_eigenL1CacheSize, m_eigenL2CacheSize, m_eigenL3CacheSize);
  }
  return StatusCode::SUCCESS;
}

