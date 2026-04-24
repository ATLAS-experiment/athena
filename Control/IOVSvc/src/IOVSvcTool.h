/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IOVSVC_IOVSVCTOOL_H
#define IOVSVC_IOVSVCTOOL_H 1


/*****************************************************************************
 *
 *  IOVSvcTool.h
 *  IOVSvc
 *
 *  Author: Charles Leggett
 *
 *  Provides automatic updating and callbacks for time dependent data
 *  This AlgTool does the real work.
 *
 *****************************************************************************/


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IOVTime.h"
#include "SGTools/DataProxy.h"
#include "IOVSvc/IIOVSvcTool.h"
#include "IOVEntry.h"

#include <string>
#include <set>
#include <map>
#include <utility>
#include <atomic>
#include <memory>

class StoreGateSvc;
class IIncidentSvc;
class Incident;
class IProxyProviderSvc;
class IClassIDSvc;
class IProxyDict;
class IToolSvc;

namespace SG {
  class TransientAddress;
  class DataProxy;
}

struct SortTADptr {
  using is_transparent = void;

  template<typename X, typename Y>
  bool operator() (X&& x, Y&& y) const {
    return x->clID() == y->clID() ? x->name() < y->name() : x->clID() < y->clID();
  }
};

struct SortDPptr {
  bool operator() ( const SG::DataProxy*,
                    const SG::DataProxy* ) const;
};

class IOVSvcTool: public extends<AthAlgTool, IIOVSvcTool, IIncidentListener> {

public:

  IOVSvcTool(const std::string& type, const std::string& name,
             const IInterface* parent);


  virtual StatusCode initialize() override;


  /////////////////////////////////////////////////////////////////////////

  // Incident handler
  virtual void handle(const Incident&) override;

  virtual
  void setStoreName(const std::string& storeName) override {
    m_storeName = storeName;    
  }
  virtual const std::string& getStoreName() const override { return m_storeName; }

  // Update Range from dB
  virtual StatusCode setRange(const CLID& clid, const std::string& key, 
                              IOVRange&) override;

  virtual StatusCode getRange(const CLID& clid, const std::string& key, 
                              IOVRange& iov) const override;

  // Subscribe method for DataProxy. key StoreGate key
  virtual StatusCode regProxy( SG::DataProxy *proxy, 
                               const std::string& key ) override;
  // Another way to subscribe
  virtual StatusCode regProxy( const CLID& clid, const std::string& key ) override;

  virtual StatusCode deregProxy( SG::DataProxy *proxy ) override;
  virtual StatusCode deregProxy( const CLID& clid, const std::string& key ) override;

  // replace method for DataProxy, to be used when an update is necessary
  virtual StatusCode replaceProxy( SG::DataProxy *pOld,
                                   SG::DataProxy *pNew) override;

  // Get IOVRange from db for current event
  virtual StatusCode getRangeFromDB(const CLID& clid, const std::string& key, 
                                    IOVRange& range, std::string &tag,
                                    std::unique_ptr<IOpaqueAddress>& ioa,
				    const IOVTime& curTime) const override;

  // Get IOVRange from db for a particular event
  virtual StatusCode getRangeFromDB(const CLID& clid, const std::string& key, 
                                    const IOVTime& time,
                                    IOVRange& range, std::string &tag,
                                    std::unique_ptr<IOpaqueAddress>& ioa) const override;

  // Set a particular IOVRange in db (and memory)
  virtual StatusCode setRangeInDB(const CLID& clid, const std::string& key, 
                                  const IOVRange& range, 
                                  const std::string &tag) override;
  
  // supply a list of TADs whose proxies will be preloaded
  virtual StatusCode preLoadTAD( const SG::TransientAddress * ) override;

  // supply a list of TADs whose data will be preloaded
  virtual StatusCode preLoadDataTAD( const SG::TransientAddress * ) override;

  virtual bool holdsProxy( SG::DataProxy* proxy ) const override;
  virtual bool holdsProxy( const CLID& clid, const std::string& key ) const override;

  virtual void resetAllProxies() override;

