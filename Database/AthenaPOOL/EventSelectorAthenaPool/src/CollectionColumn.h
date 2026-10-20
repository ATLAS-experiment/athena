/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTSELECTORATHENAPOOL_COLLECTIONCOLUMN_H
#define EVENTSELECTORATHENAPOOL_COLLECTIONCOLUMN_H

#include <string>


namespace pool {

  /** 
   * @class CollectionColumn CollectionColumn.h
   *
   * An implementation for retrieving a description of a column of a collection.
   */
  class CollectionColumn
  {
  public:
    static const CollectionColumn& tokenColumn()
    {
      static const CollectionColumn column( "Token", "Token" );
      return column;
    }

    /**
     * Constructor that takes the column properties as input.
     * 
     * @param name Name of column.
     * @param type Data type of column.
     */
    CollectionColumn( const std::string& name,
                      const std::string& type)
      : m_name( name ),
        m_type( type ) {}
  
    /// Returns the name of the column.
    const std::string& name() const { return m_name; }

    /// Returns the data type of the column.
    const std::string& type() const { return m_type; }

  private:
    /// Name of column.
    std::string m_name;

    /// Data type of column.
    std::string m_type;
  };
}

#endif
