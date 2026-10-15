/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <print>
#include <sstream>

// Local include(s):
#include "xAODEventInfo/EventInfo.h"

int main() {

   // Create a test object:
   xAOD::EventInfo ei;
   ei.makePrivateStore();

   // Set some of its basic properties:
   ei.setRunNumber( 1234 );
   ei.setEventNumber( 2345 );
   ei.setLumiBlock( 34 );
   ei.setTimeStamp( 456789 );
   ei.setTimeStampNSOffset( 567 );
   ei.setBCID( 678 );
   ei.setDetectorMask( 0x12345678, 0x12345678 );

   // Try printing it:
   std::println( "{}", ei );

   std::ostringstream s1;
   s1 << ei;
   std::ostringstream s2;
   std::print( s2, "{}", ei );
   assert( s1.str() == s2.str() );

   // Return gracefully:
   return 0;
}
