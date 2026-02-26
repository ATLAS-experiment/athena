/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "IOVSvcTool.h"
/*****************************************************************************
 *
 *  IOVSvcTool.cxx
 *  IOVSvc
 *
 *  Author: Charles Leggett
 *
 *  Tool to provide automatic updating and callbacks for time dependent data
 *
 *****************************************************************************/


#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/Guards.h"
#include "GaudiKernel/ConcurrencyFlags.h"

#include "AthenaKernel/IProxyDict.h"
#include "AthenaKernel/IProxyProviderSvc.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaKernel/IOVRange.h"
#include "SGTools/TransientAddress.h"
#include "SGTools/DataProxy.h"
#include "StoreGate/StoreGateSvc.h"

#include "IOVEntry.h"
#include "IOVSvc/IOVAddress.h"

#include <algorithm>
#include <stdint.h>
#include <cctype>
#include <stdexcept>


namespace {
  std::atomic<bool> s_firstRun(true);
}


//
///////////////////////////////////////////////////////////////////////////
//

bool
SortDPptr::operator() (const SG::DataProxy* a, const SG::DataProxy *b) const {
  if (a&&b) {
    if (a->name()!=b->name()) return a->name()<b->name();
    if (a->clID()!=b->clID()) return a->clID()<b->clID();
  }
  //Fall back to ptr comp (in principle random, but similar name and
  //clid means that the path in COOL is likely to be similar):
  return a<b;
}

//
///////////////////////////////////////////////////////////////////////////
//


