/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "JSONDeviceMagFieldProviderSvc.h"
#include "StoreGate/StoreGateSvc.h"

#include "ActsGPUEvent/TracccMagField.h"

#include "traccc/io/read_magnetic_field.hpp"

#include <string>

namespace ActsTrk {

StatusCode JSONDeviceMagFieldProviderSvc::initialize()
{
  ATH_MSG_DEBUG("Initializing  device magnetic field provider service ");

  ATH_CHECK(m_deviceFieldProviderTool.retrieve());
  ATH_CHECK(m_detStore.retrieve());
  
  traccc::magnetic_field hostMagField;
  
  if (m_magFieldFile.value().empty()) {
    ATH_MSG_FATAL("MagFieldFile is empty!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Reading mag field from file: " << PathResolverFindCalibFile(m_magFieldFile.value()));

  // Construct host magnetic field
  ATH_MSG_INFO("Loading traccc magnetic field");
  traccc::io::read_magnetic_field(
        hostMagField,
        PathResolverFindCalibFile(m_magFieldFile.value()));

  ATH_MSG_INFO("Magnetic field built from file"); 

  // Construct device field
  traccc::magnetic_field deviceMagField = m_deviceFieldProviderTool->getDeviceMagneticField(hostMagField);

  // Record host and device objects
  constexpr bool allowMods = false;
  ATH_CHECK(m_detStore->record(
      std::make_unique<traccc::magnetic_field>(std::move(hostMagField)),
      m_hostMagFieldObjectName.value(), allowMods));

  ATH_CHECK(m_detStore->record(
      std::make_unique<traccc::magnetic_field>(std::move(deviceMagField)),
      m_deviceMagFieldObjectName.value(), allowMods));
  

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}


} // namespace ActsTrk