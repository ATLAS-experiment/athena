///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EvtIdModifierSvc.cxx
// Implementation file for class EvtIdModifierSvc
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////

// AthenaServices includes
#include "EvtIdModifierSvc.h"

// STL includes
#include <algorithm>
#include <set>

// FrameWork includes
#include "StoreGate/StoreGateSvc.h"

// EventInfo includes
#include "EventInfo/EventID.h"


namespace {
constexpr int prop_per_nplet{6};

enum ModFlag {
  RUNNBR = 1 << 0,
  EVTNBR = 1 << 1,
  TIMESTAMP = 1 << 2,
  LBKNBR = 1 << 3
};

using number_type = IEvtIdModifierSvc::number_type;

}  // namespace

///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructors
////////////////
EvtIdModifierSvc::EvtIdModifierSvc(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : base_class(name, pSvcLocator) {

  declareProperty("Modifiers", m_evtNpletsProp,
                  "A list of n-uplets "
                  "(RunNumber,EvtNbr,TimeStamp,LumiBlock,Nevents,ModBit).");

  declareProperty("EvtStoreName", m_evtStoreName = "StoreGateSvc",
                  "Name of the event store whose EventIDs will be modified.");

  declareProperty("SkipEvents", m_firstEvtIdx = 0,
                  "Number of events to skip before modifying EventIDs.");
  declareProperty("SkippedEvents", m_skippedEvents = 0,
                  "Number of events skipped in the EventSelector.");
}

// Athena Service's Hooks
////////////////////////////
StatusCode EvtIdModifierSvc::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  if (const auto nplets_prop_count = m_evtNpletsProp.size();
      nplets_prop_count > 0) {
    // they should be Nplets...
    if (nplets_prop_count % prop_per_nplet != 0) {
      ATH_MSG_ERROR("invalid list of n-plets (not divisible by "
                    << prop_per_nplet << ")" << endmsg << "check your joboptions !");
      return StatusCode::FAILURE;
    }

    m_evtNplets.reserve(nplets_prop_count / prop_per_nplet);
    for (std::size_t i = 0; i < nplets_prop_count; i += prop_per_nplet) {
      m_evtNplets.push_back({
          // clang-format off
          .runnbr    = static_cast<number_type>(   m_evtNpletsProp[i + 0]),
          .evtnbr    = static_cast<event_number_t>(m_evtNpletsProp[i + 1]),
          .timestamp = static_cast<number_type>(   m_evtNpletsProp[i + 2]),
          .lbknbr    = static_cast<number_type>(   m_evtNpletsProp[i + 3]),
          .nevts     = static_cast<event_number_t>(m_evtNpletsProp[i + 4]),
          .flags     = static_cast<int>(           m_evtNpletsProp[i + 5])
          // clang-format on
      });
    }
  }

  // initialize running total of nevts
  m_numEvtTotals.clear();
  event_number_t sum = 0;
  for (const ItemModifier& elem : m_evtNplets) {
    sum += elem.nevts;
    m_numEvtTotals.push_back(sum);
  }

  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << "store being modified: [" << m_evtStoreName << "]"
                    << endmsg << "evtid-modifiers: [ ";
    for (const ItemModifier& elem : m_evtNplets) {
      msg(MSG::DEBUG) << "[" << elem.runnbr << ", " << elem.evtnbr << ", "
                      << elem.timestamp << ", " << elem.lbknbr << ", "
                      << elem.nevts << ", flags=0x" << std::hex << elem.flags
                      << std::dec << "], ";
    }
    msg(MSG::DEBUG) << "]" << endmsg;
  }

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Const methods:
///////////////////////////////////////////////////////////////////

/** @brief return the (sorted) list of run-numbers which will be modified.
 */
std::vector<number_type> EvtIdModifierSvc::run_number_list() const {
  std::set<number_type> runs;

  for (const ItemModifier& elem : m_evtNplets) {
    if (elem.flags & ModFlag::RUNNBR) {
      runs.insert(elem.runnbr);
    }
  }
  return std::vector(runs.begin(), runs.end());
}

///////////////////////////////////////////////////////////////////
// Non-const methods:
///////////////////////////////////////////////////////////////////

/** @brief modify an `EventID`'s content.
 */
void EvtIdModifierSvc::modify_evtid(EventID& evt_id, event_number_t evt_index,
                                    bool consume_stream) {
  // Left in to match old observable behaviour:
  // only when consuming stream is required do we check for a matching
  // current StoreGate name (ie: typically the case of being called from a T/P
  // cnv)
  if (consume_stream) {
    StoreGateSvc* active = StoreGateSvc::currentStoreGate();
    if (!active) {
      ATH_MSG_INFO("could not retrieve the active evtstore - bailing out");
      return;
    }

    const std::string& evtStoreName = active->name();
    ATH_MSG_DEBUG("active store: [" << evtStoreName << "]");
    if (evtStoreName != m_evtStoreName) {
      return;
    }
  }

  ATH_MSG_DEBUG("evtid before massaging: " << "(" << evt_id.run_number() << ", "
                                           << evt_id.event_number() << ", "
                                           << evt_id.time_stamp() << ", "
                                           << evt_id.lumi_block() << ")");

  // event skipping
  std::int64_t idx =
      std::int64_t(evt_index) + m_skippedEvents - std::int64_t(m_firstEvtIdx);
  std::int64_t idx_looped = idx % m_numEvtTotals.back();
  ATH_MSG_DEBUG("Got event idx " << evt_index << " --(account for skipping)--> "
                                 << idx << " --(modulo #modifiers)--> "
                                 << idx_looped);
  if (idx < 0) {
    ATH_MSG_DEBUG("skip event");
    return;
  }

  // Account for events skipped in
  std::size_t mod_idx = std::upper_bound(m_numEvtTotals.cbegin(),
                                         m_numEvtTotals.cend(), idx_looped) -
                        m_numEvtTotals.cbegin();
  ItemModifier current = m_evtNplets[mod_idx];
  ATH_MSG_DEBUG("Unique modifier index " << mod_idx
                                         << " (LB: " << current.lbknbr << ")");
  if (mod_idx >= m_numEvtTotals.size()) {
    // Shouldn't happen
    ATH_MSG_ERROR("Somehow run out of modifiers");
    return;
  }

  if (current.flags & ModFlag::RUNNBR) {
    evt_id.set_run_number(current.runnbr);
  }
  if (current.flags & ModFlag::EVTNBR) {
    evt_id.set_event_number(current.evtnbr);
  }
  if (current.flags & ModFlag::TIMESTAMP) {
    evt_id.set_time_stamp(current.timestamp);
  }
  if (current.flags & ModFlag::LBKNBR) {
    evt_id.set_lumi_block(current.lbknbr);
  }

  ATH_MSG_DEBUG("evtid after  massaging: " << "(" << evt_id.run_number() << ", "
                                           << evt_id.event_number() << ", "
                                           << evt_id.time_stamp() << ", "
                                           << evt_id.lumi_block() << ")");
}