IOVSvcTool::IOVSvcTool(const std::string& type, const std::string& name,
                       const IInterface* parent): 
  base_class( type, name, parent ),
  m_storeName("StoreGateSvc"), 
  p_cndSvc("DetectorStore",name),
  p_incSvc("IncidentSvc",name), p_PPSvc("ProxyProviderSvc",name),
  p_CLIDSvc("ClassIDSvc",name), p_toolSvc("ToolSvc",name)
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
IOVSvcTool::initialize() {

  SmartIF<IProperty> iovSvcProp{service("IOVSvc")};
  ATH_CHECK( iovSvcProp.isValid() );

  ATH_CHECK( setProperty( iovSvcProp->getProperty("preLoadRanges") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("preLoadData") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("partialPreLoadData") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("preLoadExtensibleFolders") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("updateInterval") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("sortKeys") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("forceResetAtBeginRun") ) );
  ATH_CHECK( setProperty( iovSvcProp->getProperty("OutputLevel") ) );

  int pri=100;

  // Convert to uppercase
  std::string updi = m_updateInterval;
  std::transform(updi.begin(), updi.end(), updi.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  if (updi== "JOB") {
    m_checkOnce = true;
    m_checkTrigger = "BeginRun";
    p_incSvc->addListener( this, "BeginRun", pri, true);
    msg() << MSG::INFO;
    msg().setColor(MSG::GREEN);
    msg() << "IOVRanges will be checked only ";
    msg().setColor(MSG::CYAN);
    msg() << "once";
    msg().setColor(MSG::GREEN);
    msg() << " at the start of the job" << endmsg;
  } else if (updi == "RUN") {
    m_checkTrigger = "BeginRun";
    p_incSvc->addListener( this, "BeginRun", pri, true);
    msg() << MSG::INFO;
    msg().setColor(MSG::GREEN);
    msg() << "IOVRanges will be checked at every ";
    msg().setColor(MSG::CYAN);
    msg() << "Run" << endmsg;
  } else if (updi == "EVENT") {
    m_checkTrigger = "BeginEvent";
    p_incSvc->addListener( this, "BeginEvent", pri, true);
    p_incSvc->addListener( this, "BeginRun", pri, true);
    msg() << MSG::INFO;
    msg().setColor(MSG::GREEN);
    msg() << "IOVRanges will be checked at every ";
    msg().setColor(MSG::CYAN);
    msg() << "Event" << endmsg;
  } else {
    ATH_MSG_FATAL("jobOption \"updateInterval\" must be one of "
                  << "\"event\" \"run\" or \"job\"");
    return StatusCode::FAILURE;
  }

  if (m_preLoadData) {
    msg() << MSG::INFO;
    msg().setColor(MSG::GREEN);
    msg() << "IOV Data will be preloaded at the same interval" << endmsg;
  }
    
  ATH_MSG_DEBUG("Tool initialized");

  return StatusCode::SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::handle(const Incident &inc) {

  bool initial_first = m_first;

  // Don't bother doing anything if we're handled the first run, and
  // preLoadData has been set, or if we only want to check once at the
  // beginning of the job
  if (!initial_first && m_preLoadData && m_checkOnce) {
    return;
  }
  else if (!initial_first) {
     if ( inc.type() != m_checkTrigger && inc.type() != IncidentType::BeginRun ) {
        return;
     }
  }

  std::scoped_lock lock(m_handleMutex);
  if (initial_first) {
     if (!m_first && m_preLoadData && m_checkOnce) {
        return;
     }
     // cppcheck-suppress oppositeInnerCondition
     else if (!m_first) {
        if ( inc.type() != m_checkTrigger && inc.type() != IncidentType::BeginRun ) {
           return;
        }
     }
     // cppcheck-suppress identicalInnerCondition
     if (m_first) {
        for (const auto& e : m_ignoredProxyNames) {
          SG::DataProxy* proxy = p_cndSvc->proxy(e.first,e.second);
           ATH_MSG_DEBUG("retrieving "<<fullProxyName(e.first,e.second));
           if (proxy == nullptr) {
              ATH_MSG_ERROR("ignoreProxy: could not retrieve proxy "
                            << fullProxyName(e.first,e.second) << " from store");
           } else {
              ignoreProxy( proxy );
              ATH_MSG_DEBUG("will ignore resetting proxy " << fullProxyName(proxy));
           }
        }
        m_first = false;
     }
     else {
        initial_first=false;
     }
  }//end first
  const bool first = initial_first;

  // Forcing IOV checks on the first event in the run for AthenaMP (ATEAM-439)
  if(Gaudi::Concurrency::ConcurrencyFlags::numProcs()==0) {
    if (inc.type() == IncidentType::BeginRun) {
      m_firstEventOfRun = true;
    }

    if (inc.type() == IncidentType::BeginEvent && m_firstEventOfRun) {
      m_firstEventOfRun = false;
      if (m_checkTrigger == "BeginEvent") {
	return;
      }
    }
  }

  std::set< SG::DataProxy*, SortDPptr > proxiesToReset;
  if ( inc.type() == m_checkTrigger || inc.type() == IncidentType::BeginRun ) {

    const EventIDBase& eventID = inc.context().eventID();
    const uint32_t event = eventID.lumi_block();
    const uint32_t run   = eventID.run_number();
    
    ATH_MSG_DEBUG("Got event info: " << "run="<< run << ", event=" << event);

    IOVTime curTime;
    curTime.setRunEvent(run,event);
    // get ns timestamp from event
    curTime.setTimestamp(1000000000L*(uint64_t)eventID.time_stamp() + eventID.time_stamp_ns_offset());

    if (msgLvl(MSG::DEBUG)) {
      msg().setColor(MSG::YELLOW,MSG::RED);
      msg() << inc.type() << ": [R/LB] = " << curTime << endmsg;
    }

    if (inc.type() == IncidentType::BeginRun) {
      // Signal BeginRun directly to IOVDbSvc
      SmartIF<IIOVDbSvc> iovDB{service("IOVDbSvc", false)};
      if (!iovDB) {
        ATH_MSG_DEBUG("Unable to get the IOVDbSvc");
        return;
      }
      if (StatusCode::SUCCESS != iovDB->signalBeginRun(curTime,
                                                       inc.context()))
      {
        ATH_MSG_ERROR("Unable to signal begin run to IOVDbSvc");
        return;
      }
      else {
        ATH_MSG_DEBUG("Signaled begin run to IOVDbSvc " << curTime);
      }
    }
    
    if (first) {

      for (const auto& tad : m_preLoad) {
        StatusCode sc = regProxy(tad->clID(), tad->name());
        if (StatusCode::SUCCESS != sc) {
          ATH_MSG_ERROR("handle: Could not register proxy for " <<
                        fullProxyName(tad->clID(), tad->name()));
          return;
        }
      }

      if (msgLvl(MSG::VERBOSE)) {
        PrintProxyMap();
        msg() << endmsg;
      }

      // preLoad the ranges and data if requested.
      if (preLoadProxies(inc.context()).isFailure()) {
        ATH_MSG_ERROR("Problems preloading IOVRanges");
        throw( std::runtime_error("IOVSvcTool::preLoadProxies") );
      }

      // Signal EndProxyPreload directly to IOVDbSvc
      SmartIF<IIOVDbSvc> iovDB{service("IOVDbSvc", false)};
      if (iovDB) {
        iovDB->signalEndProxyPreload();
        ATH_MSG_DEBUG("Signaled end proxy preload to IOVDbSvc " << curTime);
      }
    }// end if first
    
    // If preLoadData has been set, never check validity of data again.
    if (m_preLoadData && m_checkOnce) {
      return;
    }

    // Otherwise, do the normal check for validity


    if (msgLvl(MSG::DEBUG)) {
      PrintStartSet();
      PrintStopSet();
      msg() << endmsg;
    }
    
    //
    ////// Scan start and stop Sets for validity
    ////// We need to check both R/E and Clocktime sets
    //

    if (inc.type() == IncidentType::BeginRun && m_forceReset && !s_firstRun) {

      ATH_MSG_DEBUG("Resetting all proxies on BeginRun incident for store \""
                    << m_storeName << "\"");

      if (msgLvl(MSG::VERBOSE)) {
        for (SG::DataProxy* p : m_proxies) {
          msg() << "   " << m_names.at(p) << std::endl;
        }
        msg() << endmsg;
      }
      proxiesToReset = m_proxies;
    } else {
      scanStartSet(m_startSet_Clock,"(ClockTime)",proxiesToReset,curTime);
      scanStartSet(m_startSet_RE,"(R/E)",proxiesToReset,curTime);

      scanStopSet(m_stopSet_Clock,"(ClockTime)",proxiesToReset,curTime);
      scanStopSet(m_stopSet_RE,"(R/E)",proxiesToReset,curTime);
    }

    for (SG::DataProxy* p : m_ignoredProxies) {
      auto itr = proxiesToReset.find(p);
      if (itr != proxiesToReset.end()) {
        proxiesToReset.erase( itr );
      }
    }

    // If MT, must not call any callback functions after first event
    if (!first && !proxiesToReset.empty() &&
        ( (Gaudi::Concurrency::ConcurrencyFlags::numThreads() +
           Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents()) > 0 ) ) {
      ATH_MSG_FATAL("Cannot update Conditions via callback functions in MT after the first event");
      for (const SG::DataProxy* prox : proxiesToReset) {
        ATH_MSG_FATAL("CLID=" << prox->clID() << ", name=" << prox->name());
      }
      throw GaudiException("Cannot update Conditions via callback functions in MT after the first event",name(),StatusCode::FAILURE);
    }

    //
    //// Reset DataProxies, and call associated callback functions
    //// 
    //
    for (SG::DataProxy* prx : proxiesToReset) {
      ATH_MSG_VERBOSE("clearing proxy payload for " << m_names.at(prx));

      // Reset proxy
      
      p_cndSvc->clearProxyPayload( prx );

      // Load data if preload requested.

      if ( (m_partialPreLoadData && 
            m_partPreLoad.contains(TADkey(*prx)) )
           ||
           m_preLoadData ) {       
        ATH_MSG_VERBOSE("preloading data");

        Gaudi::Guards::AuditorGuard auditor(m_names.at(prx), auditorSvc(), "preLoadProxy");
        if (prx->accessData() == nullptr) {
          ATH_MSG_ERROR("problems preloading data for " << m_names.at(prx));
        }
      }

    }

    /// Read in the next set of IOVRanges
    for (SG::DataProxy* prx : proxiesToReset) {
      const auto pitr = m_entries.find( prx );
      if ( pitr != m_entries.end() && pitr->second->range()->isInRange(curTime) ) {
        ATH_MSG_VERBOSE("range still valid for " << m_names.at(prx));
      } else {
        ATH_MSG_DEBUG("calling provider()->udpateAddress(TAD) for " << m_names.at(prx)   );
        if (!prx->updateAddress()) {
          ATH_MSG_ERROR("handle: Could not update address");
          return;
        }
      }
      
      if (msgLvl(MSG::VERBOSE)) {
        IOpaqueAddress *ioa = prx->address();
        // Print out some debug info if this is an IOVAddress (coming 
        // from IOVASCIIDbSvc) 
        IOVAddress *iova  = dynamic_cast<IOVAddress*>(ioa);
        if (iova != nullptr) {
          ATH_MSG_VERBOSE("  range: " << iova->range());
        }
      }

    }

  }  // end if(inc.type() == m_checkTrigger)

  if ( inc.type() == IncidentType::BeginRun) {
    s_firstRun = false;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// Register a DataProxy with the service
///
StatusCode 
IOVSvcTool::regProxy( SG::DataProxy *proxy, const std::string& key) {


  if (proxy ==  nullptr) {
    ATH_MSG_ERROR("proxy == 0");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("registering proxy " << fullProxyName(proxy) << " at " << proxy);

  if (m_proxies.contains(proxy)) {
    ATH_MSG_DEBUG("Proxy for " << fullProxyName(proxy)
                  << " already registered: " << proxy->name());
    return StatusCode::SUCCESS;
  }

  std::string tname;
  ATH_CHECK( p_CLIDSvc->getTypeNameOfID(proxy->clID(), tname) );

  const std::string fullname = tname + "[" + key + "]";

  m_proxies.insert( proxy );
  m_names[ proxy ] = std::move(fullname);

  return StatusCode::SUCCESS;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// Deregister a DataProxy with the service
///
StatusCode 
IOVSvcTool::deregProxy( SG::DataProxy *proxy) {


  if (proxy == nullptr) {
    ATH_MSG_ERROR("proxy == 0");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("removing proxy " << fullProxyName(proxy) << " at " << proxy);

  const auto itr = m_proxies.find(proxy);
  if (itr == m_proxies.end()) {
    ATH_MSG_DEBUG("Proxy for " << fullProxyName(proxy)
                  << " not registered: " << proxy->name());
    return StatusCode::SUCCESS;
  }

  m_proxies.erase( itr );

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace {

  /// Helper to remove an item from a set by using its custom
  /// comparison operator if they are identical (same address).
  template <class SET>
  void removeFromSet (IOVEntry* ent, SET& set)
  {
    auto it = set.lower_bound(ent);
    while (it != set.end() && !set.key_comp()(*it, ent) && !set.key_comp()(ent,*it)) {
      if (*it == ent)
        set.erase (it++);
      else
        ++it;
    }
  }


} // anonymous namespace

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// replace a registered DataProxy with a new version
///
StatusCode 
IOVSvcTool::replaceProxy( SG::DataProxy *pOld,
                          SG::DataProxy *pNew) {

  std::scoped_lock lock(m_handleMutex);
  assert(nullptr != pOld);
  assert(nullptr != pNew);
    
  ATH_MSG_DEBUG("replace proxy " << fullProxyName(pOld)
                << " @" << pOld << " with " << fullProxyName(pNew)
                << " @" << pNew);

  //start with the proxy list
  if (0 == m_proxies.erase(pOld))  {
    ATH_MSG_DEBUG("unregProxy: original proxy "
                  << fullProxyName(pOld) << " not found. Will return now ");
    return StatusCode::SUCCESS;
  } 
  m_proxies.insert(pNew);
  //new name (possibly identical to old)
  m_names.erase(pOld);
  std::string tname;
  ATH_CHECK( p_CLIDSvc->getTypeNameOfID(pNew->clID(), tname) );

  m_names[pNew]=tname + "[" + pNew->name() + "]";

  if (pOld != pNew) {
    const auto itr = m_entries.find(pOld);
    if (itr != m_entries.end()) {
      IOVEntry* ent = itr->second.get();
      removeFromSet (ent, m_startSet_Clock);
      removeFromSet (ent, m_startSet_RE);
      removeFromSet (ent, m_stopSet_Clock);
      removeFromSet (ent, m_stopSet_RE);

      setRange_impl (pNew, *(const_cast<IOVRange*>(ent->range())));
      m_entries.erase (itr);
    }
  }

  return StatusCode::SUCCESS;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// Register a DataProxy with the service
///
StatusCode 
IOVSvcTool::regProxy( const CLID& clid, const std::string& key ) {

  SG::DataProxy* proxy = p_cndSvc->proxy(clid,key);

  if (proxy == nullptr) {
    ATH_MSG_ERROR("regProxy could not retrieve proxy "
                  << fullProxyName(clid,key) << " from store");
    return StatusCode::FAILURE;
  }

  return ( regProxy(proxy, key) );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// Deregister a DataProxy with the service
///
StatusCode 
IOVSvcTool::deregProxy( const CLID& clid, const std::string& key ) {

  SG::DataProxy* proxy = p_cndSvc->proxy(clid,key);

  if (proxy == nullptr) {
    ATH_MSG_ERROR("regProxy could not retrieve proxy "
                  << fullProxyName(clid,key) << " from store");
    return StatusCode::FAILURE;
  }

  return ( deregProxy(proxy) );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// add to a set of TADs that will be registered at start of first event
///
StatusCode 
IOVSvcTool::preLoadTAD( const SG::TransientAddress *tad_in ) {

  // check to see if it's a duplicate in preLoad
  if (m_preLoad.contains( tad_in )) {
    ATH_MSG_WARNING("preLoadTAD: TransientAddress ("
                    << tad_in->clID() << "/" << tad_in->name()
                    << ") alread in preLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  // check to see if it's a duplicate in partPreLoad
  if (m_partPreLoad.contains( TADkey(*tad_in) )) {
    ATH_MSG_WARNING("preLoadTAD: TransientAddress ("
                    << tad_in->clID() << "/" << tad_in->name()
                    << ") alread in partPreLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  m_preLoad.insert( std::make_unique<SG::TransientAddress>(tad_in->clID(),tad_in->name()) );

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// add to a set of TADs that who's data will be preLoaded
///
StatusCode 
IOVSvcTool::preLoadDataTAD( const SG::TransientAddress *tad_in ) {

  if (m_preLoad.contains(tad_in)) {
    ATH_MSG_WARNING("preLoadDataTAD: TransientAddress "
                    << fullProxyName( tad_in )
                    << " alread in preLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  if (m_partPreLoad.contains(TADkey(*tad_in))) {
    ATH_MSG_WARNING("preLoadDataTAD: TransientAddress "
                    << fullProxyName( tad_in )
                    << " alread in partPreLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  auto tad = std::make_unique<SG::TransientAddress>(tad_in->clID(),tad_in->name());
  m_partPreLoad.insert( TADkey(*tad) );
  m_preLoad.insert( std::move(tad) );

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void IOVSvcTool::setRange_impl (SG::DataProxy* proxy, IOVRange& iovr)
{
  if (iovr.start().isTimestamp()) {
    p_startSet = &m_startSet_Clock;
    p_stopSet  = &m_stopSet_Clock;
  } else {
    p_startSet = &m_startSet_RE;
    p_stopSet  = &m_stopSet_RE;
  }

  auto range = std::make_unique<IOVRange>(iovr);

  const auto itr = m_entries.find(proxy);
  if ( itr != m_entries.end() ) {

    const auto& ent = itr->second;

    if (*ent->range() == iovr) {
      ATH_MSG_DEBUG("Range has not changed. Returning");
      return;
      // is this true? still in the start and stop sets? FIXME
    }

    const auto sitr = ent->getStartITR();
    if ( !ent->removedStart() ) {
      p_startSet->erase( sitr );
    }

    const auto pitr = ent->getStopITR();
    if ( !ent->removedStop() ) {
      p_stopSet->erase( pitr );
    }

  }

  ATH_MSG_DEBUG("adding to start and stop sets");
  auto ent = std::make_unique<IOVEntry>(proxy, std::move(range));
  ent->setStartITR( p_startSet->insert( ent.get() ) );
  ent->setStopITR(  p_stopSet->insert( ent.get() ) );
  m_entries.insert_or_assign(proxy, std::move(ent));
}


StatusCode 
IOVSvcTool::setRange(const CLID& clid, const std::string& key, 
                     IOVRange& iovr)
{

  ATH_MSG_DEBUG("setRange()  for clid: " << clid << "  key: " << key
                << "  in IOVrange:" << iovr);

  if (!iovr.start().isValid() || !iovr.stop().isValid()) {
    ATH_MSG_ERROR("IOVRange " << iovr << "is not valid. Start OK: "
                  << iovr.start().isValid() << " Stop OK: " << iovr.stop().isValid()
                  << " run/evt/time min/max "
                  << IOVTime::MINRUN << "/" << IOVTime::MAXRUN << " "
                  << IOVTime::MINEVENT << "/" << IOVTime::MAXEVENT << " "
                  << IOVTime::MINTIMESTAMP << "/" << IOVTime::MAXTIMESTAMP << " ");
    return StatusCode::FAILURE;
  }

  SG::DataProxy* proxy = p_cndSvc->proxy(clid,key);

  if (proxy == nullptr) {
    ATH_MSG_ERROR("setRange: Could not locate proxy for " << fullProxyName(clid,key));
    return StatusCode::FAILURE;
  }

  std::scoped_lock lock(m_handleMutex);
  setRange_impl (proxy, iovr);
  return StatusCode::SUCCESS;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IOVSvcTool::getRange(const CLID& clid, const std::string& key, 
                     IOVRange& iov) const {

  SG::DataProxy* dp = p_cndSvc->proxy(clid,key);

  std::scoped_lock lock(m_handleMutex);
  const auto itr = m_entries.find(dp);
  if (itr == m_entries.end()) {
    return StatusCode::FAILURE;
  }

  iov = *(itr->second->range());

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IOVSvcTool::getRangeFromDB(const CLID& clid, const std::string& key, 
                           IOVRange& range, std::string &tag, 
                           std::unique_ptr<IOpaqueAddress>& ioa, 
			   const IOVTime& curTime) const {

  if (curTime.isValid()) {
    return getRangeFromDB(clid, key, curTime, range, tag, ioa);
  } else {
    ATH_MSG_ERROR("Current Event not defined");
    return StatusCode::FAILURE;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IOVSvcTool::getRangeFromDB(const CLID& clid, const std::string& key,
                           const IOVTime& time, IOVRange& range, 
                           std::string& tag,
                           std::unique_ptr<IOpaqueAddress>& ioa) const {
  StatusCode sc(StatusCode::FAILURE);
  SG::DataProxy* dp = p_cndSvc->proxy(clid,key);
  if (nullptr != dp) {
    IIOVDbSvc *idb = 
      dynamic_cast<IIOVDbSvc*>(dp->provider());
    if (idb != nullptr) {
      sc = idb->getRange(clid, key, time, range, tag, ioa);
    } else {
      ATH_MSG_ERROR("Provider is not an IIOVDbSvc");
    }
  } else {
    ATH_MSG_ERROR("No proxy found for clid " << clid << " key " << key);
  }
  return sc;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IOVSvcTool::setRangeInDB(const CLID& clid, const std::string& key, 
                         const IOVRange& range, const std::string &tag) {


  if (!range.start().isValid() || !range.stop().isValid()) {
    ATH_MSG_ERROR("IOVRange " << range << "is not valid.");
    return StatusCode::FAILURE;
  }

  SG::DataProxy* dp = p_cndSvc->proxy(clid,key);

  if (dp == nullptr) {
    ATH_MSG_ERROR("no Proxy found for " << fullProxyName( clid, key ));
    return StatusCode::FAILURE;
  }

  std::scoped_lock lock(m_handleMutex);
  if (!m_entries.contains(dp)) {
    ATH_MSG_WARNING(fullProxyName(clid,key) << " not registered with the IOVSvc");
  }

  IAddressProvider *iadp = dp->provider();
  IIOVDbSvc *idb = dynamic_cast<IIOVDbSvc*>(iadp);

  if (idb != nullptr) {
    return idb->setRange(clid, key, range, tag);
  } else {
    ATH_MSG_ERROR("Provider is not an IIOVDbSvc");
    return StatusCode::FAILURE;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IOVSvcTool::preLoadProxies(const EventContext& /*ctx*/) {
 
  ATH_MSG_DEBUG("preLoadProxies()");

  StatusCode scr(StatusCode::SUCCESS);

  SmartIF<IIOVDbSvc> iovDB{service("IOVDbSvc", false)};

  for (SG::DataProxy* dp : m_proxies) {
    Gaudi::Guards::AuditorGuard auditor(m_names[dp], auditorSvc(), "preLoadProxy");
    
    if (msgLvl(MSG::VERBOSE)) {
      msg().setColor(MSG::CYAN);
      msg() << "loading proxy for CLID: " << dp->clID()
            << "  " << m_names[dp] << endmsg;
    }

    if (dp->provider() == nullptr) {
      msg() << MSG::FATAL << "No provider found for proxy " << m_names[dp]
            << ".  It is probably  not a conditions object" << std::endl;
      msg() << "Proxy Map: ";
      PrintProxyMap(dp);
      msg() << endmsg;
      scr = StatusCode::FAILURE;
      return (scr);
    }


    StatusCode sc;
    // preload IOVRanges if jobOption set
    // This gets us to an IAddressProvider (eg IOVDbSvc)
    if (m_preLoadRanges) {
      ATH_MSG_VERBOSE("updating Range");
      if (!dp->updateAddress())
        sc = StatusCode::FAILURE;
    }

    if ( ( m_partialPreLoadData && 
           m_partPreLoad.contains(TADkey(*dp)) )
         || m_preLoadData ) {

      IIOVDbSvc::KeyInfo kinfo;
      if ( !m_preLoadExtensibleFolders && iovDB &&
           iovDB->getKeyInfo(dp->name(), kinfo) && kinfo.extensible ) {
        ATH_MSG_VERBOSE("not preloading data for extensible folder " << dp->name());
      }
      else {
        ATH_MSG_VERBOSE("preloading data for ("
                        << dp->clID() << "/"
                        << dp->name() << ")");
        if( dp->accessData() != nullptr ) {
           sc = StatusCode::SUCCESS;
        } else {
           sc = StatusCode::FAILURE;
           ATH_MSG_ERROR("preLoading proxies: accessData() failed for " <<
                         dp->clID() << "/" << dp->name() << ")");
        }
      }
    }

    if (sc.isFailure()) scr=sc;

  }

  if (scr.isFailure()) {
    ATH_MSG_ERROR("Problems preLoading proxies");
    return scr;
  }

  return scr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintStartSet() const {
  std::string objname;
  
  if (!m_startSet_Clock.empty()) {
    msg() << std::endl << "ClockTime start set: " << std::endl;
    for (const auto ent : m_startSet_Clock) {
      objname = m_names.at( ent->proxy() );
      msg() << "  " << objname << " (" << ent->proxy() << ") "
            << ent->range()->start() << std::endl;
    }
    msg() << std::endl;
  }

  if (!m_startSet_RE.empty()) {
    msg() << "Run/Event start set: " << std::endl;
    for (const auto ent : m_startSet_RE) {
      objname = m_names.at( ent->proxy() );
      msg() << "  " << objname << " (" << ent->proxy() << ") "
            << ent->range()->start() << std::endl;
    }
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintStopSet() const {
  std::string objname;
  
  if (!m_stopSet_Clock.empty()) {
    msg() << std::endl << "ClockTime stop set: " << std::endl;
    for( const auto ent : m_stopSet_Clock ) {
      objname = m_names.at(ent->proxy());
      msg() << "  " << objname << " (" << ent->proxy() << ") "
            << ent->range()->stop() << std::endl;
    }
    msg() << std::endl;
  }
  
  if (!m_stopSet_RE.empty()) {
    msg() << "Run/Event stop set: " << std::endl;
    for( const auto ent : m_stopSet_RE ) {
      objname = m_names.at(ent->proxy());
      msg() << "  " << objname << " (" << ent->proxy() << ") "
            << ent->range()->stop() << std::endl;
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintProxyMap() const{
  msg() << std::endl;
  msg() << "------------------------------  IOVSvc Proxy Map  "
        << "------------------------------" << std::endl;

  for (SG::DataProxy* p : m_proxies) {
    PrintProxyMap(p);
  }
  msg() << "----------------------------------------------------------"
        << "---------------------" << std::endl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintProxyMap(const SG::DataProxy* dp) const {

  auto it = m_names.find(dp);
  msg() << "  " << dp << "  " << dp->clID() << "  "
        << (it == m_names.end() ? "???" : it->second) << std::endl;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::scanStartSet(IOVEntry::StartSet_t &pSet, const std::string &type,
                         std::set<SG::DataProxy*, SortDPptr> &proxiesToReset,
			 const IOVTime& curTime) const {

  if (pSet.empty())  return;
  
  if (msgLvl(MSG::DEBUG)) {
    msg() << MSG::DEBUG << "--> scan for resets: start set: " << type << std::endl;
  }

  auto start_itr( pSet.begin() );
  while ( start_itr != pSet.end() ) {
    
    if ((*start_itr)->range()->start() > curTime) {
      if (msgLvl(MSG::DEBUG)) {
        msg() << "\t" << m_names.at((*start_itr)->proxy()) << ": "
              << (*start_itr)->range()->start()<<"   <- removed" << std::endl;
      }
      proxiesToReset.insert( (*start_itr)->proxy() );

      (*start_itr)->setRemovedStart( true );
      pSet.erase(start_itr++);

    } else {
      break;
    }
  }

  if (msgLvl(MSG::DEBUG)) {
    msg() << endmsg;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::scanStopSet(IOVEntry::StopSet_t &pSet, const std::string &type,
                        std::set<SG::DataProxy*, SortDPptr> &proxiesToReset,
			const IOVTime& curTime) const {

  if (pSet.empty())  return;
  if (msgLvl(MSG::DEBUG)) {
    msg() << MSG::DEBUG << "--> scan for resets: stop set: " << type << std::endl;
  }

  auto stop_itr(pSet.begin());
  while ( stop_itr != pSet.end() ) {
    
    if ((*stop_itr)->range()->stop() <= curTime) {
      if (msgLvl(MSG::DEBUG)) {
        msg() << "   " << m_names.at((*stop_itr)->proxy()) << ": "
              << (*stop_itr)->range()->stop()<< "  -> removed" << std::endl;
      }
      proxiesToReset.insert( (*stop_itr)->proxy() );
      
      (*stop_itr)->setRemovedStop( true );
      pSet.erase(stop_itr++);
      
    } else {
      break;
    }
  }
  if (msgLvl(MSG::DEBUG)) {
    msg() << endmsg;
  }  

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
IOVSvcTool::holdsProxy( SG::DataProxy* proxy ) const {

  return m_proxies.contains( proxy );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
IOVSvcTool::holdsProxy( const CLID& clid, const std::string& key ) const {

  SG::DataProxy* proxy = p_cndSvc->proxy(clid,key);

  if (proxy == nullptr) {
    ATH_MSG_ERROR("holdsProxy: could not retrieve proxy "
                  << fullProxyName(clid,key) << " from store");
    return false;
  }

  return ( holdsProxy(proxy) );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
IOVSvcTool::resetAllProxies() {

  for (SG::DataProxy* prx : m_proxies) {
    ATH_MSG_VERBOSE("clearing proxy payload for " << m_names[prx]);
    
    p_cndSvc->clearProxyPayload(prx);
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
IOVSvcTool::fullProxyName( const SG::TransientAddress* tad ) const {

  return fullProxyName(tad->clID(), tad->name());

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
IOVSvcTool::fullProxyName( const SG::DataProxy* dp ) const {
  return fullProxyName(dp->clID(), dp->name());
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
IOVSvcTool::fullProxyName( const CLID& clid, const std::string& key ) const {

  std::string fullname, tname;
  if (p_CLIDSvc->getTypeNameOfID( clid, tname ).isFailure()) {
    fullname = "[";
    fullname += std::to_string(clid);
    fullname += '/';
    fullname += key;
    fullname += ']';
  } else {
    fullname = "[";
    fullname += tname;
    fullname += ':';
    fullname += std::to_string(clid);
    fullname += '/';
    fullname += key;
    fullname += ']';
  }

  return fullname;
}
