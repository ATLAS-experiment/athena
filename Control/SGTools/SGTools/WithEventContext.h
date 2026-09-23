// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file SGTools/WithEventContext.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2026
 * @brief Temporarily change the current EventContext.
 */


#ifndef SGTOOLS_WITHEVENTCONTEXT_H
#define SGTOOLS_WITHEVENTCONTEXT_H


#include "GaudiKernel/EventContext.h"


class IProxyDict;


namespace SG {


/**
 * @brief Temporarily change the current EventContext.
 *
 * The original EventContext will be restored when this object is destroyed.
 */
class WithEventContext
{
public:
  /**
   * @brief Save current EventContext.
   *
   * This will save the current EventContext, but not change it.
   * The original EventContext will be restored when this object is destroyed.
   */
  WithEventContext();


  /**
   * @brief Save current EventContext and switch to a new one.
   * @param ctx The new EventContext.
   *
   * This will save the current EventContext and install a new one.
   * The original EventContext will be restored when this object is destroyed.
   */
  WithEventContext (const EventContext& ctx);


  /**
   * @brief Save current EventContext and update the IProxyDict pointer.
   * @param sg The new IProxyDcit.
   *
   * This will save the current EventContext and then update the current
   * context's IProxyDict pointer to sg.
   * The original EventContext will be restored when this object is destroyed.
   */
  WithEventContext (IProxyDict* sg);


  /**
   * @brief Destructor.  Restore the original EventContext.
   */
  ~WithEventContext();


private:
  /// The saved original EventContext.
  EventContext m_oldCtx;
};


} // namespace SG

#endif // not SGTOOLS_WITHEVENTCONTEXT_H
