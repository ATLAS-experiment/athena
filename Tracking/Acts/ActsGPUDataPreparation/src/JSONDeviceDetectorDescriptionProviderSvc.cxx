/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "JSONDeviceDetectorDescriptionProviderSvc.h"
#include "StoreGate/StoreGateSvc.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"

#include "traccc/io/read_detector_description.hpp"
#include "traccc/io/data_format.hpp"

#include "vecmem/utils/copy.hpp"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace ActsTrk {

StatusCode JSONDeviceDetectorDescriptionProviderSvc::initialize()
{
  ATH_MSG_DEBUG("Initializing  device detector description provider service ");

  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_copy.retrieve());

  std::unique_ptr<traccc::detector_design_description::host>     hostDesign;
  std::unique_ptr<traccc::detector_conditions_description::host> hostCond;
  std::unique_ptr<traccc::detector_design_description::buffer>    deviceDesign;
  std::unique_ptr<traccc::detector_conditions_description::buffer> deviceCond;

  ATH_CHECK(loadIdMaps());
  auto copy = m_copy->copy(EventContext{});
  ATH_CHECK(buildFromFile(m_hostMR->mr(), m_deviceMR->mr(), *copy, hostDesign, hostCond, deviceDesign, deviceCond));

  // Record device and host objects
  // Host objects are needed for EDM conversions
  ATH_CHECK(m_detStore->record(std::move(deviceDesign), m_deviceDesignObjectName.value()));
  ATH_CHECK(m_detStore->record(std::move(deviceCond), m_deviceCondObjectName.value()));
  ATH_CHECK(m_detStore->record(std::move(hostDesign), m_hostDesignObjectName.value()));
  ATH_CHECK(m_detStore->record(std::move(hostCond), m_hostCondObjectName.value()));

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

const std::unordered_map<uint64_t, Identifier>& JSONDeviceDetectorDescriptionProviderSvc::detrayToAthenaMap() const {
 return m_detrayToAthena;
}

const std::unordered_map<Identifier, uint64_t>& JSONDeviceDetectorDescriptionProviderSvc::athenaToDetrayMap() const {
 return m_athenaToDetray;
}

StatusCode JSONDeviceDetectorDescriptionProviderSvc::loadIdMaps()
{
  if (m_mapFile.value().empty()) {
    ATH_MSG_FATAL("MapFile not set — detray<->Athena maps will be empty");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Loading detray<->Athena map from "
                       << m_mapFile.value());

  std::ifstream mapFile(PathResolverFindCalibFile(m_mapFile.value()));
  if (!mapFile.is_open()) {
    ATH_MSG_FATAL("Cannot open map file: " << m_mapFile.value());
    return StatusCode::FAILURE;
  }

  std::string line;
  while (std::getline(mapFile, line)) {
    if (line.empty()) continue;
    std::stringstream ss(line);
    std::string athenaStr, detrayStr;
    if (!std::getline(ss, athenaStr, ',') ||
        !std::getline(ss, detrayStr, ',')) continue;

    if (athenaStr.empty()) {
        ATH_MSG_ERROR("Empty Athena identifier string in map file — skipping");
        return StatusCode::FAILURE;
    }
    Identifier athenaId;
    athenaId.set(athenaStr);

    uint64_t detrayId = 0;
    try {
        detrayId = std::stoull(detrayStr);
    } catch (const std::exception& e) {
        ATH_MSG_ERROR("Failed to parse detray identifier '" << detrayStr << "': " << e.what());
        return StatusCode::FAILURE;
    }

    m_athenaToDetray[athenaId] = detrayId;
    m_detrayToAthena[detrayId] = athenaId;
  }

  ATH_MSG_INFO("Loaded " << m_athenaToDetray.size()
                       << " detray<->Athena module mappings");
  return StatusCode::SUCCESS;
}

StatusCode JSONDeviceDetectorDescriptionProviderSvc::buildFromFile(
    std::pmr::memory_resource& hostMR,
    std::pmr::memory_resource& deviceMR,
    const vecmem::copy& copy,
    std::unique_ptr<traccc::detector_design_description::host>& hostDesign,
    std::unique_ptr<traccc::detector_conditions_description::host>& hostCond,
    std::unique_ptr<traccc::detector_design_description::buffer>& deviceDesign,
    std::unique_ptr<traccc::detector_conditions_description::buffer>& deviceCond)
{
  if (m_geometryFile.value().empty() ||
      m_digitizationFile.value().empty() ||
      m_conditionsFile.value().empty()) {
    ATH_MSG_FATAL("GeometryFile, " << m_geometryFile.value() <<
                            ", DigitizationFile, " << m_digitizationFile.value() << " or ConditionsFile, " << m_conditionsFile.value() << ", is empty!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Reading detector description from files:"
      << "  geometry:     " << m_geometryFile.value()
      << ",  digitization: " << m_digitizationFile.value()
      << ",  conditions:   " << m_conditionsFile.value());

  hostDesign = std::make_unique<traccc::detector_design_description::host>(hostMR);
  hostCond   = std::make_unique<traccc::detector_conditions_description::host>(hostMR);

  traccc::io::read_detector_description(
      *hostDesign, *hostCond,
      PathResolverFindCalibFile(m_geometryFile.value()),
      PathResolverFindCalibFile(m_digitizationFile.value()),
      PathResolverFindCalibFile(m_conditionsFile.value()),
      traccc::data_format::json);

  ATH_MSG_DEBUG(hostDesign->size() << " design entries, "
                << hostCond->size() << " conditions entries");

  // Copy design to device
  std::vector<unsigned int> sizes;
  sizes.reserve(hostDesign->size());
  for (std::size_t i = 0; i < hostDesign->size(); ++i) {
    const auto& e = hostDesign->at(i);
    sizes.push_back(static_cast<unsigned int>(
        std::max(e.bin_edges_x().size(), e.bin_edges_y().size())));
  }

  deviceDesign =
        std::make_unique<traccc::detector_design_description::buffer>(
          sizes, deviceMR, &hostMR,
          vecmem::data::buffer_type::resizable);
  copy.setup(*deviceDesign)->wait();
  copy(vecmem::get_data(*hostDesign), *deviceDesign)->wait();

  // Copy conditions to device
  deviceCond =
      std::make_unique<traccc::detector_conditions_description::buffer>(
          static_cast<traccc::detector_conditions_description::buffer::size_type>(
              hostCond->size()),
          deviceMR);
  copy.setup(*deviceCond)->wait();
  copy(vecmem::get_data(*hostCond), *deviceCond)->wait();

  ATH_MSG_INFO("Detector description built from files");
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk