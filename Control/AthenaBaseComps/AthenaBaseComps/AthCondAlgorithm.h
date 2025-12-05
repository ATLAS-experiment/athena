/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaBaseComps/AthCondAlgorithm.h
 * @author Frank Winklmeier
 * @date Nov, 2025
 * @brief Base class for conditions algorithms.
 */

#ifndef ATHENABASECOMPS_ATHCONDALGORITHM_H
#define ATHENABASECOMPS_ATHCONDALGORITHM_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


/**
 * @brief Base class for conditions algorithms.
 *
 * Algorithms creating conditions objects should derive from this class.
 */
class AthCondAlgorithm : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /**
   * @brief Avoid scheduling algorithm multiple times.
   *
   * With multiple concurrent events, conditions objects often expire
   * simultaneously for all slots. To avoid that the scheduler runs the
   * CondAlg in each slot, we declare it as "non-reentrant". This ensures
   * that the conditions objects are only created once.
   *
   * In case a particular CondAlg should behave differently, it can override
   * this method again and return true.
   *
   * @see ATEAM-836
   */
  virtual bool isReEntrant() const override { return false; }
};

#endif
