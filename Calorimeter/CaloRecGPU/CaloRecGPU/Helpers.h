//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_HELPERS_H
#define CALORECGPU_HELPERS_H

#include <utility>
#include <type_traits>
#include <cstring>
//For memcpy, of all things...
#include <string>
#include <cstdio>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <vector>
#include <climits>
#include <new>
#include <cmath>

#if __cpp_lib_math_constants
  #include <numbers>
#endif
//This is the best way to have pi,
//but we provide a more manual alternative.
//Of course, there's also M_PI,
//but we wanted to ensure the type matched
//to prevent any GPU-based casting shenanigans.

#if __cpp_lib_bitops
  #include <bit>
#endif

namespace CaloRecGPU
{

#ifndef CUDA_AVAILABLE

  #ifdef __CUDA_ARCH__
    #define CUDA_AVAILABLE 1
  #elif __CUDA__
    #define CUDA_AVAILABLE 1
  #elif __CUDACC__
    #define CUDA_AVAILABLE 1
  #else
    #define CUDA_AVAILABLE 0
  #endif

#endif

#if CUDA_AVAILABLE

#define CUDA_HOS_DEV __host__ __device__


  /*!
    \brief Provides a way to report errors in \c CUDA kernels.

    \remark Standard CUDA definition that can be found almost everywhere...
  */
  CUDA_HOS_DEV inline void CUDA_gpu_assert(cudaError_t code, const char * file, int line, bool abort = true)
  {
    if (code != cudaSuccess)
      {
        printf("CUDA error: %s (%s %d)\n", cudaGetErrorString(code), file, line);
        if (abort)
          {
#ifdef __CUDA_ARCH__
            asm("trap;");
#else
            exit(code);
#endif
          }
      }
  }

  /*!
    \brief Wraps up a `CUDA_gpu_assert` using `__FILE__` and `__LINE__`
    to improve error reporting (and debugging) capabilities.
  */
#define CUDA_ERRCHECK(...) CUDA_ERRCHECK_HELPER(__VA_ARGS__, true)

#define CUDA_ERRCHECK_HELPER(ans, ...) do { ::CaloRecGPU::CUDA_gpu_assert((ans), __FILE__, __LINE__, CUDA_ERRCHECK_GET_FIRST(__VA_ARGS__, true) ); } while(0)
#define CUDA_ERRCHECK_GET_FIRST(x, ...) x


#if defined(__CUDA_ARCH__) &&  __CUDA_ARCH__ > 350
  #if CUDART_VERSION >= 12000
    #define CUDA_CAN_USE_TAIL_LAUNCH 1
  #else
    #define CUDA_CAN_USE_TAIL_LAUNCH 0
  #endif
#elif defined(__CUDA_ARCH__)
  #error "CUDA compute capability at least 3.5 is needed so we can have dynamic parallelism!"
#endif


#else

#define CUDA_HOS_DEV
#define CUDA_ERRCHECK(...)

#endif

  namespace CUDA_Helpers
  {

    struct CUDAStreamPtrHolder
    {
      void * ptr = nullptr;

      template <class T = const void>
      constexpr operator T * () const
      {
        return (T *) ptr;
      }

      constexpr operator bool() const
      {
        return ptr != nullptr;
      }

      template <class T>
      CUDAStreamPtrHolder (T * p) : ptr(p)
      {
      }

      CUDAStreamPtrHolder() = default;
    };
    //Can't do much more than this
    //since cudaStream_t is a typedef...
    //Though not typesafe, it is still
    //semantically more safe than a naked void *...

    /*!
      \brief Allocates and returns the address of \p num bytes from GPU memory.
    */
    void * allocate(const size_t num);

    /*!
      \brief Deallocates \p address in GPU memory.
    */
    void deallocate(void * address);

    /*!
      \brief Allocates and returns the address of \p num bytes from CPU pinned memory.
    */
    void * allocate_pinned(const size_t num);

    /*!
      \brief Deallocates \p address in CPU pinned memory.
    */
    void deallocate_pinned(void * address);


    /*!
      \brief Copies \p num bytse from \p source in GPU memory to \p dest in CPU memory.
    */
    void GPU_to_CPU(void * dest, const void * const source, const size_t num);

    /*!
      \brief Copies \p num bytes from \p source in CPU memory to \p dest in GPU memory.
    */
    void CPU_to_GPU(void * dest, const void * const source, const size_t num);

    /*!
      \brief Copies \p num bytes from \p source to \p dest, both in GPU memory.
    */
    void GPU_to_GPU(void * dest, const void * const source, const size_t num);


    /*!
      \brief Copies \p num bytes from \p source in GPU memory to \p dest in CPU memory, asynchronously.
    */
    void GPU_to_CPU_async(void * dest, const void * const source, const size_t num, CUDAStreamPtrHolder stream = {});

    /*!
      \brief Copies \p num bytes from \p source in CPU memory to \p dest in GPU memory, asynchronously.
    */
    void CPU_to_GPU_async(void * dest, const void * const source, const size_t num, CUDAStreamPtrHolder stream = {});

    /*!
      \brief Copies \p num bytes from \p source to \p dest, both in GPU memory, asynchronously.
    */
    void GPU_to_GPU_async(void * dest, const void * const source, const size_t num, CUDAStreamPtrHolder stream = {});

    /*!
      \brief Synchronizes the \p stream. If called with no value, synchronizes with @c cudaStreamPerThread.
    */
    void GPU_synchronize(CUDAStreamPtrHolder stream = {});

    /*!
      \brief Optimizes block and grid size according to @c cudaOccupancyMaxPotentialBlockSize.
    */
    void optimize_block_and_grid_size(void * func, int & block_size, int & grid_size, const int dynamic_memory = 0, const int block_size_limit = 0);

    /*!
      \brief Optimizes block and grid size for a cooperative launch.
    */
    void optimize_block_and_grid_size_for_cooperative_launch(void * func, int & block_size, int & grid_size, const int dynamic_memory = 0, const int block_size_limit = 0, const bool multiple_blocks_per_SM = true);

    bool supports_cooperative_launches();

    bool supports_dynamic_parallelism();

    std::string GPU_name();
  }

  namespace Helpers
  {
    
    /// \brief Returns the ceiling of num/denom, with proper rounding.
    template <class T1, class T2>
    inline constexpr auto int_ceil_div(const T1 num, const T2 denom)
    {
      return num / denom + (num % denom != 0);
    }

    /// \brief Returns the floor of num/denom, with proper rounding.
    template <class T1, class T2>
    inline constexpr auto int_floor_div(const T1 num, const T2 denom)
    {
      return num / denom;
    }

