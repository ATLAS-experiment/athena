/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ut_xaodcore_clearDecorations_test.cxx 696772 2015-09-25 08:09:13Z krasznaa $

// System include(s):
#undef NDEBUG
#include <iostream>

// Core include(s):
#include "AthContainers/DataVector.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/Accessor.h"

// Local include(s):
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/tools/PrintHelpers.h"

/// Convenience function to print the payload of the test container
template< class CONTAINER >
void printContainer( const CONTAINER& c ) {

   for( const auto* obj : c ) {
      std::cout << *obj << std::endl;
   }

   return;
}

int main() {

   // Create the container to be tested:
   DataVector< SG::AuxElement > interface;
   xAOD::AuxContainerBase aux;
   interface.setStore( &aux );

   // Add some simple objects to it:
   SG::Accessor<int> IntVar1 ("IntVar1");
   SG::Accessor<float> FloatVar1 ("FloatVar1");
   for( int i = 0; i < 3; ++i ) {
      SG::AuxElement* obj = new SG::AuxElement();
      interface.push_back( obj );
      IntVar1( *obj ) = i;
      FloatVar1( *obj ) = i;
   }

   // Print what it looks like now:
   std::cout << "Container contents at the start:\n" << std::endl;
   printContainer( interface );

   // Clear the decorations, and see what happens:
   assert (interface.clearDecorations() == false);
   std::cout << "\nContainer contents after clearDecorations():\n" << std::endl;
   printContainer( interface );

   // Lock the container:
   interface.lock();

   // Create some decorations:
   SG::Decorator<int> IntVar2 ("IntVar2");
   SG::Decorator<float> FloatVar2 ("FloatVar2");
   for( const SG::AuxElement* obj : interface ) {

      IntVar2( *obj ) = 2;
      FloatVar2( *obj ) = 3.141592;
   }

   // Print what it looks like now:
   std::cout << "\nContainer contents after decoration:\n" << std::endl;
   printContainer( interface );

   // Clear the decorations, and see what happens:
   assert (interface.clearDecorations() == true);
   std::cout << "\nContainer contents after clearDecorations():\n" << std::endl;
   printContainer( interface );

   return 0;
}
