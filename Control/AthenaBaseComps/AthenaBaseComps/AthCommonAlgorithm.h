///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthCommonAlgorithm.h 
// Header file for class AthCommonAlgorithm
// Author: Charles Leggett
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHCOMMONALGORITHM_H
#define ATHENABASECOMPS_ATHCOMMONALGORITHM_H


// STL includes
#include <string>
#include <type_traits>

#include "AthenaBaseComps/AthCommonDataStore.h"
#include "AthenaBaseComps/AthCommonMsg.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMemMacros.h"


#include "Gaudi/Algorithm.h"


/**
 * @brief Common base class for algorithms.
 *
 * This is a common template class. Your algorithm should derive from @c AthReentrantAlgorithm
 * if it executes on the CPU, or @c AthAsynchronousAlgorithm if it offloads to an accelerator.
 * For legacy non-thread safe algorithms use @c AthAlgorithm.
 */
template <class BaseAlg>
class AthCommonAlgorithm
  : public AthCommonDataStore<AthCommonMsg<BaseAlg>>
{ 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public:
  using BaseAlg::execState;
  using BaseAlg::name;
  using BaseAlg::m_updateDataHandles;
  using BaseAlg::outputHandles;

  /// Constructor with parameters:
  AthCommonAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  /// Destructor: 
  virtual ~AthCommonAlgorithm() override = default;


  /** @brief Override sysInitialize
   *
   * Loop through all output handles, and if they're WriteCondHandles,
   * automatically register them and this Algorithm with the CondSvc
   */
  virtual StatusCode sysInitialize() override;


  /** @brief Specify if the algorithm is clonable.
   *
   * Only relevant for non-reentrant algorithms. Actual number of clones
   * needs to be set via the "Cardinality" property.
   */
  virtual bool isClonable() const override {
    return true;
  }


  /**
   * @brief Execute an algorithm.
   *
   * We override this in order to work around an issue with the Algorithm
   * base class storing the event context in a member variable that can
   * cause crashes in MT jobs.
   */
  virtual StatusCode sysExecute (const EventContext& ctx) override;

  
  /**
   * @brief Return the list of extra output dependencies.
   *
   * This list is extended to include symlinks implied by inheritance
   * relations.
   */
  virtual const DataObjIDColl& extraOutputDeps() const override;


  /// Get filter decision:
  virtual bool filterPassed(const EventContext& ctx) const {
    return execState( ctx ).filterPassed();
  }


  /// Set filter decision:
  virtual void setFilterPassed( bool state, const EventContext& ctx ) const {
    execState( ctx ).setFilterPassed( state );
  }


 private: 

  /// Extra output dependency collection, extended by AthAlgorithmDHUpdate
  /// to add symlinks. Empty if no symlinks were found.
  DataObjIDColl m_extendedExtraObjects;

}; 

#endif
