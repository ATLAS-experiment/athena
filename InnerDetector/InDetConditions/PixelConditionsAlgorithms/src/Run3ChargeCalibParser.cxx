/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "Run3ChargeCalibParser.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelChargeCalibUtils.h"

using PixelChargeCalib::getBecAndLayer;
using PixelChargeCalib::numChipsAndTechnology;

namespace{
  constexpr size_t FEStringSize{21};
} // namespace


namespace PixelChargeCalib{
  ChargeCalibrationBundle
  Run3ChargeCalibParser::parseImpl(unsigned int moduleHash, const nlohmann::json & data){
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    const InDetDD::SiDetectorElement *element = m_elements->getDetectorElement(wafer_hash);
    const auto & [numFE, technology] = numChipsAndTechnology(element);
    //
    ChargeCalibrationBundle b(numFE);
    //
    for (unsigned int j{}; j < numFE; j++) {
      const auto &calibArray = data.at(j);
      if (!calibArray.empty()) {
        // conventional calibration
        if (calibArray.size() != FEStringSize-1) {
          std::cout<<"Parameter size is not consistent(" << FEStringSize << ") " << calibArray.size() << " at (i,j)=(" << moduleHash << "," << j << ")\n";
          b.isValid = false;
          return b;
        }
        auto getInt = getFunc<int>(calibArray);
        auto getFloat = getFunc<float>(calibArray); 
        b.threshold.emplace_back(getInt(0), getInt(1), getInt(2), getInt(3));
        b.thresholdLong.emplace_back(getInt(4), getInt(5), getInt(6), getInt(7));
        b.thresholdGanged.emplace_back(getInt(8), getInt(9), getInt(10), getInt(11));
        b.params.emplace_back(getFloat(12), getFloat(13), getFloat(14), LegacyFitParameters::defaultOverflow);
        b.paramsGanged.emplace_back(getFloat(15), getFloat(16), getFloat(17), LegacyFitParameters::defaultOverflow);
        b.totRes.emplace_back(getFloat(18), getFloat(19));

        // Linear extrapolation above large charge
        if (m_configData->getPIXLinearExtrapolation() && technology==InDetDD::PixelReadoutTechnology::FEI3) {
          b.calibrationType = PixelChargeCalibCondData::CalibrationStrategy::RUN3PIX;
          const auto & [barrel_ec, layer] = getBecAndLayer(m_pixelID, wafer_hash);
          // search for ToT when charge exceeds threshold
          if (!(element->isDBM())) {
            //charge threshold beyond which linear fit will be used
            const int totIdxStart = m_configData->getToTThreshold(barrel_ec,layer);
            const int totIdxEnd = m_configData->getFEI3Latency(barrel_ec,layer);
            // Normal pixels
            int totlimit = b.idxAtChargeLimit(m_chargeLimit, InDetDD::PixelDiodeType::NORMAL, totIdxStart, totIdxEnd);
            b.insertLinearParams( InDetDD::PixelDiodeType::NORMAL, totlimit);
            // Ganged pixels
            totlimit = b.idxAtChargeLimit(m_chargeLimit, InDetDD::PixelDiodeType::GANGED, totIdxStart, totIdxEnd);
            b.insertLinearParams( InDetDD::PixelDiodeType::GANGED, totlimit);
          } else {    // DBM
            b.lin.emplace_back(0.f, 0.f);
            b.linGanged.emplace_back(0.f, 0.f);
          }
        } else {
          b.lin.emplace_back(0.f, 0.f);
          b.linGanged.emplace_back(0.f, 0.f);
        }
      } else {
        std::cout<<"Array size is zero in " << calibArray << " at (i,j)=(" << moduleHash << "," << j << ")\n";
        b.isValid = false;
        return b;
      }
    }
    return b;
  }
}
