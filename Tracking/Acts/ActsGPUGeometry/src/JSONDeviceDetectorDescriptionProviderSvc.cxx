/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "JSONDeviceDetectorDescriptionProviderSvc.h"
#include "StoreGate/StoreGateSvc.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"

#include "detray/geometry/tracking_surface.hpp"
#include "traccc/geometry/detector.hpp"          
#include "Acts/Geometry/GeometryIdentifier.hpp"

#include "traccc/io/read_detector_description.hpp"
#include "traccc/io/read_detector.hpp"
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

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());

  auto hostDesign = std::make_unique<traccc::detector_design_description::host>(*m_MRs->hostMR());
  auto hostCond   = std::make_unique<traccc::detector_conditions_description::host>(*m_MRs->hostMR());

  std::unique_ptr<traccc::detector_design_description::buffer>     deviceDesign;
  std::unique_ptr<traccc::detector_conditions_description::buffer> deviceCond; 

  auto copy = m_copy->copy(EventContext{});

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

  // Construct detector geometry
  ATH_MSG_INFO("Loading traccc detector");
  auto hostDetector = std::make_unique<traccc::host_detector>();
  traccc::io::read_detector(
      *hostDetector, *m_MRs->hostMR(),
      PathResolverFindCalibFile(m_geometryFile.value()));

  auto deviceDetector =
        std::make_unique<traccc::detector_buffer>(traccc::buffer_from_host_detector(*hostDetector, m_MRs->mainMR(), const_cast<vecmem::copy&>(*copy)));
  
  ATH_CHECK(loadIdMaps(hostDetector));      
  
  // Construct detector description
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
          sizes, m_MRs->mainMR(), m_MRs->hostMR(),
          vecmem::data::buffer_type::resizable);
  (*copy).setup(*deviceDesign)->wait();
  (*copy)(vecmem::get_data(*hostDesign), *deviceDesign)->wait();

  // Copy conditions to device
  deviceCond =
      std::make_unique<traccc::detector_conditions_description::buffer>(
          static_cast<traccc::detector_conditions_description::buffer::size_type>(
              hostCond->size()),
          m_MRs->mainMR());
  (*copy).setup(*deviceCond)->wait();
  (*copy)(vecmem::get_data(*hostCond), *deviceCond)->wait();

  ATH_MSG_INFO("Detector description built from files");

  // Record device and host objects
  // Host objects are needed for EDM conversions
  constexpr bool allowMods = false;
  ATH_CHECK(m_detStore->record(std::move(deviceDesign), m_deviceDesignObjectName.value(), allowMods));
  ATH_CHECK(m_detStore->record(std::move(deviceCond), m_deviceCondObjectName.value(), allowMods));
  ATH_CHECK(m_detStore->record(std::move(hostDesign), m_hostDesignObjectName.value(), allowMods));
  ATH_CHECK(m_detStore->record(std::move(hostCond), m_hostCondObjectName.value(), allowMods));

  ATH_CHECK(m_detStore->record(std::move(deviceDetector), m_deviceDetectorName.value(), allowMods));
  ATH_CHECK(m_detStore->record(std::move(hostDetector), m_hostDetectorName.value(), allowMods));

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode JSONDeviceDetectorDescriptionProviderSvc::loadIdMaps(const std::unique_ptr<traccc::host_detector>& hostDetector)
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

  // Pass 1: the Athena <-> detray map
  // Fill it into a temporary lookup keyed by detray id.
  std::unordered_map<uint64_t, Identifier> detrayToAthenaFromFile;

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
    athenaId.set(athenaStr);          // handles the 0x-prefixed hex correctly

    uint64_t detrayId = 0;
    try {
        detrayId = std::stoull(detrayStr);
    } catch (const std::exception& e) {
        ATH_MSG_ERROR("Failed to parse detray identifier '" << detrayStr << "': " << e.what());
        return StatusCode::FAILURE;
    }

    detrayToAthenaFromFile.emplace(detrayId, athenaId);
  }

  ATH_MSG_INFO("Read " << detrayToAthenaFromFile.size()
                        << " detray<->Athena entries from file");

  // Pass 2: walk the detray surfaces, read off the ACTS geometry id 
  // and combine with the file-based Athena lookup to fill the
  // full three-way GeometryIdMapping.
  m_idMapping = std::make_unique<ActsTrk::GeometryIdMapping>();
  const auto& itkDetector = hostDetector->as<traccc::itk_detector>();
  const std::size_t nSurfaces = itkDetector.surfaces().size();
  m_idMapping->reserve(nSurfaces);

  std::size_t nMatched = 0;
  for (const auto& surface : itkDetector.surfaces()) {
    const Acts::GeometryIdentifier acts_geom_id{surface.source};

    auto sf = detray::tracking_surface{itkDetector, surface};
    const auto detrayId = sf.identifier().value();

    std::optional<Identifier> athenaId; 
    if (auto it = detrayToAthenaFromFile.find(detrayId);
        it != detrayToAthenaFromFile.end()) {
      athenaId = it->second;
      ++nMatched;
    }

    m_idMapping->addEntry(detrayId, acts_geom_id.value(), athenaId);
  }                      

  ATH_MSG_INFO("Built GeometryIdMapping with " << m_idMapping->size()
               << " detray/ACTS surfaces, " << nMatched
               << " matched to an Athena module");
               
  ATH_CHECK(m_detStore->record(std::move(m_idMapping), m_geoIdMappingObjectName.value(), false));             

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk