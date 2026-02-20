//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

///@file IterateUntilCondition.h
///
///This provides functionality to iterate until a condition is reached.
///The "right" way to do this would be using CUDA's cooperative kernel
///grid-wide synchronisation.
///However, following from https://doi.org/10.1145/2983990.2984032,
///one can achieve this with reasonable portability by employing
///an appropriate discovery and synchronisation strategy.


#ifndef CALORECGPU_ITERATEUNTILCONDITION_H

#define CALORECGPU_ITERATEUNTILCONDITION_H

#include <cooperative_groups.h>

#ifndef CALORECGPU_ITERATE_UNTIL_CONDITION_DEBUG

  #define CALORECGPU_ITERATE_UNTIL_CONDITION_DEBUG 0

#endif


#ifndef CALORECGPU_ITERATE_UNTIL_CONDITION_INCLUDE_ASSERTS

  #define CALORECGPU_ITERATE_UNTIL_CONDITION_INCLUDE_ASSERTS 0

#endif

namespace IterateUntilCondition
{
  
  template <class Condition, class Before, class After, class ... Funcs>
  struct Holder;

  template <class Condition, class Before, class After, class ... Funcs, class ... Args>
  __device__ void cooperative_kernel_impl(const Holder<Condition, Before, After, Funcs...> &, Args && ... args)
  {
    cooperative_groups::grid_group grid = cooperative_groups::this_grid();

    Condition checker;

    Before{}(gridDim.x, blockIdx.x, checker, std::forward<Args>(args)...);

    while (!checker(gridDim.x, blockIdx.x, std::forward<Args>(args)...))
      {
        auto helper = [&](auto func)
        {
          func(gridDim.x, blockIdx.x, checker, std::forward<Args>(args)...);
          grid.sync();
        };

        (helper(Funcs{}), ...);
      }
    After{}(gridDim.x, blockIdx.x, checker, std::forward<Args>(args)...);
  }
  
  template <class HolderLike, class ... Args>
  __global__ void cooperative_kernel(Args ... args)
  {
    cooperative_kernel_impl(HolderLike{}, args...);
  }

  struct BasicStorage
  {
    static constexpr unsigned int NumMaxBlocks = 1024;

    unsigned int mutex_check;
    unsigned int mutex_ticket;
    unsigned int count;
    unsigned int poll_closed;
    unsigned int wait_flags[NumMaxBlocks];
  };

  struct Storage : BasicStorage
  {
    unsigned int block_indices[NumMaxBlocks];
  };

  inline __device__ bool try_lock_mutex(Storage * store)
  {
    const unsigned int ticket = atomicAdd(&store->mutex_ticket, 1U);

    unsigned int last_check = 0;

    bool was_once_valid = false;

    volatile unsigned int * to_check = static_cast<volatile unsigned int *>(&store->mutex_check);
    
    do
      {
        last_check = *to_check;
        was_once_valid = !(last_check & 0x80000000U);
      }
    while (last_check < ticket && !(last_check & 0x80000000U));

    return was_once_valid;
  }

  inline __device__ void unlock_mutex(Storage * store)
  {
    volatile unsigned int * ptr = static_cast<volatile unsigned int *>(&store->mutex_check);
    *ptr = *ptr + 1;
  }

  inline __device__ void disable_mutex(Storage * store)
  {
    volatile unsigned int * ptr = static_cast<volatile unsigned int *>(&store->mutex_check);
    *ptr = *ptr | 0x80000000U;
  }

  inline __device__ bool check_if_participating(Storage * store)
  {
    const bool locked = try_lock_mutex(store);

    unsigned int old_count = Storage::NumMaxBlocks;

    volatile unsigned int * poll_closed_ptr = static_cast<volatile unsigned int *>(&store->poll_closed);

    if (*poll_closed_ptr == 0)
      {
        old_count = atomicAdd(&store->count, 1);
        store->block_indices[blockIdx.x] = old_count;
        unlock_mutex(store);
      }
    else
      {
        if (locked)
          {
            unlock_mutex(store);
          }
        return false;
      }

    if (*poll_closed_ptr == 0)
      {
        try_lock_mutex(store);
        *poll_closed_ptr = 1;
        disable_mutex(store);
        unlock_mutex(store);
      }

    return (old_count < Storage::NumMaxBlocks);
  }

