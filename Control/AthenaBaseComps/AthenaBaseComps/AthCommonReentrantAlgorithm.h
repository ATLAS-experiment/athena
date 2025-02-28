///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// AthCommonReentrantAlgorithm.h 
// Header file for class AthCommonReentrantAlgorithm
// Author: Charles Leggett
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHCOMMONREENTRANTALGORITHM_H
#define ATHENABASECOMPS_ATHCOMMONREENTRANTALGORITHM_H 1


// STL includes
#include <string>
#include <type_traits>

#include "AthenaBaseComps/AthCommonDataStore.h"
#include "AthenaBaseComps/AthCommonMsg.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMemMacros.h"


#include "Gaudi/Algorithm.h"


/**
 * @brief An algorithm that can be simultaneously executed in multiple threads.
 *
 * This is a common template class. Your algorithm should derive from @c AthReentrantAlgorithm
 * if it executes on the CPU, or @c AthAsynchronousAlgorithm if it offloads to an accelerator.
 *
 */


template <class BaseAlg>
class AthCommonReentrantAlgorithm
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
  AthCommonReentrantAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  /// Destructor: 
  virtual ~AthCommonReentrantAlgorithm() override;

  /** @brief Override sysInitialize
   *
   * Loop through all output handles, and if they're WriteCondHandles,
   * automatically register them and this Algorithm with the CondSvc
   */
  virtual StatusCode sysInitialize() override;
  

  /** Specify if the algorithm is clonable
   *
   * Reentrant algorithms are clonable.
   */
  virtual bool isClonable() const override;


  /** Cardinality (Maximum number of clones that can exist)
   *  special value 0 means that algorithm is reentrant
   *
   * Override this to return 0 for reentrant algorithms.   */
  virtual unsigned int cardinality() const override;


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

  virtual bool filterPassed(const EventContext& ctx) const {
    return execState( ctx ).filterPassed();
  }

  virtual void setFilterPassed( bool state, const EventContext& ctx ) const {
    execState( ctx ).setFilterPassed( state );
  }


 private: 

  /// Default constructor: 
  AthCommonReentrantAlgorithm(); //> not implemented
  AthCommonReentrantAlgorithm (const AthCommonReentrantAlgorithm& ); //> not implemented
  AthCommonReentrantAlgorithm& operator= (const AthCommonReentrantAlgorithm&); //> not implemented

  /// Extra output dependency collection, extended by AthAlgorithmDHUpdate
  /// to add symlinks.  Empty if no symlinks were found.
  DataObjIDColl m_extendedExtraObjects;

}; 

#endif //> !ATHENABASECOMPS_ATHCOMMONREENTRANTALGORITHM_H
