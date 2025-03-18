/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "LUTChargeCalibParser.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h" 
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelChargeCalibUtils.h"

namespace{
  constexpr size_t FEStringSize{20};
} // namespace

using PixelChargeCalib::getBecAndLayer;
using PixelChargeCalib::numChipsAndTechnology;

namespace PixelChargeCalib{

  ChargeCalibrationBundle
  LUTChargeCalibParser::parseImpl(unsigned int moduleHash, const nlohmann::json & data, int inputSource){
    IdentifierHash wafer_hash = IdentifierHash(moduleHash);
    const InDetDD::SiDetectorElement *element = m_elements->getDetectorElement(wafer_hash);
    const auto & [numFE, technology] = numChipsAndTechnology(element);
    bool isLUTEnabled = (inputSource == 1);
    bool isChargeCalibrationFromJson = (inputSource == 2);
    //
    ChargeCalibrationBundle b(numFE,isLUTEnabled,isChargeCalibrationFromJson);
    //
    for (unsigned int j{}; j < numFE; j++) {
      //Retrieve calibArrays in different formats for Run4 (based on ITKPixV2 chip settings) and Run3 
      const auto &calibArray = (isLUTEnabled || isChargeCalibrationFromJson) ? data.at(0) : data.at(j);
      if (!calibArray.empty()) {
        // new charge calibration for RUN-3
        if ((technology == InDetDD::PixelReadoutTechnology::FEI4 && !(element->isDBM())) || ((b.useLUT || b.useTXT) && technology == InDetDD::PixelReadoutTechnology::RD53)) {
          if (calibArray.size() != FEStringSize) {
            std::cout<<"Parameter size is not consistent(" << FEStringSize << ") " << calibArray.size() << " at (i,j)=(" << moduleHash << "," << j << ")\n";
            b.isValid = false;
            return b;
          }
          auto getInt = getFunc<int>(calibArray);
          auto getFloat = getFunc<float>(calibArray);
          auto &charges = b.tot2Charges.emplace_back(PixelChargeCalibCondData::IBLCalibration());
          for (size_t k{}; k < PixelChargeCalibCondData::IBLCalibrationSize; k++) {
            charges[k] = getFloat(k + 4ul);
          }
          b.calibrationType = PixelChargeCalibCondData::CalibrationStrategy::LUTFEI4;
          b.threshold.emplace_back(getInt(0), 0.f, getInt(1), 0.f);
          b.thresholdLong.emplace_back(getInt(2), 0.f, getInt(3), 0.f);
          b.thresholdGanged.emplace_back(getInt(2), 0.f, getInt(3), 0.f);
          b.params.emplace_back(0.f, 0.f, 0.f, LegacyFitParameters::defaultOverflow);
          b.paramsGanged.emplace_back(0.f, 0.f, 0.f, LegacyFitParameters::defaultOverflow);
          b.totRes.emplace_back(0.f, 0.f);
        } else { //normal CalibrationStrategy used by ITk
          if (calibArray.size() != FEStringSize) {
            std::cout<<"Parameter size is not consistent(" << FEStringSize << ") " << calibArray.size() << " at (i,j)=(" << moduleHash << "," << j << ")\n";
            b.isValid = false;
            return b;
          }
          auto getInt = getFunc<int>(calibArray);
          auto getFloat = getFunc<float>(calibArray);
          b.threshold.emplace_back(getInt(0), getInt(1), getInt(2), getInt(3));
          b.thresholdLong.emplace_back(getInt(4), getInt(5), getInt(6), getInt(7));
          b.thresholdGanged.emplace_back(getInt(8), getInt(9), getInt(10), getInt(11));
          //note: for ITk, setting the first parameter to the constructor (A) less than totlimit
          //will result in pathological behaviour for the 'Q' method on LegacyFitParameters
          const auto totLimit = (technology == InDetDD::PixelReadoutTechnology::RD53) ? 14: LegacyFitParameters::defaultOverflow;
          b.params.emplace_back(getFloat(12), getFloat(13), getFloat(14), totLimit);
          b.paramsGanged.emplace_back(getFloat(15), getFloat(16), getFloat(17), totLimit);
          b.totRes.emplace_back(getFloat(18), getFloat(19));

          // Linear extrapolation above large charge
          if (m_configData->getPIXLinearExtrapolation()) {
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
            b.calibrationType = PixelChargeCalibCondData::CalibrationStrategy::RUN1PIX;
            b.lin.emplace_back(0.f, 0.f);
            b.linGanged.emplace_back(0.f, 0.f);
          }
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
