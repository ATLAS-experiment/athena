/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONBASE_ICOLLECTIONSERVICE_H
#define COLLECTIONBASE_ICOLLECTIONSERVICE_H

#include "CollectionBase/ICollection.h"
#include "CxxUtils/checker_macros.h"

#include <string>


namespace pool {

  class ICollectionDescription;
  class ISession;
  class ICollectionCursor;
  class IFileCatalog;

  /**
   * @class ICollectionService ICollectionService.h CollectionBase/ICollectionService.h
   *
   * An interface to a service for creating, accessing and managing an ensemble of
   * collections of event references. In many cases, an
   * object managed by the service may simply consist of a "collection fragment" which 
   * contains metadata but no event references and is meant to be added to a collection. 
   * Note that a class that inherits from this interface must also inherit from the SEAL 
   * Service base class. 
   */
  class ATLAS_NOT_THREAD_SAFE ICollectionService
  {
  public:
    /**
     * Creates or overwrites a collection or collection fragment, given a description of 
     * its properties.
     *
     * @param description Specification of collection or collection fragment properties.
     * @param overwrite Flag to distinguish creation and overwrite open modes.
     */
    virtual ICollection* create( const ICollectionDescription& description,
                                 bool overwrite = false ) = 0;

    /**
     * Creates or overwrites a collection, given a description of its properties, and registers 
     * it in a collection catalog managed by the collection service. Throws an exception if the 
     * collection has been replicated, if an attempt is being made to overwrite an existing 
     * collection when the overwrite input argument has been set to "false" or if the object 
     * being registered is a collection fragment.
     *
     * @param description Specification of collection properties.
     * @param overwrite Flag to distinguish creation and overwrite open modes.
     */
    virtual ICollection* createAndRegister( const ICollectionDescription& description,
                                            bool overwrite = false,
                                            const std::string & logicalName = "" ) = 0;

    /**
     * Registers an existing collection in a collection catalog managed by the collection
     * service, given its name, storage technology type and database connection string. 
     * Throws an exception if the collection does not exist, if a collection has already been 
     * registered in the collection catalog with identical physical name or if the object being 
     * registered is a collection fragment.
     *
     * @param name Name of collection.
     * @param type Storage technology type of collection.
     * @param connection Connection to database containing collection.
     */
    virtual bool registerExisting( const std::string & name,
                                   const std::string & type,
                                   const std::string & connection = "",
                                   const std::string & logicalName = "" ) = 0;


    /**
     * Registers an existing collection in a collection catalog managed by the collection
     * service.  
     *
     * @param collection The collection to register
     * @param overwrite If true attempt to overwrite catalog entry for the same collection name
     * @param logicalName Logical name of the collection to store in the catalog
     */
    virtual bool registerExisting( ICollection* collection,
				   bool overwrite = false,
                                   const std::string & logicalName = "" ) = 0;


    /**
     * Retrieves a handle to an existing collection or collection fragment for read or update
     * transactions, given the collection or collection fragment's name, storage technology type 
     * and database connection string. A reference to a POOL database session object must be
     * provided as input for the case where the collection being accessed is of type 
     * "ImplicitCollection".
     *
     * @param name Name of collection or collection fragment.
     * @param type Storage technology type of collection or collection fragment.
     * @param connection Connection to database containing collection or collection fragment.
     * @param readOnly Flag to distinguish read and update open modes.
     * @param session Reference to database session (need only be set for implicit collections).
     */
    virtual ICollection* handle( const std::string& name,
                                 const std::string& type,
                                 const std::string & connection = "",
                                 bool readOnly = true,
                                 ISession* session = 0 ) const = 0;

    /**
     * Retrieves a handle to an existing collection for read or update transactions, given the 
     * physical name used to register the collection in a collection catalog managed by the 
     * collection service. Throws an exception if a collection with this physical name cannot 
     * be found in the catalog.
     *
     * @param physicalName Physical name used to register collection in catalog.
     * @param readOnly Flag to distinguish read and update open modes.
     */
    virtual ICollection* openWithPhysicalName( const std::string& physicalName,
                                               pool::ICollection::OpenMode openMode = pool::ICollection::READ ) = 0;

    /**
     * Retrieves a handle to an existing collection for read or update transactions, given the 
     * logical name used to register the collection in a collection catalog managed by the 
     * collection service. Throws an exception if a collection with this logical name cannot be 
     * found in the catalog.
     *
     * @param logicalName Logical name used to register collection in catalog.
     * @param readOnly Flag to distinguish read and update open modes.
     */
    virtual ICollection* openWithLogicalName( const std::string& logicalName, 
                                              bool readOnly = true ) = 0;
                    
    /**
     * Retrieves a handle to an existing collection for read or update transactions, given the 
     * globally unique identifier used to register the collection in a collection catalog 
     * managed by the collection service. Throws an exception if a collection with this GUID 
     * cannot be found in the catalog.
     *
     * @param guid Globally unique identifier used to register collection in catalog.
     * @param readOnly Flag to distinguish read and update open modes.
     */
    virtual ICollection* openWithGuid( const std::string& guid,
                                       bool readOnly = true ) = 0;

    /// Sets default values for the write and read collection catalogs.
    virtual bool setDefaultCatalog() = 0;

    /**
     * Sets the name of the catalog in which to register new collections.
     *
     * @param name Name of collection catalog for registering collections.
     */
    virtual bool setWriteCatalog( const std::string& name ) = 0;

    /**
     * Adds a catalog to the list of catalogs in which to search for registered collections.
     *
     * @param name Name of collection catalog to add to search list.
     */
    virtual bool addReadCatalog( const std::string& name ) = 0;

    /**
     * Returns the collection catalog used by the service
     */
    virtual IFileCatalog* getCatalog( ) = 0;

   /**
     * Forces the service to use externally provided file catalogs
     * The catalogs are not deleted by the service
     */
    virtual void setCatalog( IFileCatalog* collCat ) = 0;

  protected:
    /// Empty destructor.
    virtual ~ICollectionService() {}
  };

}

#endif

