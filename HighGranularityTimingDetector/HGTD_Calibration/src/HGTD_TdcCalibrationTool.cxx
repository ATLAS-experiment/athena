/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/src/HGTD_TdcCalibrationTool.cxx
 *
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @author Yuriy Volkotrub <yuriy.volkotrub@cern.ch> (CREST integration)
 *
 * @date May, 2025
 *
 * @brief Modified to optionally read TDC calibration from conditions DB.
 */

#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/ReadCondHandle.h"

#include <cmath>
#include <numeric>

HGTD_TdcCalibrationTool::HGTD_TdcCalibrationTool(const std::string &type,
                                            const std::string &name,
                                            const IInterface *parent)
    : AthAlgTool(type, name, parent)
{}

StatusCode HGTD_TdcCalibrationTool::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  if (!std::isfinite(m_toa_bin_size.value()) ||
      m_toa_bin_size.value() <= 0.0F) {
    ATH_MSG_FATAL("Invalid TOABinSize " << m_toa_bin_size
                  << " ns; expected a finite positive value");
    return StatusCode::FAILURE;
  }

  m_fallbackBinSizes =
      std::make_shared<const ToaBinSizes>(ToaBinSizes{m_toa_bin_size.value()});

  if (m_useCondDB) {
    ATH_MSG_INFO("Will read TDC calibration from conditions DB"
                 << " (key: " << m_calibDataKey.key() << ")");
    ATH_CHECK(m_calibDataKey.initialize());
  } else {
    ATH_MSG_INFO("Using configured TOABinSize = " << m_toa_bin_size
                 << " ns (conditions DB disabled)");
    ATH_CHECK(m_calibDataKey.initialize(false));
  }

  return StatusCode::SUCCESS;
}

HGTD_TdcCalibrationTool::ToaBinSizes
HGTD_TdcCalibrationTool::readToaBinSizes(
    const CondAttrListCollection& attrListColl) const {
  if (attrListColl.size() == 0 ||
      attrListColl.size() > HGTD::TOA_OVERLFLOW_MASK) {
    ATH_MSG_FATAL("Expected between 1 and " << HGTD::TOA_OVERLFLOW_MASK
                  << " TOA calibration channels, received "
                  << attrListColl.size());
    throw GaudiException("Invalid HGTD TDC calibration vector size",
                         name(), StatusCode::FAILURE);
  }

  ToaBinSizes toaBinSizes;
  toaBinSizes.reserve(attrListColl.size());
  unsigned int expectedChannel = 0;
  for (const auto& [channel, attrList] : attrListColl) {
    if (channel != expectedChannel) {
      ATH_MSG_FATAL("Expected TOA calibration channel " << expectedChannel
                    << " but found channel " << channel);
      throw GaudiException("Non-contiguous HGTD TDC calibration channels",
                           name(), StatusCode::FAILURE);
    }
    if (!attrList.exists("toa_bin_size") ||
        attrList["toa_bin_size"].isNull()) {
      ATH_MSG_FATAL("Attribute 'toa_bin_size' is missing or null in channel "
                    << channel);
      throw GaudiException("Missing HGTD TDC calibration payload",
                           name(), StatusCode::FAILURE);
    }

    const coral::Attribute& attribute = attrList["toa_bin_size"];
    if (attribute.specification().type() != typeid(float)) {
      ATH_MSG_FATAL("Expected Float toa_bin_size in channel " << channel
                    << " but found " << attribute.specification().typeName());
      throw GaudiException("Invalid HGTD TDC calibration payload type",
                           name(), StatusCode::FAILURE);
    }

    const float toaBinSize = attribute.data<float>();
    if (!std::isfinite(toaBinSize) || toaBinSize <= 0.0F) {
      ATH_MSG_FATAL("Invalid toa_bin_size " << toaBinSize
                    << " ns in channel " << channel
                    << "; expected finite positive values");
      throw GaudiException("Invalid HGTD TDC calibration value",
                           name(), StatusCode::FAILURE);
    }
    toaBinSizes.push_back(toaBinSize);
    ++expectedChannel;
  }

  return toaBinSizes;
}

std::shared_ptr<const HGTD_TdcCalibrationTool::ToaBinSizes>
HGTD_TdcCalibrationTool::getToaBinSizes() const {
  if (!m_useCondDB) {
    return m_fallbackBinSizes;
  }

  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<CondAttrListCollection> calibHandle{m_calibDataKey, ctx};
  if (!calibHandle.isValid()) {
    ATH_MSG_FATAL("Could not retrieve " << m_calibDataKey.key()
                  << " from conditions store");
    throw GaudiException("Invalid HGTD TDC calibration conditions handle",
                         name(), StatusCode::FAILURE);
  }

  const CondAttrListCollection* source = calibHandle.cptr();
  if (source == nullptr) {
    ATH_MSG_FATAL("Conditions object " << m_calibDataKey.key() << " is null");
    throw GaudiException("Null HGTD TDC calibration conditions object",
                         name(), StatusCode::FAILURE);
  }

  std::scoped_lock lock{*m_cacheMutex.get(ctx)};
  CalibrationCache* cache = m_cache.get(ctx);
  if (cache->source != source) {
    auto toaBinSizes =
        std::make_shared<const ToaBinSizes>(readToaBinSizes(*source));
    cache->source = source;
    cache->toaBinSizes = std::move(toaBinSizes);

    if (cache->toaBinSizes->size() == 1) {
      const float toaBinSize = cache->toaBinSizes->front();
      ATH_MSG_INFO("Read toa_bin_size = " << toaBinSize
                   << " ns (" << toaBinSize * 1000
                   << " ps) from conditions DB");
    } else {
      ATH_MSG_INFO("Read " << cache->toaBinSizes->size()
                   << " TOA bin sizes from conditions DB");
    }
  }

  return cache->toaBinSizes;
}

