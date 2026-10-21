// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_SOACOPY_H
#define ATHDEVICEUTILS_SOACOPY_H

// Local include(s).
#include "AthDeviceUtils/SoAColumn.h"

// Project include(s).
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/DataVector.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "GaudiKernel/EventContext.h"

// VecMem include(s).
#include <vecmem/edm/view.hpp>

// System include(s).
#include <tuple>

namespace AthDevice {

/// Utility to help with copying data between xAOD and VecMem SoA containers
template <Concepts::SoAColumn... COLUMNS>
class SoACopy {

 public:
  /// Constructor with a parameter pack
  SoACopy(COLUMNS... columns);
  /// Constructor with a tuple
  SoACopy(const std::tuple<COLUMNS...>& columns);

  /// Copy data from an input xAOD container into a VecMem SoA container
  ///
  /// @param input The input xAOD container
  /// @param output The output VecMem SoA container
  /// @param ctx The event context
  /// @param mrs The memory resources tool
  /// @param copies The copies tool
  ///
  template <typename SCHEMA>
  void operator()(const SG::AuxVectorBase& input,
                  vecmem::edm::view<SCHEMA>& output, const EventContext& ctx,
                  const IMemoryResourcesTool& mrs,
                  const ICopiesTool& copies) const;

  /// Copy data from an input VecMem SoA container into an xAOD container
  ///
  /// @param input The input VecMem SoA container
  /// @param output The output xAOD container
  /// @param ctx The event context
  /// @param mrs The memory resources tool
  /// @param copies The copies tool
  ///
  template <typename SCHEMA, typename INTERFACE, typename BASE>
  void operator()(const vecmem::edm::view<SCHEMA>& input,
                  DataVector<INTERFACE, BASE>& output, const EventContext& ctx,
                  const IMemoryResourcesTool& mrs,
                  const ICopiesTool& copies) const;

 private:
  /// Columns to copy back and forth between the containers
  std::tuple<COLUMNS...> m_columns;

};  // class SoACopy

/// @name Type deduction guide(s) for the SoACopy class
/// @{

template <Concepts::SoAColumn... COLUMNS>
SoACopy(COLUMNS...) -> SoACopy<COLUMNS...>;

template <Concepts::SoAColumn... COLUMNS>
SoACopy(const std::tuple<COLUMNS...>&) -> SoACopy<COLUMNS...>;

/// @}

}  // namespace AthDevice

// Include the implementation.
#include "AthDeviceUtils/impl/SoACopy.icc"

#endif  // ATHDEVICEUTILS_SOACOPY_H
