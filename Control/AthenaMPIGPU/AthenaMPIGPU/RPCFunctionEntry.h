// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHENAMPIGPU_RPCFUNCTIONENTRY_H
#define ATHENAMPIGPU_RPCFUNCTIONENTRY_H

// Local include(s)
#include "AthenaKernel/ClusterMessage.h"
#include "HostDevicePtr.h"

// System include(s)
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

namespace RemoteCall {

using RPCFunctionID = std::size_t;

/// Describes an RPC argument / return value
struct RPCArgEntry {
  /// Receive destination for arguments, or source memory for results.
  const Destination destination;
  const std::type_info* valType =
      nullptr;              ///< Registered scalar or range type.
  std::size_t elementSize;  ///< Size of one transferred element in bytes.
  std::size_t align;        ///< Required buffer alignment in bytes.
  bool isRange;             ///< Whether the registered type describes a range.

  /// Describe an argument from its wrapper type.
  template <RPCMemory M, RPCSupported T>
  RPCArgEntry(RPCArg<M, T> placeholder);

  /// Describe a result from its wrapper type without taking ownership.
  template <RPCMemory M, RPCSupported T>
  RPCArgEntry(const RPCRet<M, T>& placeholder);
};

/// Holds a result allocation until the dispatcher finishes sending it.
struct RPCResultBuffer {
  void* ptr;                    ///< Address of the result data.
  std::size_t len;              ///< Number of bytes to send.
  std::size_t align;            ///< Required buffer alignment in bytes.
  std::shared_ptr<void> owner;  ///< Allocation owner and its deallocator.
};

/// Describes an RPC callable function
class RPCFunctionEntry {
 public:
  /// Invoke the registered function using received buffers and return owned
  /// results.
  using Wrapper = std::move_only_function<std::vector<RPCResultBuffer>(
      const std::vector<ClusterMessage::DataDescr>&) const>;

  /// Register a callable and derive its argument and result descriptions.
  /// @param f Function whose parameters use RPCArg and whose results use
  /// RPCRet.
  /// @param name Name used to identify the remote function.
  template <RPCCallable F>
  RPCFunctionEntry(F&& f, std::string name);

  const std::string& name() const noexcept;
  const std::vector<RPCArgEntry>& arguments() const noexcept;
  const std::vector<RPCArgEntry>& returnVals() const noexcept;

  /// Validate that the pack of argument value types matches what's expected
  template <RPCSupported... Args>
  bool validateArgTypes() const;

  /// Check the requested return count and types against the registration.
  template <RPCSupported... Rs>
  bool validateReturnTypes() const;

  /// Transfer received buffers into typed results with their original
  /// deallocators.
  /// @param returnVals Received result descriptors, consumed on successful
  /// validation.
  /// @return Owned host buffers in the registered result order.
  /// @throws std::logic_error if types, counts or buffer descriptions do not
  /// match.
  template <RPCSupported... Rs>
  std::tuple<RPCRet<Host, Rs>...> retrieve(
      std::vector<ClusterMessage::DataDescr>&& returnVals) const;

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

#ifndef ATHENAMPIGPU_RPCFUNCTIONENTRY_ICC
#include "RPCFunctionEntry.icc"
#endif

#endif  // ATHENAMPIGPU_RPCFUNCTIONENTRY_H
