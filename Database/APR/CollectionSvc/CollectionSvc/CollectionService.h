/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONSERVICE_H
#define COLLECTIONSVC_COLLECTIONSERVICE_H

#include "ICollection.h"
#include "CxxUtils/checker_macros.h"


namespace pool {

  class ICollectionDescription;
  class ISession;

  /**
   * @class CollectionService CollectionService.h CollectionSvc/CollectionService.h
   *
   * A service for creating, accessing and managing an ensemble of collections of event 
   * references. In many cases, the individual unit managed 
   * by the service may simply consist of a collection fragment containing only a subset 
   * of the metadata of an existing collection.
   */
  class CollectionService
  {
 
  public:
    /**
     * Constructor: 
     *
     * @param context Local context provided by the service.
     */
    CollectionService() = default;


    /// Default destructor.
    virtual ~CollectionService() = default;

    /**
     * Creates or overwrites a collection or collection fragment, given a description of 
     * its properties.
     *
     * @param description Specification of collection or collection fragment properties.
     * @param overwrite Flag to distinguish creation and overwrite open modes.
     */
    virtual ICollection* create( const ICollectionDescription& description,
                                 bool overwrite = false );

    virtual ICollection* open( const std::string & name,
                               const std::string & type,
                               const std::string & connection = "",
                               bool readOnly = true) const
    {
       return handle(name, type, connection, readOnly, 0);
    }


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
    virtual ICollection* handle( const std::string & name,
                                 const std::string & type,
                                 const std::string & connection = "",
                                 bool readOnly = true,
                                 ISession* session = 0 ) const;

    /**
     * suppress (or enable) warning about a missing MessageSvc (logging)
     */
    static void setMessageSvcQuiet( bool quiet=true );

    pool::ICollection* plugin( const ICollectionDescription& description,
                                 ICollection::OpenMode openMode,
                                 ISession* session = 0 ) const;

  };
}

#endif
