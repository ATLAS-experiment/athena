// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TPYSTORE_H
#define XAODROOTACCESS_TPYSTORE_H

// Local include(s).
#include "xAODRootAccess/TStore.h"

// System include(s).
#include <string>

namespace xAOD {

/// Python interface to @c xAOD::TStore
///
/// In order to make it possible to record objects that are created in
/// Python, into an @c xAOD::TStore object (to be able to pass it to some tools
/// that expect the object to be in the event for instance), this class
/// extends the @c xAOD::TStore object with some non-template functions.
/// Functions that are inconvenient to use from C++, but which allow for
/// much more flexibility in PyROOT.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class TPyStore : public TStore {

 public:
  /// Inherit the constructor(s) of @c xAOD::TStore
  using TStore::TStore;

  /// Function checking if an object with a given key and type is available
  bool pyContains(const std::string& key, const std::string& type) const;
  /// Function checking if an object with a given key and type is constant
  bool pyIsConst(const std::string& key, const std::string& type) const;

  /// Record an object into the store in a typeless way
  StatusCode pyRecord(void* obj, const std::string& key,
                      const std::string& type);

  /// Print the contents of the store using a Python friendly function
  void dump() const;

};  // class TPyStore

}  // namespace xAOD

#endif  // XAODROOTACCESS_TPYSTORE_H