    /// \brief Returns 2 to the power of \p exp.
    template <class Base = float, class Exp = int>
    inline constexpr Base compile_time_pow2(const Exp exp)
    {
      Base ret = 1;
      if (exp < 0)
        {
          for (Exp i = 0; i < -exp; ++i)
            {
              ret /= Base(2);
            }
        }
      else
        {
          for (Exp i = 0; i < exp; ++i)
            {
              ret *= Base(2);
            }
        }
      return ret;
    }
    //Though we could possibly bit-hack stuff due to IEEE-754 reliance elsewhere,
    //it's not valid and type-safe C++...
    //Since it's compile-time, this being a trifle slower is meaningless.
    
    /// \brief Returns the ceiling of the base-2 logarithm of a number
    /// (i. e. the minimum number of bits to represent this number).
    template <class T>
    inline constexpr unsigned int int_ceil_log_2(T num)
    {
#if __cpp_lib_bitops
      return sizeof(T) * CHAR_BIT - std::countl_zero(num);
#else
      unsigned int ret = 64;
    
      for (unsigned long long int mask = 0x8000000000000000U; mask > 0; mask >>= 1U)
        {
          if (num & mask)
            {
              return ret;
            }
          --ret;
        }

      return ret;
#endif
    }

    /*! Calculates a Pearson hash from @ number.
    */
    template <class T>
    inline constexpr unsigned char Pearson_hash(const T number)
    {
      constexpr unsigned char initial_value = 42;
      //The answer.

      constexpr unsigned char c_mult = 7;
      constexpr unsigned char c_add = 1;
      //For our "look up table": table[i] = c_mult * i + c_add
      //For an appropriate choice of constants (such as this),
      //this will be bijective (modulo 255), as required.

      unsigned char ret = initial_value;

      for (unsigned int i = 0; i < sizeof(T); i += sizeof(unsigned char))
        {
          const unsigned char to_hash = number >> (i * CHAR_BIT);
          const unsigned char operand = ret ^ to_hash;
          ret = c_mult * operand + c_add;
        }

      return ret;
    }


    /*! Calculates a 16-bit Pearson hash from @ number.
    */
    template <class T>
    inline constexpr unsigned short Pearson_hash_16_bit(const T number)
    {
      constexpr unsigned short initial_value = 42754;
      //The answer and the standard.

      constexpr unsigned short c_mult = 7;
      constexpr unsigned short c_add = 1;
      //For our "look up table": table[i] = c_mult * i + c_add
      //For an appropriate choice of constants (such as this),
      //this will be bijective (modulo 255), as required.

      unsigned short ret = initial_value;

      for (unsigned int i = 0; i < sizeof(T); i += sizeof(unsigned short))
        {
          const unsigned short to_hash = number >> (i * CHAR_BIT);
          const unsigned short operand = ret ^ to_hash;
          ret = c_mult * operand + c_add;
        }

      return ret;
    }


    ///! \brief Just a wrapper around constants with fallback.
    namespace Constants
    {
#ifdef __cpp_lib_math_constants
  template <class T>
  inline constexpr T pi = std::numbers::pi_v<T>;

  template <class T>
  inline constexpr T sqrt2 = std::numbers::sqrt2_v<T>;
#else
  template <class T>
  inline constexpr T pi = T(3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624L);

  template <class T>
  inline constexpr T sqrt2 = T(1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727350138462309122970249248360558507372126441214970999358314132226659275055927557999505011527820605714701095599716059702745345968620147285174186408891986095523292304843087143214508397626036279952514079896872533965463318088296406206152583523950547457502877599617298355752203375318570113543746034084988471603868999706990048150305440277903164542478230684929369186215805784631115966687130130156185689872372352885092649L);
#endif

      template <class T>
      inline constexpr T inv_sqrt2 = T(0.70710678118654752440084436210484903928483593768847403658833986899536623923105351942519376716382078636750692311545614851246241802792536860632206074854996791570661133296375279637789997525057639103028573505477998580298513726729843100736425870932044459930477616461524215435716072541988130181399762570399484362669827316590441482031030762917619752737287514387998086491778761016876592850567718730170424942358019344998534950240751527201389515822712391153424646845931079028923155579833435650650780928449361861764425463243L);
      //Why is this not in the C++ constants?!

    }

    CUDA_HOS_DEV static inline
    float erf_inv_wrapper (const float x)
    {
      using namespace std;
#ifdef __CUDA_ARCH__
      return erfinvf(x);
#else
      //Copied directly from ROOT...

      int kMaxit    = 50;
      float kEps   = 1e-14f;
      float kConst = 0.8862269254527579f;     // sqrt(pi)/2.0

      if (abs(x) <= kEps)
        {
          return kConst * x;
        }

      // Newton iterations
      float erfi, derfi, y0, y1, dy0, dy1;
      if (fabsf(x) < 1.0f)
        {
          erfi  = kConst * fabsf(x);
          y0    = erff(0.9f * erfi);
          derfi = 0.1f * erfi;
          for (int iter = 0; iter < kMaxit; iter++)
            {
              y1  = 1.f - erfc(erfi);
              dy1 = fabsf(x) - y1;
              if (fabsf(dy1) < kEps)
                {
                  if (x < 0)
                    {
                      return -erfi;
                    }
                  else
                    {
                      return erfi;
                    }
                }
              dy0    = y1 - y0;
              derfi *= dy1 / dy0;
              y0     = y1;
              erfi  += derfi;
              if (fabsf(derfi / erfi) < kEps)
                {
                  if (x < 0.f)
                    {
                      return -erfi;
                    }
                  else
                    {
                      return erfi;
                    }
                }
            }
        }
      return 0; //did not converge
#endif
    }

    //Food for thought: any sort of proper argument reduction here?
    //(E. g. Cody-Waite or Payne-Hanek algorithm?)

    CUDA_HOS_DEV static inline
    float regularize_angle(const float b, const float a = 0.f)
    //a. k. a. proxim in Athena code.
    {
      using namespace std;
      constexpr float pi = Helpers::Constants::pi<float>;
      constexpr float two_pi = 2 * pi;
      const float ret = remainderf(b, two_pi);
      return ret + ((ret < a - pi) - (ret > a + pi)) * two_pi;
    }

    CUDA_HOS_DEV static inline
    double regularize_angle(const double b, const double a = 0.)
    //a. k. a. proxim in Athena code.
    {
      using namespace std;
      constexpr double pi = Helpers::Constants::pi<double>;
      constexpr double two_pi = 2 * pi;
      const double ret = remainderf(b, two_pi);
      return ret + ((ret < a - pi) - (ret > a + pi)) * two_pi;
    }

    template <class T>
    CUDA_HOS_DEV static inline
    T angular_difference(const T x, const T y)
    {
      return regularize_angle(regularize_angle(x) - regularize_angle(y));
    }

