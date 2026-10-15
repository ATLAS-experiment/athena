//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MonitoredMemoryResourceTool.h"

#include "MonitoredMemoryResource.h"

namespace AthDevice {

StatusCode MonitoredMemoryResourceTool::initialize() {

  // Retrieve the upstream tool.
  ATH_CHECK(m_mrTool.retrieve());

  // Retrieve the monitoring service.
  ATH_CHECK(m_monitorSvc.retrieve());

  // Get the name of the tool's parent.
  std::string component;
  const INamedInterface* parent =
      dynamic_cast<const INamedInterface*>(this->parent());
  if (parent) {
    component = parent->name();
  } else {
    ATH_MSG_WARNING("Could not retrieve the name of the parent component");
    component = "Unknown";
  }

  // Construct the monitored resource.
  m_mr = std::make_unique<MonitoredMemoryResource>(
      m_mrTool->mr(), *m_monitorSvc, component, std::string{deviceName()});

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& MonitoredMemoryResourceTool::mr() const {

  assert(m_mr);
  return *m_mr;
}

std::string_view MonitoredMemoryResourceTool::deviceName() const {

  assert(m_mrTool.isValid());
  return m_mrTool->deviceName();
}

}  // namespace AthDevice
