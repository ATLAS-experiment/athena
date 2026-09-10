// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#undef NDEBUG
#include "../src/RPCFunctionEntry.h"

#include <array>
#include <cassert>
#include <cstring>
#include <list>
#include <map>
#include <span>
#include <vector>

using namespace RemoteCall;

static_assert(RPCSupported<int>);
static_assert(RPCSupported<std::array<double, 4>>);
static_assert(RPCSupported<std::span<double>>);
static_assert(RPCSupported<std::vector<double>>);
static_assert(!RPCSupported<std::list<double>>);
static_assert(!RPCSupported<std::vector<bool>>);
static_assert(!RPCSupported<std::vector<std::string>>);
static_assert(!std::is_copy_constructible_v<RPCRet<Host, int>>);
static_assert(std::is_move_constructible_v<RPCRet<Host, int>>);
static_assert(!std::is_constructible_v<RPCRet<Device, int>, int*>);

/// Callable signature accepted by the RPC API.
using Valid = RPCRet<Host, int> (*)(RPCArg<Device, int>);
/// Callable signature rejected because a parameter owns its buffer.
using BadArgument = RPCRet<Host, int> (*)(RPCRet<Host, int>);
/// Callable signature rejected because its result borrows its buffer.
using BadReturn = RPCArg<Host, int> (*)(RPCArg<Host, int>);
static_assert(RPCCallable<Valid>);
static_assert(!RPCCallable<BadArgument>);
static_assert(!RPCCallable<BadReturn>);

/// Record allocations and check the resource, size and alignment used for
/// cleanup.
class TrackingResource : public std::pmr::memory_resource {
 public:
  /// Outstanding allocations, indexed by address, with byte count and
  /// alignment.
  std::map<void*, std::pair<std::size_t, std::size_t>> live;
  std::size_t frees = 0;  ///< Number of completed deallocations.

 private:
  /// Allocate host storage and record its byte count and alignment.
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    void* p = std::pmr::new_delete_resource()->allocate(bytes, alignment);
    live.emplace(p, std::pair{bytes, alignment});
    return p;
  }
  /// Verify the allocation metadata before freeing the buffer.
  void do_deallocate(void* p, std::size_t bytes,
                     std::size_t alignment) override {
    assert(live.at(p) == (std::pair{bytes, alignment}));
    live.erase(p);
    ++frees;
    std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
  }
  /// Resources compare equal only when they are the same instance.
  bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }
};

/// Copy sent buffers into tracked host allocations and decode their
/// descriptors. This helper exercises receive ownership without running MPI.
std::vector<ClusterMessage::DataDescr> receive(
    const std::vector<RPCResultBuffer>& sent, TrackingResource& resource) {
  std::vector<ClusterMessage::DataDescr> result;
  for (const auto& buffer : sent) {
    void* p = resource.allocate(buffer.len, buffer.align);
    if (buffer.len != 0)
      std::memcpy(p, buffer.ptr, buffer.len);
    ClusterMessage message(
        ClusterMessageType::Data,
        ClusterMessage::DataDescr(p, buffer.len, buffer.align));
    // Decode the wire descriptor before transferring ownership to the result.
    ClusterMessage decoded(message.wire_msg(), &resource);
    result.push_back(
        std::move(std::get<ClusterMessage::DataDescr>(decoded.payload)));
  }
  return result;
}

/// Check range dispatch, result ordering, ownership transfer and invalid sizes.
int main() {
  TrackingResource source;
  TrackingResource destination;
  RPCFunctionEntry entry(
      [&source](RPCArg<Host, std::span<double>> values) {
        auto* data = static_cast<double*>(
            source.allocate(values.size * sizeof(double), alignof(double)));
        RPCRet<Host, std::span<double>> doubled(data, values.size, source);
        for (std::size_t i = 0; i < values.size; ++i)
          doubled.ptr[i] = 2 * values.ptr[i];
        return std::tuple{std::move(doubled), RPCRet<Host, int>(new int(37))};
      },
      "double_range");
  assert(entry.validateArgTypes<std::span<double>>());
  assert(!entry.validateArgTypes<std::vector<double>>());
  assert((entry.validateReturnTypes<std::span<double>, int>()));
  assert((!entry.validateReturnTypes<int, std::span<double>>()));

  std::array<double, 3> input{1.0, -2.0, 3.5};
  for (std::size_t count : {std::size_t(0), input.size()}) {
    std::vector<ClusterMessage::DataDescr> args;
    args.emplace_back(input.data(), count);
    auto sent = entry.wrapper()(args);
    assert(source.live.size() ==
           1);  // The buffer remains valid after the function returns.
    auto received = receive(sent, destination);
    sent.clear();
    assert(
        source.live
            .empty());  // Releasing the sent results frees the source buffer.
    {
      auto [values, scalar] =
          entry.retrieve<std::span<double>, int>(std::move(received));
      assert(values.size == count);
      assert(*scalar ==
             37);  // Check the order of results with different types.
      for (std::size_t i = 0; i < count; ++i)
        assert(values.ptr[i] == 2 * input[i]);
      auto moved = std::move(values);
      assert(values.ptr == nullptr && values.size == 0);
      assert(destination.live.size() == 2);
    }
    assert(destination.live.empty());
  }

  // Use host storage to check that a Device result frees memory through its
  // resource.
  {
    auto* p = static_cast<int*>(source.allocate(sizeof(int), alignof(int)));
    RPCRet<Device, int> device(p, 1, source);
    auto moved = std::move(device);
    assert(device.ptr == nullptr);
    assert(source.live.size() == 1);
  }
  assert(source.live.empty());

  // A result with an invalid size must release all received buffers when
  // validation throws.
  {
    std::vector<ClusterMessage::DataDescr> args;
    args.emplace_back(input.data(), input.size());
    auto sent = entry.wrapper()(args);
    sent[0].len = 1;
    sent[0].align = 1;
    auto received = receive(sent, destination);
    bool threw = false;
    try {
      (void)entry.retrieve<std::span<double>, int>(std::move(received));
    } catch (const std::logic_error&) {
      threw = true;
    }
    assert(threw);
    assert(destination.live.empty());
  }
  assert(source.live.empty());
}
