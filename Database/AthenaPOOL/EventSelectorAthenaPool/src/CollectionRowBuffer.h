/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTSELECTORATHENAPOOL_COLLECTIONROWBUFFER_H
#define EVENTSELECTORATHENAPOOL_COLLECTIONROWBUFFER_H

#include "CoralBase/AttributeList.h"
#include "CxxUtils/checker_macros.h"

#include <string>

class Token;

namespace pool {

  /** 
   * @class CollectionRowBuffer CollectionRowBuffer.h
   *
   * A class representing a row of a collection. It contains a list of named attributes
   * and a single POOL object reference. It is used when creatoing a new collection or when
   * iterating over an existing one.
   */
  class CollectionRowBuffer
  {
  public:
    /// Default Constructor.
    CollectionRowBuffer();

    /**
     * Copy Constructor.
     *
     * @param rhs Object to be copied.
     */
    CollectionRowBuffer( const CollectionRowBuffer& rhs );

    /// Default destructor.
    ~CollectionRowBuffer();

    CollectionRowBuffer& operator=( const CollectionRowBuffer& rhs ) = delete;

    /**
     * Sets the Attribute list schema.
     *
     * @param attributeList List of Attributes.
     */
    void setAttributeList( const coral::AttributeList& attributeList );

    /// Returns an object reference.
    Token& token();
    const Token& token() const;
    const std::string& tokenName() const;

    /// Returns a reference to the list of Attributes.
    coral::AttributeList& attributeList();

    /// Returns a constant reference to the list of Attributes.
    const coral::AttributeList& attributeList() const;

  private:
    Token*   				      m_token;

    /// List of Attributes.
    // Changed to a pointed to be able to avoid thread-safety checker
    // warnings about AttributeList.  We can change back to holding
    // this by value once those warnings are removed.
    coral::AttributeList*	m_attributeList;

    bool deleteAL ATLAS_NOT_THREAD_SAFE ();
  };
}

#endif

