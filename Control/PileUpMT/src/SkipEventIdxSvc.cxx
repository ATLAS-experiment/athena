/*
 * Copyright (C) 2023, 2025 CERN for the benefit of the ATLAS collaboration.
 */
#include "SkipEventIdxSvc.h"

#include <GaudiKernel/DataIncident.h>  // This header contains ContextIncident
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/IIncidentSvc.h>

#include <boost/core/typeinfo.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view.hpp>
#include <string>

#include "AthenaKernel/IEvtIdModifierSvc.h"
#include "xAODEventInfo/EventInfo.h"
#include "PersistentDataModel/AthenaAttributeList.h"

using namespace std::literals;
namespace rv = ranges::views;

namespace {
template <typename propType, typename T>
const Gaudi::Property<propType>&
getProp(SmartIF<T>& iface, const std::string& name)
{
  auto prop_iface = iface.template as<IProperty>();
  const auto& prop = prop_iface->getProperty(name);
  try {
    const auto& conv_prop =
        dynamic_cast<const Gaudi::Property<propType>&>(prop);
    return conv_prop;
  } catch (const std::bad_cast&) {
    std::string if_name = "UNKNOWN";
    try {
      if_name = iface.template as<INamedInterface>()->name();
    } catch (...) {
    }
    const std::string what = std::format(
        "The {} object's {} property has type {} not Gaudi::Property<{}>",
        if_name, name, boost::core::demangled_name(typeid(prop)),
        boost::core::demangled_name(typeid(propType)));
    throw std::logic_error(what);
  }
}

template <typename T>
StatusCode setProp(SmartIF<T>& iface, const std::string& name,
                   const std::string& val) {
  auto prop_iface = iface.template as<IProperty>();
  return prop_iface->setProperty(name, val);
}
}  // namespace

SkipEventIdxSvc::SkipEventIdxSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc) {}

