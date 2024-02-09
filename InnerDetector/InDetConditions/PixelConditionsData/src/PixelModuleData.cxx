/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <sstream>
#include <utility>

#include "PixelConditionsData/PixelModuleData.h"


void PixelModuleData::setDefaultBarrelAnalogThreshold(const std::vector<int> &barrelAnalogThreshold) { m_defaultBarrelAnalogThreshold = barrelAnalogThreshold; }
void PixelModuleData::setDefaultEndcapAnalogThreshold(const std::vector<int> &endcapAnalogThreshold) { m_defaultEndcapAnalogThreshold = endcapAnalogThreshold; }
void PixelModuleData::setDefaultDBMAnalogThreshold(const std::vector<int> &DBMAnalogThreshold) { m_defaultDBMAnalogThreshold = DBMAnalogThreshold; }

int PixelModuleData::getDefaultAnalogThreshold(int barrel_ec, int layer) const
{
  // only for charge calibration cond algs
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_defaultBarrelAnalogThreshold.size())
    return m_defaultBarrelAnalogThreshold[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_defaultEndcapAnalogThreshold.size())
    return m_defaultEndcapAnalogThreshold[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_defaultDBMAnalogThreshold.size())
    return m_defaultDBMAnalogThreshold[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getDefaultAnalogThreshold(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setDefaultBarrelAnalogThresholdSigma(const std::vector<int> &barrelAnalogThresholdSigma) { m_defaultBarrelAnalogThresholdSigma = barrelAnalogThresholdSigma; }
void PixelModuleData::setDefaultEndcapAnalogThresholdSigma(const std::vector<int> &endcapAnalogThresholdSigma) { m_defaultEndcapAnalogThresholdSigma = endcapAnalogThresholdSigma; }
void PixelModuleData::setDefaultDBMAnalogThresholdSigma(const std::vector<int> &DBMAnalogThresholdSigma) { m_defaultDBMAnalogThresholdSigma = DBMAnalogThresholdSigma; }

int PixelModuleData::getDefaultAnalogThresholdSigma(int barrel_ec, int layer) const
{
  // only for charge calibration cond algs
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_defaultBarrelAnalogThresholdSigma.size())
    return m_defaultBarrelAnalogThresholdSigma[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_defaultEndcapAnalogThresholdSigma.size())
    return m_defaultEndcapAnalogThresholdSigma[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_defaultDBMAnalogThresholdSigma.size())
    return m_defaultDBMAnalogThresholdSigma[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getDefaultAnalogThresholdSigma(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setDefaultBarrelAnalogThresholdNoise(const std::vector<int> &barrelAnalogThresholdNoise) { m_defaultBarrelAnalogThresholdNoise = barrelAnalogThresholdNoise; }
void PixelModuleData::setDefaultEndcapAnalogThresholdNoise(const std::vector<int> &endcapAnalogThresholdNoise) { m_defaultEndcapAnalogThresholdNoise = endcapAnalogThresholdNoise; }
void PixelModuleData::setDefaultDBMAnalogThresholdNoise(const std::vector<int> &DBMAnalogThresholdNoise) { m_defaultDBMAnalogThresholdNoise = DBMAnalogThresholdNoise; }

int PixelModuleData::getDefaultAnalogThresholdNoise(int barrel_ec, int layer) const
{
  // only for charge calibration cond algs
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_defaultBarrelAnalogThresholdNoise.size())
    return m_defaultBarrelAnalogThresholdNoise[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_defaultEndcapAnalogThresholdNoise.size())
    return m_defaultEndcapAnalogThresholdNoise[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_defaultDBMAnalogThresholdNoise.size())
    return m_defaultDBMAnalogThresholdNoise[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getDefaultAnalogThresholdNoise(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setDefaultBarrelInTimeThreshold(const std::vector<int> &barrelInTimeThreshold) { m_defaultBarrelInTimeThreshold = barrelInTimeThreshold; }
void PixelModuleData::setDefaultEndcapInTimeThreshold(const std::vector<int> &endcapInTimeThreshold) { m_defaultEndcapInTimeThreshold = endcapInTimeThreshold; }
void PixelModuleData::setDefaultDBMInTimeThreshold(const std::vector<int> &DBMInTimeThreshold) { m_defaultDBMInTimeThreshold = DBMInTimeThreshold; }

int PixelModuleData::getDefaultInTimeThreshold(int barrel_ec, int layer) const
{
  // only for charge calibration cond algs
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_defaultBarrelInTimeThreshold.size())
    return m_defaultBarrelInTimeThreshold[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_defaultEndcapInTimeThreshold.size())
    return m_defaultEndcapInTimeThreshold[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_defaultDBMInTimeThreshold.size())
    return m_defaultDBMInTimeThreshold[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getDefaultInTimeThreshold(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBarrelToTThreshold(const std::vector<int> &barrelToTThreshold) { m_barrelToTThreshold = barrelToTThreshold; }
void PixelModuleData::setEndcapToTThreshold(const std::vector<int> &endcapToTThreshold) { m_endcapToTThreshold = endcapToTThreshold; }
void PixelModuleData::setDBMToTThreshold(const std::vector<int> &DBMToTThreshold) { m_DBMToTThreshold = DBMToTThreshold; }

int PixelModuleData::getToTThreshold(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_barrelToTThreshold.size())
    return m_barrelToTThreshold[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_endcapToTThreshold.size())
    return m_endcapToTThreshold[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_DBMToTThreshold.size())
    return m_DBMToTThreshold[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getToTThreshold(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBarrelCrossTalk(const std::vector<double> &barrelCrossTalk) { m_barrelCrossTalk = barrelCrossTalk; }
void PixelModuleData::setEndcapCrossTalk(const std::vector<double> &endcapCrossTalk) { m_endcapCrossTalk = endcapCrossTalk; }
void PixelModuleData::setDBMCrossTalk(const std::vector<double> &DBMCrossTalk) { m_DBMCrossTalk = DBMCrossTalk; }

double PixelModuleData::getCrossTalk(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_barrelCrossTalk.size())
    return m_barrelCrossTalk[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_endcapCrossTalk.size())
    return m_endcapCrossTalk[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_DBMCrossTalk.size())
    return m_DBMCrossTalk[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getCrossTalk(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBarrelThermalNoise(const std::vector<double> &barrelThermalNoise) { m_barrelThermalNoise = barrelThermalNoise; }
void PixelModuleData::setEndcapThermalNoise(const std::vector<double> &endcapThermalNoise) { m_endcapThermalNoise = endcapThermalNoise; }
void PixelModuleData::setDBMThermalNoise(const std::vector<double> &DBMThermalNoise) { m_DBMThermalNoise = DBMThermalNoise; }

double PixelModuleData::getThermalNoise(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_barrelThermalNoise.size())
    return m_barrelThermalNoise[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_endcapThermalNoise.size())
    return m_endcapThermalNoise[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_DBMThermalNoise.size())
    return m_DBMThermalNoise[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getThermalNoise(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBarrelNoiseOccupancy(const std::vector<double> &barrelNoiseOccupancy) { m_barrelNoiseOccupancy = barrelNoiseOccupancy; }
void PixelModuleData::setEndcapNoiseOccupancy(const std::vector<double> &endcapNoiseOccupancy) { m_endcapNoiseOccupancy = endcapNoiseOccupancy; }
void PixelModuleData::setDBMNoiseOccupancy(const std::vector<double> &DBMNoiseOccupancy) { m_DBMNoiseOccupancy = DBMNoiseOccupancy; }

double PixelModuleData::getNoiseOccupancy(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_barrelNoiseOccupancy.size())
    return m_barrelNoiseOccupancy[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_endcapNoiseOccupancy.size())
    return m_endcapNoiseOccupancy[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_DBMNoiseOccupancy.size())
    return m_DBMNoiseOccupancy[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getNoiseOccupancy(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBarrelDisableProbability(const std::vector<double> &barrelDisableProbability) { m_barrelDisableProbability = barrelDisableProbability; }
void PixelModuleData::setEndcapDisableProbability(const std::vector<double> &endcapDisableProbability) { m_endcapDisableProbability = endcapDisableProbability; }
void PixelModuleData::setDBMDisableProbability(const std::vector<double> &DBMDisableProbability) { m_DBMDisableProbability = DBMDisableProbability; }

double PixelModuleData::getDisableProbability(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_barrelDisableProbability.size())
    return m_barrelDisableProbability[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_endcapDisableProbability.size())
    return m_endcapDisableProbability[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_DBMDisableProbability.size())
    return m_DBMDisableProbability[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getDisableProbability(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBarrelNoiseShape(const std::vector<std::vector<float>> &barrelNoiseShape) { m_barrelNoiseShape = barrelNoiseShape; }
void PixelModuleData::setEndcapNoiseShape(const std::vector<std::vector<float>> &endcapNoiseShape) { m_endcapNoiseShape = endcapNoiseShape; }
void PixelModuleData::setDBMNoiseShape(const std::vector<std::vector<float>> &DBMNoiseShape) { m_DBMNoiseShape = DBMNoiseShape; }

const std::vector<float> &PixelModuleData::getNoiseShape(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_barrelNoiseShape.size())
    return m_barrelNoiseShape[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_endcapNoiseShape.size())
    return m_endcapNoiseShape[layerIndex];

  if (std::abs(barrel_ec) == 4 && layerIndex < m_DBMNoiseShape.size())
    return m_DBMNoiseShape[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getNoiseShape(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setFEI3BarrelLatency(const std::vector<int> &FEI3BarrelLatency) { m_FEI3BarrelLatency = FEI3BarrelLatency; }
void PixelModuleData::setFEI3EndcapLatency(const std::vector<int> &FEI3EndcapLatency) { m_FEI3EndcapLatency = FEI3EndcapLatency; }

int PixelModuleData::getFEI3Latency(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_FEI3BarrelLatency.size())
    return m_FEI3BarrelLatency[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_FEI3EndcapLatency.size())
    return m_FEI3EndcapLatency[layerIndex];
  std::string msg="PixelModuleData::getFEI3Latency(" + std::to_string(barrel_ec) +", "+std::to_string(layer)+") out of bounds";
  throw std::range_error(msg);
}

void PixelModuleData::setFEI3BarrelTimingSimTune(const std::vector<int> &FEI3BarrelTimingSimTune) { m_FEI3BarrelTimingSimTune = FEI3BarrelTimingSimTune; }
void PixelModuleData::setFEI3EndcapTimingSimTune(const std::vector<int> &FEI3EndcapTimingSimTune) { m_FEI3EndcapTimingSimTune = FEI3EndcapTimingSimTune; }

int PixelModuleData::getFEI3TimingSimTune(int barrel_ec, int layer) const
{
  size_t layerIndex = static_cast<size_t>(layer);

  if (barrel_ec == 0 && layerIndex < m_FEI3BarrelTimingSimTune.size())
    return m_FEI3BarrelTimingSimTune[layerIndex];

  if (std::abs(barrel_ec) == 2 && layerIndex < m_FEI3EndcapTimingSimTune.size())
    return m_FEI3EndcapTimingSimTune[layerIndex];

  std::stringstream error;
  error << "PixelModuleData::getFEI3TimingSimTune(" << barrel_ec << ", " << layer << "): array out of bounds";
  throw std::range_error(error.str());
}

void PixelModuleData::setBLayerTimingIndex(const std::vector<float> &BLayerTimingIndex) { m_BLayerTimingIndex = BLayerTimingIndex; }
void PixelModuleData::setLayer1TimingIndex(const std::vector<float> &Layer1TimingIndex) { m_Layer1TimingIndex = Layer1TimingIndex; }
void PixelModuleData::setLayer2TimingIndex(const std::vector<float> &Layer2TimingIndex) { m_Layer2TimingIndex = Layer2TimingIndex; }
void PixelModuleData::setEndcap1TimingIndex(const std::vector<float> &Endcap1TimingIndex) { m_Endcap1TimingIndex = Endcap1TimingIndex; }
void PixelModuleData::setEndcap2TimingIndex(const std::vector<float> &Endcap2TimingIndex) { m_Endcap2TimingIndex = Endcap2TimingIndex; }
void PixelModuleData::setEndcap3TimingIndex(const std::vector<float> &Endcap3TimingIndex) { m_Endcap3TimingIndex = Endcap3TimingIndex; }

void PixelModuleData::setBLayerTimingProbability(const std::vector<float> &BLayerTimingProbability) { m_BLayerTimingProbability = BLayerTimingProbability; }
void PixelModuleData::setLayer1TimingProbability(const std::vector<float> &Layer1TimingProbability) { m_Layer1TimingProbability = Layer1TimingProbability; }
void PixelModuleData::setLayer2TimingProbability(const std::vector<float> &Layer2TimingProbability) { m_Layer2TimingProbability = Layer2TimingProbability; }
void PixelModuleData::setEndcap1TimingProbability(const std::vector<float> &Endcap1TimingProbability) { m_Endcap1TimingProbability = Endcap1TimingProbability; }
void PixelModuleData::setEndcap2TimingProbability(const std::vector<float> &Endcap2TimingProbability) { m_Endcap2TimingProbability = Endcap2TimingProbability; }
void PixelModuleData::setEndcap3TimingProbability(const std::vector<float> &Endcap3TimingProbability) { m_Endcap3TimingProbability = Endcap3TimingProbability; }

std::vector<float> PixelModuleData::getTimingIndex(int barrel_ec, int layer) const {
  if (barrel_ec==0) {
    if (layer==1) { return m_BLayerTimingIndex; }   // b-layer
    if (layer==2) { return m_Layer1TimingIndex; }   // Layer-1
    if (layer==3) { return m_Layer2TimingIndex; }   // Layer-2
  }
  else if (std::abs(barrel_ec)==2) {
    if (layer==0) { return m_Endcap1TimingIndex; }   // Endcap-1
    if (layer==1) { return m_Endcap2TimingIndex; }   // Endcap-2
    if (layer==2) { return m_Endcap3TimingIndex; }   // Endcap-3
  }
  return std::vector<float>(0.0);
}

std::vector<float> PixelModuleData::getTimingProbability(int barrel_ec, int layer, int eta) const {
  std::vector<float> prob;
  if (barrel_ec==0) {
    if (layer==1) { prob=m_BLayerTimingProbability; }   // b-layer
    if (layer==2) { prob=m_Layer1TimingProbability; }   // Layer-1
    if (layer==3) { prob=m_Layer2TimingProbability; }   // Layer-2
  }
  else if (std::abs(barrel_ec)==2) {
    if (layer==0) { prob=m_Endcap1TimingProbability; }   // Endcap-1
    if (layer==1) { prob=m_Endcap2TimingProbability; }   // Endcap-2
    if (layer==2) { prob=m_Endcap3TimingProbability; }   // Endcap-3
  }
  int nCalibrationPoints = barrel_ec==0 ? prob.size()/7 : prob.size();
  if (nCalibrationPoints!=(int)getTimingIndex(barrel_ec,layer).size()) {
    std::stringstream error;
    error << "PixelModuleData::getTimingProbability: array size(" << nCalibrationPoints << ") mismatch with index array(" << getTimingIndex(barrel_ec,layer).size() << ")";
    throw std::range_error(error.str());
  }

  std::vector<float> etaprob;
  for (int i=0; i<nCalibrationPoints; i++) {
    etaprob.push_back(prob.at(i+nCalibrationPoints*std::abs(eta)));
  }
  return etaprob;
}

// Charge calibration parameters
void PixelModuleData::setDefaultQ2TotA(float paramA) { m_paramA = paramA; }
void PixelModuleData::setDefaultQ2TotE(float paramE) { m_paramE = paramE; }
void PixelModuleData::setDefaultQ2TotC(float paramC) { m_paramC = paramC; }
float PixelModuleData::getDefaultQ2TotA() const { return m_paramA; }
float PixelModuleData::getDefaultQ2TotE() const { return m_paramE; }
float PixelModuleData::getDefaultQ2TotC() const { return m_paramC; }

void PixelModuleData::setPIXLinearExtrapolation(bool doLinearExtrapolation) { m_doLinearExtrapolation = doLinearExtrapolation; }
bool PixelModuleData::getPIXLinearExtrapolation() const { return m_doLinearExtrapolation; }

// DCS parameters
void PixelModuleData::setDefaultBiasVoltage(float biasVoltage) { m_biasVoltage = biasVoltage; }
float PixelModuleData::getDefaultBiasVoltage() const { return m_biasVoltage; }

// Radiation damage fluence maps
void PixelModuleData::setFluenceLayer(const std::vector<double> &fluenceLayer) { m_fluenceLayer = fluenceLayer; }
const std::vector<double> &PixelModuleData::getFluenceLayer() const { return m_fluenceLayer; }

void PixelModuleData::setRadSimFluenceMapList(const std::vector<std::string> &radSimFluenceMapList) { m_radSimFluenceMapList = radSimFluenceMapList; }
const std::vector<std::string> &PixelModuleData::getRadSimFluenceMapList() const { return m_radSimFluenceMapList; }

void PixelModuleData::setFluenceLayer3D(const std::vector<double> &fluenceLayer) { m_fluenceLayer3D = fluenceLayer; }
const std::vector<double> &PixelModuleData::getFluenceLayer3D() const { return m_fluenceLayer3D; }

void PixelModuleData::setRadSimFluenceMapList3D(const std::vector<std::string> &radSimFluenceMapList3D) { m_radSimFluenceMapList3D = radSimFluenceMapList3D; }
const std::vector<std::string> &PixelModuleData::getRadSimFluenceMapList3D() const { return m_radSimFluenceMapList3D; }

// Cabling parameters
void PixelModuleData::setCablingMapToFile(bool cablingMapToFile) { m_cablingMapToFile = cablingMapToFile; }
bool PixelModuleData::getCablingMapToFile() const { return m_cablingMapToFile; }

void PixelModuleData::setCablingMapFileName(const std::string &cablingMapFileName) { m_cablingMapFileName = cablingMapFileName; }
const std::string &PixelModuleData::getCablingMapFileName() const { return m_cablingMapFileName; }

