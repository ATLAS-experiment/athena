/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//
// Simple unit testing for the standalone CLASS_DEF/ClassID_traits code.
//

// System include(s):
#include <print>

// Local include(s):
#include "xAODCore/CLASS_DEF.h"

class TestClassA {
public:
   TestClassA() {}
};
class TestClassB{};

namespace xAOD {
   class TestClassC {};
}

CLASS_DEF( TestClassA, 887887, 1 )
CLASS_DEF( xAOD::TestClassC, 11223344, 1 )

int main() {

   if( ClassID_traits< TestClassA >::ID() != 887887 ) {
      std::println( "Wrong class ID for TestClassA" );
      return -1;
   }
   std::println( "Class ID worked" );

   if( ClassID_traits< TestClassA >::typeName() != "TestClassA" ) {
      std::println( "Wrong type name for TestClassA" );
      return -1;
   }
   std::println( "Class typeName worked" );

   if( ClassID_traits< xAOD::TestClassC >::typeName() != "xAOD::TestClassC" ) {
      std::println( "Wrong type name for xAOD::TestClassC" );
      return -1;
   }
   std::println( "Namespaces class typeName worked" );

   //ClassID_traits<TestClassB>::ID(); // this should fail to compile with readable message

   std::println( "Test ok" );
   return 0;
}
