/*
  Copyright (C) 2025 CERN for the benefit of the ATLAS collaboration
*/
// AthAsynchronousAlgorithm.cxx
// Implementation file for class AthAsynchronousAlgorithm
// Author: Beojan Stanislaus

#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"
#include "CxxUtils/checker_macros.h"

StatusCode AthAsynchronousAlgorithm::sysExecute(const EventContext& ctx) {
  ATH_MSG_VERBOSE("Starting sysExecute for AthAsynchronousAlgorithm on slot "
                  << ctx.slot());
  if (m_currentCtx.get() == nullptr) {
    // const_cast because fiber_specific_ptr doesn't support const pointers
    // The const is never actually violated
    EventContext* ctx_temp ATLAS_THREAD_SAFE = const_cast<EventContext*>(&ctx);
    m_currentCtx.reset(ctx_temp);
  } else if (m_currentCtx->evt() != ctx.evt() ||
             m_currentCtx->slot() != ctx.slot()) {
    ATH_MSG_ERROR("m_currentCtx is set to "
                  << m_currentCtx->evt() << " (slot " << m_currentCtx->slot()
                  << ") but incorrect. It should be " << ctx.evt() << "(slot "
                  << ctx.slot() << ")");
    return StatusCode::FAILURE;
  }
  return Gaudi::AsynchronousAlgorithm::sysExecute(ctx);
}

StatusCode AthAsynchronousAlgorithm::restoreAfterSuspend() const {
  auto* currentCtx = m_currentCtx.get();
  ATH_MSG_DEBUG("Setting current context to " << (void*)currentCtx);
  Gaudi::Hive::setCurrentContext(currentCtx);
  return StatusCode::SUCCESS;
}
