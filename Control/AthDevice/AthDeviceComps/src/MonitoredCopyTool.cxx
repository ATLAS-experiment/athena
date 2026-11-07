//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MonitoredCopyTool.h"

#include "MonitoredCopy.h"

namespace AthDevice {

StatusCode MonitoredCopyTool::initialize() {

  // Retrieve the upstream tool.
  ATH_CHECK(m_copyTool.retrieve());

  // Retrieve the monitoring service.
  ATH_CHECK(m_monitorSvc.retrieve());

  // Get the name of the tool's parent.
  const INamedInterface* parent =
      dynamic_cast<const INamedInterface*>(this->parent());
  if (parent) {
    m_component = parent->name();
  } else {
    ATH_MSG_WARNING("Could not retrieve the name of the parent component");
    m_component = "Unknown";
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::shared_ptr<const vecmem::copy> MonitoredCopyTool::copy(
    const EventContext& ctx) const {

  assert(m_copyTool.isValid());
  return std::make_shared<MonitoredCopy>(m_copyTool->copy(ctx), *m_monitorSvc,
                                         m_component,
                                         std::string{m_copyTool->flavor()});
}

std::string_view MonitoredCopyTool::flavor() const {

  assert(m_copyTool.isValid());
  return m_copyTool->flavor();
}

}  // namespace AthDevice
