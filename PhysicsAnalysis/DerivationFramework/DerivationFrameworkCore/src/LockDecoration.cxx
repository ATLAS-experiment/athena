// This file's extension implies that it's C, but it's really -*- C++ -*-.
/**
 * @file DerivationFrameworkCore/src/LockDecoration.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Algorithm to explicitly lock a decoration.
 */


#include "LockDecoration.h"
#include "StoreGate/DecorKeyHelpers.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxVectorBase.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ConcurrencyFlags.h"


namespace DerivationFramework {


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode LockDecoration::initialize()
{
  if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
    ATH_MSG_WARNING( "LockDecoration used in MT job.  This configuration is likely not thread-safe." );
  }
  ATH_CHECK( m_decoration.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Execute algorithm.
 * @param ctx The event context.
 */
StatusCode LockDecoration::execute (const EventContext& ctx) const
{
  const auto& r = SG::AuxTypeRegistry::instance();
  SG::ReadHandle<SG::AuxVectorBase> h (m_decoration.contHandleKey(), ctx);
  SG::auxid_t auxid = r.findAuxID (SG::decorKeyFromKey (m_decoration.key()));
  if (auxid == SG::null_auxid) {
    ATH_MSG_ERROR( "Cannot find decoration " << m_decoration.key() );
    return StatusCode::FAILURE;
  }
  SG::AuxVectorBase& avd ATLAS_THREAD_SAFE = const_cast<SG::AuxVectorBase&> (*h);
  avd.lockDecoration (auxid);
  return StatusCode::SUCCESS;
}


} // namespace DerivationFramework
