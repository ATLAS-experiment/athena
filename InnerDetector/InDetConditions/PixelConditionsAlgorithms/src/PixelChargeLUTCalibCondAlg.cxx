/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/EventIDRange.h"
#include "Identifier/IdentifierHash.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelChargeLUTCalibCondAlg.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include "PixelConditionsData/PixelChargeCalibUtils.h" //getBecAndLayer
#include "IChargeCalibrationParser.h"
#include "LUTChargeCalibParser.h"

#include <nlohmann/json.hpp>
#include "PathResolver/PathResolver.h"

#include <fstream>
#include <stdexcept>
#include <memory>
#include <sstream>


using namespace PixelChargeCalib; //containing LegacyFitParameters etc

using InDetDD::enum2uint;
namespace{
  constexpr int halfModuleThreshold{8};
} // namespace


PixelChargeLUTCalibCondAlg::PixelChargeLUTCalibCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthCondAlgorithm(name, pSvcLocator){
}

StatusCode PixelChargeLUTCalibCondAlg::initialize() {
  ATH_MSG_DEBUG("PixelChargeLUTCalibCondAlg::initialize()");
  ATH_CHECK(detStore()->retrieve(m_pixelID, m_pixelIDName.value()));
  ATH_CHECK(m_pixelDetEleCollKey.initialize());
  ATH_CHECK(m_configKey.initialize());
  ATH_CHECK(m_readKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_writeKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PixelChargeLUTCalibCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("PixelChargeLUTCalibCondAlg::execute()");

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
  std::unique_ptr<PixelChargeCalibCondData> writeCdo(std::make_unique<PixelChargeCalibCondData>(m_pixelID->wafer_hash_max()));

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};
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
      const CondAttrListCollection::AttributeList &payload = attrList.second;

      // RUN-3 format
      if (payload.exists("data_array") and not payload["data_array"].isNull()) {
        ATH_MSG_DEBUG("Using LUTChargeCalibParser");
        pParser = std::make_unique<LUTChargeCalibParser>(configData, elements, m_pixelID);
	std::string fileName = PathResolver::find_file(m_jsonFileName, "DATAPATH");   
	if (m_inputSource == 2 && fileName.empty()) {
	  ATH_MSG_FATAL("Input file" << fileName << " not found! Change the inputSource!");
	  return StatusCode::FAILURE; 
	}
	std::ifstream infile(fileName.c_str());
	//Read the contents of the text file into a string 
	std::string fileContent((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
	const nlohmann::json & jsonData = (m_inputSource == 0 || m_inputSource == 1) ? 
	  nlohmann::json::parse(payload["data_array"].data<std::string>()) :  
	  nlohmann::json::parse(fileContent);
        for (const auto &[hash, data] : jsonData.items()) {
          const unsigned int moduleHash = std::stoul(hash);
          IdentifierHash wafer_hash = IdentifierHash(moduleHash);
          const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
          const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
          const ChargeCalibrationBundle &  b = pParser->parse(moduleHash, data, m_inputSource);
          if (not b.isValid){
            ATH_MSG_FATAL("Parsing failed");
            return StatusCode::FAILURE;
          }
          // Special calibration
          if (!b.tot2Charges.empty()) {
            writeCdo -> setTot2Charges(moduleHash, b.tot2Charges);
          }
          //calibration strategy
          writeCdo -> setAllFromBundle(moduleHash, b);
          
          // Ganged/large pixel
          if (p_design->getReadoutTechnology() == InDetDD::PixelReadoutTechnology::RD53) {
            writeCdo -> setThresholds(InDetDD::PixelDiodeType::LARGE, moduleHash, b.thresholdGanged);
            writeCdo -> setLegacyFitParameters(InDetDD::PixelDiodeType::LARGE, moduleHash, b.paramsGanged); //uses the ganged answer
            writeCdo -> setLinearFitParameters(InDetDD::PixelDiodeType::LARGE, moduleHash, b.linGanged);
          } else {
            writeCdo -> setThresholds(InDetDD::PixelDiodeType::GANGED, moduleHash, b.thresholdGanged);
            writeCdo -> setLegacyFitParameters(InDetDD::PixelDiodeType::GANGED, moduleHash,  b.paramsGanged);
            writeCdo -> setLinearFitParameters(InDetDD::PixelDiodeType::GANGED, moduleHash,  b.linGanged);
          }
        }
      }
    }
  } else {
    for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
      const auto & becLayer = getBecAndLayer(m_pixelID, moduleHash);
      IdentifierHash wafer_hash = IdentifierHash(moduleHash);
      const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
      const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
      unsigned int numFE = p_design->numberOfCircuits() < halfModuleThreshold ? p_design->numberOfCircuits() : 2 * p_design->numberOfCircuits();
      writeCdo -> setAllFromConfigData(moduleHash, configData, becLayer, numFE);
    }
  }

  // Scan over if the DB contents need to be overwritten.
  // This is useful for threshold study. So far only threshold value.
  for (unsigned int moduleHash{}; moduleHash < m_pixelID->wafer_hash_max(); moduleHash++) {
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    Identifier wafer_id = m_pixelID->wafer_id(wafer_hash);
    int barrel_ec = m_pixelID->barrel_ec(wafer_id);
    int layer     = m_pixelID->layer_disk(wafer_id);
    const InDetDD::SiDetectorElement *element = elements->getDetectorElement(wafer_hash);
    const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&element->design());
    // in some cases numberOfCircuits returns FEs per half-module
    unsigned int numFE = p_design->numberOfCircuits() < halfModuleThreshold ? p_design->numberOfCircuits() : 2 * p_design->numberOfCircuits();
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