StatusCode SkipEventIdxSvc::initialize() {
  // This is a special service that runs through all the input events ahead of
  // time, and records the run and lb numbers
  ATH_MSG_INFO("Initializing SkipEventIdxSvc");
  SmartIF<IProperty> propMgr(serviceLocator());
  std::string evt_sel_name{};
  ATH_CHECK(propMgr->getProperty("EvtSel", evt_sel_name));
  if (evt_sel_name.empty()) {
    // No event selector
    //
    ATH_MSG_WARNING("No event selector");
    m_started = true;
    return StatusCode::SUCCESS;
  }
  auto sg = serviceLocator()->service<StoreGateSvc>("StoreGateSvc/StoreGateSvc",
                                                    false);
  auto evtSel = serviceLocator()->service<IEvtSelector>(std::move(evt_sel_name), false);
  if (!sg.isValid() || !evtSel.isValid()) {
    ATH_MSG_WARNING("Event selector or storegate is invalid");
    return StatusCode::FAILURE;
  }

  auto modSvc = serviceLocator()->service<IEvtIdModifierSvc>(
      "EvtIdModifierSvc/EvtIdModifierSvc", false);
  std::uint64_t evts_skipped_before_mod = 0;
  std::vector<EvtId> modifier_evts{};
  if (modSvc.isValid()) {
    // Steal it's configuration
    try {
      evts_skipped_before_mod =
          getProp<std::uint64_t&>(modSvc, "SkipEvents").value();
      ATH_MSG_INFO("Skipping " << evts_skipped_before_mod
                               << " events before modifying");
      std::vector<std::uint64_t> lst = 
        getProp<std::vector<std::uint64_t>&>(modSvc, "Modifiers").value();
      ATH_MSG_DEBUG("Printing EvtId modifier config. There are "
                   << lst.size() / 6 << " entries.");
      std::string config_str{};
      auto config_str_iter = std::back_inserter(config_str);
      modifier_evts =
          lst | rv::chunk(6) |
          rv::for_each([&config_str_iter](const auto& rec) {
            const int mod_bitset = rec[5];
            const bool mod_run_num = mod_bitset & 1;
            const bool mod_evt_num = mod_bitset & (1 << 1);
            const bool mod_lb_num = mod_bitset & (1 << 3);

            const std::uint64_t runNum = mod_run_num ? rec[0] : 0;
            const std::uint64_t evtNum = mod_evt_num ? rec[1] : 0;
            const std::uint64_t lbNum = mod_lb_num ? rec[3] : 0;
            const std::uint64_t numEvts = rec[4];

            std::format_to(config_str_iter,
                           "Run: {} [{:c}] LB: {} [{:c}] EVT: {} [{:c}] "
                           "NumEvts: {}\n",
                           runNum, mod_run_num ? 'Y' : 'N', lbNum,
                           mod_lb_num ? 'Y' : 'N', evtNum,
                           mod_evt_num ? 'Y' : 'N', numEvts);
            return ranges::yield_from(
                rv::repeat_n(EvtId{static_cast<uint32_t>(runNum),
                                   static_cast<uint32_t>(lbNum), evtNum},
                             numEvts));
          }) |
          ranges::to<std::vector<EvtId>>;
      ATH_MSG_DEBUG(config_str);
    } catch (const std::bad_cast&) {
      ATH_MSG_ERROR("Wrong type for property of EvtIdModifierSvc.");
    }
  } else {
    ATH_MSG_INFO("No EvtIdModifierSvc found");
  }

  m_initial_skip_events = getProp<int>(evtSel, "SkipEvents");
  ATH_CHECK(setProp(evtSel, "SkipEvents", "0"));
  IEvtSelector::Context* ctx = nullptr;
  ATH_CHECK(sg->clearStore());
  ATH_CHECK(evtSel->createContext(ctx));

  std::uint64_t idx = 0;
  Service* evtSelSvc = dynamic_cast<Service*>(evtSel.get());
  if (!evtSelSvc) {
    ATH_MSG_FATAL("Cannot cast to Service");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(evtSelSvc->start());
  while (evtSel->next(*ctx).isSuccess()) {
    EvtId evt_id{};
    // Load event
    IOpaqueAddress* addr = nullptr;
    ATH_CHECK(evtSel->createAddress(*ctx, addr));
    ATH_CHECK(sg->recordAddress(addr));
    ATH_CHECK(sg->loadEventProxies());

    // Read EventInfo
    // We don't look for the legacy EventInfo, only an Input attribute list and
    // if that doesn't exist, and xAOD::EventInfo
    std::vector<std::string> attr_lists;
    sg->keys<AthenaAttributeList> (attr_lists);
    const auto* attr_list_p =
        sg->tryConstRetrieve<AthenaAttributeList>("Input");
    if (attr_list_p != nullptr && attr_list_p->size() > 6) {
      try {
        const AthenaAttributeList& attr_list = *attr_list_p;
        const auto runNum = attr_list["RunNumber"].data<unsigned>();
        const auto evtNum = attr_list["EventNumber"].data<unsigned long long>();
        const auto lbNum = attr_list["LumiBlockN"].data<unsigned>();
        evt_id = EvtId{runNum, lbNum, evtNum};
      } catch (...) {
      }
    }
    ATH_CHECK(sg->clearStore());
    if (idx >= evts_skipped_before_mod && !modifier_evts.empty()) {
      const std::size_t mod_idx =
          (idx - evts_skipped_before_mod) % modifier_evts.size();
      evt_id.runNum = modifier_evts[mod_idx].runNum != 0U
                          ? modifier_evts[mod_idx].runNum
                          : evt_id.runNum;
      evt_id.lbNum = modifier_evts[mod_idx].lbNum != 0U
                         ? modifier_evts[mod_idx].lbNum
                         : evt_id.lbNum;
      evt_id.evtNum = modifier_evts[mod_idx].evtNum != 0U
                          ? modifier_evts[mod_idx].evtNum
                          : evt_id.evtNum;
    }
    evt_id.evtIdx = idx;
    m_events.push_back(evt_id);
    ++idx;
  }

  // reset current storegate
  ATH_MSG_INFO("Setting SkipEvents back to " << m_initial_skip_events
                                             << " and rewinding");
  ATH_CHECK(
      setProp(evtSel, "SkipEvents", std::format("{}", m_initial_skip_events)));
  ATH_CHECK(evtSel->rewind(*ctx));
  ATH_MSG_INFO("Recorded a total of " << m_events.size() << " events");

  // Register as incident handler
  ServiceHandle<IIncidentSvc> incident_svc("IncidentSvc", name());
  ATH_CHECK(incident_svc.retrieve());
  incident_svc->addListener(this, "BeginRun");
  incident_svc->addListener(this, "SkipEvents");
  return StatusCode::SUCCESS;
}

StatusCode SkipEventIdxSvc::start() {
  ATH_MSG_INFO("Starting SkipEventIdxSvc");
  // Call callbacks if any have already been added
  for (auto&& callback : m_callbacks) {
    auto begin = m_events.cbegin();
    auto end = m_events.cbegin() + m_initial_skip_events;
    ATH_CHECK(std::invoke(callback, begin, end));
  }
  m_started = true;
  return StatusCode::SUCCESS;
}

StatusCode SkipEventIdxSvc::registerCallback(
    std::function<StatusCode(EvtIter, EvtIter)>&& callback) {
  m_callbacks.push_back(callback);
  // Call callback immediately for events skipped in EventSelector if already
  // started
  if (m_started && !m_events.empty()) {
    auto begin = m_events.cbegin();
    auto end = m_events.cbegin() +
               std::min(std::size_t(m_initial_skip_events), m_events.size());
    ATH_CHECK(std::invoke(callback, begin, end));
  }
  return StatusCode::SUCCESS;
}

void SkipEventIdxSvc::handle(const Incident& inc) {
  if (inc.type() != "BeginRun"s && inc.type() != "SkipEvents"s) {
    return;
  }
  ATH_MSG_DEBUG("Received incident of type " << inc.type() << " from "
                                            << inc.source());
  if (inc.type() == "SkipEvents"s) {
    ATH_MSG_DEBUG("Running callbacks");
    const auto& cInc =
        dynamic_cast<const ContextIncident<std::tuple<int, int>>&>(inc);
    auto begin = m_events.cbegin() + std::get<0>(cInc.tag());
    auto end = m_events.cbegin() + std::get<1>(cInc.tag()) +
               1;  // plus 1 for 1 after end
    for (auto&& fn : m_callbacks) {
      if (!std::invoke(fn, begin, end).isSuccess()) {
        throw std::runtime_error(
            "A skipEvent callback returned a failure error code!");
      }
    }
  }
}
