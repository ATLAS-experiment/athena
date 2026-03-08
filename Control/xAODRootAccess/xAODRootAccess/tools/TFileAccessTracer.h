// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TOOLS_TFILEACCESSTRACER_H
#define XAODROOTACCESS_TOOLS_TFILEACCESSTRACER_H

// System include(s):
#include <memory>
#include <string>
#include <string_view>

namespace xAOD {

/// Helper class keeping track of the files that got accessed
///
/// This class helps in keeping track of which files get accessed during a
/// job. To be able to report about them to the DDM (Rucio) system in a way
/// that doesn't rely on the grid middleware. (So backdoor access to files
/// stored at data centres becomes visible to the DDM system.)
///
/// The class can also send information about which branches/variables got
/// accessed in the job. This it simply gets from the xAODCore code.
///
/// @c xAOD::TEvent and @c xAOD::REvent use this class to send information about
/// xAOD access to the central Rucio monitoring infrastructure. This default
/// behaviour can be influenced in two ways:
///   - Using the the public functions on the singleton instance of this
///     class;
///   - Setting the following environment variables before running a job:
///      * @c "XAOD_ACCESSTRACER_FRACTION": This is a floating point value,
///        which is the fraction of jobs that should be monitored within that
///        session.
///      * @c "XAOD_ACCESSTRACER_SERVER": This is the address of the server to
///        which the monitoring information should be sent.
///
class TFileAccessTracer {

 public:
  /// Destructor
  ~TFileAccessTracer();

  /// Access the singleton instance of this class
  static TFileAccessTracer& instance();

  /// Add information about a new file that got accessed
  void add(std::string_view fileName);

  /// The address of the server that information is sent to
  const std::string& serverAddress() const;
  /// Set the address of the server that information is sent to
  void setServerAddress(const std::string& addr);

  /// Fraction of jobs that should send monitoring information
  double monitoredFraction() const;
  /// Set the fraction of jobs that should send monitoring information
  void setMonitoredFraction(double value);

  /// Function for turning data submission on/off
  void enableDataSubmission(bool value);

 private:
  /// Default constructor
  TFileAccessTracer();

  /// Implementation data for the class
  struct Impl;
  /// Pointer to the implementation of the class
  std::unique_ptr<Impl> m_impl;

};  // class TFileAccessTracer

}  // namespace xAOD

#endif  // XAODROOTACCESS_TOOLS_TFILEACCESSTRACER_H
