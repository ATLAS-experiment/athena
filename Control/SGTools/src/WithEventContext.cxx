/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file SGTools/src/WithEventContext.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2026
 * @brief Temporarily change the current EventContext.
 */


#include "SGTools/WithEventContext.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"


namespace SG {


/**
 * @brief Save current EventContext.
 *
 * This will save the current EventContext, but not change it.
 * The original EventContext will be restored when this object is destroyed.
 */
WithEventContext::WithEventContext()
  : m_oldCtx (Gaudi::Hive::currentContext())
{
}


/**
 * @brief Save current EventContext and switch to a new one.
 * @param ctx The new EventContext.
 *
 * This will save the current EventContext and install a new one.
 * The original EventContext will be restored when this object is destroyed.
 */
WithEventContext::WithEventContext (const EventContext& ctx)
  : WithEventContext()
{
  Gaudi::Hive::setCurrentContext (ctx);
}


/**
 * @brief Save current EventContext and update the IProxyDict pointer.
 * @param sg The new IProxyDcit.
 *
 * This will save the current EventContext and then update the current
 * context's IProxyDict pointer to sg.
 * The original EventContext will be restored when this object is destroyed.
 */
WithEventContext::WithEventContext (IProxyDict* sg)
  : WithEventContext()
{
  EventContext ctx = m_oldCtx;

  Atlas::ExtendedEventContext* ectx = Atlas::tryGetExtendedEventContext(ctx);
  if (ectx) {
    ectx->setProxy (sg);
  }
  else {
    ctx.setExtension (Atlas::ExtendedEventContext (sg));
  }
  Gaudi::Hive::setCurrentContext (std::move (ctx));
}


/**
 * @brief Destructor.  Restore the original EventContext.
 */
WithEventContext::~WithEventContext()
{
  Gaudi::Hive::setCurrentContext (m_oldCtx);
}


} // namespace SG
