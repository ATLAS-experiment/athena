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
#include "CBTree.h"
#include "IOVSvc/IOVCallbackError.h"

#include <stdint.h>
#include <ctype.h>
#include <stdexcept>
#include <atomic>

using SG::DataProxy;
using SG::TransientAddress;

std::string toUpper(const std::string& str) {
  const char *cstr = str.c_str();
  std::string str2("");
  for (unsigned int i=0; i < str.length(); ++i) {
    str2 += toupper(*(cstr+i));
  }

  return str2;
}

namespace {
  std::atomic<bool> s_firstRun(true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

using namespace std;

//
///////////////////////////////////////////////////////////////////////////
//

bool
SortTADptr::operator() ( const SG::TransientAddress* x, 
                         const SG::TransientAddress* y) const {

  if ( x->clID() == y->clID() ) {
    return ( x->name() < y->name() );
  } else {
    return ( x->clID() < y->clID() );
  }

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
  p_CLIDSvc("ClassIDSvc",name), p_toolSvc("ToolSvc",name),
  p_startSet(nullptr),
  p_stopSet(nullptr)
{
  m_trigTree = new CBTree();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOVSvcTool::~IOVSvcTool() {

  // cleanup

  std::map<const DataProxy*, IOVEntry*>::iterator itr;
  for (itr = m_entries.begin(); itr != m_entries.end(); ++itr) {
    IOVEntry *ent = itr->second;
    delete (ent);
  }

  std::set< const TransientAddress*, SortTADptr >::const_iterator titr;
  for (titr = m_preLoad.begin(); titr != m_preLoad.end(); ++titr)
    {
      delete *titr;
    }
  
  delete m_trigTree;

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

  std::string updi = toUpper(m_updateInterval);

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

  std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
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
           DataProxy* proxy = p_cndSvc->proxy(e.first,e.second);
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

  set< DataProxy*, SortDPptr > proxiesToReset;
  if ( inc.type() == m_checkTrigger || inc.type() == IncidentType::BeginRun ) {

    const EventContext& context = inc.context();

    IOVTime curTime;
    
    const EventIDBase& eventID = context.eventID();
    uint32_t event = eventID.lumi_block();
    uint32_t run   = eventID.run_number();
    
    ATH_MSG_DEBUG("Got event info: " << "run="<< run << ", event=" << event);
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

      std::set< const TransientAddress*, SortTADptr >::const_iterator titr;
      for (titr = m_preLoad.begin(); titr != m_preLoad.end(); ++titr) {
        const TransientAddress *tad = *titr;
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

      if (msgLvl(MSG::DEBUG)) {
        msg() << "Callback Tree:" << endmsg;
        m_trigTree->printTree();
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
        std::set< const SG::DataProxy* >::const_iterator pit;
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

    for (auto p : m_ignoredProxies) {
      auto itr = proxiesToReset.find(p);
      if (itr != proxiesToReset.end()) {
        proxiesToReset.erase( itr );
      }
    }

    // If MT, must not call any callback functions after first event
    if (!first && proxiesToReset.size() > 0 &&
        ( (Gaudi::Concurrency::ConcurrencyFlags::numThreads() +
           Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents()) > 0 ) ) {
      ATH_MSG_FATAL("Cannot update Conditions via callback functions in MT after the first event");
      for (const auto* prox : proxiesToReset) {
        ATH_MSG_FATAL("CLID=" << prox->clID() << ", name=" << prox->name());
      }
      throw GaudiException("Cannot update Conditions via callback functions in MT after the first event",name(),StatusCode::FAILURE);
    }

    //
    //// Reset DataProxies, and call associated callback functions
    //// 
    //
    for (DataProxy* prx : proxiesToReset) {
      ATH_MSG_VERBOSE("clearing proxy payload for " << m_names.at(prx));

      // Reset proxy
      
      p_cndSvc->clearProxyPayload( prx );

      m_trigTree->cascadeTrigger(true, prx);

      // Load data if preload requested.

      if ( (m_partialPreLoadData && 
            m_partPreLoad.find(TADkey(*prx)) != m_partPreLoad.end())
           ||
           m_preLoadData ) {       
        ATH_MSG_VERBOSE("preloading data");

        Gaudi::Guards::AuditorGuard auditor(m_names.at(prx), auditorSvc(), "preLoadProxy");
        if (prx->accessData() == nullptr) {
          ATH_MSG_ERROR("problems preloading data for " << m_names.at(prx));
        }
      }

    }

    /// Clear trigger tree
    m_trigTree->clearTrigger();

    /// Read in the next set of IOVRanges
    std::map<const DataProxy*, IOVEntry*>::iterator pitr;
    for (DataProxy* prx : proxiesToReset) {
      pitr = m_entries.find( prx );
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
IOVSvcTool::regProxy( DataProxy *proxy, const std::string& key) {


  if (proxy ==  nullptr) {
    ATH_MSG_ERROR("proxy == 0");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("registering proxy " << fullProxyName(proxy) << " at " << proxy);

  if (m_proxies.find(proxy) != m_proxies.end()) {
    ATH_MSG_DEBUG("Proxy for " << fullProxyName(proxy)
                  << " already registered: " << proxy->name());
    return StatusCode::SUCCESS;
  }

  std::string tname, fullname;
  ATH_CHECK( p_CLIDSvc->getTypeNameOfID(proxy->clID(), tname) );

  fullname = tname + "[" + key + "]";

  m_proxies.insert( proxy );
  m_names[ proxy ] = fullname;

  m_trigTree->addNode(proxy,fullname);

  return StatusCode::SUCCESS;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// Deregister a DataProxy with the service
///
StatusCode 
IOVSvcTool::deregProxy( DataProxy *proxy) {


  if (proxy == nullptr) {
    ATH_MSG_ERROR("proxy == 0");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("removing proxy " << fullProxyName(proxy) << " at " << proxy);

  std::set<SG::DataProxy*, SortDPptr>::iterator itr = m_proxies.find(proxy);
  if (itr == m_proxies.end()) {
    ATH_MSG_DEBUG("Proxy for " << fullProxyName(proxy)
                  << " not registered: " << proxy->name());
    return StatusCode::SUCCESS;
  }

  m_proxies.erase( itr );

  m_trigTree->delNode(proxy);

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace {

  template <class SET>
  void removeFromSet (IOVEntry* ent, SET& set)
  {
    typename SET::iterator it = set.lower_bound(ent);
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
  std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
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
    std::map< const SG::DataProxy*, IOVEntry*>::iterator ent =
      m_entries.find(pOld);
    if (ent != m_entries.end()) {
      removeFromSet (ent->second, m_startSet_Clock);
      removeFromSet (ent->second, m_startSet_RE);
      removeFromSet (ent->second, m_stopSet_Clock);
      removeFromSet (ent->second, m_stopSet_RE);

      setRange_impl (pNew, *(const_cast<IOVRange*>(ent->second->range())));
      delete ent->second;
      m_entries.erase (ent);
    }
  }

  return (m_trigTree->replaceProxy(pOld, pNew) ?
          StatusCode::SUCCESS :
          StatusCode::FAILURE );

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// Register a DataProxy with the service
///
StatusCode 
IOVSvcTool::regProxy( const CLID& clid, const std::string& key ) {

  DataProxy* proxy = p_cndSvc->proxy(clid,key);

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

  DataProxy* proxy = p_cndSvc->proxy(clid,key);

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
IOVSvcTool::preLoadTAD( const TransientAddress *tad_in ) {

  // check to see if it's a duplicate in preLoad
  if (m_preLoad.find( tad_in ) != m_preLoad.end()) {
    ATH_MSG_WARNING("preLoadTAD: TransientAddress ("
                    << tad_in->clID() << "/" << tad_in->name()
                    << ") alread in preLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  // check to see if it's a duplicate in partPreLoad
  if (m_partPreLoad.find( TADkey(*tad_in) ) != m_partPreLoad.end()) {
    ATH_MSG_WARNING("preLoadTAD: TransientAddress ("
                    << tad_in->clID() << "/" << tad_in->name()
                    << ") alread in partPreLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  TransientAddress* tad = new TransientAddress (tad_in->clID(),tad_in->name());
  m_preLoad.insert( tad );

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

///
/// add to a set of TADs that who's data will be preLoaded
///
StatusCode 
IOVSvcTool::preLoadDataTAD( const TransientAddress *tad_in ) {

  if (m_preLoad.find(tad_in) != m_preLoad.end()) {
    ATH_MSG_WARNING("preLoadDataTAD: TransientAddress "
                    << fullProxyName( tad_in )
                    << " alread in preLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  if (m_partPreLoad.find(TADkey(*tad_in)) != m_partPreLoad.end()) {
    ATH_MSG_WARNING("preLoadDataTAD: TransientAddress "
                    << fullProxyName( tad_in )
                    << " alread in partPreLoad set. Not inserting");
    return StatusCode::SUCCESS;
  }

  TransientAddress* tad = new TransientAddress (tad_in->clID(),tad_in->name());
  m_preLoad.insert( tad );
  m_partPreLoad.insert( TADkey(*tad) );

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

  IOVRange *range = new IOVRange(iovr);

  map<const DataProxy*, IOVEntry*>::iterator itr = m_entries.find(proxy);
  if ( itr != m_entries.end() ) {

    IOVEntry *ent = itr->second;
    const IOVRange *irn = ent->range();

    if (*irn == iovr) {
      ATH_MSG_DEBUG("Range has not changed. Returning");
      delete range;
      return;
      // is this true? still in the start and stop sets? FIXME
    }


    startITR sitr = ent->getStartITR();
    if ( !ent->removedStart() ) {
      p_startSet->erase( sitr );
    }



    stopITR pitr = ent->getStopITR();
    if ( !ent->removedStop() ) {
      p_stopSet->erase( pitr );
    }

    delete ent;
  }

  ATH_MSG_DEBUG("adding to start and stop sets");
  IOVEntry *ent = new IOVEntry(proxy,range);
  
  m_entries[ proxy ] = ent;

  ent->setStartITR( p_startSet->insert( ent ) );
  ent->setStopITR(  p_stopSet->insert( ent ) );
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

  DataProxy* proxy = p_cndSvc->proxy(clid,key);

  if (proxy == nullptr) {
    ATH_MSG_ERROR("setRange: Could not locate proxy for " << fullProxyName(clid,key));
    return StatusCode::FAILURE;
  }

  std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
  setRange_impl (proxy, iovr);
  return StatusCode::SUCCESS;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
IOVSvcTool::getRange(const CLID& clid, const std::string& key, 
                     IOVRange& iov) const {

  DataProxy* dp = p_cndSvc->proxy(clid,key);

  std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
  std::map<const DataProxy*,IOVEntry*>::const_iterator itr(m_entries.find(dp));
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
  DataProxy* dp = p_cndSvc->proxy(clid,key);
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

  DataProxy* dp = p_cndSvc->proxy(clid,key);

  if (dp == nullptr) {
    ATH_MSG_ERROR("no Proxy found for " << fullProxyName( clid, key ));
    return StatusCode::FAILURE;
  }

  std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
  std::map<const DataProxy*,IOVEntry*>::const_iterator itr(m_entries.find(dp));
  if (itr == m_entries.end()) {
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

  for (DataProxy* dp : m_proxies) {
    Gaudi::Guards::AuditorGuard auditor(m_names[dp], auditorSvc(), "preLoadProxy");
    
    if (msgLvl(MSG::VERBOSE)) {
      msg().setColor(MSG::CYAN);
      msg() << "loading proxy for CLID: " << dp->clID()
            << "  " << m_names[dp] << endmsg;
    }

    if (dp->provider() == nullptr) {
      msg() << MSG::FATAL << "No provider found for proxy " << m_names[dp]
            << ".  It is probably  not a conditions object" << endl;
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
           m_partPreLoad.find(TADkey(*dp)) != m_partPreLoad.end() )
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

    CBNode* cn = m_trigTree->findNode( dp );
    if (cn != nullptr) {
      m_trigTree->cascadeTrigger(1, cn);
    }

  }

  if (scr.isFailure()) {
    ATH_MSG_ERROR("Problems preLoading proxies");
    return scr;
  }


  m_trigTree->clearTrigger();

  return scr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintStartSet() const {
  startITR start_itr;
  std::string objname;
  
  if (m_startSet_Clock.begin() != m_startSet_Clock.end()) {
    msg() << endl << "ClockTime start set: " << endl;
    for (start_itr = m_startSet_Clock.begin(); start_itr!=m_startSet_Clock.end(); ++start_itr ) {
      objname = m_names.at( (*start_itr)->proxy() );
      msg() << "  " << objname << " (" << (*start_itr)->proxy() << ") "
            << (*start_itr)->range()->start() << endl;    
    }
    msg() << endl;
  }

  if (m_startSet_RE.begin() != m_startSet_RE.end()) {
    msg() << "Run/Event start set: " << endl;
    for (start_itr = m_startSet_RE.begin(); start_itr!=m_startSet_RE.end();++start_itr ) {
      objname = m_names.at( (*start_itr)->proxy() );
      msg() << "  " << objname << " (" << (*start_itr)->proxy() << ") "
            << (*start_itr)->range()->start() << endl;    
    }
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintStopSet() const {
  stopITR  stop_itr;
  std::string objname;
  
  if (m_stopSet_Clock.begin() != m_stopSet_Clock.end()) {
    msg() << endl << "ClockTime stop set: " << endl;
    for( stop_itr=m_stopSet_Clock.begin(); stop_itr!=m_stopSet_Clock.end(); ++stop_itr ) {
      objname = m_names.at((*stop_itr)->proxy());
      msg() << "  " << objname << " (" << (*stop_itr)->proxy() << ") "
            << (*stop_itr)->range()->stop() << endl;    
    }
    msg() << endl;
  }
  
  if (m_stopSet_RE.begin() != m_stopSet_RE.end()) {
    msg() << "Run/Event stop set: " << endl;
    for( stop_itr=m_stopSet_RE.begin(); stop_itr!=m_stopSet_RE.end(); ++stop_itr ) {
      objname = m_names.at((*stop_itr)->proxy());
      msg() << "  " << objname << " (" << (*stop_itr)->proxy() << ") "
            << (*stop_itr)->range()->stop() << endl;    
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintProxyMap() const{
  msg() << endl;
  msg() << "------------------------------  IOVSvc Proxy Map  "
        << "------------------------------" << endl;

  for (DataProxy* p : m_proxies) {
    PrintProxyMap(p);
  }
  msg() << "----------------------------------------------------------"
        << "---------------------" << endl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::PrintProxyMap(const SG::DataProxy* dp) const {

  auto it = m_names.find(dp);
  msg() << "  " << dp << "  " << dp->clID() << "  "
        << (it == m_names.end() ? "???" : it->second) << endl;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
IOVSvcTool::scanStartSet(startSet &pSet, const std::string &type, 
                         std::set<SG::DataProxy*, SortDPptr> &proxiesToReset,
			 const IOVTime& curTime) const {

  if (pSet.begin()==pSet.end())  return;
  
  if (msgLvl(MSG::DEBUG)) {
    msg() << MSG::DEBUG << "--> scan for resets: start set: " << type << endl;
  }

  startITR start_itr( pSet.begin() );
  while ( start_itr != pSet.end() ) {
    
    if ((*start_itr)->range()->start() > curTime) {
      if (msgLvl(MSG::DEBUG)) {
        msg() << "\t" << m_names.at((*start_itr)->proxy()) << ": "
              << (*start_itr)->range()->start()<<"   <- removed"<<endl;
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
IOVSvcTool::scanStopSet(stopSet &pSet, const std::string &type,
                        std::set<SG::DataProxy*, SortDPptr> &proxiesToReset,
			const IOVTime& curTime) const {

  if (pSet.begin()==pSet.end())  return;
  if (msgLvl(MSG::DEBUG)) {
    msg() << MSG::DEBUG << "--> scan for resets: stop set: " << type << endl;
  }

  stopITR  stop_itr(pSet.begin());
  while ( stop_itr != pSet.end() ) {
    
    if ((*stop_itr)->range()->stop() <= curTime) {
      if (msgLvl(MSG::DEBUG)) {
        msg() << "   " << m_names.at((*stop_itr)->proxy()) << ": "
              << (*stop_itr)->range()->stop()<< "  -> removed"<<endl;
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
IOVSvcTool::holdsProxy( DataProxy* proxy ) const {

  return ! ( m_proxies.find( proxy ) == m_proxies.end() );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
IOVSvcTool::holdsProxy( const CLID& clid, const std::string& key ) const {

  DataProxy* proxy = p_cndSvc->proxy(clid,key);

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

  for (DataProxy* prx : m_proxies) {
    ATH_MSG_VERBOSE("clearing proxy payload for " << m_names[prx]);
    
    p_cndSvc->clearProxyPayload(prx);
    
    m_trigTree->cascadeTrigger(true, prx);

  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
IOVSvcTool::fullProxyName( const TransientAddress* tad ) const {

  return fullProxyName(tad->clID(), tad->name());

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
IOVSvcTool::fullProxyName( const DataProxy* dp ) const {
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
