/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SiPropertiesTool.h"

StatusCode 
SiPropertiesTool::initialize()
{ 
  ATH_MSG_INFO("SiPropertiesTool Initialized");
  
  if ((m_detectorName != "Pixel") and (m_detectorName != "SCT") and (m_detectorName != "PLR")) {
    ATH_MSG_FATAL("Invalid detector name: " << m_detectorName.value() << ". Must be Pixel, SCT or PLR.");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_propertiesVector.initialize());

  return StatusCode::SUCCESS;
}

StatusCode 
SiPropertiesTool::finalize()
{
  return StatusCode::SUCCESS;
}

const InDet::SiliconProperties&
SiPropertiesTool::getSiProperties(const IdentifierHash& elementHash, const EventContext& ctx) const {
  SG::ReadCondHandle<InDet::SiliconPropertiesVector> handle{m_propertiesVector, ctx};
  if (handle.isValid()) {
    const InDet::SiliconPropertiesVector* vector{*handle};
    if (vector) {
      return vector->getSiProperties(elementHash);
    }
  }

  ATH_MSG_WARNING("SG::ReadCondHandle<InDet::SiliconPropertiesVector> is not valid. " << 
                  "Return default InDet::SiliconProperties for IdentifierHash for " <<
                  elementHash);
  return s_defaultProperties;
}

const InDet::SiliconProperties SiPropertiesTool::s_defaultProperties{};
