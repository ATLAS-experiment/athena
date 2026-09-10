// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
/// @file
/// Wrappers for RPC arguments and results in host or device memory.
#ifndef ATHEXMPI_HOSTDEVICEPTR_H
#define ATHEXMPI_HOSTDEVICEPTR_H

// Boost include(s)
#include <boost/callable_traits.hpp>
#include <boost/mp11.hpp>

// System include(s)
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

/// Remote function registration, transport and buffer types.
namespace RemoteCall {

/// Identifies host memory.
struct Host {};
/// Identifies device memory.
struct Device {};

/// Accepts the Host and Device memory tags.
template <typename T>
concept RPCMemory = std::same_as<T, Host> || std::same_as<T, Device>;

/// A value that can be transferred as bytes.
/// Payloads must not contain pointers. These traits cannot inspect class
/// members to enforce that restriction.
template <typename T>
concept RPCValue = std::is_trivially_copyable_v<T> &&
                   std::is_trivially_default_constructible_v<T> &&
                   !std::is_const_v<T> && !std::is_volatile_v<T>;

/// A contiguous range with a known element count and supported element type.
template <typename T>
concept RPCRange =
    std::ranges::contiguous_range<T> && std::ranges::sized_range<T> &&
    RPCValue<std::ranges::range_value_t<T>>;

/// Accept a fixed-size value or a contiguous range of supported values.
template <typename T>
concept RPCSupported = RPCRange<T> || (!std::ranges::range<T> && RPCValue<T>);

/// Select the value type or range element type addressed by the buffer pointer.
template <RPCSupported T, bool = RPCRange<T>>
struct RPCDataTraits {
  using element_type = T;                 ///< Type of the transferred scalar.
  static constexpr bool isRange = false;  ///< This payload is a scalar.
};

/// Buffer traits for the elements of a contiguous range.
template <RPCSupported T>
struct RPCDataTraits<T, true> {
  using element_type =
      std::ranges::range_value_t<T>;     ///< Transferred element type.
  static constexpr bool isRange = true;  ///< This payload is a range.
};

/// Borrows the argument buffer for the duration of the remote call.
/// For a range, ptr points to the first element and size is the element count.
/// Host or Device selects the memory in which the server receives the data.
template <RPCMemory Memory, RPCSupported T>
struct RPCArg {
  using memory_type = Memory;  ///< Memory location on the server.
  using value_type = T;        ///< Registered scalar or range type.
  /// Type addressed by ptr.
  using element_type = typename RPCDataTraits<T>::element_type;
  element_type* ptr =
      nullptr;  ///< Address of the scalar or first range element.
  /// Number of elements in the buffer.
  /// Default construction supplies no range length, so the range starts empty.
  /// A scalar represents one value.
  std::size_t size = RPCDataTraits<T>::isRange ? 0 : 1;
};

/// Owns the result buffer. Host or Device describes its memory on the server.
/// The client receives results in host memory.
/// Keep ptr and size unchanged after construction.
template <RPCMemory Memory, RPCSupported T>
class RPCRet {
 public:
  using memory_type = Memory;  ///< Memory location on the server.
  using value_type = T;        ///< Registered scalar or range type.
  /// Type addressed by ptr.
  using element_type = typename RPCDataTraits<T>::element_type;

  element_type* ptr =
      nullptr;  ///< Address of the scalar or first range element.
  /// Number of elements in the buffer.
  /// Default construction supplies no range length, so the range starts empty.
  /// A scalar represents one value.
  std::size_t size = RPCDataTraits<T>::isRange ? 0 : 1;

  /// Construct an empty wrapper. Assign a result before returning a scalar.
  RPCRet() = default;

  /// Adopt a host allocation and release it with delete or delete[].
  /// @param data Scalar allocated with new, or range elements allocated with
  /// new[].
  /// @param count Element count. A scalar requires one.
  /// @throws std::invalid_argument if the pointer or count is invalid.
  RPCRet(element_type* data, std::size_t count = 1)
    requires std::same_as<Memory, Host>
      : ptr(data), size(count), m_owner(data, [](element_type* p) {
          if constexpr (RPCDataTraits<T>::isRange) {
            delete[] p;
          } else {
            delete p;
          }
        }) {
    validate();
  }

