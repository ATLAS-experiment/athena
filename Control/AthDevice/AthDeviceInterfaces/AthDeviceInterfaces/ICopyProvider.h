//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_ICOPYPROVIDER_H
#define ATHDEVICEINTERFACES_ICOPYPROVIDER_H

// Framework include(s).
#include "GaudiKernel/EventContext.h"

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

// System include(s).
#include <memory>

namespace AthDevice {

/// Interface for a component that provides a @c vecmem::copy object
class ICopyProvider {

 public:
  /// Destructor
  virtual ~ICopyProvider() = default;

  /// Get the provided @c vecmem::copy object
  ///
  /// @param ctx The event context for which the copy object is requested
  /// @returns The @c vecmem::copy object to use for the current event context
  ///
  virtual std::shared_ptr<const vecmem::copy> copy(
      const EventContext& ctx) const = 0;

};  // class ICopyProvider

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_ICOPYPROVIDER_H
