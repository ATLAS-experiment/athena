// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// $Id: Utils.h 601421 2014-06-12 11:44:08Z krasznaa $
#ifndef XAODROOTACCESS_TOOLS_UTILS_H
#define XAODROOTACCESS_TOOLS_UTILS_H

// System include(s):
#include <string>
#include <typeinfo>
#include <stdexcept>
extern "C" {
#include <stdint.h>
}

// ROOT include(s):
#include <TDataType.h>
#include <TTree.h>
// #include <xAODMetaData/FileMetaDataAuxInfo.h>
// #include "AthContainers/AuxElement.h"


#include <ROOT/RNTupleReader.hxx>

#include "CxxUtils/sgkey_t.h"

namespace xAOD {

   namespace Utils {
      using RNTupleReader = ROOT::Experimental::RNTupleReader;

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

      /// Get the character describing a given primitive type for ROOT
      char rootType( char typeidType );

      /// Get the type name as it is known to ROOT, based on std::type_info
      std::string getTypeName( const std::type_info& ti );

      /// Search for branches, returns search term on no result
      std::string getFirstBranchMatch( TTree* tree, const std::string& pre );

      /// Search for fields, returns search term on no result
      std::string getFirstFieldMatch(
          RNTupleReader& nupleReader,
          const std::string& pre );

      /// Checks if a given field exists in the ntuple
      ::Bool_t fieldExists(
          std::string fieldName,
          RNTupleReader& ntupleReader );

   }  // namespace Utils

}  // namespace xAOD

#endif  // XAODROOTACCESS_TOOLS_UTILS_H
