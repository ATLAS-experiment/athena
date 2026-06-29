/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthenaBaseComps includes
#include "AthenaBaseComps/AthCommonAlgorithm.h"
#include "AthAlgorithmDHUpdate.h"
#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/ServiceHandle.h"

// Gaudi includes
#include "Gaudi/Algorithm.h"
#include "Gaudi/AsynchronousAlgorithm.h"


template <class BaseAlg>
AthCommonAlgorithm<BaseAlg>::AthCommonAlgorithm( const std::string& name, 
                                                 ISvcLocator* pSvcLocator ) :
  ::AthCommonDataStore<AthCommonMsg<BaseAlg>>   ( name, pSvcLocator )
{

  // Set up to run AthAlgorithmDHUpdate in sysInitialize before
  // merging depedency lists.  This extends the output dependency
  // list with any symlinks implied by inheritance relations.
  m_updateDataHandles =
    std::make_unique<AthenaBaseComps::AthAlgorithmDHUpdate>
    (m_extendedExtraObjects,
     std::move (m_updateDataHandles));
}


/**
 * @brief Execute an algorithm.
 *
 * We override this in order to work around an issue with the Algorithm
 * base class storing the event context in a member variable that can
 * cause crashes in MT jobs.
 */
template <class BaseAlg>
StatusCode AthCommonAlgorithm<BaseAlg>::sysExecute (const EventContext& ctx)
{
  return BaseAlg::sysExecute (ctx);
}


/**
 * @brief Return the list of extra output dependencies.
 *
 * This list is extended to include symlinks implied by inheritance
 * relations.
 */
template <class BaseAlg>
const DataObjIDColl& AthCommonAlgorithm<BaseAlg>::extraOutputDeps() const
{
  // If we didn't find any symlinks to add, just return the collection
  // from the base class.  Otherwise, return the extended collection.
  if (!m_extendedExtraObjects.empty()) {
    return m_extendedExtraObjects;
  }
  return BaseAlg::extraOutputDeps();
}


/**
 * @brief Override sysInitialize from the base class
 *
 * Scan through all outputHandles, and if they're WriteCondHandles,
 * register them with the CondSvc
 */
template <class BaseAlg>
StatusCode AthCommonAlgorithm<BaseAlg>::sysInitialize() {
  StatusCode sc=AthCommonDataStore<AthCommonMsg<BaseAlg>>::sysInitialize();

  if (sc.isFailure()) {
    return sc;
  }
  
  ServiceHandle<ICondSvc> cs("CondSvc",name());
  for (auto h : outputHandles()) {
    if (h->isCondition() && h->mode() == Gaudi::DataHandle::Writer) {
      // do this inside the loop so we don't create the CondSvc until needed
      if ( cs.retrieve().isFailure() ) {
        ATH_MSG_WARNING("no CondSvc found: won't autoreg WriteCondHandles");
        return StatusCode::SUCCESS;
      }      
      if (cs->regHandle(this,*h).isFailure()) {
        sc = StatusCode::FAILURE;
        ATH_MSG_ERROR("unable to register WriteCondHandle " << h->fullKey()
                      << " with CondSvc");
      }
    }
  }
  return sc;  
}

/// instantiate for Gaudi::Algorithm
template class AthCommonAlgorithm<Gaudi::Algorithm>;
/// instantiate for Gaudi::AsynchronousAlgorithm
template class AthCommonAlgorithm<Gaudi::AsynchronousAlgorithm>;
