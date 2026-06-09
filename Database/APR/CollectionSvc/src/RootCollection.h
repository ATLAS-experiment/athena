/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTCOLLECTION_ROOTCOLLECTION_H
#define ROOTCOLLECTION_ROOTCOLLECTION_H

#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionRowBuffer.h"
#include "StorageSvc/DbPrint.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbContainer.h"
#include "CollectionCursor.h"

#include <string>
#include <memory>


namespace pool {

   class ISession;
   class IStorageSvc;

   class Attribute;

   // Create and Overwrite is only option, we'll never update
   constexpr const char* const poolOptToRootOpt[] = {"UPDATE", "READ"};
   // Collection open mode mapping to StorageSvc open mode
   inline const Io::IoFlag collModeToPoolMode[]   = { Io::WRITE, Io::READ };


   /**
      @brief Collection implementation
      - Suppoerts a single Token plus meta data attributes
   */
   class RootCollection :  public ICollection, public APRMessaging {

     public:
        /// Constructor
        /// @param description The description of the collection, including name and connection
        /// @param mode The open mode of the collection
        /// @param session If you want to access the referenced objects you have to provide an ISession
        RootCollection( const pool::CollectionDescription* description,
                        pool::ICollection::OpenMode mode,
                        pool::ISession* session );

        /// Destructor
        ~RootCollection();


        /// Explicitly re-opens the collection after it has been closed.
        virtual void open() final override;

        /// Adds a new row of data to the collection.
        virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) final override;

        /// Commits the last changes made to the collection
        virtual void commit( bool restartTransaction = false ) final override;

        /// Explicitly closes the collection
        virtual void close() final override;

        /// Returns an object used to describe the collection properties.
        virtual const CollectionDescription& description() const final override;

        /// Returns a cursor for the collection.
        virtual std::unique_ptr<ICollectionCursor> cursor() final override;

     private:

        /// copying unimplemented in this class.
        RootCollection(const RootCollection &) = delete;
        RootCollection& operator = (const RootCollection &) = delete;

        void writeColumn( const std::string& columnName, const void* data,  const std::type_info& typeInfo);


        CollectionDescription                m_description;

        std::string                          m_name;
        std::string                          m_fileName;
        /// The common prefix for branch container names for attributes
        std::string                          m_containerPrefix;
        ICollection::OpenMode                m_mode;

        ISession*                            m_session;
        bool                                 m_open;

        std::unique_ptr<IStorageSvc>         m_storageSvc;
        pool::FileDescriptor                 m_fileDescr;
        ContainerMap                         m_containerMap;
   };
}
#endif
