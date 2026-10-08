///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthAsynchronousAlgorithm.h
// Header file for class AthAsynchronousAlgorithm
// Author: Beojan Stanislaus
///////////////////////////////////////////////////////////////////
#ifndef ATHENABASECOMPS_ATHASYNCHRONOUSALGORITHM_H
#define ATHENABASECOMPS_ATHASYNCHRONOUSALGORITHM_H 1

// Framework includes
#include "AthenaBaseComps/AthCommonAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "Gaudi/AsynchronousAlgorithm.h"

// Other includes
#include <boost/fiber/all.hpp>

/**
 * @brief An algorithm that can be suspended while work is offloaded to an
 * accelerator
 *
 */
class AthAsynchronousAlgorithm
    : public AthCommonAlgorithm<Gaudi::AsynchronousAlgorithm> {
  using AthCommonAlgorithm<
      Gaudi::AsynchronousAlgorithm>::AthCommonAlgorithm;

 public:
  StatusCode sysExecute(const EventContext& ctx) override;

  /// Restore after suspend
  virtual StatusCode restoreAfterSuspend() const override;

 protected:
  // This is local to each fiber so it can't be accessed from multiple threads
  // at once
  /// Pointer to current context
  mutable boost::fibers::fiber_specific_ptr<EventContext> m_currentCtx
      ATLAS_THREAD_SAFE{nullptr};
};

#endif  //> !ATHENABASECOMPS_ATHASYNCHRONOUSALGORITHM_H
