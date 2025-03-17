/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelChargeCalibCondAlg.h"
#include "Identifier/IdentifierHash.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h" //Diode types
#include "PixelConditionsData/ChargeCalibParameters.h" //LegacyFitParameters, LinearFitParameters, Thresholds, Resolutions
#include "PixelConditionsData/ChargeCalibrationBundle.h"
#include "PixelConditionsData/PixelChargeCalibUtils.h"

#include "IChargeCalibrationParser.h"
#include "Run3ChargeCalibParser.h"
#include "Run2ChargeCalibParser.h"
#include "GaudiKernel/EventIDRange.h"
#include <memory>
#include <sstream>

#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>


using  PixelChargeCalib::getBecAndLayer;
using  PixelChargeCalib::numChipsAndTechnology;

using namespace PixelChargeCalib; //containing LegacyFitParameters etc
using InDetDD::enum2uint;



PixelChargeCalibCondAlg::PixelChargeCalibCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator){
   
}

StatusCode PixelChargeCalibCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelChargeCalibCondAlg::initialize()");
  ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));
  ATH_CHECK(m_pixelDetEleCollKey.initialize());
  ATH_CHECK(m_configKey.initialize());
  ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_writeKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PixelChargeCalibCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelChargeCalibCondAlg::execute()");

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

  SG::ReadCondHandle<PixelModuleData> configDataHandle(m_configKey, ctx);
  const PixelModuleData *configData = *configDataHandle;
  

  // Construct the output Cond Object and fill it in
  auto writeCdo = std::make_unique<PixelChargeCalibCondData>(m_pixelID->wafer_hash_max());
  //
  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  EventIDRange rangeW{start, stop};
  unsigned int channel_warnings=0;
  unsigned int max_channel_warnings=10;
  unsigned int min_invalid_channel=std::numeric_limits<unsigned int>::max();
  unsigned int max_invalid_channel=0;
  if (!m_readKey.empty()) {
    SG::ReadCondHandle<CondAttrListCollection> readHandle(m_readKey, ctx);
    const CondAttrListCollection* readCdo = *readHandle;
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }
    // Get the validitiy range
    if (not readHandle.range(rangeW)) {
      ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_INFO("Range of input is " << rangeW);

    std::unique_ptr<IChargeCalibrationParser> pParser{};
    for (const auto & attrList : *readCdo) {
      const CondAttrListCollection::ChanNum &channelNumber = attrList.first;
      const CondAttrListCollection::AttributeList &payload = attrList.second;
      // RUN-3 format
      if (payload.exists("data_array") and not payload["data_array"].isNull()) {
        pParser = std::make_unique<Run3ChargeCalibParser>(configData, elements, m_pixelID);
        const nlohmann::json &jsonData = nlohmann::json::parse(payload["data_array"].data<std::string>());
        for (const auto &[hash, data] : jsonData.items()) {
          const unsigned int moduleHash = std::stoul(hash);
          const ChargeCalibrationBundle &  b = pParser->parse(moduleHash, data);
          if (not b.isValid){
            ATH_MSG_FATAL("Parsing failed");
            return StatusCode::FAILURE;
          }
          writeCdo -> setAllFromBundle(moduleHash, b);
        }
      } else if (payload.exists("data") and not payload["data"].isNull()) { // RUN-2 format
        pParser = std::make_unique<Run2ChargeCalibParser>(configData, elements, m_pixelID);
        // ignore invalid channelNumbers
        // otherwise usage of e.g. CONDBR2-HLTP-2018-03 will lead to range errors.
        if (channelNumber >= m_pixelID->wafer_hash_max()) {
           min_invalid_channel = std::min(min_invalid_channel,channelNumber);
           max_invalid_channel = std::max(max_invalid_channel,channelNumber);
           if (channel_warnings++ < max_channel_warnings) {
              ATH_MSG_WARNING("Invalid module hash (COOL channel number: " << channelNumber << " !< " <<  m_pixelID->wafer_hash_max() << ")."
                              << (channel_warnings==max_channel_warnings ? " Further such warnings will not be reported." : ""));
           }
           continue;
        }
        const unsigned int moduleHash = channelNumber;
        std::string stringStatus = payload["data"].data<std::string>();
        const ChargeCalibrationBundle & b = pParser->parse(moduleHash, stringStatus);
        if (not b.isValid){
          ATH_MSG_FATAL("Parsing failed");
          return StatusCode::FAILURE;
        }
        writeCdo -> setAllFromBundle(moduleHash, b);
      } else {
        ATH_MSG_ERROR("payload[\"data\"] does not exist for ChanNum " << channelNumber);
        return StatusCode::FAILURE;
      }
    }
  } else {
    for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
      IdentifierHash wafer_hash = IdentifierHash(moduleHash);
      const auto & [barrel_ec, layer] = getBecAndLayer(m_pixelID, wafer_hash);
      const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
      const auto & [numFE, technology] = numChipsAndTechnology(element);
      const Thresholds defaultThreshold{configData->getDefaultAnalogThreshold(barrel_ec, layer), configData->getDefaultAnalogThresholdSigma(barrel_ec, layer),
            configData->getDefaultAnalogThresholdNoise(barrel_ec, layer), configData->getDefaultInTimeThreshold(barrel_ec, layer)};
      const std::vector<Thresholds> allDefaultThresholds(numFE, defaultThreshold);
      //
      const LegacyFitParameters defaultParam{configData->getDefaultQ2TotA(), configData->getDefaultQ2TotE(), configData->getDefaultQ2TotC(), LegacyFitParameters::defaultOverflow};
      const std::vector<LegacyFitParameters> allDefaultFitParams(numFE, defaultParam);
      //
      const LinearFitParameters defaultLinParam{0.0f, 0.0f};
      const std::vector<LinearFitParameters> allDefaultLinearParams(numFE, defaultLinParam);
      //
      for (InDetDD::PixelDiodeType type : diodeTypes) {
        writeCdo -> setThresholds(type, moduleHash, allDefaultThresholds);
        writeCdo -> setLegacyFitParameters(type, moduleHash, allDefaultFitParams);
        writeCdo -> setLinearFitParameters(type, moduleHash, allDefaultLinearParams);
      }
      writeCdo -> setTotResolutions(moduleHash, std::vector<Resolutions>(numFE, {0.f, 0.f}));
    }
  }
  if (channel_warnings>max_channel_warnings) {
     ATH_MSG_WARNING("Encountered " << channel_warnings << " invalid channel numbers (range " << min_invalid_channel << " .. "
                     << max_invalid_channel << " !< " <<  m_pixelID->wafer_hash_max() << ")");
  }

  // Scan over if the DB contents need to be overwritten.
  // This is useful for threshold study. So far only threshold value.
  for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    const auto & [barrel_ec, layer] = getBecAndLayer(m_pixelID, wafer_hash);
    const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
    const auto & [numFE, technology] = numChipsAndTechnology(element);
    Thresholds defaults{configData->getDefaultAnalogThreshold(barrel_ec, layer), configData->getDefaultAnalogThresholdSigma(barrel_ec, layer),
            configData->getDefaultAnalogThresholdNoise(barrel_ec, layer), configData->getDefaultInTimeThreshold(barrel_ec, layer)};
    if (defaults.value > -0.1) {
      for (InDetDD::PixelDiodeType type : diodeTypes) {
        writeCdo -> setThresholds(type, moduleHash, std::vector<Thresholds>(numFE, defaults));
      }
    }
  }

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelChargeCalibCondData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}

