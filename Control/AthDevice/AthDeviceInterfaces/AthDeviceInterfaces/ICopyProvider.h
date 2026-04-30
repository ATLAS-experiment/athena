//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICEINTERFACES_ICOPYPROVIDER_H
#define ATHDEVICEINTERFACES_ICOPYPROVIDER_H

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

namespace AthDevice {

/// Interface for a component that provides a "copy object"
class ICopyProvider {

 public:
  /// Destructor
  virtual ~ICopyProvider() = default;

  /// Get the provided @c vemcmem::copy object
  virtual vecmem::copy& copy() const = 0;

};  // class ICopyProvider

}  // namespace AthDevice

#endif  // ATHDEVICEINTERFACES_ICOPYPROVIDER_H