float HGTD_TdcCalibrationTool::activeWindowUpperBound(const InDetDD::SolidStateDetectorElementBase* element) const{

  //NB this "expected time" will change once we need to follow the beamspot!!
  float hit_time_expected = element->center().norm() / Gaudi::Units::c_light;

  //Should never happen, but testing anyway
  if(hit_time_expected > m_lhc_rise_edge + m_active_window/2){
    ATH_MSG_DEBUG("expected hit above calibration range");
    return m_lhc_rise_edge.value() + m_active_window;
  }
  else if(hit_time_expected < m_lhc_rise_edge - 1.5*m_active_window){
    ATH_MSG_DEBUG("expected hit below calibration range");
    return m_lhc_rise_edge.value() - m_active_window;
  }

  float window_walk = (hit_time_expected + m_active_window/2) - m_lhc_rise_edge;

  int window_steps;
  float window_upper_bound;

  if(window_walk > m_ps_large_step){
    window_steps = (window_walk - m_ps_large_step)/m_ps_small_step;
    window_upper_bound = m_lhc_rise_edge.value() + m_ps_large_step + window_steps*m_ps_small_step;
  }
  else if(window_walk < - m_ps_large_step){
    window_steps = (window_walk + m_ps_large_step)/m_ps_small_step;
    window_upper_bound = m_lhc_rise_edge.value() - m_ps_large_step + window_steps*m_ps_small_step;
  }
  else{
    window_steps = window_walk/m_ps_small_step;
    window_upper_bound = m_lhc_rise_edge.value() + window_steps*m_ps_small_step;
  }

  return window_upper_bound;
}


uint8_t HGTD_TdcCalibrationTool::Time2TOA(const InDetDD::SolidStateDetectorElementBase* element, float hit_time) const {

  float  window_upper_bound = activeWindowUpperBound(element);
  const std::shared_ptr<const ToaBinSizes> toaBinSizes = getToaBinSizes();

  //TOA time will be the distance between hit_time and upper bound of active window
  float tdc_time =  window_upper_bound - hit_time;

  // Check if hit is within the measurement window, if not return overflow flag
  if( tdc_time < 0 || tdc_time > 2.5){
    ATH_MSG_DEBUG("charge at " << hit_time
                                << " outside of TOA TDC range ["
                                << window_upper_bound - 2.5 << ", "
                                << window_upper_bound << "]" );
   return HGTD::TOA_OVERLFLOW_MASK;
  }

  uint8_t toa = HGTD::TOA_OVERLFLOW_MASK;
  if (toaBinSizes->size() == 1) {
    const unsigned int toaCode = tdc_time / toaBinSizes->front();
    if (toaCode < HGTD::TOA_OVERLFLOW_MASK) {
      toa = static_cast<uint8_t>(toaCode);
    }
  } else {
    float upperBinEdge = 0.0F;
    for (std::size_t index = 0; index < toaBinSizes->size(); ++index) {
      upperBinEdge += (*toaBinSizes)[index];
      if (tdc_time < upperBinEdge) {
        toa = static_cast<uint8_t>(index);
        break;
      }
    }
  }

  if (checkTOAoverflow(toa)) {
    ATH_MSG_DEBUG("charge at " << hit_time
                  << " outside of configured TOA bin range");
    return toa;
  }

  ATH_MSG_DEBUG("hit time: "<< hit_time <<
    " hit time digitized: " << toa2Time(element, toa, *toaBinSizes) <<
    " TOA: " <<  static_cast<unsigned int>(toa));

  return toa;

}

float HGTD_TdcCalibrationTool::TOA2Time(const InDetDD::SolidStateDetectorElementBase* element, uint8_t toa) const{
  const std::shared_ptr<const ToaBinSizes> toaBinSizes = getToaBinSizes();
  return toa2Time(element, toa, *toaBinSizes);
}

float HGTD_TdcCalibrationTool::toa2Time(
    const InDetDD::SolidStateDetectorElementBase* element, uint8_t toa,
    const ToaBinSizes& toaBinSizes) const {
  float tdcTime = 0.0F;
  if (toaBinSizes.size() == 1) {
    tdcTime = (toa + 0.5F) * toaBinSizes.front();
  } else {
    if (toa >= toaBinSizes.size()) {
      ATH_MSG_FATAL("TOA code " << static_cast<unsigned int>(toa)
                    << " is outside calibration vector of size "
                    << toaBinSizes.size());
      throw GaudiException("TOA code outside HGTD calibration vector",
                           name(), StatusCode::FAILURE);
    }
    tdcTime = std::accumulate(toaBinSizes.begin(),
                              toaBinSizes.begin() + toa, 0.0F);
    tdcTime += 0.5F * toaBinSizes[toa];
  }

  // Using middle of bin as estimate to recover the digitized time
  return activeWindowUpperBound(element) - tdcTime;
}
