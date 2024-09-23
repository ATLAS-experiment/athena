/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JiveXML/DataType.h"

namespace JiveXML{

   /**
    * Empty/Default constructor
    */
   DataType::DataType() {
   }

   /**
    * Return the string
    */
   const std::string& DataType::toString() const {
     return m_thing;
   }
  
  /**
   * Allow the DataType to be streamed in an ostream
   * @param os the stream
   * @param dt the DataType object
   */
  std::ostream& operator<<( std::ostream& os, const DataType &dt ){
    return os << dt.toString();
  }

} //namespace



