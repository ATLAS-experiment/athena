/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelConditionsData/ChargeCalibrationBundle.h"
#include "PixelConditionsData/PixelModuleData.h"


#include <stdexcept>
#include <sstream>
#include <cfloat> //for FLT_MAX



using namespace PixelChargeCalib;
using InDetDD::enum2uint; //in PixelReadoutDefinitions.h; will use ADL anyway,but make it explicit

namespace{
  const std::out_of_range 
  generateError(const char * functionName, InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "("<<enum2uint(type,__func__)<< ", " << moduleHash << ", " << FE << "): out of bounds";
    return std::out_of_range(error.str());
  }
  const std::out_of_range 
  generateError(const char * functionName, unsigned int moduleHash, unsigned int FE, unsigned int val){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "(" << moduleHash << ", " << FE << ", "<< val << "): out of bounds";
    return std::out_of_range(error.str());
  }
  const std::out_of_range 
  generateError(const char * functionName,  unsigned int moduleHash, unsigned int FE){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "(" << moduleHash << ", " << FE << "): out of bounds";
    return std::out_of_range(error.str());
  }
  const std::out_of_range 
  generateError(const char * functionName,  unsigned int moduleHash){
    std::stringstream error;
    error << "PixelChargeCalibCondData::"<< functionName << "(" << moduleHash << "): out of bounds";
    return std::out_of_range(error.str());
  }
}

PixelChargeCalibCondData::PixelChargeCalibCondData(std::size_t max_module_hash) : m_sizeOfHashVector(max_module_hash){
  //nop
}

void 
PixelChargeCalibCondData::setAllFromBundle(unsigned int moduleHash, const ChargeCalibrationBundle& b){
  //calibration strategy
  setCalibrationStrategy(moduleHash, b.calibrationType);
  // Normal pixel
  setThresholds(InDetDD::PixelDiodeType::NORMAL, moduleHash, b.threshold);
  setLegacyFitParameters(InDetDD::PixelDiodeType::NORMAL, moduleHash, b.params); 
  setLinearFitParameters(InDetDD::PixelDiodeType::NORMAL, moduleHash, b.lin); 
  setTotResolutions(moduleHash, b.totRes);
  // Long pixel
  setThresholds(InDetDD::PixelDiodeType::LONG, moduleHash, b.thresholdLong);
  setLegacyFitParameters(InDetDD::PixelDiodeType::LONG, moduleHash, b.params); 
  setLinearFitParameters(InDetDD::PixelDiodeType::LONG, moduleHash, b.lin); 
  // Ganged/large pixel
  setThresholds(InDetDD::PixelDiodeType::GANGED, moduleHash, b.thresholdGanged);
  setLegacyFitParameters(InDetDD::PixelDiodeType::GANGED, moduleHash, b.paramsGanged);
  setLinearFitParameters(InDetDD::PixelDiodeType::GANGED, moduleHash, b.linGanged);
}

void 
PixelChargeCalibCondData::setAllFromConfigData(unsigned int moduleHash, const PixelModuleData * configData, const std::pair<int, int> &becLayer, unsigned int numFE){
  const auto & [barrel_ec, layer] = becLayer;
  for (size_t i{}; i != s_NPixelDiodes; ++i) {
    const auto t = static_cast<InDetDD::PixelDiodeType>(i);
    const Thresholds thresholds{configData->getDefaultAnalogThreshold(barrel_ec, layer), configData->getDefaultAnalogThresholdSigma(barrel_ec, layer),
      configData->getDefaultAnalogThresholdNoise(barrel_ec, layer), configData->getDefaultInTimeThreshold(barrel_ec, layer)};
    setThresholds(t, moduleHash, std::vector<Thresholds>(numFE, thresholds));
    //
    const LegacyFitParameters defaultParams{configData->getDefaultQ2TotA(), configData->getDefaultQ2TotE(), configData->getDefaultQ2TotC()};
    setLegacyFitParameters(t, moduleHash, std::vector<LegacyFitParameters>(numFE, defaultParams));
    //
    const auto zeroLinFit = PixelChargeCalib::LinearFitParameters();
    setLinearFitParameters(t, moduleHash, std::vector<PixelChargeCalib::LinearFitParameters>(numFE, zeroLinFit));
  }
  const auto zeroResolution = PixelChargeCalib::Resolutions();
  setTotResolutions(moduleHash, std::vector<PixelChargeCalib::Resolutions>(numFE, zeroResolution));
}

