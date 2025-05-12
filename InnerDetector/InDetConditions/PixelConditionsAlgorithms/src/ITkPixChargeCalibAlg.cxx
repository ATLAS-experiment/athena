/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixChargeCalibAlg.h"
#include "Identifier/IdentifierHash.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h" //Diode types
#include "PixelConditionsData/ChargeCalibParameters.h" //LegacyFitParameters, LinearFitParameters, Thresholds, Resolutions
#include "PixelConditionsData/ChargeCalibrationBundle.h"
#include "PixelConditionsData/PixelChargeCalibUtils.h"

#include "IChargeCalibrationParser.h"
#include "GaudiKernel/EventIDRange.h"
//
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>


using  PixelChargeCalib::getBecAndLayer;
using  PixelChargeCalib::numChipsAndTechnology;

using namespace PixelChargeCalib; //containing LegacyFitParameters etc
using InDetDD::enum2uint;

namespace{
  //default values of ITk thresholds, hard coded for now
  Thresholds 
  defaultThresholds(int bec, int layer){
    static const Thresholds t{600, 24, 75, 1000}; //value, sigma, noise, inTimeThresh
    static const Thresholds ec0{600, 24, 75, 1500};
    static const Thresholds b0{900, 36, 110, 1000}; //different for first layer of barrel
    if (layer == 0){
      return bec==0 ? b0 : ec0;
    }
    return t;
  }
  
  LegacyFitParameters 
  defaultLegacyParameters(int bec, int layer){
    // the first parameter should match the threshold
    static const LegacyFitParameters t{14.0f, -600.f, 4500.f, 14.0f};
    static const LegacyFitParameters ec0{14.0f, -600.f, 4500.f, 14.0f};
    static const LegacyFitParameters b0{14.0f, -900.f, 4000.f, 14.0f}; //different for first layer of barrel
    if (layer == 0){
      return bec==0 ? b0 : ec0;
    }
    return t;
  }
  
  Resolutions
  defaultResolutions(){
    static const Resolutions r{0.4, 0.02};
    return r;
  }
  
  EventIDRange
  infiniteEventRange(){
    const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
    const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
    return EventIDRange{start, stop};
  }
}


ITkPixChargeCalibAlg::ITkPixChargeCalibAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator){
   
}

StatusCode ITkPixChargeCalibAlg::initialize() {
  ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));
  ATH_CHECK(m_pixelDetEleCollKey.initialize());
  ATH_CHECK(m_writeKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode ITkPixChargeCalibAlg::execute(const EventContext& ctx) const {

  SG::WriteCondHandle<PixelChargeCalibCondData> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
  if (not pixelDetEleHandle.isValid() or elements==nullptr) {
    ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }
  static constexpr std::array<InDetDD::PixelDiodeType, enum2uint(InDetDD::PixelDiodeType::N_DIODETYPES)> diodeTypes
    = {InDetDD::PixelDiodeType::NORMAL, InDetDD::PixelDiodeType::LONG, InDetDD::PixelDiodeType::GANGED, InDetDD::PixelDiodeType::LARGE};
  
  // Construct the output Cond Object and fill it in
  auto pChargeCalibData = std::make_unique<PixelChargeCalibCondData>(m_pixelID->wafer_hash_max());
  //
  for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    const auto & [barrel_ec, layer] = getBecAndLayer(m_pixelID, wafer_hash);
    const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
    const auto & [numFE, technology] = numChipsAndTechnology(element);
    const std::vector<Thresholds> allDefaultThresholds(numFE, defaultThresholds(barrel_ec, layer));
    //
    const std::vector<LegacyFitParameters> allDefaultFitParams(numFE, defaultLegacyParameters(barrel_ec, layer));
    //
    const std::vector<LinearFitParameters> allDefaultLinearParams(numFE, LinearFitParameters{0.0f, 0.0f});
    //
    for (InDetDD::PixelDiodeType type : diodeTypes) {
      pChargeCalibData -> setThresholds(type, moduleHash, allDefaultThresholds);
      pChargeCalibData -> setLegacyFitParameters(type, moduleHash, allDefaultFitParams);
      pChargeCalibData -> setLinearFitParameters(type, moduleHash, allDefaultLinearParams);
    }
    pChargeCalibData -> setTotResolutions(moduleHash, std::vector<Resolutions>(numFE, defaultResolutions()));
  }
  
  const auto & rangeW = infiniteEventRange();
  if (writeHandle.record(rangeW, std::move(pChargeCalibData)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelChargeCalibCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}

