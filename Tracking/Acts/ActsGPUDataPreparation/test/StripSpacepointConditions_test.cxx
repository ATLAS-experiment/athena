/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StripSpacepointConditions.h"

#include <exception>
#include <future>
#include <iostream>
#include <stdexcept>
#include <vecmem/memory/host_memory_resource.hpp>
#include <vector>

#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "SGTools/TestStore.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteCondHandle.h"
#include "TestTools/initGaudi.h"

namespace {
void check(bool value) {
  if (!value)
    throw std::runtime_error("Strip SP conditions test failed");
}
EventIDBase time(unsigned run) {
  return EventIDBase(run, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM,
                     EventIDBase::UNDEFNUM, 0);
}
EventIDRange range(unsigned first, unsigned last) {
  return {time(first), time(last)};
}
using Payload = ActsTrk::StripSpacepointConditions;
std::unique_ptr<Payload> payload(unsigned count) {
  return std::make_unique<Payload>(
      std::make_unique<vecmem::host_memory_resource>(), count, count);
}
struct AllocationState {
  unsigned outstanding{0};
  bool destroyed{false};
};
// Check the CDO's allocator/buffer destruction order independently of Gaudi
// services: the resource must still exist when each buffer is deallocated.
class CountingResource : public vecmem::memory_resource {
 public:
  explicit CountingResource(std::shared_ptr<AllocationState> state)
      : m_state(std::move(state)) {}
  ~CountingResource() override { m_state->destroyed = true; }

 private:
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    void* result = m_upstream.allocate(bytes, alignment);
    ++m_state->outstanding;
    return result;
  }
  void do_deallocate(void* ptr, std::size_t bytes,
                     std::size_t alignment) override {
    m_upstream.deallocate(ptr, bytes, alignment);
    --m_state->outstanding;
  }
  bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }
  std::shared_ptr<AllocationState> m_state;
  vecmem::host_memory_resource m_upstream;
};
void testOwnership() {
  auto state = std::make_shared<AllocationState>();
  {
    auto data = std::make_unique<Payload>(
        std::make_unique<CountingResource>(state), 2, 3);
    check(state->outstanding > 0 && !state->destroyed);
  }
  check(state->destroyed && state->outstanding == 0);
  try {
    Payload invalid(nullptr, 1, 1);
    throw std::logic_error("Expected invalid allocator exception");
  } catch (const std::invalid_argument&) {
  }
}

void testConditions() {
  ISvcLocator* locator = nullptr;
  check(Athena_test::initGaudi(
      "ActsGPUDataPreparation/StripSpacepointConditions_test.txt", locator));
  SmartIF<StoreGateSvc> store{locator->service("StoreGateSvc/ConditionStore")};
  check(store.isValid());
  SG::WriteCondHandleKey<Payload> elementsKey{"TestElements"};
  SG::WriteCondHandleKey<Payload> propertiesKey{"TestProperties"};
  SG::WriteCondHandleKey<Payload> outputKey{"TestStripSPConditions"};
  check(elementsKey.initialize().isSuccess());
  check(propertiesKey.initialize().isSuccess());
  check(outputKey.initialize().isSuccess());
  SG::ReadCondHandleKey<Payload> elementsRead{"TestElements"};
  SG::ReadCondHandleKey<Payload> propertiesRead{"TestProperties"};
  SG::ReadCondHandleKey<Payload> outputRead{"TestStripSPConditions"};
  check(elementsRead.initialize().isSuccess());
  check(propertiesRead.initialize().isSuccess());
  check(outputRead.initialize().isSuccess());
  SGTest::TestStore eventStore;
  EventContext ctx(0, 0);
  ctx.setEventID(time(1));
  ctx.setExtension(Atlas::ExtendedEventContext(&eventStore));
  Gaudi::Hive::setCurrentContext(ctx);
  {
    SG::WriteCondHandle<Payload> e(elementsKey, ctx);
    SG::WriteCondHandle<Payload> p(propertiesKey, ctx);
    check(e.record(range(1, 3), payload(1)).isSuccess());
    check(p.record(range(1, 2), payload(1)).isSuccess());
  }
  unsigned uploads = 0;
  // Exercise real Conditions Store handles, not a replacement cache. The
  // synthetic inputs isolate both IOV boundaries without detector geometry.
  auto get = [&](unsigned run) -> const Payload* {
    ctx.setEventID(time(run));
    SG::WriteCondHandle<Payload> output(outputKey, ctx);
    if (!output.isValid()) {
      SG::ReadCondHandle<Payload> elements(elementsRead, ctx);
      SG::ReadCondHandle<Payload> properties(propertiesRead, ctx);
      output.addDependency(elements);
      output.addDependency(properties);
      check(output.record(payload(++uploads)).isSuccess());
    }
    SG::ReadCondHandle<Payload> read(outputRead, ctx);
    check(read.isValid());
    return *read;
  };
  const Payload* first = get(1);
  check(get(1) == first && uploads == 1);
  {
    ctx.setEventID(time(2));
    SG::WriteCondHandle<Payload> p(propertiesKey, ctx);
    check(p.record(range(2, 4), payload(1)).isSuccess());
  }
  const Payload* second = get(2);
  check(second != first && uploads == 2);
  {
    ctx.setEventID(time(3));
    SG::WriteCondHandle<Payload> e(elementsKey, ctx);
    check(e.record(range(3, 5), payload(1)).isSuccess());
  }
  const Payload* third = get(3);
  check(third != second && uploads == 3);
  check(get(1) == first && get(2) == second && uploads == 3);
  check(first->surfaces.capacity() == 1 && third->surfaces.capacity() == 3);
  // Concurrent reads of old and new IOVs must resolve to the same immutable
  // objects, without rebuilding or replacing an entry for the other IOV.
  std::vector<std::future<void>> jobs;
  for (unsigned i = 0; i < 16; ++i) {
    jobs.push_back(std::async(std::launch::async, [&, i] {
      EventContext threadCtx(i, 0);
      threadCtx.setEventID(time(i % 2 ? 1 : 3));
      threadCtx.setExtension(Atlas::ExtendedEventContext(&eventStore));
      SG::ReadCondHandle<Payload> read(outputRead, threadCtx);
      check(read.isValid() && *read == (i % 2 ? first : third));
    }));
  }
  for (auto& job : jobs)
    job.get();
  check(uploads == 3);
  std::cout
      << "PASS: IOV intersection, either dependency invalidation, same-IOV "
         "reuse, old/new IOV coexistence and 16 concurrent readers\n";
}
}  // namespace
int main() {
  try {
    testOwnership();
    testConditions();
    std::cout << "PASS: owned allocator outlives all buffers\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Strip SP conditions test failed: " << error.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Strip SP conditions test failed: unknown exception\n";
    return 1;
  }
}
