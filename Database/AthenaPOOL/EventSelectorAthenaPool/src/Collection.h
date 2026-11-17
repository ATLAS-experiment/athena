/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTSELECTORATHENAPOOL_COLLECTION_H
#define EVENTSELECTORATHENAPOOL_COLLECTION_H

#include "CoralBase/AttributeList.h"
#include "PersistentDataModel/Token.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbPrint.h"
#include "StorageSvc/FileDescriptor.h"

#include <cstddef>
#include <map>
#include <string>

/**
   @brief Read-only collection implementation
   - Supports a single Token plus meta data attributes
*/
class Collection : public APRMessaging {

     public:
        /// Construct from an already-connected file descriptor.
        explicit Collection( FileDescriptor& fileDescr );

        /// Advances to the next row.
        bool next();

        /// Returns the number of rows without advancing the reader.
        std::size_t size();

        /// Positions the reader before the requested row; next() reads it.
        bool seek(std::size_t position);

        const Token& eventRef() const { return m_token; }
        const coral::AttributeList& attributeList() const { return m_attributeList; }

     private:
        Collection(const Collection &) = delete;
        Collection& operator = (const Collection &) = delete;

        /// Attribute-name to container mapping for EventTag attributes.
        std::map<std::string, DbContainer>  m_containerMap;
        DbContainer                         m_tokenContainer;
        Token                               m_token;
        coral::AttributeList                m_attributeList;
        std::size_t                         m_idx;
};

#endif