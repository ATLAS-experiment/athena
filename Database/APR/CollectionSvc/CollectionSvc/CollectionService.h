/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONSERVICE_H
#define COLLECTIONSVC_COLLECTIONSERVICE_H

#include "ICollection.h"

#include "CxxUtils/checker_macros.h"


namespace pool {

  class CollectionDescription;
  class ISession;

  // this has to be a class so the methods are visible to PyROOT
  class CollectionService
  {
  public:
  /**
   * Creates or overwrites a collection, given a description of its properties.
   *
   * @param description Specification of collection properties.
   */
  static ICollection* create ATLAS_NOT_THREAD_SAFE ( const CollectionDescription& description );

  /**
   * Opens an existing collection for reading.
   * A Session object can be provided if a session is already open.
   *
   * @param name Name of the collection.
   * @param connection Connection string to the database containing the collection.
   * @param session Reference a to database session (optional).
   */
   static ICollection* open( const std::string & name,
                             const std::string & connection = "",
                             ISession* session = 0 );

   /**
    * suppress (or enable) warning about a missing MessageSvc (logging)
    */
   static void setMessageSvcQuiet( bool quiet=true );
   };
}

#endif
