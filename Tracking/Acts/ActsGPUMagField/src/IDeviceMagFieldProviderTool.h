/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_IDEVICEMAGFIELDPROVIDERTOOL_H
#define ACTSTRK_IDEVICEMAGFIELDPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <traccc/utils/algorithm.hpp>
#include <traccc/bfield/magnetic_field.hpp>

namespace ActsTrk {

class IDeviceMagFieldProviderTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDeviceMagFieldProviderTool, 1, 0);

  
  virtual traccc::magnetic_field getDeviceMagneticField(traccc::magnetic_field const& host_bfield) const = 0;

};

} // namespace ActsTrk

#endif // ACTSTRK_IDEVICEMAGFIELDPROVIDERTOOL_H