/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#include <optional>

#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoringKernel/HistogramDef.h"
#include "AthenaMonitoringKernel/IMonitoredVariable.h"

#include "HistogramFiller/HistogramFiller.h"
#include "HistogramFiller/HistogramFillerFactory.h"

using namespace Monitored;

GenericMonitoringTool::~GenericMonitoringTool() { }

StatusCode GenericMonitoringTool::initialize() {
  ATH_CHECK(m_histSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode GenericMonitoringTool::start() {
  if ( not m_explicitBooking ) {
    ATH_MSG_DEBUG("Proceeding to histogram booking");
    return book();
  }
  return StatusCode::SUCCESS;
}

StatusCode GenericMonitoringTool::stop() {
  m_alwaysCreateFillers.clear();
  m_fillers.clear();
  if (m_registerHandler) {
    ATH_MSG_DEBUG("Deregistering incident handler");
    SmartIF<IIncidentSvc> incSvc{service("IncidentSvc")};
    ATH_CHECK(incSvc.isValid());
    incSvc->removeListener(this, IncidentType::BeginEvent);
  }
  return StatusCode::SUCCESS;
}

void GenericMonitoringTool::handle( const Incident& ) {
  for (const auto& filler : m_alwaysCreateFillers) {
    filler->touch();
  }
}

StatusCode GenericMonitoringTool::book() {

  // If no histogram path given use parent or our own name
  if (m_histoPath.empty()) {
    auto named = dynamic_cast<const INamedInterface*>(parent());
    m_histoPath = named ? named->name() : name();
  }

  // Replace dot (e.g. MyAlg.MyTool) with slash to create sub-directory
  std::replace( m_histoPath.begin(), m_histoPath.end(), '.', '/' );

  ATH_MSG_DEBUG("Booking histograms in path: " << m_histoPath.value());

  HistogramFillerFactory factory(this, m_histoPath);

  for (const std::string& item : m_histograms) {
    if (item.empty()) {
      ATH_MSG_DEBUG( "Skipping empty histogram definition" );
      continue;
    }
    ATH_MSG_DEBUG( "Configuring monitoring for: " << item );
    HistogramDef def = HistogramDef::parse(item);

    if (def.ok) {
      std::shared_ptr<HistogramFiller> filler(factory.create(def));

      if (filler) {
        if (def.kAlwaysCreate) {
          if (m_registerHandler) {
            m_alwaysCreateFillers.push_back(filler); // prepare list of fillers for handler
          } else {
            filler->touch(); // create now and be done with it
          }
        }
      	m_fillers.push_back(std::move(filler));
      } else {
        ATH_MSG_WARNING( "The histogram filler cannot be instantiated for: " << def.name );
      }
    } else {
      ATH_MSG_ERROR( "Unparsable histogram definition: " << item );
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG( "Monitoring for variable " << def.name << " prepared" );
  }

  if ( m_fillers.empty() && m_failOnEmpty ) {
    std::string hists;
    for (const auto &h : m_histograms) hists += (h+",");
    ATH_MSG_ERROR("No monitored variables created based on histogram definition: [" << hists <<
                  "] Remove this monitoring tool or check its configuration.");
    return StatusCode::FAILURE;
  }

  // are there some histograms that should always be made?
  // then register to be notified on every event
  if (! m_alwaysCreateFillers.empty() && m_registerHandler) {
    ATH_MSG_DEBUG("Registering incident handler");
    SmartIF<IIncidentSvc> incSvc{service("IncidentSvc")};
    ATH_CHECK(incSvc.isValid());
    incSvc->addListener(this, IncidentType::BeginEvent);
  }

  return StatusCode::SUCCESS;
}

namespace Monitored {
    std::ostream& operator<< ( std::ostream& os, const std::reference_wrapper<Monitored::IMonitoredVariable>& rmv ) {
        std::string s = rmv.get().name();
        return os << s;
    }
}

namespace {
  void invokeFillersDebug(MsgStream& log,
                          const Monitored::HistogramFiller::VariablesPack& vars,
                          const std::shared_ptr<Monitored::HistogramFiller>& filler,
                          const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& monitoredVariables) {
    bool reasonFound = false;
    if (ATH_UNLIKELY(!filler->histogramWeightName().empty() && !vars.weight)) {
      reasonFound = true;
      log << MSG::DEBUG << "Filler weight not found in monitoredVariables:"
          << "\n  Filler weight               : " << filler->histogramWeightName()
          << "\n  Asked to fill from mon. tl_vars: " << monitoredVariables << endmsg;
    }
    if (ATH_UNLIKELY(!filler->histogramCutMaskName().empty() && !vars.cut)) {
      reasonFound = true;
      log << MSG::DEBUG << "Filler cut mask not found in monitoredVariables:"
          << "\n  Filler cut mask             : " << filler->histogramCutMaskName()
          << "\n  Asked to fill from mon. tl_vars: " << monitoredVariables << endmsg;
    }
    if ( not reasonFound ) {
      log << MSG::DEBUG << "Filler has different variables than monitoredVariables:"
          << "\n  Filler variables            : " << filler->histogramVariablesNames()
          << "\n  Asked to fill from mon. tl_vars: " << monitoredVariables
          << "\n  Selected monitored variables: " << vars.names() << endmsg;
    }
  }

  /**
   * Concatenate the monitored variable names to create a key to be used in the ConcurrentStr map.
   * This may seem very inefficient but is actually faster than the previous solution of storing a
   * std::vector<std::string> in a std::map + mutex.
   */
  std::string fillerKey(const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& v) {
    std::string r;
    for (const auto& m : v) r.append(m.get().name());
    return r;
  }
}

void GenericMonitoringTool::invokeFillers(const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& monitoredVariables) const {
  // This is the list of fillers to consider in the invocation.
  // If we are using the cache then this may be a proper subset of m_fillers; otherwise will just be m_fillers
  const std::vector<std::shared_ptr<Monitored::HistogramFiller>>* fillerList{&m_fillers};
  // pointer to list of matched fillers, if we need to update the cache (default doesn't create the vector)
  std::optional<std::vector<std::shared_ptr<Monitored::HistogramFiller>> > matchedFillerList;
  if (m_useCache) {
    const auto match = m_fillerCacheMap.find(fillerKey(monitoredVariables));
    if (match != m_fillerCacheMap.end()) {
      fillerList = &match->second;
    } else {
      // make new cache entry
      matchedFillerList.emplace();
    }
  }

  for ( auto filler: *fillerList ) {
    const int fillerCardinality = filler->histogramVariablesNames().size() + (filler->histogramWeightName().empty() ? 0: 1) + (filler->histogramCutMaskName().empty() ? 0 : 1);

    if ( fillerCardinality == 1 ) { // simplest case, optimising this to be super fast
      for ( auto& var: monitoredVariables ) {
        if ( var.get().name().compare( filler->histogramVariablesNames()[0] ) == 0 )  {
          {
            auto guard{filler->getLock()};
            filler->fill({&var.get()});
          }
          if (matchedFillerList) {
            matchedFillerList->push_back(std::move(filler));
          }
          break;
        }
      }
    } else { // a more complicated case, and cuts or weights
      int matchesCount = 0;
      Monitored::HistogramFiller::VariablesPack vars;
      for ( const auto& var: monitoredVariables ) {
        bool matched = false;
        for ( unsigned fillerVarIndex = 0; fillerVarIndex < filler->histogramVariablesNames().size(); ++fillerVarIndex ) {
          if ( var.get().name().compare( filler->histogramVariablesNames()[fillerVarIndex] ) == 0 ) {
            vars.set(fillerVarIndex, &var.get());
            matched = true;
            matchesCount++;
            break;
          }
        }
        if ( matchesCount == fillerCardinality ) break;
        if ( not matched ) { // may be a weight or cut variable still
          if ( var.get().name().compare( filler->histogramWeightName() ) == 0 )  {
            vars.weight = &var.get();
            matchesCount ++;
          } else if ( var.get().name().compare( filler->histogramCutMaskName() ) == 0 )  {
            vars.cut = &var.get();
            matchesCount++;
         }
        }
        if ( matchesCount == fillerCardinality ) break;
      }
      if ( matchesCount == fillerCardinality ) {
        {
          auto guard{filler->getLock()};
          filler->fill( vars );
        }
        if (matchedFillerList) {
          matchedFillerList->push_back(std::move(filler));
        }
      } else if ( ATH_UNLIKELY( msgLvl(MSG::DEBUG) && matchesCount != 0 ) ) { // something has matched, but not all, worth informing user
        invokeFillersDebug(msg(), vars, filler, monitoredVariables);
      }
    }
  }

  if (matchedFillerList) {
    // We may hit this multiple times. If another thread has updated the cache in the meanwhile,
    // nothing will be done here.
    m_fillerCacheMap.emplace(fillerKey(monitoredVariables), std::move(*matchedFillerList));
  }
}

uint32_t GenericMonitoringTool::runNumber() {
  return Gaudi::Hive::currentContext().eventID().run_number();
}

uint32_t GenericMonitoringTool::lumiBlock() {
  return Gaudi::Hive::currentContext().eventID().lumi_block();
}
