// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXMPI_RPCFUNCTIONENTRY_H
#define ATHEXMPI_RPCFUNCTIONENTRY_H

// Local include(s)
#include "HostDevicePtr.h"

// System include(s)
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace RemoteCall {

using RPCFunctionID = std::size_t;

/// Describes an RPC argument / return value
struct RPCArgEntry {
  enum class ArgType : char { HostPtr, DevicePtr };
  const ArgType argType;
  const std::type_info* valType = nullptr;
  std::size_t len;
  std::size_t align;

  template <typename T>
  RPCArgEntry(HostPtr<T> placeholder);

  template <typename T>
  RPCArgEntry(DevicePtr<T> placeholder);
};

/// Describes an RPC callable function
class RPCFunctionEntry {
 public:
  using Wrapper = std::move_only_function<std::vector<void*>(
      const std::vector<void*>&) const>;

  template <RPCCallable F>
  RPCFunctionEntry(F&& f, std::string name);

  const std::string& name() const noexcept;
  const std::vector<RPCArgEntry>& arguments() const noexcept;
  const std::vector<RPCArgEntry>& returnVals() const noexcept;

  /// Validate that the pack of argument value types matches what's expected
  template <RPCSupported... Args>
  bool validateArgTypes() const;

  /// Convert a type-erased vector of void* to a tuple of unique_ptrs
  template <RPCSupported... Rs>
  std::tuple<std::unique_ptr<Rs>...> retrieve(
      std::vector<void*>&& returnVals) const;

  const Wrapper& wrapper() const noexcept;

 private:
  std::string m_name;  // Registry key
  std::vector<RPCArgEntry> m_arguments;
  std::vector<RPCArgEntry> m_returnVals;
  Wrapper m_wrapper;
};

/// Custom hash for RPCFunctionEntry for use with concurrent_unordered_set
/// Transparent hash that uses only the name, so functions can't have duplicate
/// names. Also need to be able to look up a function using only the hash (so
/// wire messages can be fixed length).
class RPCFunctionEntryHash {
 public:
  class transparent_key_equal {
   public:
    using is_transparent = void;
    bool operator()(const RPCFunctionEntry& lhs,
                    const RPCFunctionEntry& rhs) const noexcept;

    bool operator()(const RPCFunctionEntry& lhs,
                    std::size_t rhs) const noexcept;

    bool operator()(const RPCFunctionEntry& lhs,
                    std::string_view rhs) const noexcept;

    bool operator()(std::size_t lhs,
                    const RPCFunctionEntry& rhs) const noexcept;

    bool operator()(std::string_view lhs,
                    const RPCFunctionEntry& rhs) const noexcept;
  };

  std::size_t operator()(const RPCFunctionEntry& x) const;
  std::size_t operator()(std::size_t nameHash) const;
  std::size_t operator()(std::string_view name) const;
};

}  // namespace RemoteCall

#ifndef ATHEXMPI_RPCFUNCTIONENTRY_ICC
#include "RPCFunctionEntry.icc"
#endif

#endif  // ATHEXMPI_RPCFUNCTIONENTRY_H