  template <class Condition, class Before, class After, class ... Funcs, class ... Args>
  __device__ void normal_kernel_impl(const Holder<Condition, Before, After, Funcs...> &, Storage * store, Args && ... args)
  {
    __shared__ bool is_participating;

    const bool is_reference_thread = (threadIdx.x == 0 && threadIdx.y == 0 && threadIdx.z == 0);

    if (is_reference_thread)
      {
        is_participating = check_if_participating(store);
      }

    __syncthreads();

    const unsigned int this_block_index = store->block_indices[blockIdx.x];
    const unsigned int total_blocks = min(*static_cast<volatile unsigned int *>(&store->count), Storage::NumMaxBlocks);
        
    if (is_participating)
      {
        const bool is_reference_block = (this_block_index == 0);

        const unsigned int this_thread_index = threadIdx.z * blockDim.y * blockDim.x +
                                               threadIdx.y * blockDim.x +
                                               threadIdx.x;

        const unsigned int num_threads_per_block = blockDim.x * blockDim.y * blockDim.z;

        Condition checker;

        
        Before{}(total_blocks, this_block_index, checker, std::forward<Args>(args)...);

        while (!checker(total_blocks, this_block_index, std::forward<Args>(args)...))
          {
            auto helper = [&](auto func)
            {

              func(total_blocks, this_block_index, checker, std::forward<Args>(args)...);

              //Technically, for the foreseeable future,
              //this could be simply the if,
              //as the maximum number of concurrent blocks
              //in all devices is smaller than 1024...
              if (is_reference_block)
                {
                  for (unsigned int block_to_check = this_thread_index + 1; block_to_check < total_blocks; block_to_check += num_threads_per_block)
                    {
                      unsigned int last_check = 0;
                      
                      volatile unsigned int * to_check = static_cast<volatile unsigned int*>(&store->wait_flags[block_to_check]);

                      do
                        {
                          last_check = *to_check;
                        }
                      while (last_check == 0);
                      //When porting to non-CUDA, this may need to be some form of atomic load.
                    }

                  __syncthreads();

                  for (unsigned int block_to_check = this_thread_index + 1; block_to_check < total_blocks; block_to_check += num_threads_per_block)
                    {
                      __threadfence();
                      //*static_cast<volatile unsigned int*>(&store->wait_flags[block_to_check]) = 0;
                      atomicAnd(&(store->wait_flags[block_to_check]), 0U);
                    }

                  __syncthreads();
                }
              else
                {
                  __syncthreads();

                  if (is_reference_thread)
                    {                      
                      atomicOr(&(store->wait_flags[this_block_index]), 1U);

                      unsigned int last_check = 1;
                      
                      volatile unsigned int * to_check = static_cast<volatile unsigned int*>(&store->wait_flags[this_block_index]);

                      __threadfence();
                      
                      do
                        {
                          last_check = *to_check;
                        }
                      while (last_check != 0);
                      //When porting to non-CUDA, this may need to be some form of atomic load.
                    }

                  __syncthreads();
                }

            };

            (helper(Funcs{}), ...);
          }

        After{}(total_blocks, this_block_index, checker, std::forward<Args>(args)...);
      }
    
#if CALORECGPU_ITERATE_UNTIL_CONDITION_DEBUG
    if (is_reference_thread)
      {
        printf("%d | %d | %u %u \n", blockIdx.x, static_cast<int>(is_participating), total_blocks, this_block_index);
      }
#endif
  }
  
  template <class HolderLike, class ... Args>
  __global__ void normal_kernel(Storage * store, Args ... args)
  {
    normal_kernel_impl(HolderLike{}, store, args...);
  }

  /** @p Condition, @p Before, @p After and @p Funcs must all be functor classes.
   *  They will receive two unsigned ints for grid size and block index
   *  (for simplicity, we only handle 1D grids),
   *  a reference to a mutable @p Condition (except for @p Condition)
   *  and any arguments you pass to the `execute` of this return.
   *  The functions should not use the actual block indices,
   *  but the thread indices inside the block are respected.
   *  Condition must return a boolean, with true meaning we
   *  have reached the end of the iterations, all others are void.
   *  @p Condition may be (locally) stateful as the same local instance is used
   *  throughout the iterations, while the others must be stateless
   *  (being constructed every iteration in the case of @p Funcs).
   */
  template <class Condition, class Before, class After, class ... Funcs>
  struct Holder
  {
    template <class ... Args>
    static void execute(const bool use_native_sync,
                        const dim3 & grid_size,
                        const dim3 & block_size,
                        size_t shared_memory,
                        cudaStream_t stream,
                        Storage * gpu_ptr,
                        Args ... args)
    {
#if CALORECGPU_ITERATE_UNTIL_CONDITION_INCLUDE_ASSERTS
      assert(grid_size.x <= Storage::NumMaxBlocks);
      assert(grid_size.y == 1);
      assert(grid_size.z == 1);
#endif

      if (use_native_sync)
        {
          void * arg_ptrs[] = { static_cast<void *>(&args)... };

          cudaLaunchCooperativeKernel((void *) cooperative_kernel<Holder, Args...>,
                                      grid_size,
                                      block_size,
                                      arg_ptrs,
                                      shared_memory,
                                      stream);
        }
      else
        {
          cudaMemsetAsync(static_cast<BasicStorage *>(gpu_ptr), 0, sizeof(BasicStorage), stream);

          normal_kernel<Holder, Args...> <<< grid_size, block_size, shared_memory, stream>>>(gpu_ptr, args...);
        }
    }

  };

  /** Must pass functors!
   *  They will receive two unsigned ints for grid size and block index
   *  (for simplicity, we only handle 1D grids),
   *  a reference to a mutable @p Condition (except for @p Condition)
   *  and any arguments you pass to the `execute` of this return.
   *  The functions should not use the actual block indices,
   *  but the thread indices inside the block are respected.
   *  Condition must return a boolean, with true meaning we
   *  have reached the end of the iterations, all others are void.
   *  @p Condition may be (locally) stateful as the same local instance is used
   *  throughout the iterations, while the others must be stateless
   *  (being constructed every iteration in the case of @p Funcs).
   */
  template <class Condition, class Before, class After, class ... Funcs>
  auto make_holder(Condition c, Before b, After a, Funcs ... fs)
  {
    return Holder<Condition, Before, After, Funcs...> {};
  }
}

#endif