  /// Adopt an allocation from a memory resource.
  /// @param data Storage allocated by resource with alignof(element_type).
  /// @param count Element count used to determine the allocation size in bytes.
  /// @param resource Resource used for allocation and cleanup. It must remain
  /// alive until the result has been destroyed or its transmission has
  /// finished.
  /// @throws std::invalid_argument if the pointer or count is invalid.
  RPCRet(element_type* data, std::size_t count,
         std::pmr::memory_resource& resource)
      : ptr(data),
        size(count),
        m_owner(data, [&resource, count](element_type* p) {
          if (p != nullptr) {
            resource.deallocate(p, count * sizeof(element_type),
                                alignof(element_type));
          }
        }) {
    validate();
  }

  /// Adopt received storage and retain its existing deallocator.
  /// @param storage Owner of a buffer containing count elements.
  /// @param count Number of elements in the buffer.
  /// @throws std::invalid_argument if the pointer or count is invalid.
  RPCRet(std::shared_ptr<void> storage, std::size_t count)
      : ptr(static_cast<element_type*>(storage.get())),
        size(count),
        m_owner(std::move(storage), ptr) {
    validate();
  }

  /// Copying result ownership is disabled.
  RPCRet(const RPCRet&) = delete;
  /// Copy assignment is disabled.
  RPCRet& operator=(const RPCRet&) = delete;
  /// Transfer ownership from rhs and clear its pointer and element count.
  RPCRet(RPCRet&& rhs) noexcept
      : ptr(std::exchange(rhs.ptr, nullptr)),
        size(std::exchange(rhs.size, 0)),
        m_owner(std::move(rhs.m_owner)) {}
  /// Release this buffer and transfer ownership from rhs. Self-assignment is
  /// safe.
  RPCRet& operator=(RPCRet&& rhs) noexcept {
    if (this != &rhs) {
      m_owner = std::move(rhs.m_owner);
      ptr = std::exchange(rhs.ptr, nullptr);
      size = std::exchange(rhs.size, 0);
    }
    return *this;
  }

  /// Access the scalar or first range element in host memory.
  /// @pre ptr addresses a valid element.
  element_type& operator*() const
    requires std::same_as<Memory, Host>
  {
    return *ptr;
  }
  /// Return a borrowed view of the host buffer.
  /// The view remains valid only while its allocation remains owned.
  std::span<element_type> view() const
    requires std::same_as<Memory, Host>
  {
    return {ptr, size};
  }

  /// Transfer the allocation to the dispatcher and clear this wrapper.
  /// @return An owner that retains the buffer and its deallocator.
  std::shared_ptr<void> takeOwnership() && {
    ptr = nullptr;
    size = 0;
    return std::move(m_owner);
  }

 private:
  /// Check the pointer and element count.
  /// @throws std::invalid_argument if the buffer description is invalid.
  void validate() const {
    if ((!RPCDataTraits<T>::isRange && size != 1) ||
        (size != 0 && ptr == nullptr) ||
        size > std::numeric_limits<std::size_t>::max() / sizeof(element_type)) {
      throw std::invalid_argument("Invalid RPC result buffer");
    }
  }
  /// Allocation owner carrying the deallocator selected at construction.
  std::shared_ptr<element_type> m_owner;
};

/// MP11 predicate identifying RPCArg wrappers.
/// mp_similar checks that the types use the same wrapper template.
template <typename T>
using is_RPCArg = boost::mp11::mp_similar<RPCArg<Host, int>, T>;
/// MP11 predicate identifying RPCRet wrappers.
template <typename T>
using is_RPCRet = boost::mp11::mp_similar<RPCRet<Host, int>, T>;

/// Valid RPC callables.
/// Arguments must use RPCArg, and at least one argument is required.
/// Return values must use RPCRet or a non-empty tuple of RPCRet wrappers.
template <typename C>
concept RPCCallable =
    boost::mp11::mp_all_of<boost::callable_traits::args_t<C>,
                           is_RPCArg>::value &&
    !boost::mp11::mp_empty<boost::callable_traits::args_t<C>>::value &&
    (is_RPCRet<boost::callable_traits::return_type_t<C>>::value ||
     (boost::mp11::mp_similar<
          std::tuple<int>, boost::callable_traits::return_type_t<C>>::value &&
      boost::mp11::mp_all_of<boost::callable_traits::return_type_t<C>,
                             is_RPCRet>::value &&
      !boost::mp11::mp_empty<boost::callable_traits::return_type_t<C>>::value));

}  // namespace RemoteCall
#endif  // ATHEXMPI_HOSTDEVICEPTR_H
