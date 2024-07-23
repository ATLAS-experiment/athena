//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//

#include "CxxUtils/checker_macros.h"

// Local include(s).
#include "AthCUDACore/TaskArena.h"

#include <tbb/task_arena.h>

namespace AthCUDA {

   tbb::task_arena& taskArena() {

      static tbb::task_arena arena ATLAS_THREAD_SAFE ( 1, 0 );
      return arena;
   }

  void enqueueTask( std::function<void()> f ) {

      taskArena().enqueue( f );
   }

} // namespace AthCUDA
