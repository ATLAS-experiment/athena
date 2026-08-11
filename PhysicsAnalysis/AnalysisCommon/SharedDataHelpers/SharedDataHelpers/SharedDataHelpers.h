/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SHARED_DATA_HELPERS_SHARED_DATA_HELPERS_H
#define SHARED_DATA_HELPERS_SHARED_DATA_HELPERS_H

#include <AsgMessaging/StatusCode.h>
#include <functional>
#include <memory>
#include <typeinfo>

namespace asg
{
  namespace detail
  {
    /// @brief the type-erased version of @ref getMakeShared
    StatusCode getMakeSharedDataVoid (const std::string& name, const std::type_info& type, std::shared_ptr<const void>& data, const std::function<StatusCode (std::shared_ptr<const void>&)>& generator);
  }

  /// @brief a helper for sharing calibration data between different components
  ///
  /// This function is meant to allow different CP Tool instances (as
  /// well as other components) to share calibration data between them.
  /// The basic use case is that the tools generally read their
  /// calibration data from a file (usually as a `TH2` or similar), and
  /// then if multiple instances of the tool are configured with the
  /// same or similar settings, the data can be shared between them.
  ///
  /// There are a couple of alternative approaches:
  /// - Just have each tool hold its data independently: This is the
  ///   traditional approach, and it works fine if there is usually just
  ///   one tool instance, or if the calibration data is small. However,
  ///   if there are multiple instances of the tool and the calibration
  ///   data is large this can waste a lot of memory.
  /// - Make a custom service to share the data for that specific tool:
  ///   This works fine and can add extra value, but it can also be
  ///   complete overkill if all you want is share the same `TH2`
  ///   object you read straight from a file.
  /// - Make the tool public: This is the traditional pre-AthenaMT
  ///   approach. The main issue is that it only works if the tools have
  ///   no declared data dependencies, which they are encouraged to have
  ///   for scheduling. Also, in AnalysisBase there isn't the same
  ///   deduplication mechanism as for ComponentAccumulator.
  ///
  ///
  /// The way it works is that each piece of data to be shared is
  /// assigned a unique name, and if the data already exists it gets
  /// immediately returned as a `std::shared_ptr`. If it isn't already
  /// known the service calls a provided generator function to read it.
  ///
  /// The assumption is that the caller will then cache the
  /// `std::shared_ptr` for as long as they need them, not try to
  /// re-read it on every use. That's no change from present practice,
  /// in which you also wouldn't reread calibration data each time a
  /// tool is called.
  ///
  ///
  /// The basic usage would look something like this:
  /// ```
  /// std::shared_ptr<const TH2> my_data;
  /// ASSERT_SUCCESS (getMakeSharedData ("my_data", my_data, [] (std::shared_ptr<const TH2>& ptr) {
  ///   ptr = ... // read the data from a file or generate it in some other way
  ///   return StatusCode::SUCCESS;
  /// }));
  /// ```

  template<typename T,typename Function>
    requires requires (Function func, std::shared_ptr<const T>& ptr) { { func(ptr) } -> std::convertible_to<StatusCode>; }
  StatusCode getMakeSharedData (const std::string& name, std::shared_ptr<const T>& data, Function&& generator)
  {
    std::shared_ptr<const void> void_data;
    if (detail::getMakeSharedDataVoid (name, typeid(T), void_data, [&generator](std::shared_ptr<const void>& cache_data) {
      std::shared_ptr<const T> typed_data;
      if (StatusCode sc = generator(typed_data); sc.isFailure())
        return StatusCode::FAILURE;
      cache_data = typed_data;
      return StatusCode::SUCCESS;
    }).isFailure())
      return StatusCode::FAILURE;

    data = std::static_pointer_cast<const T>(void_data);
    return StatusCode::SUCCESS;
  }
}

#endif
