///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IEvtIdModifierSvc.h
// Header file for class IEvtIdModifierSvc
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef ATHENAKERNEL_IEVTIDMODIFIERSVC_H
#define ATHENAKERNEL_IEVTIDMODIFIERSVC_H 1

/** @class IEvtIdModifierSvc
 */

// STL includes
#include <vector>

// FrameWork includes
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/IInterface.h"

// AthenaKernel includes

// fwd declares
class EventID;

// EventInfo type
using event_number_t = EventIDBase::event_number_t;

// Special global thread_local to pass event index to EventInfoCnv
namespace EventInfoCnvParams {
inline thread_local event_number_t eventIndex{0};
}

class IEvtIdModifierSvc : virtual public ::IInterface {
  ///////////////////////////////////////////////////////////////////
  // Public typedefs:
  ///////////////////////////////////////////////////////////////////
 public:
  DeclareInterfaceID(IEvtIdModifierSvc, 1, 0);

  using number_type = EventIDBase::number_type;

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:
  /** Destructor:
   */
  virtual ~IEvtIdModifierSvc();

  ///////////////////////////////////////////////////////////////////
  // Const methods:
  ///////////////////////////////////////////////////////////////////

  /** @brief return the (sorted) list of run-numbers which will be modified.
   */
  virtual std::vector<number_type> run_number_list() const = 0;

  ///////////////////////////////////////////////////////////////////
  // Non-const methods:
  ///////////////////////////////////////////////////////////////////

  /** @brief modify an `EventID`'s content
   */
  virtual void modify_evtid(EventID& evt_id, event_number_t eventIndex,
                            bool consume_stream) = 0;
};

#endif  //> !ATHENAKERNEL_IEVTIDMODIFIERSVC_H
