/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IOVSVC_IOVSVC_H
#define IOVSVC_IOVSVC_H

/*****************************************************************************
 *
 *  IOVSvc.h
 *  IOVSvc
 *
 *  Author: Charles Leggett
 *
 *  Provides automatic updating and callbacks for time dependent data
 *
 *****************************************************************************/

#include "IOVEntry.h"

#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/IIOVSvc.h"
#include "AthenaKernel/IOVTime.h"
#include "IOVSvc/IIOVSvcTool.h"

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ICondSvc.h"

#include <map>
#include <string>
#include <mutex>


class StoreGateSvc;
class IClassIDSvc;
class IToolSvc;

namespace SG {
  class TransientAddress;
  class DataProxy;
}

class IOVSvc: public extends<AthService, IIOVSvc>
{
public:

  IOVSvc( const std::string& name, ISvcLocator* svc );

  virtual StatusCode initialize() override;
  virtual StatusCode reinitialize() override;

  virtual StatusCode createIOVTool( const std::string& storeName ) override;
  virtual std::vector<std::string> getStoreNames() const override;

  /// Update Range from dB
  virtual StatusCode setRange(const CLID& clid, const std::string& key,
                              IOVRange& io,
                              const std::string& storeName) override;

  virtual StatusCode getRange(const CLID& clid, const std::string& key, 
                              IOVRange& io) const override;

  /// Subscribe method for DataProxy. key StoreGate key
  virtual StatusCode regProxy( SG::DataProxy *proxy, 
                               const std::string& key,
                               const std::string& storeName) override;

  /// replace a registered DataProxy with a new version
  virtual StatusCode replaceProxy(       SG::DataProxy *pOld,
                                         SG::DataProxy *pNew,
                                   const std::string& storeName) override;


  /// Another way to subscribe
  virtual StatusCode regProxy( const CLID& clid, const std::string& key,
                               const std::string& storeName ) override;


  virtual StatusCode deregProxy( SG::DataProxy *proxy ) override;
  virtual StatusCode deregProxy( const CLID& clid, const std::string& key ) override;



  /// Get IOVRange from db for current event
  virtual StatusCode getRangeFromDB(const CLID& clid, const std::string& key, 
                                    IOVRange& range, std::string &tag,
                                    std::unique_ptr<IOpaqueAddress>& ioa,
                                    const EventIDBase& now) const override;

  /// Get IOVRange from db for a particular event
  virtual StatusCode getRangeFromDB(const CLID& clid, const std::string& key, 
                                    const IOVTime& time,
                                    IOVRange& range, std::string &tag,
                                    std::unique_ptr<IOpaqueAddress>& ioa) const override;

  /// Set a particular IOVRange in db (and memory)
  virtual StatusCode setRangeInDB(const CLID& clid, const std::string& key, 
                                  const IOVRange& range, 
                                  const std::string &tag) override;

  /// Drop the associated object from the db and trigger reload
  virtual StatusCode dropObjectFromDB(const CLID& clid, const std::string& key,
                                      const std::string& storeName) override;

  /// supply a list of TADs whose proxies will be preloaded
  virtual StatusCode preLoadTAD( const SG::TransientAddress *,
                                 const std::string& storeName ) override;

  /// supply a list of TADs whose data will be preloaded
  virtual StatusCode preLoadDataTAD( const SG::TransientAddress *,
                                     const std::string& storeName ) override;

  virtual void resetAllProxies() override;

  virtual void ignoreProxy(const CLID& clid, const std::string& key,
                           const std::string& storeName) override;

  virtual StatusCode createCondObj(CondContBase*, const DataObjID&, 
                                   const EventIDBase&) override;


private:

  StatusCode createIOVTool(const std::string& storeName, IIOVSvcTool*& tool);
  IIOVSvcTool* getTool( const std::string& storeName, 
                        bool createIF=true );
  IIOVSvcTool* getTool( const CLID& clid, const std::string& key) const;
  IIOVSvcTool* getTool( SG::DataProxy* proxy ) const;
  std::string fullProxyName( const SG::DataProxy* proxy ) const;
  std::string fullProxyName( const CLID& clid, const std::string& key ) const;

  Gaudi::Property<bool> m_preLoadRanges{this, "preLoadRanges", false};
  Gaudi::Property<bool> m_preLoadData{this, "preLoadData", false};
  Gaudi::Property<bool> m_partialPreLoadData{this, "partialPreloadData", true};
  Gaudi::Property<bool> m_preLoadExtensibleFolders{this, "preLoadExtensibleFolders", true};
  Gaudi::Property<bool> m_sortKeys{this, "sortKeys", true};
  Gaudi::Property<bool> m_forceReset{this, "forceResetAtBeginRun", false};
  Gaudi::Property<std::string> m_updateInterval{this, "updateInterval", "Event"};

  ServiceHandle<IToolSvc> p_toolSvc;
  ServiceHandle<IClassIDSvc> p_CLIDSvc;
  ServiceHandle<StoreGateSvc> p_sgs, p_detStore;
  ServiceHandle<ICondSvc> p_condSvc;

  std::map< std::string, IIOVSvcTool* > m_toolMap;

  // Recursive because createCondObj dereferences a DataProxy, which can
  // eventually recursively call back to IOVSvc.
  mutable std::recursive_mutex m_lock;

};

#endif