  virtual
  void ignoreProxy( const CLID& clid, const std::string& key ) override{
    m_ignoredProxyNames.insert( std::make_pair(clid,key) );
  }
  virtual
  void ignoreProxy(SG::DataProxy* proxy) override {
    m_ignoredProxies.insert(proxy);
  }

private:

  StatusCode preLoadProxies(const EventContext& ctx);
  StatusCode preLoadData();
  std::string fullProxyName( const SG::TransientAddress* ) const;
  std::string fullProxyName( const SG::DataProxy* ) const;
  std::string fullProxyName( const CLID&, const std::string& ) const;
  void setRange_impl (SG::DataProxy* proxy, IOVRange& iovr);

  std::string m_storeName;

  ServiceHandle<StoreGateSvc> p_cndSvc;
  ServiceHandle<IIncidentSvc> p_incSvc;
  ServiceHandle<IProxyProviderSvc> p_PPSvc;
  ServiceHandle<IClassIDSvc> p_CLIDSvc;
  ServiceHandle<IToolSvc> p_toolSvc;

  std::map< const SG::DataProxy*, std::string> m_names;

  mutable std::recursive_mutex m_handleMutex ATLAS_THREAD_SAFE;
   // meant to protect: m_first, m_entries,
   //    m_startSet...,  m_stopSet... .
   // Locked by "handle" and "setRange", where setRange
   // is called also via preLoadProxies which calls
   // SG::DataProxy::updateAddress which then calls
   // setRange. So, without refactoring a recursive
   // mutex is needed.

  std::set< SG::DataProxy*, SortDPptr > m_proxies;

  std::set<SG::DataProxy*> m_ignoredProxies;
  std::set< std::pair<CLID, std::string> > m_ignoredProxyNames;

  std::map< const SG::DataProxy*, std::unique_ptr<IOVEntry> > m_entries;

  IOVEntry::StartSet_t* p_startSet{nullptr};
  IOVEntry::StopSet_t* p_stopSet{nullptr};

  IOVEntry::StartSet_t m_startSet_Clock, m_startSet_RE;
  IOVEntry::StopSet_t m_stopSet_Clock, m_stopSet_RE;

  std::set< std::unique_ptr<const SG::TransientAddress>, SortTADptr > m_preLoad;

  typedef std::tuple <CLID, std::string> TADkey_t;
  TADkey_t TADkey (const SG::DataProxy& p)
  { return TADkey_t (p.clID(), p.name()); }
  TADkey_t TADkey (const SG::TransientAddress& t)
  { return TADkey_t (t.clID(), t.name()); }
  std::set< TADkey_t > m_partPreLoad;

  std::atomic<bool> m_first{true};
  bool m_checkOnce{false};
  bool m_firstEventOfRun{false};
  std::string m_checkTrigger;

  Gaudi::Property<bool> m_preLoadRanges{this, "preLoadRanges", false};
  Gaudi::Property<bool> m_preLoadData{this, "preLoadData", false};
  Gaudi::Property<bool> m_partialPreLoadData{this, "partialPreLoadData", true};
  Gaudi::Property<bool> m_preLoadExtensibleFolders{this, "preLoadExtensibleFolders", true};
  Gaudi::Property<bool> m_sortKeys{this, "sortKeys", true};
  Gaudi::Property<bool> m_forceReset{this, "forceResetAtBeginRun", false};
  Gaudi::Property<std::string> m_updateInterval{this, "updateInterval", "Event"};


  void scanStartSet(IOVEntry::StartSet_t &pSet, const std::string &type,
                    std::set<SG::DataProxy*, SortDPptr> &proxiesToReset,
		    const IOVTime& curTime) const;
  void scanStopSet(IOVEntry::StopSet_t &pSet, const std::string &type,
                   std::set<SG::DataProxy*, SortDPptr> &proxiesToReset,
		   const IOVTime& curTime) const;

  void PrintStartSet() const;
  void PrintStopSet() const;
  void PrintProxyMap() const;
  void PrintProxyMap(const SG::DataProxy*) const;

};

#endif
