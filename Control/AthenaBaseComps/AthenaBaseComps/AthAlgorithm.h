/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthAlgorithm.h 
// Header file for class AthAlgorithm
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHALGORITHM_H
#define ATHENABASECOMPS_ATHALGORITHM_H 1

// STL includes
#include <string>
#include <type_traits>


// Framework includes
#include "AthenaBaseComps/AthCommonDataStore.h"
#include "AthenaBaseComps/AthCommonMsg.h"
#include "AthenaBaseComps/AthMemMacros.h"
#include "CxxUtils/checker_macros.h"
#include "Gaudi/Algorithm.h"

class EventContext;

/** @class AthAlgorithm AthAlgorithm.h AthenaBaseComps/AthAlgorithm.h
 *
 *  Base class from which non-reentrant (not thread-safe)
 *  Athena algorithm classes should be derived.
 *
 *  For thread-safe Algorithms use @c AthReentrantAlgorithm.
 *
 *  In order for a concrete algorithm class to do anything
 *  useful the methods initialize(), execute() and finalize() 
 *  should be overridden.
 *  The base class provides utility methods for accessing 
 *  standard services (StoreGate service etc.); for declaring
 *  properties which may be configured by the job options 
 *  service; and for creating sub algorithms.
 *  The only base class functionality which may be used in the
 *  constructor of a concrete algorithm is the declaration of 
 *  member variables as properties. All other functionality, 
 *  i.e. the use of services and the creation of sub-algorithms,
 *  may be used only in initialize() and afterwards (see the 
 *  Gaudi and Athena user guides).
 *
 *  @author Sebastien Binet
 *  @date   2008
 */ 

class AthAlgorithm 
  : public AthCommonDataStore<AthCommonMsg< Gaudi::Algorithm >>
{ 
 public: 

  /// Constructor with parameters: 
  AthAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  /// Destructor: 
  virtual ~AthAlgorithm(); 

  /** @brief Override sysInitialize
   *
   * Loop through all output handles, and if they're WriteCondHandles,
   * automatically register them and this Algorithm with the CondSvc.
   */
  virtual StatusCode sysInitialize() override;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  /** @brief Execute method
   *
   * Provides access to the EventContext if needed but is non-const
   * as opposed to AthReentrantAlgorithm.
   */
  virtual StatusCode execute(const EventContext& ctx) = 0;

 private:
  // This is the base-class execute method that gets called by the scheduler.
  virtual StatusCode execute ( const EventContext& ctx ) const override final {
    // "Thread-safe" because scheduler ensures algorithm never gets called concurrently.
    auto nc_this ATLAS_THREAD_SAFE = const_cast<AthAlgorithm*>( this );
    return nc_this->execute( ctx );
  }

#pragma GCC diagnostic pop

 public:

  /**
   * @brief Return the list of extra output dependencies.
   *
   * This list is extended to include symlinks implied by inheritance
   * relations.
   */
  virtual const DataObjIDColl& extraOutputDeps() const override;

  ///@{
  /** Deprecated methods (use the ones with EventContext) */
  const EventContext& getContext() const;
  bool filterPassed() const;
  void setFilterPassed(bool state) const;
  ///@}

 protected:
  /// Legacy algorithms are not thread-safe
  virtual bool isReEntrant() const override final { return false; }

 private:
  DataObjIDColl m_extendedExtraObjects;

}; 

#endif //> !ATHENABASECOMPS_ATHALGORITHM_H
