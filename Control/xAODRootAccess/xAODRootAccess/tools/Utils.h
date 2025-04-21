// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_UTILS_H
#define XAODROOTACCESS_TOOLS_UTILS_H

// Framework include(s).
#include "CxxUtils/sgkey_t.h"

// ROOT include(s):
#include <TDataType.h>
#include <TTree.h>

// System include(s):
#include <string>
#include <string_view>
#include <typeinfo>
#include <stdexcept>
extern "C" {
#include <stdint.h>
}

namespace xAOD {

   namespace Utils {

      /// Function creating a hash out of a "key name"
      SG::sgkey_t hash( const std::string& key );

      /// Get the dynamic auxiliary variable prefix based on a container name
      /// (for TTree)
      std::string dynBranchPrefix( const std::string& key );

      /// Get the dynamic auxiliary variable prefix based on a container name
      /// (for RNTuple)
      std::string dynFieldPrefix( const std::string& key );

      /// Get the type info of a primitive variable, as declared by ROOT
      const std::type_info& getTypeInfo( EDataType type );

      /// Get the type info of a primitive variable, in an RNTuple
      const std::type_info& getTypeInfo( std::string_view typeName );

      /// Check if the type name describes a primitive type
      bool isPrimitiveType( std::string_view typeName );

      /// Get the character describing a given primitive type for ROOT
      char rootType( char typeidType );

      /// Get the type name as it is known to ROOT, based on std::type_info
      std::string getTypeName( const std::type_info& ti );

      /// Search for branches, returns search term on no result
      std::string getFirstBranchMatch( TTree* tree, const std::string& pre );

   }  // namespace Utils

}  // namespace xAOD

#endif  // XAODROOTACCESS_TOOLS_UTILS_H