    CUDA_HOS_DEV static inline
    float eta_from_coordinates(const float x, const float y, const float z)
    {
      using namespace std;
      
      if (x != 0 || y != 0)
        {
#ifdef __CUDA_ARCH__
          const float m = norm3df(x, y, z);
#else
          const float m = hypot(x, y, z);
#endif
          return 0.5f * logf((m + z) / (m - z));
        }
      else
        {
          constexpr float s_etaMax = 22756.0f;
          return z + ((z > 0) - (z < 0)) * s_etaMax;
        }
    }

    CUDA_HOS_DEV static inline
    double eta_from_coordinates(const double x, const double y, const double z)
    {
      using namespace std;
      if (x != 0 || y != 0)
        {
#ifdef __CUDA_ARCH__
          const float m = norm3d(x, y, z);
#else
          const float m = hypot(x, y, z);
#endif
          return 0.5 * log((m + z) / (m - z));
        }
      else
        {
          constexpr double s_etaMax = 22756.0;
          return z + ((z > 0) - (z < 0)) * s_etaMax;
        }
    }

    ///Implements one step of a Kahan-Babushka-Neumaier sum
    ///by adding @p to_add to @p sum with the correction term @p corr.
    CUDA_HOS_DEV static inline
    void partial_kahan_babushka_neumaier_sum(const float & to_add, float & sum, float & corr)
    {
      const float t = sum + to_add;

      const bool test = fabsf(sum) >= fabsf(to_add);

      const float opt_1 = (sum - t) + to_add;
      const float opt_2 = (to_add - t) + sum;

      corr += (test) * opt_1 + (!test) * opt_2;

      sum = t;
    }

    ///Adds a list of floats using the Kahan-Babushka-Neumaier algorithm
    ///for greater precision (at the cost of additional operations).
    template < class ... Floats, class disabler = std::enable_if_t < (std::is_same_v<std::decay_t<Floats>, float> && ...) > >
    CUDA_HOS_DEV inline
    float sum_kahan_babushka_neumaier(const Floats & ... fs)
    {
      float ret = 0.f;
      float corr = 0.f;

      (partial_kahan_babushka_neumaier_sum(fs, ret, corr), ...);

      return ret + corr;
    }
    
    
#if CUDA_AVAILABLE
    ///Adds the contribution of a number @p v to the
    ///Kahan-Babushka-Neumaier summation that uses
    ///the pointed-to @p sum_arr and @p corr_arr
    ///as the accumulator and correction term, respectively.
    ///Can optionally treat these two pointers as pointers to an array
    ///and @p as the index into that array, for convenience.
    __device__ static inline
    void device_kahan_babushka_neumaier(float * sum_arr, float * corr_arr, const float v, const int idx = 0)
    {
      const float old_sum = atomicAdd(sum_arr + idx, v);
      const float new_sum = old_sum + v;
      if (fabsf(old_sum) >= fabsf(v))
        {
          atomicAdd(corr_arr + idx, (old_sum - new_sum) + v);
        }
      else
        {
          atomicAdd(corr_arr + idx, (v - new_sum) + old_sum);
        }
    }
#endif

    //Algorithm that calculates a * b + c * d with better precision using FMA,
    //following "Error bounds on complex floating-point multiplication with an FMA"
    //by Jeannerod et. al.
    CUDA_HOS_DEV static inline
    float product_sum_cornea_harrison_tang(const float a, const float b, const float c, const float d)
    {
      using namespace std;

      const float w_1 = a * b;
      const float w_2 = c * d;

      const float e_1 = fmaf(a, b, -w_1);
      const float e_2 = fmaf(c, d, -w_2);

      return sum_kahan_babushka_neumaier(w_1, w_2, e_1, e_2);
    }

    //Generalization of the Cornea-Harrison-Tang algorithm for dot products.
    CUDA_HOS_DEV inline static
    float corrected_dot_product(const float a_1, const float a_2, const float a_3,
                                const float b_1, const float b_2, const float b_3)
    {
      using namespace std;

      const float w_1 = a_1 * b_1;
      const float w_2 = a_2 * b_2;
      const float w_3 = a_3 * b_3;

      const float e_1 = fmaf(a_1, b_1, -w_1);
      const float e_2 = fmaf(a_2, b_2, -w_2);
      const float e_3 = fmaf(a_3, b_3, -w_3);

      return sum_kahan_babushka_neumaier(w_1, w_2, w_3, e_1, e_2, e_3);
    }

    //Generalization of the Cornea-Harrison-Tang algorithm for dot products.
    inline CUDA_HOS_DEV
    float corrected_dot_product(const float (&a)[3], const float (&b)[3])
    {
      return corrected_dot_product(a[0], a[1], a[2], b[0], b[1], b[2]);
    }

    //Cross product using the Cornea-Harrison-Tang algorithm.
    CUDA_HOS_DEV inline static
    void corrected_cross_product(float (&res)[3], const float a1, const float a2, const float a3, const float b1, const float b2, const float b3)
    {
      res[0] = product_sum_cornea_harrison_tang(a2, b3, -a3, b2);
      res[1] = product_sum_cornea_harrison_tang(a3, b1, -a1, b3);
      res[2] = product_sum_cornea_harrison_tang(a1, b2, -a2, b1);
    }

    //Cross product using the Cornea-Harrison-Tang algorithm.
    CUDA_HOS_DEV inline static
    void corrected_cross_product(float (&res)[3], const float (&x)[3], const float (&y)[3])
    {
      corrected_cross_product(res, x[0], x[1], x[2], y[0], y[1], y[2]);
    }

    //Magnitude of a cross product using the generalization of the Cornea-Harrison-Tang algorithm.
    CUDA_HOS_DEV inline static
    float corrected_magn_cross_product(const float a1, const float a2, const float a3, const float b1, const float b2, const float b3)
    {
      using namespace std;

      const float r_1 = product_sum_cornea_harrison_tang(a2, b3, -a3, b2);
      const float r_2 = product_sum_cornea_harrison_tang(a3, b1, -a1, b3);
      const float r_3 = product_sum_cornea_harrison_tang(a1, b2, -a2, b1);

#ifdef __CUDA_ARCH__
      return norm3df(r_1, r_2, r_3);
#else
      return hypot(r_1, r_2, r_3);
#endif

    }

    //Magnitude of a cross product using the generalization of the Cornea-Harrison-Tang algorithm.
    CUDA_HOS_DEV inline static
    float corrected_magn_cross_product(const float (&x)[3], const float (&y)[3])
    {
      return corrected_magn_cross_product(x[0], x[1], x[2], y[0], y[1], y[2]);
    }

