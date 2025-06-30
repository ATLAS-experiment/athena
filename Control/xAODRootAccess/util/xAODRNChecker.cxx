/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <cstring>
#include <memory>

// ROOT include(s):
// #include <TFile.h>
#include "../Root/ROOTTypes.h"
#include <TError.h>

#include "AsgMessaging/MessageCheck.h"
#include "CxxUtils/checker_macros.h"

// Local include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/REvent.h"
#include "xAODRootAccess/tools/RFileChecker.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

int main ATLAS_NOT_THREAD_SAFE ( int argc, char* argv[] ) {

   ANA_CHECK_SET_TYPE (int);
   using namespace asg::msgUserCode;

   // The application's name:
   const char* APP_NAME = argv[ 0 ];

   // Check that the application will be able to run:
   if( ( argc == 1 ) ||
       ( ( argc == 2 ) && ( ::strcmp( argv[ 1 ], "-h" ) == 0 ) ) ) {
      ::Info( APP_NAME, "Usage: %s <xAOD file1> [xAOD file2] ...", APP_NAME );
      return 0;
   }

   // Initialise the application's environment:
   ANA_CHECK( xAOD::Init() );

   // The object used in the checks:
   xAOD::Experimental::RFileChecker checker;
   checker.setStopOnError( kFALSE ); // Don't stop on errors, print them all

   // Loop over the files:
   for( int i = 1; i < argc; ++i ) {

      // The file name:
      const char* fname = argv[ i ];

      // // Open the file:
      ::Info( APP_NAME, "Opening file: %s", fname );

      // Later on the code should find all the top level event trees in the
      // input file. But for now let's just assume that only "CollectionTree" is
      // in the file.

      // Set up reading from the file:
      xAOD::Experimental::REvent event;
      ANA_CHECK( event.readFrom( fname ) );

      // Run the sanity checks:
      ANA_CHECK( checker.check( event ) );
   }

   // Return gracefully:
   return 0;
}
