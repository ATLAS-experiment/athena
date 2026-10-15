//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MemoryMonitorSvc.h"

// System include(s).
#include <algorithm>
#include <format>
#include <iomanip>

namespace {

/// Helper for printing byte amounts in a nice human-readable format.
std::string formatBytes(std::size_t bytes) {

  if (bytes < 1024) {
    return std::format("{} B", bytes);
  } else if (bytes < 1024 * 1024) {
    return std::format("{:.2f} kB", static_cast<double>(bytes) / 1024.0);
  } else if (bytes < 1024 * 1024 * 1024) {
    return std::format("{:.2f} MB",
                       static_cast<double>(bytes) / (1024.0 * 1024.0));
  } else {
    return std::format("{:.2f} GB",
                       static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
  }
}

}  // namespace

namespace AthDevice {

StatusCode MemoryMonitorSvc::finalize() {

  // Find the longest component name, to make the output more readable.
  std::size_t longestComponentName = 0u;
  auto helper = [&longestComponentName](const auto& map) {
    for (const auto& [key, bytes] : map) {
      longestComponentName =
          std::max(longestComponentName, key.component.size());
    }
  };
  helper(m_allocations);
  helper(m_deallocations);

  // Print the memory allocation monitoring data.
  ATH_MSG_INFO("Memory allocations:");
  std::string currentDevice = "";
  for (const auto& [key, bytes] : m_allocations) {
    if (key.device != currentDevice) {
      ATH_MSG_INFO("  - " << key.device);
      currentDevice = key.device;
    }
    ATH_MSG_INFO("    * " << std::setw(longestComponentName) << std::left
                          << key.component << " : " << std::setw(15)
                          << std::right << formatBytes(bytes));
  }

  // Print the memory deallocation monitoring data.
  ATH_MSG_INFO("Memory deallocations:");
  currentDevice = "";
  for (const auto& [key, bytes] : m_deallocations) {
    if (key.device != currentDevice) {
      ATH_MSG_INFO("  - " << key.device);
      currentDevice = key.device;
    }
    ATH_MSG_INFO("    * " << std::setw(longestComponentName) << std::left
                          << key.component << " : " << std::setw(15)
                          << std::right << formatBytes(bytes));
  }

  // TODO: Print the memory copy monitoring data.

  // Return gracefully.
  return StatusCode::SUCCESS;
}

void MemoryMonitorSvc::memoryAllocated(const std::string& component,
                                       const std::string& device,
                                       std::size_t bytes) {

  std::lock_guard lock(m_allocationsMutex);
  m_allocations[{component, device}] += bytes;
}

void MemoryMonitorSvc::memoryDeallocated(const std::string& component,
                                         const std::string& device,
                                         std::size_t bytes) {

  std::lock_guard lock(m_deallocationsMutex);
  m_deallocations[{component, device}] += bytes;
}

void MemoryMonitorSvc::memoryCopied(const std::string& component,
                                    vecmem::copy::type::copy_type type,
                                    std::size_t bytes) {

  std::lock_guard lock(m_copiesMutex);
  m_copies[{component, type}] += bytes;
}

}  // namespace AthDevice
