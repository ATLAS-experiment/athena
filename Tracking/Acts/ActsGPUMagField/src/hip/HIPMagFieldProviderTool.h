/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUMAGFIELD_HIPMAGFIELDPROVIDERTOOL_H
#define ACTSGPUMAGFIELD_HIPMAGFIELDPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "../IDeviceMagFieldProviderTool.h"

#include "traccc/hip/utils/make_magnetic_field.hpp"

#include <Gaudi/Property.h>
#include <string>

namespace ActsTrk {

/**
 * @class HIPMagFieldProviderTool
 *
 * @brief Tool providing HIP-based traccc magnetic field algorithm.
 *
 * This tool constructs the traccc device magnetic field
 * algorithm, configured to run on with HIP backend. 
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class HIPMagFieldProviderTool
    : public extends<AthAlgTool, IDeviceMagFieldProviderTool>
{
public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc device hip magnetic field
  /// @return hip magnetic field 
  virtual traccc::magnetic_field getDeviceMagneticField(traccc::magnetic_field const& host_bfield) const override;

  private:
  traccc::hip::magnetic_field_storage m_storage{traccc::hip::magnetic_field_storage::global_memory};
  Gaudi::Property<std::string> m_magFieldStorage{
    this, "MagFieldStorage", "global_memory",
    "Storage method for the HIP device magnetic field; global or texture memory"};


};

} // namespace ActsTrk

#endif // ACTSGPUMAGFIELD_HIPMAGFIELDPROVIDERTOOL_H