    ///! Holds dummy classes just to identify the place in which memory lives.
    namespace MemoryContext
    {
      struct CPU
      {
        constexpr static char const * name = "CPU";
      };
      struct CUDAGPU
      {
        constexpr static char const * name = "CUDA GPU";
      };
      struct CUDAPinnedCPU
      {
        constexpr static char const * name = "CUDA Pinned CPU";
      };
    }

    ///! Handles allocation of a type \p T, using \p indexer as the integer type to indicate sizes.
    template <class T, class indexer>
    class MemoryManagement
    {
     private:
      template <class C, class dummy = void> struct unary_helper;

      template <class dummy> struct unary_helper<MemoryContext::CPU, dummy>
      {
        static inline T * allocate(const indexer size)
        {
          return new T[size];
        }

        static inline void deallocate(T *& arr)
        {
          delete[] arr;
        }

      };

      template <class dummy> struct unary_helper<MemoryContext::CUDAGPU, dummy>
      {
        static inline T * allocate(const indexer size)
        {
          return static_cast<T *>(CUDA_Helpers::allocate(sizeof(T) * size));
        }

        static inline void deallocate(T *& arr)
        {
          CUDA_Helpers::deallocate(arr);
        }
      };


      template <class dummy> struct unary_helper<MemoryContext::CUDAPinnedCPU, dummy>
      {
        static inline T * allocate(const indexer size)
        {
          return static_cast<T *>(CUDA_Helpers::allocate_pinned(sizeof(T) * size));
        }

        static inline void deallocate(T *& arr)
        {
          CUDA_Helpers::deallocate_pinned(arr);
        }
      };

      template <class C1, class C2, class dummy = void> struct copy_helper;

