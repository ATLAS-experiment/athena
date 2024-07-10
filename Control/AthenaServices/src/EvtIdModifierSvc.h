///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EvtIdModifierSvc.h
// Header file for class EvtIdModifierSvc
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef ATHENASERVICES_EVTIDMODIFIERSVC_H
#define ATHENASERVICES_EVTIDMODIFIERSVC_H 1

// STL includes
#include <string>
#include <vector>

// FrameWork includes
#include "AthenaBaseComps/AthService.h"

// AthenaKernel
#include "AthenaKernel/IEvtIdModifierSvc.h"

// event includes
#include "EventInfo/EventID.h"

// Forward declaration
class ISvcLocator;
template <class TYPE>
class SvcFactory;

struct ItemModifier {
  EventID::number_type runnbr{0};
  event_number_t evtnbr{0};
  EventID::number_type timestamp{0};
  EventID::number_type lbknbr{0};
  event_number_t nevts{0};
  int flags{0};
};

class EvtIdModifierSvc : virtual public ::IEvtIdModifierSvc,
                         public ::AthService {
  friend class SvcFactory<EvtIdModifierSvc>;

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:
  /// Constructor with parameters:
  EvtIdModifierSvc(const std::string& name, ISvcLocator* pSvcLocator);

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode queryInterface(const InterfaceID& riid,
                                    void** ppvInterface) override;
  //@}

  ///////////////////////////////////////////////////////////////////
  // Const methods:
  ///////////////////////////////////////////////////////////////////

  /** @brief return the (sorted) list of run-numbers which will be modified.
   */
  virtual std::vector<number_type> run_number_list() const override;

  ///////////////////////////////////////////////////////////////////
  // Non-const methods:
  ///////////////////////////////////////////////////////////////////

  static const InterfaceID& interfaceID();

  /** @brief modify an `EventID`'s lumi block content.
   */
  virtual void modify_evtid(EventID& evt_id, event_number_t evt_index,
                            bool consume_stream) override;

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
 private:
  /// Default constructor:
  EvtIdModifierSvc();

  /// (prop) number of events skipped in the event selector
  event_number_t m_skippedEvents;
  /// (prop) first event number at which we begin to modify event ids
  event_number_t m_firstEvtIdx;

  /// (prop) list of n-plets
  /// (run-nbr, evt-nbr, time-stamp, lbk-nbr, nbr-of-events-per-lbk, mod-bit)
  std::vector<uint64_t> m_evtNpletsProp;

  /// (prop) Name of the event store whose EventIDs will be modified.
  std::string m_evtStoreName;

  /// db of list of ItemModifiers:
  /// (run-nbr, evt-nbr, time-stamp, lbk-nbr, nbr-of-events-per-lbk, mod-bit)
  std::vector<ItemModifier> m_evtNplets;

  /// Running total of numEvts before each modifier
  std::vector<event_number_t> m_numEvtTotals;
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

inline const InterfaceID& EvtIdModifierSvc::interfaceID() {
  return IEvtIdModifierSvc::interfaceID();
}

#endif  //> !ATHENASERVICES_EVTIDMODIFIERSVC_H
