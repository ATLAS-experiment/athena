/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INCLUDE_IMPLICITCOLLECTION_IMPLICITCOLLECTION_H
#define INCLUDE_IMPLICITCOLLECTION_IMPLICITCOLLECTION_H

#include "CollectionBase/ICollection.h"
#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/CollectionRowBuffer.h"
#include "POOLCore/DbPrint.h"

#include "CxxUtils/checker_macros.h"
#include "Gaudi/PluginService.h"

namespace pool {

  // forward declarations
   class ISession;
   class IContainer;
   class ICollectionQuery;
   class ICollectionIterator;
   class ImplicitCollectionIterator;

  /// An implicit collection implementation of the ICollection interface
  class ATLAS_NOT_THREAD_SAFE ImplicitCollection : virtual public ICollection, public APRMessaging
  //    ^ due to not thread-safe ImplicitCollectionIterator
  {
  public:
    typedef Gaudi::PluginService::Factory<ICollection*( const ICollectionDescription*, ICollection::OpenMode, ISession*)> Factory;  

    /** Constructor - old style
       Throws POOL exception.
       @param session the session object
       @param connection database connection string. It has the format databaseNameType:databaseName, where databaseNameType can be FID, PFN or LFN.
       @param name the container name in the database
       @param mode collection's open mode. For the moment only READONLY mode is allowed.
    */
    ImplicitCollection( ISession* session,
                        const std::string& connection,
                        const std::string& name,
                        ICollection::OpenMode mode );

    /// Constructor compying to the new Collections API
    /// parameters as above, but name and connection passed in description
    ImplicitCollection( const ICollectionDescription* description,
                        ICollection::OpenMode mode,
                        ISession* session );
    
    /// Destructor
    ~ImplicitCollection();

    ImplicitCollection (const ImplicitCollection&) = delete;
    ImplicitCollection& operator= (const ImplicitCollection&) = delete;

    /// Return openMode
    virtual ICollection::OpenMode openMode() const; 
    

    /// Adds a new row of data to the collection. Will always throw exception.
    virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer );

    /// Commits the last changes made to the collection. Will always return true.
    void commit(bool reopen=false);

    /// Aborts the last changes made to the collection.  Will always return true.
    void rollback();

    ///  no-op at the moment
    void close();

    ///  no-op at the moment
    void open();

    /// Checks if the collection is open.
    bool isOpen() const;

    /// Returns an object used to describe the collection properties.
    virtual const ICollectionDescription& description() const;

    /// Returns a cursor for the collection.
    virtual ICollectionCursor&         cursor() final override;

  protected:
    void open( ICollection::OpenMode mode, ISession* session );

  private:
    /// The underlying container handle
    IContainer*                 m_container;

    CollectionDescription       m_description;
  };
}

#endif
