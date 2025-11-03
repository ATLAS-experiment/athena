/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file guidFallback_test
 * @brief Unit test for Guid fallback support
 */

#undef NDEBUG
#include "PersistentDataModel/Guid.h"
#include <cassert>
//coverity[root_function]
int main() {
   std::string badstring ("A17F6DA3-2C0A-4F06-82D6-8302F63B786");
   std::string goodstring("A17F6DA3-2C0A-4F06-82D6-8302F63B7806");
   //Bad strings are rejected and will not work with main method
   assert(Guid(badstring) != Guid(goodstring));

   Guid badfallback(badstring, Guid::FallBack{});
   Guid goodmain(goodstring);
   Guid goodfallback(goodstring, Guid::FallBack{});

   assert(goodmain == badfallback);
   assert(goodfallback == badfallback);
   assert(badfallback == badfallback);
   assert(badfallback.toString() == goodfallback.toString());
   assert(badfallback.toString() == goodmain.toString());
   return 0;
}
