/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#undef NDEBUG
#include <print>

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
      std::println( "{}", *obj );
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
   std::println( "Container contents at the start:\n" );
   printContainer( interface );

   // Clear the decorations, and see what happens:
   assert (interface.clearDecorations() == false);
   std::println( "\nContainer contents after clearDecorations():\n" );
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
   std::println( "\nContainer contents after decoration:\n" );
   printContainer( interface );

   // Clear the decorations, and see what happens:
   assert (interface.clearDecorations() == true);
   std::println( "\nContainer contents after clearDecorations():\n" );
   printContainer( interface );

   return 0;
}
