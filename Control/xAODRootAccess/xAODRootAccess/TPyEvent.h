// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TPYEVENT_H
#define XAODROOTACCESS_TPYEVENT_H

// Local include(s).
#include "xAODRootAccess/TEvent.h"

// Python include(s).
#include <Python.h>

// System include(s).
#include <string>

namespace xAOD {

/// Python interface to @c xAOD::TEvent
///
/// In order to make it possible to record objects that are created in
/// Python, into an @c xAOD::TEvent object (in order to record selected objects
/// into an output file for instance), this class extends the @c xAOD::TEvent
/// object with some non-template functions. Functions that are inconvenient
/// to use from C++, but which allow for much more flexibility in PyROOT.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class TPyEvent : public TEvent {

 public:
  /// Constructor with an access mode
  ///
  /// It has to be implemented explicitly, as PyROOT is having some issues
  /// with callin non-default constructors inherited from the base class.
  ///
  TPyEvent(EAuxMode mode = kClassAccess);

  /// Return the object with a given key as a PyObject.
  PyObject* pyRetrieve(const std::string& key);

  /// Function checking if an object is available from the store
  bool pyContains(const std::string& key, const std::string& type);
  /// Function checking if an object is already in memory
  bool pyTransientContains(const std::string& key,
                           const std::string& type) const;

  /// Add an output object to the event
  StatusCode pyRecord(void* obj, const std::string& key,
                      const std::string& type);

};  // class TPyEvent

}  // namespace xAOD

#endif  // XAODROOTACCESS_TPYEVENT_H
