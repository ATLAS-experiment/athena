// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaServices/AthEnvironmentSvc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2026
 * @brief Environment settings, including Eigen cache sizes.
 */


#ifndef ATHENASERVICES_ATHENVIRONMENTSVC_H
#define ATHENASERVICES_ATHENVIRONMENTSVC_H


#include "AthenaBaseComps/AthService.h"


/**
 * @brief Environment settings, including Eigen cache sizes.
 *
 * This service makes settings in the execution environment that we want
 * for Athena but which don't really fit anywhere else.
 *
 * For large matrices, Eigen breaks down operations into rectangular panels,
 * which are sized to fit in cache.  By default, Eigen queries the hardware
 * to find the actual cache sizes.  However, changing the panel size changes
 * the order in which operations are done, which can slightly change results.
 * The upshot is that by default, results of large matrix operations can
 * vary slightly depending on the hardware.  To avoid this, we fix the
 * cache sizes that Eigen assumes rather than letting it query the hardware.
 * The defaults used here match what is currently (Jul 2026) observed
 * on lxplus (AMD EPYC-Genoa).  Some other machines, such as the TDAQ test bed,
 * have been seen to have larger cache sizes.
 *
 * For more information, see ATLASRECTS-8013.
 */
class AthEnvironmentSvc : public AthService
{
public:
  using AthService::AthService;

  /// Gaudi initialize method.
  virtual StatusCode initialize() override;


private:
  IntegerProperty m_eigenL1CacheSize
  { this, "EigenL1CacheSize", 32768, "L1 cache size to declare to Eigen, to allow for for reproducibility across machines.  Set all cache sizes to 0 to use instead the actual hardware cache sizes." };

  IntegerProperty m_eigenL2CacheSize
  { this, "EigenL2CacheSize", 1048576, "L2 cache size to declare to Eigen, to allow for for reproducibility across machines.  Set all cache sizes to 0 to use instead the actual hardware cache sizes." };

  IntegerProperty m_eigenL3CacheSize
  { this, "EigenL3CacheSize", 33554432, "L3 cache size to declare to Eigen, to allow for for reproducibility across machines.  Set all cache sizes to 0 to use instead the actual hardware cache sizes." };
};


#endif // not ATHENASERVICES_ATHENVIRONMENTSVC_H
