/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLLECTIONSVC_COLLECTIONCOLUMN_H
#define COLLECTIONSVC_COLLECTIONCOLUMN_H

#include "CoralBase/AttributeSpecification.h"

#include <typeinfo>


namespace pool {

  /** 
   * @class CollectionColumn CollectionColumn.h CollectionSvc/CollectionColumn.h
   *
   * An implementation for retrieving a description of a column of a collection.
   */
  class CollectionColumn
  {
  public:
    /// Default constructor.
    CollectionColumn()
      : m_name( "" ),
        m_type( "" ),
        m_id( 0 ) {}

    /**
     * Constructor that takes the column properties as input.
     * 
     * @param name Name of column.
     * @param type Data type of column.
     */
    CollectionColumn( const std::string& name,
                      const std::string& type)
      : m_name( name ),
        m_type( type ),
        m_id( 0 ) {}

    /// Copy constructor.
    CollectionColumn( const CollectionColumn& rhs )
	  : m_name( rhs.m_name ),
	    m_type( rhs.m_type ),
	    m_id( rhs.m_id ) 
    {}
     
    /// Default destructor.
    virtual ~CollectionColumn() {}

    /**
     * Sets the name of the column.
     *
     * @param name Name of column.
     */
    virtual void setName( const std::string& name ) { m_name = name; }

    /**
     * Sets the data type of the column.
     *
     * @param type Data type of column.
     */
    virtual void setType( const std::string& type ) { m_type = type; }

    /**
     * Sets the data type of the column.
     *
     * @param type Data type of column.
     */
    virtual void setType( const std::type_info& type )
    { m_type = coral::AttributeSpecification::typeNameForId( type ); }

    /// Sets the position of the column in its associated collection fragment.
    virtual void setId( int id ) { m_id = id; }
  
    /// Returns the name of the column.
    virtual const std::string& name() const { return m_name; }

    /// Returns the data type of the column.
    virtual const std::string& type() const { return m_type; }

    /// Returns the position of the column in its associated collection fragment.
    virtual int id() const { return m_id; }

  private:
    /// Name of column.
    std::string m_name;

    /// Data type of column.
    std::string m_type;

    /// Position of column in associated collection fragment.
    int m_id;
  };
}

#endif
