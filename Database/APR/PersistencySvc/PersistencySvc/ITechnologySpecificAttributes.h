/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INCLUDE_PERSISTENCYSVC_ITECHNOLOGYSPECIFICATTRIBUTES_H
#define INCLUDE_PERSISTENCYSVC_ITECHNOLOGYSPECIFICATTRIBUTES_H

// includes
#include <exception>
#include <string>
#include <sstream>
#include <typeinfo>

namespace pool {

  /** @class ITechnologySpecificAttributes ITechnologySpecificAttributes.h PersistencySvc/ITechnologySpecificAttributes.h
   *
   *  ITechnologySpecificAttributes is the interface for an object holding technology-specific attributes
   *
   */

  class ITechnologySpecificAttributes {
  public:
    /// Templated method to retrieve an attribute
    template< class T > T attribute( const std::string& attributeName,
                                     const std::string& option = "" ) {
      T data;
      const std::type_info& typeInfo = typeid(T);
      if ( ! this->attributeOfType( attributeName,
                                    static_cast< void* >( &data ),
                                    typeInfo,
                                    option ) ) {
        std::ostringstream error;
        error << "Failed to retrieve attribute " << attributeName << " of type " << typeInfo.name();
        throw std::runtime_error( error.str() + " (APR: \" ITechnologySpecificAttributes \" from \" PersistencySvc \")");
      }
      return data;
    }

    /// Templated method to set an attribute
    template< class T > bool setAttribute( const std::string& attributeName,
                                           const T& atttibuteValue,
                                           const std::string& option = "" ) {
      return this->setAttributeOfType( attributeName,
                                       static_cast< const void* >( &atttibuteValue ),
                                       typeid(T),
                                       option );
    }

  protected:
    /// Default destructor
    virtual ~ITechnologySpecificAttributes() {}

    /// The actual method returning the attribute data given a name
    virtual bool attributeOfType( const std::string& attributeName,
                                  void* data,
                                  const std::type_info& typeInfo,
                                  const std::string& option ) = 0;

    /// The actual method setting the attribute data given a name
    virtual bool setAttributeOfType( const std::string& attributeName,
                                     const void* data,
                                     const std::type_info& typeInfo,
                                     const std::string& option ) = 0;
  };

}

#endif
