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
        /// What to read for each row.
        enum class Content {
           TokenAndAttributes,  ///< the Token and all EventTag attributes
           TokenOnly            ///< the Token only; attributeList() stays empty
        };

        /// Construct from an already-connected file descriptor.
        explicit Collection( FileDescriptor& fileDescr, Content content = Content::TokenAndAttributes );

        Collection(const Collection &) = delete;
        Collection& operator = (const Collection &) = delete;

        /// Advances to the next row.
        bool next();

        /// Returns the number of rows without advancing the reader.
        std::size_t size();

        /// Positions the reader before the requested row; next() reads it.
        bool seek(std::size_t position);

        /// Token of the DataHeader of the current row.
        const Token& eventRef() const { return m_token; }
        /// Attribute values of the current row; empty with Content::TokenOnly.
        const coral::AttributeList& attributeList() const { return m_attributeList; }

     private:
        /// Attribute-name to container mapping for the EventTag attribute columns.
        std::map<std::string, DbContainer>  m_attributeContainers;
        /// Container providing the tokens: the EventTag Token column or the DataHeader container.
        DbContainer                         m_tokenContainer;
        /// DataHeader token of the current row.
        Token                               m_token;
        /// Attribute values of the current row.
        coral::AttributeList                m_attributeList;
        /// Index of the row that the next call to next() reads.
        std::size_t                         m_nextRow{0};
};

#endif