/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLCOLLECTIONCONVERTER_H
#define POOLCOLLECTIONCONVERTER_H

/** @file PoolCollectionConverter.h
 *  @brief This file contains the class definition for the PoolCollectionConverter class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include <string>
#include <memory>

// Forward declarations
class IPoolSvc;
namespace pool {
   class ICollection;
   class ICollectionCursor;
   class DbType;
}
class StatusCode;

/** @class PoolCollectionConverter
 *  @brief This class provides an interface to POOL collections.
 **/
class PoolCollectionConverter {

public:
   /// Constructor
   /// @param collectionType [IN] type of the collection
   /// ("RootCollection", or "ImplicitCollection").
   /// @param svc [IN] pointer to the PoolSvc.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   PoolCollectionConverter(const std::string& collectionType,
		   const std::string& inputCollection,
		   unsigned int contextId,
		   const IPoolSvc* svc);

   /// Destructor
   virtual ~PoolCollectionConverter();

   /// Required by all Gaudi Services
   StatusCode initialize();

   /// @return a pointer to a Pool Collection.
   /// @param collectionType [IN] string containing the collection type.
   /// @param connection [IN] string containing the connection.
   /// @param collectionName [IN] string containing the persistent name of the collection.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   std::unique_ptr<pool::ICollection> createCollection(const std::string& connection,
           const std::string& collectionName,
           const pool::DbType& collectionType,
           unsigned int contextId) const;

   /// Disconnect Database
   StatusCode disconnectDb();

   /// Check whether has valid pool::ICollection*
   StatusCode isValid() const;

   /// @return ICollectionCursor over all entries
   std::unique_ptr<pool::ICollectionCursor> selectAll();

private: // data
   std::string m_collectionType;
   std::string m_inputCollection;
   unsigned int m_contextId;
   const IPoolSvc* m_poolSvc;
   std::unique_ptr<pool::ICollection> m_poolCollection;

private: // hide copy and assignment
   PoolCollectionConverter(const PoolCollectionConverter& rhs);
   PoolCollectionConverter& operator=(const PoolCollectionConverter& rhs);
};
#endif
