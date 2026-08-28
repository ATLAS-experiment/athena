// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ACTSGPUINTERFACES_DEVICEALGORITHMT_H
#define ACTSGPUINTERFACES_DEVICEALGORITHMT_H

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

// System include(s).
#include <memory>

namespace ActsTrk {

/// Convenience type for the "algorithm provider" tools
template <typename ALGORITHM_TYPE>
class DeviceAlgorithmT {

 public:
  /// Type of the held algorithm
  using algorithm_type = ALGORITHM_TYPE;

  /// Constructor with the algorithm and the copy object that it uses
  DeviceAlgorithmT(std::shared_ptr<const vecmem::copy> copy,
                   std::shared_ptr<const algorithm_type> algorithm)
      : m_copy(copy), m_algorithm(algorithm) {}
  /// Copy constructor
  DeviceAlgorithmT(const DeviceAlgorithmT&) = default;
  /// Move constructor
  DeviceAlgorithmT(DeviceAlgorithmT&&) = default;

  /// Copy assignment operator
  DeviceAlgorithmT& operator=(const DeviceAlgorithmT&) = default;
  /// Move assignment operator
  DeviceAlgorithmT& operator=(DeviceAlgorithmT&&) = default;

  /// Get the copy object that the algorithm uses
  const vecmem::copy& copy() const { return *m_copy; }

  /// Get the algorithm itself
  const algorithm_type& algorithm() const { return *m_algorithm; }

  /// Access the algorithm with a "pointer dereferencing formalism"
  const algorithm_type& operator*() const { return *m_algorithm; }
  /// Access the algorithm with a "pointer formalism"
  const algorithm_type* operator->() const { return m_algorithm.get(); }

 private:
  /// The copy object that the algorithm uses
  std::shared_ptr<const vecmem::copy> m_copy;
  /// The algorithm itself
  std::shared_ptr<const algorithm_type> m_algorithm;

};  // class DeviceAlgorithmT

}  // namespace ActsTrk

#endif  // ACTSGPUINTERFACES_DEVICEALGORITHMT_H
