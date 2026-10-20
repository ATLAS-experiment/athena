/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTSELECTORATHENAPOOL_COLLECTION_H
#define EVENTSELECTORATHENAPOOL_COLLECTION_H

#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbPrint.h"
#include "StorageSvc/FileDescriptor.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pool {

   class CollectionColumn;
   class CollectionCursor;

   /**
      @brief Read-only collection implementation
      - Supports a single Token plus meta data attributes
   */
   class Collection : public APRMessaging {

     public:
        /// Construct from an already-connected file descriptor.
        explicit Collection( FileDescriptor& fileDescr );

        /// Defined out of line because CollectionColumn is incomplete here.
        ~Collection();

        /// Returns a cursor for the collection.
        std::unique_ptr<CollectionCursor> cursor();

     private:
        Collection(const Collection &) = delete;
        Collection& operator = (const Collection &) = delete;

        std::vector<CollectionColumn>        m_attributeColumns;

        /// Map of container names to container handles for all EventTag attributes in the file
        std::map<std::string, DbContainer>   m_containerMap;
        /// The DataHeader container, if present, in the file
        DbContainer                          m_dhCont;
   };
}
#endif