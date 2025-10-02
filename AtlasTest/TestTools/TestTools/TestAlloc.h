// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file TestTools/TestAlloc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief To test handling of non-standard memory allocators.
 */


#ifndef TESTTOOLS_TESTALLOC_H
#define TESTTOOLS_TESTALLOC_H


namespace Athena_test {


/**
 * @brief To test handling of non-standard memory allocators.
 *
 * This is an allocator that adds a header to each block allocated,
 * and then checks it on free.  This ensures that the frees are
 * matched with the allocations.
 */
template <class T>
class TestAlloc
{
public:
  /// Standard STL allocator typedefs.
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;

  constexpr static size_t MAGIC = 0xdeadbeeffeedabba;

  struct Head {
    size_t i[2];
  };
  pointer allocate (size_type n, const void*  = 0)
  {
    void* p = malloc (sizeof(Head) + n*sizeof(T));
    if (!p) std::abort();
    Head* h = (Head*)p;
    h->i[0] = n;
    h->i[1] = MAGIC;
    return reinterpret_cast<pointer>(h+1);
  }

  void deallocate (pointer p, size_type n)
  {
    Head* h = reinterpret_cast<Head*>(p);
    --h;
    if (h->i[0] != n ||
        h->i[1] != MAGIC) {
      std::abort();
    }
    free (h);
  }

  size_type max_size() const throw()
  {
    return 99999;
  }

  template <class... Args>
  void construct (pointer p, Args&&... args)
  {
    // Suppress bogus warning seen with gcc 14.1.
#if __GNUC__ >= 14
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
    new (p) T(std::forward<Args>(args)...);
#if __GNUC__ >= 14
# pragma GCC diagnostic pop
#endif
  }

  void destroy (pointer p)
  {
    p->~T();
  }

  pointer address (reference x) const
  {
    return &x;
  }

  const_pointer address (const_reference x) const
  {
    return &x;
  }

  // We have no state, so consider all instances equal.
  bool operator== (const TestAlloc& /*other*/) const
  { return true; }
};


} // namespace Athena_test


#endif // not TESTTOOLS_TESTALLOC_H
