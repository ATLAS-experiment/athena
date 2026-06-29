/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENABASECOMPS_ATHALGORITHM_H
#define ATHENABASECOMPS_ATHALGORITHM_H 1

// STL includes
#include <string>

// Framework includes
#include "AthenaBaseComps/AthCommonAlgorithm.h"
#include "Gaudi/Algorithm.h"

class EventContext;


/** @class AthAlgorithm
 *
 *  Base class from which non-reentrant (not thread-safe)
 *  Athena algorithm classes should be derived.
 *
 *  For thread-safe Algorithms use @c AthReentrantAlgorithm.
 *  The only difference is the (non) const-ness of execute().
 */
class AthAlgorithm : public AthCommonAlgorithm<Gaudi::Algorithm>
{
 public: 

  /// Constructor
  AthAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  /// Execute method
  virtual StatusCode execute(const EventContext& ctx) = 0;

 private:
  /// Hide the const execute method from the base class
  virtual StatusCode execute (const EventContext& ctx) const override final;

#pragma GCC diagnostic pop

 protected:
  /// Legacy algorithms are not thread-safe
  virtual bool isReEntrant() const override final { return false; }

};

#endif //> !ATHENABASECOMPS_ATHALGORITHM_H