      template <class dummy> struct copy_helper<MemoryContext::CPU, MemoryContext::CPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          std::memcpy(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CPU, MemoryContext::CUDAGPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          CUDA_Helpers::GPU_to_CPU(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CUDAGPU, MemoryContext::CUDAGPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          CUDA_Helpers::GPU_to_GPU(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CUDAGPU, MemoryContext::CPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          CUDA_Helpers::CPU_to_GPU(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CUDAPinnedCPU, MemoryContext::CPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          std::memcpy(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CPU, MemoryContext::CUDAPinnedCPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          std::memcpy(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CUDAPinnedCPU, MemoryContext::CUDAPinnedCPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          std::memcpy(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CUDAPinnedCPU, MemoryContext::CUDAGPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          CUDA_Helpers::GPU_to_CPU(dest, source, sizeof(T) * sz);
        }
      };

      template <class dummy> struct copy_helper<MemoryContext::CUDAGPU, MemoryContext::CUDAPinnedCPU, dummy>
      {
        static inline void copy (T * dest, const T * const source, const indexer sz)
        {
          CUDA_Helpers::CPU_to_GPU(dest, source, sizeof(T) * sz);
        }
      };


      template <class C1, class C2, class dummy = void> struct move_helper;

      template <class C1, class C2, class dummy> struct move_helper
      {
        inline static void move(T *& dest,  T *& source, const indexer sz)
        {
          dest = MemoryManagement<T, indexer>::template allocate<C1>(sz);
          MemoryManagement<T, indexer>::template copy<C1, C2>(dest, source, sz);
          MemoryManagement<T, indexer>::template deallocate<C2>(source);
        }
      };

      template <class C, class dummy> struct move_helper<C, C, dummy>
      {
        inline static void move(T *& dest,  T *& source, const indexer)
        {
          dest = source;
          source = nullptr;
        }
      };

     public:
      ///! \brief Allocates \p size elements from memory context \p Context
      template <class Context> static inline T * allocate(const indexer size)
      {
        T * ret = nullptr;
        if (size > 0)
          {
            ret = unary_helper<Context>::allocate(size);
          }
#if CALORECGPU_HELPERS_DEBUG
        std::cerr << "ALLOCATED " << size << " x " << sizeof(T) << " in " << Context::name << ": " << ret << std::endl;
#endif
        return ret;
      }

      ///! \brief Deallocates \p arr from memory context \p Context
      template <class Context> static inline void deallocate(T *& arr)
      {
        if (arr == nullptr)
          //This check is to ensure the code behaves on non-CUDA enabled platforms
          //where some destructors might still be called with nullptr.
          {
            return;
          }
        unary_helper<Context>::deallocate(arr);
#if CALORECGPU_HELPERS_DEBUG
        std::cerr << "DEALLOCATED in " << Context::name << ": " << arr << std::endl;
#endif
        arr = nullptr;
      }


      ///! \brief Copies \p sz bytes from \p source in \p SourceContext to \p dest in \p DestContext
      template <class DestContext, class SourceContext>
      static inline void copy(T * dest, const T * const source, const indexer sz)
      {
        if (sz > 0 && source != nullptr)
          {
            copy_helper<DestContext, SourceContext>::copy(dest, source, sz);
          }
#if CALORECGPU_HELPERS_DEBUG
        std::cerr << "COPIED " << sz << " from " << SourceContext::name << " to " << DestContext::name << ": " << source << " to " << dest << std::endl;
#endif
      }


      /**! \brief Moves \p sz bytes from \p source in \p SourceContext to \p dest in \p DestContext
            (performing the necessary data transfers and deallocations when the contexts are different).
            \p source is set to null and \p dest may be nulled if \p sz is 0 or \p source is null too.

      */
      template <class DestContext, class SourceContext>
      static inline void move(T *& dest,  T *& source, const indexer sz)
      {
#if CALORECGPU_HELPERS_DEBUG
        std::cerr << "MOVED " << sz << " from " << SourceContext::name << " to " << DestContext::name << ": " << source << " to " << dest;
#endif
        if (sz > 0 && source != nullptr)
          {
            move_helper<DestContext, SourceContext, std::is_same<DestContext, SourceContext>>::move(dest, source, sz);
          }
        else
          {
            dest = nullptr;
            deallocate<SourceContext>(source);
          }
#if CALORECGPU_HELPERS_DEBUG
        std::cerr << " | " << source << " to " << dest << std::endl;
#endif
      }

    };

    /*! Holds a run-time amount of objects of type \T, measuring sizes with \p indexer,
        in memory context \p Context. Automatically handles memory transfers to and from
        other contexts in the constructors and assignment operators.
        If \p hold_arrays is \p false, functions as a simple non-holding pointer,
        just keeping track of the \p Context for possible copies/transfers between memory contexts.
    */
    template <class T, class indexer, class Context, bool hold_arrays = true>
    class SimpleContainer;

    template <class T, class indexer, class Context>
    class SimpleContainer<T, indexer, Context, true>
    {
      static_assert(std::is_trivially_copyable<T>::value, "SimpleContainer only works with a trivially copyable type.");
      T * m_array;
      indexer m_size;

      template <class a, class b, class c, bool d> friend class SimpleContainer;

      using Manager = MemoryManagement<T, indexer>;

     public:

      CUDA_HOS_DEV inline indexer size() const
      {
        return m_size;
      }

      CUDA_HOS_DEV inline T & operator[] (const indexer i)
      {
        return m_array[i];
      }

      CUDA_HOS_DEV inline const T & operator[] (const indexer i) const
      {
        return m_array[i];
      }

      inline void clear()
      {
        Manager::deallocate(m_array);
        m_size = 0;
      }

      inline void resize(const indexer new_size)
      {
        if (new_size == 0)
          {
            clear();
          }
        else if (new_size != m_size)
          {
            T * temp = m_array;
            m_array = Manager::template allocate<Context>(new_size);
            Manager::template copy<Context, Context>(m_array, temp, (m_size < new_size ? m_size : new_size));
            Manager::template deallocate<Context>(temp);
            m_size = new_size;
          }
      }

      SimpleContainer() : m_array(nullptr), m_size(0)
      {
      }

      SimpleContainer(const indexer sz) : m_size(sz)
      {
        m_array = Manager::template allocate<Context>(sz);
      }

      /*!
        \warning We assume the pointer is in a valid memory location!
      */
      SimpleContainer(T * other_array, const indexer sz) : m_size(sz)
      {
        m_array = Manager::template allocate<Context>(sz);
        Manager::template copy<Context, Context>(m_array, other_array, sz);
      }

      SimpleContainer(const SimpleContainer & other) : m_size(other.m_size)
      {
        m_array = Manager::template allocate<Context>(m_size);
        Manager::template copy<Context, Context>(m_array, other.m_array, m_size);
      }

      SimpleContainer(SimpleContainer && other) : m_size(other.m_size)
      {
        m_array = nullptr;
        Manager::template move<Context, Context>(m_array, other.m_array, m_size);
        other.m_size = 0;
      }

      template <class other_indexer, class other_context, bool other_hold>
      SimpleContainer(const SimpleContainer<T, other_indexer, other_context, other_hold> & other) :
        m_size(other.m_size)
      {

        m_array = Manager::template allocate<Context>(m_size);
        Manager::template copy<Context, other_context>(m_array, other.m_array, m_size);
      }

      template <class other_indexer, class other_context>
      SimpleContainer(SimpleContainer<T, other_indexer, other_context, true> && other) :
        m_size(other.m_size)
      {
        m_array = nullptr;
        Manager::template move<Context, other_context>(m_array, other.m_array, m_size);
        other.m_size = 0;
      }

      SimpleContainer & operator= (const SimpleContainer & other)
      {
        if (this == &other)
          {
            return (*this);
          }
        else
          {
            resize(other.size());
            Manager::template copy<Context, Context>(m_array, other.m_array, m_size);
            return (*this);
          }
      }

      SimpleContainer & operator= (SimpleContainer && other)
      {
        if (this == &other)
          {
            return (*this);
          }
        else
          {
            clear();
            Manager::template move<Context, Context>(m_array, other.m_array, other.size());
            m_size = other.m_size;
            other.m_size = 0;
            return (*this);
          }
      }


      template <class other_indexer, class other_context, bool other_hold>
      SimpleContainer & operator= (const SimpleContainer<T, other_indexer, other_context, other_hold> & other)
      {
        resize(other.m_size);
        Manager::template copy<Context, other_context>(m_array, other.m_array, m_size);
        return (*this);
      }

      template <class other_indexer, class other_context>
      SimpleContainer & operator= (SimpleContainer<T, other_indexer, other_context, true> && other)
      {
        clear();
        Manager::template move<Context, other_context>(m_array, other.m_array, other.m_size);
        m_size = other.m_size;
        other.m_size = 0;
        return (*this);
      }

      ~SimpleContainer()
      {
        Manager::template deallocate<Context>(m_array);
        m_size = 0;
      }

      CUDA_HOS_DEV operator const T * () const
      {
        return m_array;
      }

      CUDA_HOS_DEV operator T * ()
      {
        return m_array;
      }
      
      CUDA_HOS_DEV operator const void * () const
      {
        return m_array;
      }
      
      CUDA_HOS_DEV operator void * ()
      {
        return m_array;
      }

      template <class stream, class str = std::basic_string<typename stream::char_type> >
      void textual_output(stream & s, const str & separator = " ") const
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            s << m_size << separator;
            for (indexer i = 0; i < m_size - 1; ++i)
              {
                s << m_array[i] << separator;
              }
            s << m_array[m_size - 1];
          }
        else
          {
            SimpleContainer<T, indexer, MemoryContext::CPU, true> other(*this);
            other.textual_output(s, separator);
          }
      }

      template <class stream>
      void textual_input(stream & s)
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            indexer new_size;
            s >> new_size >> std::ws;
            if (s.fail())
              {
                //Throw errors, perhaps? Don't know if we can/should use exceptions...
                std::cerr << "FAILED READING " << this << "!" << std::endl;
                new_size = 0;
              }
            resize(new_size);
            for (indexer i = 0; i < m_size - 1; ++i)
              {
                s >> m_array[i];
                s >> std::ws;
              }
            s >> m_array[m_size - 1];
          }
        else
          {
            SimpleContainer<T, indexer, MemoryContext::CPU, true> other;
            other.textual_input(s);
            (*this) = other;
          }
      }

      template <class stream>
      void binary_output(stream & s) const
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            s.write(reinterpret_cast<const char *>(&m_size), sizeof(indexer));
            for (indexer i = 0; i < m_size; ++i)
              {
                s.write(reinterpret_cast<const char *>(m_array + i), sizeof(T));
              }
          }
        else
          {
            SimpleContainer<T, indexer, MemoryContext::CPU, true> other(*this);
            other.binary_output(s);
          }
      }

