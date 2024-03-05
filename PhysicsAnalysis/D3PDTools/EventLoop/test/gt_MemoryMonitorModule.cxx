/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTesting/UnitTest.h>
#include <EventLoop/ModuleData.h>
#include <EventLoop/MemoryMonitorModule.h>

//
// unit test
//

namespace EL
{
  namespace Detail
  {
    // this tests that the memory monitor can detect memory allocation between
    // two calls.  by the nature of the module it doesn't matter which two calls
    // I pick (this is meant as a white-box test).
    TEST (MemoryMonitorModuleTest, simpleTest)
    {
      auto module = std::make_unique<MemoryMonitorModule> ("MemoryMonitorModule");
      ModuleData data;

      ASSERT_SUCCESS (module->firstInitialize (data));
      const auto before = module->lastRSS.value();

      constexpr std::size_t size = 64*1024;
      constexpr std::size_t numEntries = size*1024/sizeof (unsigned);

      // this is purposely using `volatile` to avoid the compiler optimizing
      // away the memory access (and thereby actual physical allocation), as it
      // may otherwise do when using strong enough compiler optimization.
      std::unique_ptr<volatile unsigned[]> memory (new volatile unsigned[numEntries]);
      for (std::size_t iter = 0; iter < numEntries; ++ iter)
        memory[iter] = 0;
      ASSERT_SUCCESS (module->onInitialize (data));

      // this gives a generous margin for memory usage, which accounts for the
      // actual low-level memory allocation not necessarily matching the memory
      // we requested.
      constexpr std::size_t margin = 4*1024;
      ASSERT_GT (module->lastRSS.value(), before+size-margin);
      ASSERT_LT (module->lastRSS.value(), before+size+margin);
    }
  }
}

ATLAS_GOOGLE_TEST_MAIN
