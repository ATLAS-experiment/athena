/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/src/AllocTestAuxContainer_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#undef NDEBUG
#include "DataModelTestDataRead/versions/AllocTestAuxContainer_v1.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/xmalloc.h"


namespace {


/**
 * @brief Custom memory resource used to test allocator handling.
 *
 * This adds a header to each allocated block, so that we can catch cases
 * if allocations and deallocations are not done in the same way.
 */
class TestMemResource
  : public std::pmr::memory_resource
{
public:
  constexpr static size_t MAGIC = 0xdeadbeefcafefeed;

  virtual void* do_allocate (size_t bytes, std::size_t /*alignment*/) override
  {
    size_t* p = reinterpret_cast<size_t*>(CxxUtils::xmalloc (bytes + 2*sizeof(size_t)));
    p[0] = bytes;
    p[1] = MAGIC;
    return p + 2;
  }

  virtual void do_deallocate (void* p, [[maybe_unused]] std::size_t bytes, std::size_t /*alignment*/) override
  {
    size_t* pp = reinterpret_cast<size_t*>(p);
    pp -= 2;
    assert (pp[0] == bytes);
    assert (pp[1] == MAGIC);
    free (pp);
  }

  virtual bool do_is_equal (const std::pmr::memory_resource& other) const noexcept override
  {
    return dynamic_cast<const TestMemResource*> (&other) != nullptr;
  }
};


// No state, so thread-safe.
static TestMemResource memRes ATLAS_THREAD_SAFE;


}


namespace DMTest {


AllocTestAuxContainer_v1::AllocTestAuxContainer_v1()
  : xAOD::AuxContainerBase(&memRes)
{
}


AllocTestAuxContainer_v1::AllocTestAuxContainer_v1 (std::pmr::memory_resource* r)
  : xAOD::AuxContainerBase(),
    atInt1 (r)
{
}


} // namespace DMTest