      template <class stream>
      void binary_input(stream & s)
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            indexer new_size;
            s.read(reinterpret_cast<char *>(&new_size), sizeof(indexer));
            if (s.fail())
              {
                //Throw errors, perhaps? Don't know if we can/should use exceptions...
                std::cerr << "FAILED READING " << this << "!" << std::endl;
                new_size = 0;
              }
            resize(new_size);
            for (indexer i = 0; i < m_size; ++i)
              {
                s.read(reinterpret_cast<char *>(m_array + i), sizeof(T));
              }
          }
        else
          {
            SimpleContainer<T, indexer, MemoryContext::CPU, true> other;
            other.binary_input(s);
            (*this) = other;
          }
      }

    };

    template <class T, class indexer, class Context>
    class SimpleContainer<T, indexer, Context, false>
    {
      static_assert(std::is_trivially_copyable<T>::value, "SimpleContainer only works with a trivially copyable type.");
      T * m_array;
      indexer m_size;

      using Manager = MemoryManagement<T, indexer>;

      template <class a, class b, class c, bool d> friend class SimpleContainer;

     public:

      CUDA_HOS_DEV inline indexer size() const
      {
        return m_size;
      }

      CUDA_HOS_DEV inline T & operator[] (const indexer i)
      {
        return m_array[i];
      }

      CUDA_HOS_DEV inline const T & operator[] (const indexer i) const
      {
        return m_array[i];
      }

      // cppcheck-suppress  uninitMemberVar
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleContainer() : m_array(nullptr), m_size(0)
      {
      }

      /*!
        \warning We assume the pointer is in a valid memory location!
      */
      // cppcheck-suppress  uninitMemberVar
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleContainer(T * other_array, const indexer sz) : m_array(other_array), m_size(sz)
      {
      }

      template <class other_indexer, bool other_hold>
      // cppcheck-suppress  uninitMemberVar
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleContainer(const SimpleContainer<T, other_indexer, Context, other_hold> & other) :
        m_size(other.m_size),
        m_array(other.m_array)
      {
      }

      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleContainer & operator= (const SimpleContainer & other)
      {
        if (this == &other)
          {
            return (*this);
          }
        else
          {
            m_array = other.m_array;
            m_size = other.m_size;
            return (*this);
          }
      }

      template <class other_indexer, bool other_hold>
      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleContainer & operator= (const SimpleContainer<T, other_indexer, Context, other_hold> & other)
      {
        m_size = other.m_size;
        m_array = other.m_array;
        return (*this);
      }

      CUDA_HOS_DEV operator const T * () const
      {
        return m_array;
      }

      CUDA_HOS_DEV operator T * ()
      {
        return m_array;
      }
      
      CUDA_HOS_DEV operator const void * () const
      {
        return m_array;
      }
      
      CUDA_HOS_DEV operator void * ()
      {
        return m_array;
      }
    };

    /// \brief Holds a run-time specified amount of objects of type \p T in CPU memory.
    template <class T, class indexer = unsigned int>
    using CPU_array = SimpleContainer<T, indexer, MemoryContext::CPU, true>;

    /// \brief Holds a run-time specified amount of objects of type \p T in CUDA GPU memory.
    template <class T, class indexer = unsigned int>
    using CUDA_array = SimpleContainer<T, indexer, MemoryContext::CUDAGPU, true>;

    /// \brief Non-owning pointer to an array of \p T in CUDA GPU memory.
    template <class T, class indexer = unsigned int>
    using CUDA_kernel_array = SimpleContainer<T, indexer, MemoryContext::CUDAGPU, false>;

    /*! Holds one objects of type \T in memory context \p Context.
        Automatically handles memory transfers to and from
        other contexts in the constructors and assignment operators.
        If \p hold_object is \p false, functions as a simple non-holding pointer,
        just keeping track of the \p Context for possible copies/transfers between memory contexts.
    */
    template <class T, class Context, bool hold_object = true>
    class SimpleHolder;

    template <class T, class Context>
    class SimpleHolder<T, Context, true>
    {
      static_assert(std::is_trivially_copyable<T>::value, "SimpleHolder only works with a trivially copyable type.");

      using indexer = unsigned int;

      T * m_object;

      using Manager = MemoryManagement<T, indexer>;

      template <class a, class b, bool c> friend class SimpleHolder;

     public:

      CUDA_HOS_DEV const T & operator *() const
      {
        return *m_object;
      }

      CUDA_HOS_DEV T & operator *()
      {
        return *m_object;
      }

      CUDA_HOS_DEV const T * operator ->() const
      {
        return m_object;
      }

      CUDA_HOS_DEV T * operator ->()
      {
        return m_object;
      }

      CUDA_HOS_DEV inline bool valid() const
      {
        return m_object != nullptr;
      }

      inline void clear()
      {
        Manager::template deallocate<Context>(m_object);
      }

      inline void allocate()
      {
        if (m_object == nullptr)
          {
            m_object = Manager::template allocate<Context>(1);
          }
      }

      SimpleHolder(): m_object(nullptr)
      {
      }

      SimpleHolder(const bool really_allocate)
      {
        if (really_allocate)
          {
            m_object = Manager::template allocate<Context>(1);
          }
        else
          {
            m_object = nullptr;
          }
      }

      /*!
        \warning We assume the pointer is in a valid memory location!
      */
      template < class X, class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      explicit SimpleHolder(X * other_p)
      {
        m_object = Manager::template allocate<Context>(1);
        Manager::template copy<Context, Context>(m_object, other_p, 1);
      }

      /*!
         \warning We assume the object is in a valid memory location!
       */
      template < class X, class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      SimpleHolder(const X & other_v) : SimpleHolder(&other_v)
      {


      }

      SimpleHolder(const SimpleHolder & other)
      {
        if (other.valid())
          {
            m_object = Manager::template allocate<Context>(1);
            Manager::template copy<Context, Context>(m_object, other.m_object, other.valid());
          }
        else
          {
            m_object = nullptr;
          }
      }

      template < class X, class other_context, bool other_hold,
                 class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      SimpleHolder(const SimpleHolder<X, other_context, other_hold> & other)
      {
        if (other.valid())
          {
            m_object = Manager::template allocate<Context>(1);
            Manager::template copy<Context, other_context>(m_object, other.m_object, other.valid());
          }
        else
          {
            m_object = nullptr;
          }
      }

      SimpleHolder(SimpleHolder && other)
      {
        m_object = nullptr;
        Manager::template move<Context, Context>(m_object, other.m_object, other.valid());
      }

      template < class X, class other_context,
                 class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      SimpleHolder(SimpleHolder<X, other_context, true> && other)
      {
        m_object = nullptr;
        Manager::template move<Context, other_context>(m_object, other.m_object, other.valid());
      }

      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      SimpleHolder & operator= (const SimpleHolder & other)
      {
        if (!valid() && other.valid())
          {
            allocate();
          }
        if (&other != this)
          {
            Manager::template copy<Context, Context>(m_object, other.m_object, other.valid());
          }
        return (*this);
      }

      template < class X, class other_context, bool other_hold,
                 class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      SimpleHolder & operator= (const SimpleHolder<X, other_context, other_hold> & other)
      {
        if (!valid() && other.valid())
          {
            allocate();
          }
        Manager::template copy<Context, other_context>(m_object, other.m_object, other.valid());
        return (*this);
      }

      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      SimpleHolder & operator= (SimpleHolder && other)
      {
        if (&other != this)
          {
            clear();
            Manager::template move<Context, Context>(m_object, other.m_object, other.valid());
          }
        return (*this);
      }

      template < class X, class other_context,
                 class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      SimpleHolder & operator= (SimpleHolder<X, other_context, true> && other)
      {
        clear();
        Manager::template move<Context, other_context>(m_object, other.m_object, other.valid());
        return (*this);
      }

      ~SimpleHolder()
      {
        Manager::template deallocate<Context>(m_object);
      }

      template < class X, class disabler = typename std::enable_if < std::is_base_of<X, T>::value || std::is_same<T, X>::value >::type >
      CUDA_HOS_DEV operator const X * () const
      {
        return m_object;
      }

      template < class X, class disabler = typename std::enable_if < std::is_base_of<X, T>::value || std::is_same<T, X>::value >::type >
      CUDA_HOS_DEV operator X * ()
      {
        return m_object;
      }
      
      CUDA_HOS_DEV operator const void * () const
      {
        return m_object;
      }
      
      CUDA_HOS_DEV operator void * ()
      {
        return m_object;
      }

      template <class stream, class str = std::basic_string<typename stream::char_type> >
      void textual_output(stream & s, const str & separator = " ") const
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            if (m_object == nullptr)
              {
                s << 0;
              }
            else
              {
                s << 1 << separator << (*m_object);
              }
          }
        else
          {
            SimpleHolder<T, MemoryContext::CPU, true> other(*this);
            other.textual_output(s, separator);
          }
      }

      template <class stream>
      void textual_input(stream & s)
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            bool is_valid;
            s >> is_valid >> std::ws;
            if (s.fail())
              {
                //Throw errors, perhaps? Don't know if we can/should use exceptions...
                std::cerr << "FAILED READING " << this << "!" << std::endl;
                is_valid = false;
              }
            if (is_valid)
              {
                allocate();
                s >> (*m_object);
              }
            else
              {
                clear();
              }
          }
        else
          {
            SimpleHolder<T, MemoryContext::CPU, true> other;
            other.textual_input(s);
            (*this) = other;
          }
      }

      template <class stream>
      void binary_output(stream & s) const
      {
        if (m_object == nullptr)
          {
            return;
          }
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            s.write(reinterpret_cast<const char *>(m_object), sizeof(T));
          }
        else
          {
            SimpleHolder<T, MemoryContext::CPU, true> other(*this);
            other.binary_output(s);
          }
      }

      template <class stream>
      void binary_input(stream & s)
      {
        if (std::is_same<Context, MemoryContext::CPU>::value)
          {
            allocate();
            s.read(reinterpret_cast<char *>(m_object), sizeof(T));
          }
        else
          {
            SimpleHolder<T, MemoryContext::CPU, true> other;
            other.binary_input(s);
            (*this) = other;
          }
      }

    };

    template <class T, class Context>
    class SimpleHolder<T, Context, false>
    {
      static_assert(std::is_trivially_copyable<T>::value, "SimpleHolder only works with a trivially copyable type.");

      using indexer = unsigned int;

      T * m_object;

      using Manager = MemoryManagement<T, indexer>;

      template <class a, class b, bool c> friend class SimpleHolder;

     public:

      CUDA_HOS_DEV const T & operator *() const
      {
        return *m_object;
      }

      CUDA_HOS_DEV T & operator *()
      {
        return *m_object;
      }

      CUDA_HOS_DEV const T * operator ->() const
      {
        return m_object;
      }

      CUDA_HOS_DEV T * operator ->()
      {
        return m_object;
      }

      CUDA_HOS_DEV inline bool valid() const
      {
        return m_object != nullptr;
      }

      // cppcheck-suppress  uninitMemberVar
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleHolder() : m_object(nullptr)
      {
      }

      /*!
        \warning We assume the pointer is in a valid memory location!
      */
      template < class X, class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      // cppcheck-suppress  uninitMemberVar
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleHolder(X * other_p)
      {
        m_object = other_p;
      }

      template < class X, bool other_hold,
                 class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      // cppcheck-suppress  uninitMemberVar
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleHolder(const SimpleHolder<X, Context, other_hold> & other)
      {
        m_object = other.m_object;
      }

      template < class X, bool other_hold,
                 class disabler = typename std::enable_if < std::is_base_of<T, X>::value || std::is_same<T, X>::value >::type >
      // cppcheck-suppress  operatorEqVarError
      //Try to suppress the uninitialized member thing that is probably being thrown off by the CUDA_HOS_DEV macro...
      CUDA_HOS_DEV SimpleHolder & operator= (const SimpleHolder<X, Context, other_hold> & other)
      {
        m_object = other.m_object;
        return (*this);
      }

      template < class X, class disabler = typename std::enable_if < std::is_base_of<X, T>::value || std::is_same<T, X>::value >::type >
      CUDA_HOS_DEV operator const X * () const
      {
        return m_object;
      }

      template < class X, class disabler = typename std::enable_if < std::is_base_of<X, T>::value || std::is_same<T, X>::value >::type >
      CUDA_HOS_DEV operator X * ()
      {
        return m_object;
      }

      CUDA_HOS_DEV operator const void * () const
      {
        return m_object;
      }
      
      CUDA_HOS_DEV operator void * ()
      {
        return m_object;
      }
    };

    /// \brief Holds an object of type \p T in CPU memory.
    template <class T>
    using CPU_object = SimpleHolder<T, MemoryContext::CPU, true>;

    /// \brief Holds an object of type \p T in CUDA GPU memory.
    template <class T>
    using CUDA_object = SimpleHolder<T, MemoryContext::CUDAGPU, true>;

    /// \brief Non-owning pointer to an object of type \p T in CUDA GPU memory.
    template <class T>
    using CUDA_kernel_object = SimpleHolder<T, MemoryContext::CUDAGPU, false>;

    /// \brief Holds an object of type \p T in CUDA GPU memory.
    template <class T>
    using CUDA_pinned_CPU_object = SimpleHolder<T, MemoryContext::CUDAPinnedCPU, true>;

    /*! \brief Manages objects of type \p T in a thread-safe way,
               ensuring that there's an object available for each separate thread
               while minimizing the number of allocations.

        Internally uses a \c std::vector of \c std::thread::id to manage the different threads,
        thus it is especially efficient for relatively small numbers of concurrent threads
        (which matches the use case, e. g. 64 threads )

    */
    template <class T>
    class separate_thread_holder
    {
     private:
      std::vector< std::unique_ptr<T> > m_held;
      std::vector< typename std::thread::id > m_thread_equivs;
      //For a sufficiently small number of threads
      //(not much more than 100 or so?)
      //it's faster to have linear search+insert
      //than any other addressing mode
      //(e. g. unordered_map)
      //We could still consider a more sophisticated solution...

      //Simple alternative: some sort of stack for non-assigned objects,
      //pushing and popping instead of linear searching.
      //(But with constant memory -> no (de)allocations.)

      mutable std::shared_mutex m_mutex;

      //Assumes a (read) lock is taken!
      //Returns nullptr if the current thread
      //has no objects associated with it.
      T * get_pointer_if_available() const
      {
        const std::thread::id this_id = std::this_thread::get_id();
        
        for (size_t i = 0; i < m_thread_equivs.size(); ++i)
          {
            if (m_thread_equivs[i] == this_id)
              {
                return m_held[i].get();
              }
          }
        return nullptr;
      }

      T & add_one_and_return()
      {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        const std::thread::id this_id = std::this_thread::get_id();
        const std::thread::id invalid_id{};

        bool empty_found = false;
        
        size_t first_empty;
        
        for (size_t i = 0; i < m_thread_equivs.size(); ++i)
          {
            if (m_thread_equivs[i] == this_id)
              {
                return *(m_held[i]);
              }
            else if (!empty_found && m_thread_equivs[i] == invalid_id)
              {
                empty_found = true;
                first_empty = i;
              }
          }

        if (empty_found)
          {
            m_thread_equivs[first_empty] = this_id;
            return *(m_held[first_empty]);
          }
        else
          {
            m_held.emplace_back(std::make_unique<T>());
            m_thread_equivs.emplace_back(std::this_thread::get_id());
            return *(m_held.back());
          }
      }

     public:
      T & get_one()
      {
        {
          std::shared_lock<std::shared_mutex> lock(m_mutex);

          T * to_return = get_pointer_if_available();

          if (to_return != nullptr)
            {
              return *to_return;
            }
        }
        return add_one_and_return();
      }

      ///\pre Assumes the thread already has an allocated object (through @p get_one).
      T & get_for_thread() const
      {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        T * to_return = get_pointer_if_available();

        if (to_return != nullptr)
          {
            return *to_return;
          }
        else
          {
            //Here would be a good place for an unreachable.
            //C++23?
            return *(m_held.back());
          }
      }

      void release_one()
      {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        std::thread::id this_id = std::this_thread::get_id();
        const std::thread::id invalid_id{};
        for (size_t i = 0; i < m_thread_equivs.size(); ++i)
          {
            if (m_thread_equivs[i] == this_id)
              {
                m_thread_equivs[i] = invalid_id;
              }
          }
      }

      void resize(const size_t new_size)
      {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (new_size < m_held.size())
          {
            m_held.resize(new_size);
            m_thread_equivs.resize(new_size);
          }
        else if (new_size > m_held.size())
          {
            const size_t to_add = new_size - m_held.size();
            const std::thread::id invalid_id{};
            for (size_t i = 0; i < to_add; ++i)
              {
                m_held.emplace_back(std::make_unique<T>());
                m_thread_equivs.emplace_back(invalid_id);
              }
          }
      }

      template <class F, class ... Args>
      void operate_on_all(F && f, Args && ... args)
      {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (std::unique_ptr<T> & obj : m_held)
          {
            f(*obj, std::forward<Args>(args)...);
          }
      }

      size_t held_size() const
      {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_held.size();
      }

      size_t available_size() const
      {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        size_t count = 0;
        const std::thread::id invalid_id{};
        for (const auto & id : m_thread_equivs)
          {
            if (id == invalid_id)
              {
                ++count;
              }
          }
        return count;
      }

      size_t filled_size() const
      {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        size_t count = 0;
        const std::thread::id invalid_id{};
        for (const auto & id : m_thread_equivs)
          {
            if (id == invalid_id)
              {
                ++count;
              }
          }
        return m_held.size() - count;
      }
    };

    /**! \brief Utility to access a \p separate_thread_holder to get a pointer and call \p release_one at scope exit.
    */
    template <class T>
    struct separate_thread_accessor
    {
     private:
      separate_thread_holder<T> & m_sth;
      T * m_held;
     public:
      separate_thread_accessor(separate_thread_holder<T> & s):
        m_sth(s), m_held(nullptr)
      {
      }
      T & get_one()
      {
        if (m_held == nullptr)
          {
            m_held = &(m_sth.get_one());
          }
        return *m_held;
      }
      void release_one()
      {
        if (m_held != nullptr)
          {
            m_sth.release_one();
            m_held = nullptr;
          }
      }
      ~separate_thread_accessor()
      {
        if (m_held != nullptr)
          {
            m_sth.release_one();
          }
      }
      separate_thread_accessor(separate_thread_holder<T> & s, T *& ptr):
        separate_thread_accessor(s)
      {
        get_one();
        ptr = m_held;
      }
    };

    /** \brief Possibly holds an object in its internal buffer.
               Useful to forego heap allocations, but still
               have the option not to construct something...
    */
    template <class T>
    struct maybe_allocate
    {
     private:

      alignas(T) char m_buf[sizeof(T)];
      T * m_object = nullptr;

     public:

      maybe_allocate(const bool allocate, const T & t)
      {
        if (allocate)
          {
            m_object = new (m_buf) T(t);
          }
      }

      maybe_allocate(const bool allocate, T && t)
      {
        if (allocate)
          {
            m_object = new (m_buf) T(t);
          }
      }

      template <class ... Args>
      maybe_allocate(const bool allocate, Args && ... args)
      {
        if (allocate)
          {
            m_object = new (m_buf) T(std::forward<Args>(args)...);
          }
      }

      maybe_allocate(const maybe_allocate & other) : maybe_allocate(other.get())
      {
      }


      maybe_allocate(maybe_allocate && other) : maybe_allocate(other.get())
      {
      }

      maybe_allocate & operator= (const maybe_allocate & other)
      {
        if (&other != this)
          {
            if (m_object != nullptr)
              {
                (*m_object) = other.get();
              }
            else
              {
                m_object = new (m_buf) T(other.get());
              }
          }
        return (*this);
      }


      maybe_allocate & operator= (maybe_allocate && other)
      {
        if (&other != this)
          {
            if (m_object != nullptr)
              {
                (*m_object) = other.get();
              }
            else
              {
                m_object = new (m_buf) T(other.get());
              }
          }
        return (*this);
      }

      ~maybe_allocate()
      {
        if (m_object != nullptr)
          {
            m_object->~T();
          }
      }

      bool valid() const
      {
        return m_object != nullptr;
      }

      T && get() &&
      {
        return *m_object;
      }

      T & get() &
      {
        return *m_object;
      }

      const T & get() const &
      {
        return *m_object;
      }

      const T * operator ->() const
      {
        return m_object;
      }

      T * operator ->()
      {
        return m_object;
      }

      operator T & ()
      {
        return *m_object;
      }

      operator T && () &&
      {
        return *m_object;
      }

      operator const T & () const
      {
        return *m_object;
      }
    };
  }

}

#endif  // CALORECGPU_HELPERS_H