void 
PixelChargeCalibCondData::setThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::Thresholds> & thresholds){
  if (moduleHash >= m_sizeOfHashVector) throw generateError(__func__, moduleHash);
  auto & hashIndexedVector = m_thresholds[enum2uint(type,__func__)];
  if (hashIndexedVector.size()<=moduleHash){
    hashIndexedVector.resize(m_sizeOfHashVector);
  }
  hashIndexedVector[moduleHash] = thresholds;
}

PixelChargeCalib::Thresholds 
PixelChargeCalibCondData::getThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  auto idx = enum2uint(type,__func__);
  const auto & hashIndexedVector = m_thresholds[idx];
  const auto &feIndexedVector = hashIndexedVector.at(moduleHash);
  if (FE < feIndexedVector.size()) {
    return feIndexedVector[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

void 
PixelChargeCalibCondData::setLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LegacyFitParameters> &parameters){
  if (moduleHash >= m_sizeOfHashVector) throw generateError(__func__, moduleHash);
  auto & thisVector = m_legacyFit.at(enum2uint(type,__func__));
  if (thisVector.size()<=moduleHash){
    thisVector.resize(m_sizeOfHashVector);
  }
  thisVector[moduleHash] = parameters;
}

PixelChargeCalib::LegacyFitParameters 
PixelChargeCalibCondData::getLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  if (moduleHash >= m_sizeOfHashVector) throw generateError(__func__, type, moduleHash, FE);
  auto idx = enum2uint(type,__func__);
  const auto & thisVector = m_legacyFit.at(idx);
  const auto &v = thisVector.at(moduleHash);
  if (FE < v.size()) {
    return v[FE];
  }
  throw generateError(__func__, type, moduleHash, FE);
}

void 
PixelChargeCalibCondData::setLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LinearFitParameters> &parameters){
  if (moduleHash >= m_sizeOfHashVector) throw generateError(__func__, moduleHash);
  auto & thisVector = m_linFit.at(enum2uint(type,__func__));
  if ( thisVector.size()<=moduleHash){
    thisVector.resize(m_sizeOfHashVector);
  }
  thisVector[moduleHash] =  parameters;
}


PixelChargeCalib::LinearFitParameters 
PixelChargeCalibCondData::getLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const{
  if (type==InDetDD::PixelDiodeType::NONE) throw generateError(__func__, type, moduleHash, FE);
  auto idx = enum2uint(type,__func__);
  const auto &typeMap = m_linFit.at(idx);
  //
  const auto &linFit = typeMap.at(moduleHash);
  if (FE < linFit.size()) {
    return typeMap.at(moduleHash).at(FE);
  }
  throw generateError(__func__, type, moduleHash, FE);
}

void 
PixelChargeCalibCondData::setTotResolutions(unsigned int moduleHash, const std::vector<PixelChargeCalib::Resolutions> &value){
  if (moduleHash >= m_sizeOfHashVector) throw generateError(__func__, moduleHash);
  if (moduleHash >= m_totRes.size()) m_totRes.resize(m_sizeOfHashVector);
  m_totRes[moduleHash] = value;
}

float 
PixelChargeCalibCondData::getTotRes(unsigned int moduleHash, unsigned int FE, float Q) const{
  Resolutions r;
  if (const auto &res = m_totRes.at(moduleHash); FE < res.size()) {
    r = res[FE];
  } else {
    throw generateError(__func__, moduleHash, FE);
  }
  return r.total(Q);
}

float 
PixelChargeCalibCondData::getToT(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float Q) const{
  if (getCalibrationStrategy(moduleHash) == CalibrationStrategy::LUTFEI4) {
    return getToTLUTFEI4(moduleHash, FE, Q);
  }
  if (type == InDetDD::PixelDiodeType::NONE) return 0.f;
  const LegacyFitParameters & legacy = getLegacyFitParameters(type, moduleHash, FE);
  float tot = legacy.ToT(Q);

  // Protection for large charge
  float exth = 1e5f;    // the calibration function is analytically connected at threshold exth.
  if (Q>exth && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) {
    const LinearFitParameters & lin = getLinearFitParameters(type, moduleHash, FE);
    if ( float tot1 = lin.ToT(Q); tot1 != 0.f) return tot1;
  }
  return tot;
}


