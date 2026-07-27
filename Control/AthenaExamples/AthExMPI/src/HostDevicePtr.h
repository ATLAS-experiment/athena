// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
/// These are simple wrapper classes used to indicate that a pointer
/// points to either host memory or device memory
#ifndef ATHEXMPI_HOSTDEVICEPTR_H
#define ATHEXMPI_HOSTDEVICEPTR_H

// Boost include(s)
#include <boost/callable_traits.hpp>
#include <boost/mp11.hpp>

// System include(s)
#include <tuple>
#include <type_traits>

namespace RemoteCall {

/// Describe a type supported as input or output for RPC
/// Additionally, the type should not contain any pointers as these will not be
/// meaningful on the remote end
template <typename T>
concept RPCSupported = std::is_trivially_copyable_v<T> and
                       std::is_trivially_default_constructible_v<T>;

/// Wrap a host pointer argument
template <RPCSupported T>
struct HostPtr {
  T* ptr = nullptr;
};

/// Wrap a device pointer argument
template <RPCSupported T>
struct DevicePtr {
  T* ptr = nullptr;
};

/// Valid argument types for RPC functions
// mp_similar checks that the wrapping template matches between its arguments
template <typename T>
concept RPCArg = boost::mp11::mp_similar<HostPtr<int>, T>::value or
                 boost::mp11::mp_similar<DevicePtr<int>, T>::value;

/// Utility for MP11
template <typename T>
using is_RPCArg = std::bool_constant<RPCArg<T>>;

/// Extract contained pointer type from HostPtr / DevicePtr
template <RPCArg T>
using ptr_t = decltype(T::ptr);

/// Valid RPC callables
// - Each argument must be an RPCArg
// - Return value must be either an RPCArg, or a tuple of RPCArgs
template <typename C>
concept RPCCallable =
    boost::mp11::mp_all_of<boost::callable_traits::args_t<C>,
                           is_RPCArg>::value and
    not boost::mp11::mp_empty<boost::callable_traits::args_t<C>>::value and
    (RPCArg<boost::callable_traits::return_type_t<C>> or
     (boost::mp11::mp_similar<
          std::tuple<int>, boost::callable_traits::return_type_t<C>>::value and
      boost::mp11::mp_all_of<boost::callable_traits::return_type_t<C>,
                             is_RPCArg>::value and
      not boost::mp11::mp_empty<
          boost::callable_traits::return_type_t<C>>::value));

}  // namespace RemoteCall

#endif  // ATHEXMPI_HOSTDEVICEPTR_H
