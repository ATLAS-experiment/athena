/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_IMPLICITCOLLECTION_H
#define COLLECTIONSVC_IMPLICITCOLLECTION_H

#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionRowBuffer.h"
#include "StorageSvc/DbPrint.h"

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
    virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) override final;

    /// Base interface methods that do nothing. Open/Close happens in xtor and dtor, and commit is a no-op.
    virtual void commit(bool /*reopen*/=false) override final { };
    virtual void close() override final { };
    virtual void open() override final { };

    /// Returns an object used to describe the collection properties.
    virtual const CollectionDescription& description() const override final;

    /// Returns a cursor for the collection.
    virtual std::unique_ptr<ICollectionCursor> cursor() override final;

  protected:
    void open( ICollection::OpenMode mode, ISession* session );

  private:
    /// The underlying container handle
    IContainer*                 m_container;

    CollectionDescription       m_description;
  };
}

#endif
