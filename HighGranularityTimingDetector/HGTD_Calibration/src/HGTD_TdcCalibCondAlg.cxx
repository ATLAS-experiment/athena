/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/src/HGTD_TdcCalibCondAlg.cxx
 *
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch>
 *
 * @date 2025
 *
 * @brief Reads TDC calibration data from conditions DB and produces
 *        HGTD_TdcCalibData conditions object.
 */

#include "HGTD_TdcCalibCondAlg.h"

#include <cmath>
#include <exception>
#include <memory>

HGTD_TdcCalibCondAlg::HGTD_TdcCalibCondAlg(const std::string& name,
                                             ISvcLocator* pSvcLocator)
    : AthCondAlgorithm(name, pSvcLocator) {}

StatusCode HGTD_TdcCalibCondAlg::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HGTD_TdcCalibCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("execute " << name());

  SG::WriteCondHandle<HGTD_TdcCalibData> writeHandle{m_writeKey, ctx};
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey()
                  << " is already valid. Skipping.");
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
  if (!readHandle.isValid()) {
    ATH_MSG_FATAL("Could not read conditions data from " << m_readKey.key());
    return StatusCode::FAILURE;
  }

  writeHandle.addDependency(readHandle);

  const CondAttrListCollection* attrListColl = readHandle.cptr();
  if (attrListColl == nullptr) {
    ATH_MSG_FATAL("Conditions object " << m_readKey.key() << " is null");
    return StatusCode::FAILURE;
  }

  // Phase 1: single global toa_bin_size value from channel 0
  const auto chanIt = attrListColl->chanAttrListPair(0);
  if (chanIt == attrListColl->end()) {
    ATH_MSG_FATAL("Channel 0 not found in " << m_readKey.key());
    return StatusCode::FAILURE;
  }

  const coral::AttributeList& attrList = chanIt->second;
  if (!attrList.exists("toa_bin_size") || attrList["toa_bin_size"].isNull()) {
    ATH_MSG_FATAL("Attribute 'toa_bin_size' is missing or null in channel 0");
    return StatusCode::FAILURE;
  }

  float toaBinSize = 0.0F;
  try {
    toaBinSize = attrList["toa_bin_size"].data<float>();
  } catch (const std::exception& error) {
    ATH_MSG_FATAL("Could not read 'toa_bin_size' as Float: " << error.what());
    return StatusCode::FAILURE;
  }

  if (!std::isfinite(toaBinSize) || toaBinSize <= 0.0F) {
    ATH_MSG_FATAL("Invalid toa_bin_size " << toaBinSize
                  << " ns; expected a finite positive value");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Read toa_bin_size = " << toaBinSize
               << " ns (" << toaBinSize * 1000 << " ps) from conditions DB");

  auto calibData = std::make_unique<HGTD_TdcCalibData>(toaBinSize);
  if (writeHandle.record(std::move(calibData)).isFailure()) {
    ATH_MSG_FATAL("Could not record " << writeHandle.key()
                  << " with range " << writeHandle.getRange()
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Recorded HGTD_TdcCalibData with range "
               << writeHandle.getRange());

  return StatusCode::SUCCESS;
}
