/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ut_xaodcore_printhelpers_test.cxx 646509 2015-02-12 16:43:18Z krasznaa $

// System include(s):
#include <cstdint>
#include <iostream>
#include <vector>

// EDM include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/Accessor.h"

// Local include(s):
#include "xAODCore/tools/PrintHelpers.h"

/// Dummy type
struct TypeA {
   int m_value;
};

int main() {

   // Construct a standalone, empty object:
   SG::AuxElement empty;
   std::cout << empty << std::endl;

   // Create an object with a private store, and fill it with some variables:
   SG::AuxElement standalone;
   standalone.makePrivateStore();
   SG::Accessor< int > IntValue( "IntValue" );
   IntValue( standalone ) = 12;
   SG::Accessor< float > FloatValue( "FloatValue" );
   FloatValue( standalone ) = 3.14;
   SG::Accessor< std::vector< int > > VecIntValue( "VecIntValue" );
   VecIntValue( standalone ) = std::vector< int >{ 1, 2, 3 };
   SG::Accessor< std::vector< float > > VecFloatValue( "VecFloatValue" );
   VecFloatValue( standalone ) = std::vector< float >{ 1.2, 2.3, 3.4 };
   SG::Accessor< TypeA > TypeAValue( "TypeAValue" );
   TypeAValue( standalone ) = TypeA{ 5 };
   SG::Accessor< std::vector< TypeA > > VecTypeAValue( "VecTypeAValue" );
   VecTypeAValue( standalone ) = std::vector< TypeA >{ TypeA{ 6 }, TypeA{ 7 } };
   std::cout << standalone << std::endl;

   // Create a vector, and decorate its first element:
   DataVector< SG::AuxElement > vec;
   SG::AuxStoreInternal store;
   vec.setStore( &store );
   SG::AuxElement* element = new SG::AuxElement();
   vec.push_back( element );
   SG::Accessor< long > LongValue( "LongValue" );
   LongValue( *element ) = 234;
   SG::Accessor< uint32_t > UInt32Value( "UInt32Value" );
   UInt32Value( *element ) = 0x123;
   std::cout << *element << std::endl;

   // Finally, do one test with the dump(...) function as well:
   xAOD::dump( standalone );

   // Return gracefully:
   return 0;
}
