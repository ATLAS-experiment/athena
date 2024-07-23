// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDACORE_TASKARENA_H
#define ATHCUDACORE_TASKARENA_H

#include <functional>

namespace AthCUDA {

   /// Enqueue a function to the TBB task arena used for CUDA calls
   ///
   /// Since many (most?) CUDA calls use global locks, in order to avoid
   /// waiting times in the threads, one needs to schedule all CUDA operations
   /// using a single task arena, which takes care of executing all
   /// operations one by one, using the TBB threads allocated for the rest of
   /// the job.
   void enqueueTask( std::function<void()> f );

} // namespace AthCUDA


#endif // ATHCUDACORE_TASKARENA_H
