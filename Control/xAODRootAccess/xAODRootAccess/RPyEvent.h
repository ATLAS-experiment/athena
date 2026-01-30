// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODRootAccess/RPyEvent.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2025
 * @brief Python interface to xAOD::REvent
 */


#ifndef XAODROOTACCESS_RPYEVENT_H
#define XAODROOTACCESS_RPYEVENT_H


#include "xAODRootAccess/REvent.h"
#include "Python.h"


namespace xAOD::Experimental {


/// Python interface to @c xAOD::REvent
///
/// In order to make it possible to use python objects with an
/// @c xAOD::Experimental::REvent, this class extends
/// @c xAOD::Experimental::REvent with some non-template functions
/// that are more convenient to use from Python.
///
class RPyEvent : public REvent {

 public:
  using REvent::REvent;

  /// Return the object with a given key as a PyObject.
  PyObject* pyRetrieve(const std::string& key);

  /// Function checking if an object is available from the store
  bool pyContains(const std::string& key, const std::string& type);
  /// Function checking if an object is already in memory
  bool pyTransientContains(const std::string& key,
                           const std::string& type) const;
};


} // namespace xAOD::Experimental


#endif // not XAODROOTACCESS_RPYEVENT_H
