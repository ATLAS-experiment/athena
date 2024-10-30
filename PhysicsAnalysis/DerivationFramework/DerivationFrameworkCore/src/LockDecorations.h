// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DerivationFrameworkCore/src/LockDecorations.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Algorithm to explicitly lock a set of decorations.
 */


#ifndef DERIVATIONFRAMEWORKCORE_LOCKDECORATION_H
#define DERIVATIONFRAMEWORKCORE_LOCKDECORATION_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"


namespace DerivationFramework {


/**
 * @brief Algorithm to explicitly lock a set of decorations.
 *
 * In some derivation configurations, multiple algorthims can be modifying
 * the same decorations (for example TruthDressingTool).  In such cases,
 * we cannot lock the decorations at the end of the algorithm, as is usually
 * done, since this would cause subsequent algorithms to fail.  However,
 * we also don't want to leave the decorations unlocked, as the fix
 * for ATLASRECTS-8008 would cause those decorations to be lost in a deep copy.
 * Instead, in those cases, we can use this algorithm to explicitly lock
 * the decorations after all algorithms that modify it have completed.
 *
 * Note, though, that such configurations are likely not compatible with MT.
 */
class LockDecorations : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  /**
   * @brief Standard Gaudi initialize method.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Execute algorithm.
   * @param ctx The event context.
   */
  virtual StatusCode execute (const EventContext& ctx) const override;


private:
  /// The decoration to lock.
  SG::WriteDecorHandleKeyArray<SG::AuxVectorBase> m_decorations
  { this, "Decorations", {} };
};


} // namespace DerivationFramework


#endif // not DERIVATIONFRAMEWORKCORE_LOCKDECORATION_H
