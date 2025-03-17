/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "Run2ChargeCalibParser.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelChargeCalibUtils.h" //for getBecAndLayer



namespace{
  constexpr size_t FEStringSize{21};
} // namespace

using PixelChargeCalib::getBecAndLayer;
using PixelChargeCalib::numChipsAndTechnology;

namespace PixelChargeCalib{
  ChargeCalibrationBundle
  Run2ChargeCalibParser::parseImpl(unsigned int moduleHash, const std::string & data){
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    const InDetDD::SiDetectorElement *element = m_elements->getDetectorElement(wafer_hash);
    const auto & [numChips, technology] = PixelChargeCalib::numChipsAndTechnology(element);
    //
    std::stringstream ss(data);
    std::vector<std::string> component;
    std::string buffer;
    std::getline(ss, buffer, '\n'); // skip first line
    while (std::getline(ss, buffer, '\n')) { component.push_back(buffer); }
    const size_t numFE = component.size();
    if (numFE != numChips){
      std::cout << "Warning that the number of chips in the DB string and the number of chips according to readout technology are not equal\n";
    }
    //
    ChargeCalibrationBundle b(numFE);
    //
    // loop over FEs
    for (size_t i{}; i < numFE; i++) {
      std::stringstream checkFE(component[i]);
      std::vector<std::string> FEString;
      while (std::getline(checkFE, buffer, ' ')) { FEString.push_back(buffer); }

      if (FEString.size() < FEStringSize) {
        b.isValid=false;
        return b;
      }
      
      auto getInt = getFunc<int>(FEString);
      auto getFloat = getFunc<float>(FEString);
      //
      b.threshold.emplace_back(getInt(1), getInt(2), getInt(3), getInt(4));
      b.thresholdLong.emplace_back(getInt(5), getInt(6), getInt(7), getInt(8));
      b.thresholdGanged.emplace_back(getInt(9), getInt(10), getInt(11), getInt(12));
      b.params.emplace_back(getFloat(13), getFloat(14), getFloat(15), LegacyFitParameters::defaultOverflow);
      b.paramsGanged.emplace_back(getFloat(16), getFloat(17), getFloat(18), LegacyFitParameters::defaultOverflow);
      b.totRes.emplace_back(getFloat(19), getFloat(20));

      // Linear extrapolation above large charge
      if (m_configData->getPIXLinearExtrapolation() && technology==InDetDD::PixelReadoutTechnology::FEI3) {
        b.calibrationType =  PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX;
        const auto & [barrel_ec, layer] = getBecAndLayer(m_pixelID, wafer_hash);
        // search for ToT when charge exceeds threshold
        int totlimit = -1;
        int start =  m_configData->getToTThreshold(barrel_ec,layer);
        int end = m_configData->getFEI3Latency(barrel_ec,layer);
        // Normal pixels
        totlimit = b.idxAtChargeLimit(m_chargeLimit, InDetDD::PixelDiodeType::NORMAL, start, end);
        b.insertLinearParams(InDetDD::PixelDiodeType::NORMAL, totlimit);
        // Ganged pixels
        totlimit = b.idxAtChargeLimit(m_chargeLimit, InDetDD::PixelDiodeType::GANGED, start, end);
        b.insertLinearParams(InDetDD::PixelDiodeType::GANGED, totlimit);
      } else {
        b.lin.emplace_back(0.f, 0.f);
        b.linGanged.emplace_back(0.f, 0.f);
      }
    }
    return b;
  }
}
