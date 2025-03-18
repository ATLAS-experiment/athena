// This file's extension implies that it's C, but it's really -*- C++ -*-.
/**
 * @file DerivationFrameworkCore/src/LockDecoration.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Algorithm to explicitly lock a set of decorations.
 */


#include "LockDecorations.h"
#include "StoreGate/DecorKeyHelpers.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxVectorBase.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ConcurrencyFlags.h"


namespace DerivationFramework {


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode LockDecorations::initialize()
{
  if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
    ATH_MSG_WARNING( "LockDecoration used in MT job.  This configuration is likely not thread-safe." );
  }
  ATH_CHECK( m_decorations.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Execute algorithm.
 * @param ctx The event context.
 */
StatusCode LockDecorations::execute (const EventContext& ctx) const
{
  const auto& r = SG::AuxTypeRegistry::instance();
  for (const SG::WriteDecorHandleKey<SG::AuxVectorBase>& k : m_decorations) {
    SG::ReadHandle<SG::AuxVectorBase> h (k.contHandleKey(), ctx);
    if (h->size_v() > 0) {
      SG::auxid_t auxid = r.findAuxID (SG::decorKeyFromKey (k.key()));
      if (auxid == SG::null_auxid) {
        ATH_MSG_ERROR( "Cannot find decoration " << k.key() );
        return StatusCode::FAILURE;
      }
      SG::AuxVectorBase& avd ATLAS_THREAD_SAFE = const_cast<SG::AuxVectorBase&> (*h);
      avd.lockDecoration (auxid);
    }
  }
  return StatusCode::SUCCESS;
}


} // namespace DerivationFramework
