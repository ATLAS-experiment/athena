/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_IPOOLSVC_H
#define POOLSVC_IPOOLSVC_H

/** @file IPoolSvc.h
 *  @brief This file contains the class definition for the IPoolSvc interface class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "GaudiKernel/IService.h"
#include "GaudiKernel/IFileMgr.h"

#include "DataModelRoot/RootType.h"

#include <string>

namespace coral {
   class Context;
}
namespace pool {
   class DbType;
   class ISession;
}

class Placement;
class Token;


/** @class IPoolSvc
 *  @brief This class provides the interface to the APR persistency software.
 **/
class IPoolSvc : virtual public IService {

public: // static members
   /// Stream to distinguish the POOL Stream instances
   enum PoolStream { kInputStream, kOutputStream };

public: // Non-static members
   /// Declare interface ID
   DeclareInterfaceID(IPoolSvc, 1, 0);

   /// @return a token to a Data Object written to Pool.
   /// @param placement [IN] pointer to the placement hint.
   /// @param obj [IN] pointer to the Data Object to be written to Pool.
   /// @param classDesc [IN] pointer to the Seal class description for the Data Object.
   virtual Token* registerForWrite(const Placement* placement,
                                   const void* obj,
                                   const RootType& classDesc) = 0;

   /// @return void
   /// @param obj [OUT] pointer to the Data Object.
   /// @param token [IN] token of the Data Object for which a Pool Ref is filled.
   virtual void setObjPtr(void*& obj, const Token* token) = 0;

   /// @return an Id for an output context (POOL persistency service) and create it if needed.
   /// @param label [IN] string label to name new context and allow sharing (returns existing contextId)
   virtual unsigned int getOutputContext(const std::string& label) = 0;

   /// @return an Id for an input context (POOL persistency service) and create it if needed.
   /// @param label [IN] string label to name new context and allow sharing (returns existing contextId)
   /// @param maxFile [IN] maximum number of open input files.
   virtual unsigned int getInputContext(const std::string& label, unsigned int maxFile = 0) = 0;

   /// @return copy of the map of all labelled input contexts.
   virtual std::map<std::string, unsigned int> getInputContextMap() const  = 0;

   /// @return size of the map of all labelled input contexts.
   virtual unsigned int getInputContextMapSize() const = 0;

   /// @return size of the map of all labelled input contexts.
   virtual pool::ISession* getInputContextSession(unsigned int contextId) const = 0;

   /// @return void
   /// @param shareCat [IN] bool to share the file catalog.
   virtual void setShareMode(bool shareCat) = 0;

   /// @return void
   virtual void startCatalog() = 0;

   /// @return void
   virtual void commitCatalog() = 0;

   /// @return void
   /// @param dbID [IN] database ID to be translated
   /// @param pfn [OUT] string PFN of database
   /// @param type [OUT] string filetype of database
   virtual void lookupBestPfn(const std::string& dbID, std::string& pfn, std::string& type) const = 0;

   /// @return status of connect
   /// @param connection [IN] string containing the connection.
   /// @param collectionName [IN] string containing the persistent name of the collection.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   virtual StatusCode connectCollection(const std::string& connection,
	   const std::string& collectionName,
	   unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// @return status of check
   /// @param connection [IN] string containing the connection.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   /// @param noContainer [IN] if no collection was found check whether file exists or had no events
   virtual
   StatusCode checkCollection(const std::string& connection,
           unsigned int contextId,
           bool noContainer) const = 0;

   /// @return a shared Token ptr for a container entry.
   /// @param connection [IN] string containing the connection/file name.
   /// @param collection [IN] string containing the persistent name of the collection.
   /// @param ientry [IN] entry number for the token to be returned
   virtual Token* getToken(const std::string& connection,
	   const std::string& collection,
	   const unsigned long ientry) const = 0;

   /// Connect to a logical database unit; PersistencySvc is chosen according to transaction type (accessmode).
   virtual StatusCode connect(Io::IoFlag type,
	   unsigned int contextId = IPoolSvc::kInputStream) = 0;

   /// Commit data for a given stream and flush buffer.
   /// @param stream [IN] poolStream to be commited.
   virtual StatusCode commit(unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// Commit data for a given stream and hold buffer.
   /// @param stream [IN] poolStream to be commited.
   virtual StatusCode commitAndHold(unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// Disconnect PersistencySvc associated with a stream.
   /// @param stream [IN] poolStream to be disconnected.
   virtual StatusCode disconnect(unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// Disconnect single Database.
   /// @param connection [IN] connection string for Database to be disconnected.
   /// @param contextId [IN] context id of database to be disconnected.
   virtual StatusCode disconnectDb(const std::string& connection,
	   unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// Get POOL attributes - domain
   virtual StatusCode getAttribute(const std::string& optName,
	   std::string& data,
	   long tech,
	   unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// Get POOL attributes - db/file, container/collection
   virtual StatusCode getAttribute(const std::string& optName,
	   std::string& data,
	   long tech,
	   const std::string& dbName,
	   const std::string& contName = "",
	   unsigned int contextId = IPoolSvc::kInputStream) const = 0;

   /// Set POOL attributes - domain
   virtual StatusCode setAttribute(const std::string& optName,
	   const std::string& data,
	   long tech,
	   unsigned int contextId = IPoolSvc::kOutputStream) const = 0;

   /// Set POOL attributes - db/file, container/collection
   virtual StatusCode setAttribute(const std::string& optName,
	   const std::string& data,
	   long tech,
	   const std::string& dbName,
	   const std::string& contName = "",
	   unsigned int contextId = IPoolSvc::kOutputStream) const = 0;

};

#endif