float 
PixelChargeCalibCondData::getCharge(InDetDD::PixelDiodeType type,
				    const CalibrationStrategy calibStrategy,
				    unsigned int moduleHash,
				    unsigned int FE,
				    float ToT) const{

  if (calibStrategy == CalibrationStrategy::LUTFEI4) {
    return getChargeLUTFEI4(moduleHash, FE, ToT);
  }
  
  if (type == InDetDD::PixelDiodeType::NONE) return 0.f;
  const LegacyFitParameters & legacy = getLegacyFitParameters(type, moduleHash, FE);
  float charge = legacy.Q(ToT);
  // Protection for small charge
  const auto & thresholds = getThresholds(type,moduleHash,FE);
  const auto  analogueThreshold = thresholds.value;
  if (charge<analogueThreshold && calibStrategy==CalibrationStrategy::RUN3PIX) { charge=analogueThreshold; }
  // Protection for large charge
  float exth = 1e5f;    // the calibration function is analytically connected at threshold exth.
  if (charge>exth && calibStrategy==CalibrationStrategy::RUN3PIX) {
    const LinearFitParameters & lin  = getLinearFitParameters(type, moduleHash, FE);
    if (float charge1 = lin.Q(ToT); charge1 != 0.f) return charge1;
  }
  return charge;
}


float 
PixelChargeCalibCondData::getCharge(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float ToT) const{
  if (getCalibrationStrategy(moduleHash) == CalibrationStrategy::LUTFEI4) {
    return getChargeLUTFEI4(moduleHash, FE, ToT);
  }
  if (type == InDetDD::PixelDiodeType::NONE) return 0.f;
  const LegacyFitParameters & legacy = getLegacyFitParameters(type, moduleHash, FE);
 
  float charge = legacy.Q(ToT);

  // Protection for small charge
  const auto & thresholds = getThresholds(type,moduleHash,FE);
  const auto  analogueThreshold = thresholds.value;
  if (charge<analogueThreshold && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) { charge=analogueThreshold; }
  // Protection for large charge
  float exth = 1e5f;    // the calibration function is analytically connected at threshold exth.
  if (charge>exth && getCalibrationStrategy(moduleHash)==CalibrationStrategy::RUN3PIX) {
    const LinearFitParameters & lin  = getLinearFitParameters(type, moduleHash, FE);
    if (float charge1 = lin.Q(ToT); charge1 != 0.f) return charge1;
  }
  return charge;
}

void 
PixelChargeCalibCondData::setCalibrationStrategy(unsigned int moduleHash, CalibrationStrategy strategy){ 
  if (moduleHash >= m_sizeOfHashVector) {
    throw generateError(__func__, moduleHash, static_cast<int>(strategy));
  }
  m_calibrationStrategy[moduleHash] = strategy;
}

PixelChargeCalibCondData::CalibrationStrategy 
PixelChargeCalibCondData::getCalibrationStrategy(unsigned int moduleHash) const{
  if (moduleHash >= m_sizeOfHashVector){
    throw generateError(__func__, moduleHash);
  }
  auto itr = m_calibrationStrategy.find(moduleHash);
  if (itr != m_calibrationStrategy.end()) {
    return itr->second;
  }
  return CalibrationStrategy::RUN1PIX;
}

void 
PixelChargeCalibCondData::setTot2Charges(unsigned int moduleHash, IBLModule charges){ 
  if (moduleHash >= m_sizeOfHashVector){
    throw generateError(__func__, moduleHash);
  }
  m_tot2Charges[moduleHash] = std::move(charges);
}

const PixelChargeCalibCondData::IBLCalibration &
PixelChargeCalibCondData::getTot2Charges(unsigned int moduleHash, unsigned int FE) const{
  auto it = m_tot2Charges.find(moduleHash);
  if (it != m_tot2Charges.end() && FE < it->second.size()) {
    return it->second.at(FE);
  }
  throw generateError(__func__, moduleHash,FE);
}

float 
PixelChargeCalibCondData::getChargeLUTFEI4(unsigned int moduleHash, unsigned int FE, unsigned int ToT) const{
  if (ToT < 1 || ToT > IBLCalibrationSize) {
    throw generateError(__func__, moduleHash,FE, ToT);
  }

  const IBLCalibration &charges = getTot2Charges(moduleHash,FE);
  return charges[ToT - 1];
}

float 
PixelChargeCalibCondData::getToTLUTFEI4(unsigned int moduleHash, unsigned int FE, float Q) const{
  int tot = -1;
  float minDiff = FLT_MAX;
  for (size_t t = 0; t < IBLCalibrationSize; t++) {
    float charge = getChargeLUTFEI4(moduleHash, FE, t + 1);
    float diff = std::fabs(charge - Q);
    if (diff < minDiff) {
      minDiff = diff;
      tot = t + 1;
    }
  }
  return tot;
}
