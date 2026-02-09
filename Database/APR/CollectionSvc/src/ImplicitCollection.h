/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_IMPLICITCOLLECTION_H
#define COLLECTIONSVC_IMPLICITCOLLECTION_H

#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionRowBuffer.h"
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
    typedef Gaudi::PluginService::Factory<ICollection*( const CollectionDescription*, ICollection::OpenMode, ISession*)> Factory;  

    /// Constructor compying to the new Collections API
    /// parameters as above, but name and connection passed in description
    ImplicitCollection( const CollectionDescription* description,
                        ICollection::OpenMode mode,
                        ISession* session );
    
    /// Destructor
    ~ImplicitCollection();

    ImplicitCollection (const ImplicitCollection&) = delete;
    ImplicitCollection& operator= (const ImplicitCollection&) = delete;

    /// Adds a new row of data to the collection. Will always throw exception.
    virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) override;

    /// Commits the last changes made to the collection. Will always return true.
    void commit(bool reopen=false) override;

    ///  no-op at the moment
    void close() override;

    ///  no-op at the moment
    void open() override;

    /// Returns an object used to describe the collection properties.
    virtual const CollectionDescription& description() const override;

    /// Returns a cursor for the collection.
    virtual ICollectionCursor& cursor() final override;

  protected:
    void open( ICollection::OpenMode mode, ISession* session );

  private:
    /// The underlying container handle
    IContainer*                 m_container;

    CollectionDescription       m_description;
  };
}

#endif
