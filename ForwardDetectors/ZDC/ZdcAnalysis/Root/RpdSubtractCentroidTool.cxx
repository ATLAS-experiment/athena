/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <numbers>

#include "ZdcAnalysis/RpdSubtractCentroidTool.h"
#include "ZdcAnalysis/RPDDataAnalyzer.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "ZdcUtils/RPDUtils.h"
#include "ZdcUtils/ZdcEventInfo.h"

namespace ZDC
{

RpdSubtractCentroidTool::RpdSubtractCentroidTool(const std::string& name)
  : asg::AsgTool(name)
{
  declareProperty("ZDCModuleContainerName", m_ZDCModuleContainerName = "ZdcModules", "Location of ZDC processed data");
  declareProperty("ZDCSumContainerName", m_ZDCSumContainerName = "ZdcSums", "Location of ZDC processed sums");
  declareProperty("Configuration", m_configuration = "default");
  declareProperty("WriteAux", m_writeAux = true, "If true, write AOD decorations");
  declareProperty("AuxSuffix", m_auxSuffix = "", "Suffix to add to AOD decorations for reading and writing");
  declareProperty("MinZDCEnergy", m_forceMinZDCEnergy, "Minimum (calibrated) ZDC energy for valid centroid (negative to disable); per side");
  declareProperty("MaxZDCEnergy", m_forceMaxZDCEnergy, "Maximum (calibrated) ZDC energy for valid centroid (negative to disable); per side");
  declareProperty("MinEMEnergy", m_forceMinEMEnergy, "Minimum (calibrated) EM energy for valid centroid (negative to disable); per side");
  declareProperty("MaxEMEnergy", m_forceMaxEMEnergy, "Minimum (calibrated) EM energy for valid centroid (negative to disable); per side");
  declareProperty("PileupMaxFrac", m_forcePileupMaxFrac, "Maximum fractional pileup allowed in an RPD channel for valid centroid; per side");
  declareProperty("ExcessiveSubtrUnderflowFrac", m_forceMaximumNegativeSubtrAmpFrac, "If any RPD channel subtracted amplitude is negative and its fraction of subtracted amplitude sum is greater than or equal to this number, the centroid is invalid; per side");
  declareProperty("UseRPDSumAdc", m_forceUseRPDSumAdc, "If true, use RPD channel sum ADC for centroid calculation, else use RPD channel max ADC");
  declareProperty("UseCalibDecorations", m_forceUseCalibDecorations, "If true, use RPD channel sum/max ADC decorations with output calibration factors applied during reconstruction, else use decorations with raw values");
}

StatusCode RpdSubtractCentroidTool::initializeKey(std::string const& containerName, SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> & writeHandleKey, std::string const& key) {
  writeHandleKey = containerName + key + m_auxSuffix;
  return writeHandleKey.initialize();
}

StatusCode RpdSubtractCentroidTool::initialize() {
  // first initialize reconstruction parameters
  m_minZDCEnergy = {-1.0, -1.0};
  m_maxZDCEnergy = {-1.0, -1.0};
  m_minEMEnergy = {-1.0, -1.0};
  m_maxEMEnergy = {-1.0, -1.0};
  m_pileupMaxFrac = {1.0, 1.0};
  m_maximumNegativeSubtrAmpFrac = {1.0, 1.0};
  m_useRPDSumAdc = true;
  m_useCalibDecorations = true;

  // then overwrite inidividual parameters from configuration if any were provided
  if (m_forceMinZDCEnergy.has_value()) {
    m_minZDCEnergy = m_forceMinZDCEnergy.value();
  }
  if (m_forceMaxZDCEnergy.has_value()) {
    m_maxZDCEnergy = m_forceMaxZDCEnergy.value();
  }
  if (m_forceMinEMEnergy.has_value()) {
    m_minEMEnergy = m_forceMinEMEnergy.value();
  }
  if (m_forceMaxEMEnergy.has_value()) {
    m_maxEMEnergy = m_forceMaxEMEnergy.value();
  }
  if (m_forcePileupMaxFrac.has_value()) {
    m_pileupMaxFrac = m_forcePileupMaxFrac.value();
  }
  if (m_forceMaximumNegativeSubtrAmpFrac.has_value()) {
    m_maximumNegativeSubtrAmpFrac = m_forceMaximumNegativeSubtrAmpFrac.value();
  }
  if (m_forceUseRPDSumAdc.has_value()) {
    m_useRPDSumAdc = m_forceUseRPDSumAdc.value();
  }
  if (m_forceUseCalibDecorations.has_value()) {
    m_useCalibDecorations = m_forceUseCalibDecorations.value();
  }

  // if any ZDC/EM energy threshold is nonnegative, ZDC decorations must be read
  m_readZDCDecorations = anyNonNegative(m_minZDCEnergy) || anyNonNegative(m_maxZDCEnergy) || anyNonNegative(m_minEMEnergy) || anyNonNegative(m_maxEMEnergy);

  for (auto const side : RPDUtils::sides) {
    if (m_minZDCEnergy.at(side) < 0) m_minZDCEnergy.at(side) = -std::numeric_limits<float>::infinity();
    if (m_maxZDCEnergy.at(side) < 0) m_maxZDCEnergy.at(side) = std::numeric_limits<float>::infinity();
    if (m_minEMEnergy.at(side) < 0) m_minEMEnergy.at(side) = -std::numeric_limits<float>::infinity();
    if (m_maxEMEnergy.at(side) < 0) m_maxEMEnergy.at(side) = std::numeric_limits<float>::infinity();
  }

  ATH_MSG_DEBUG("RpdSubtractCentroidTool reconstruction parameters:");
  ATH_MSG_DEBUG("config = " << m_configuration);
  ATH_MSG_DEBUG("minZDCEnergy = " << RPDUtils::vecToString(m_minZDCEnergy));
  ATH_MSG_DEBUG("maxZDCEnergy = " << RPDUtils::vecToString(m_maxZDCEnergy));
  ATH_MSG_DEBUG("minEMEnergy = " << RPDUtils::vecToString(m_minEMEnergy));
  ATH_MSG_DEBUG("maxEMEnergy = " << RPDUtils::vecToString(m_maxEMEnergy));
  ATH_MSG_DEBUG("pileupMaxFrac = " << RPDUtils::vecToString(m_pileupMaxFrac));
  ATH_MSG_DEBUG("maximumNegativeSubtrAmpFrac = " << RPDUtils::vecToString(m_maximumNegativeSubtrAmpFrac));
  ATH_MSG_DEBUG("useRPDSumAdc = " << m_useRPDSumAdc);
  ATH_MSG_DEBUG("useCalibDecorations = " << m_useCalibDecorations);

  if (m_readZDCDecorations) {
    ATH_MSG_DEBUG("RpdSubtractCentroidTool is configured to check ZDC or EM energy; ZDC-related ReadDecorHandleKey's will be initialized");
    m_ZDCSideStatus = {0, 0};
    m_ZDCFinalEnergy = {0, 0};
    m_EMCalibEnergy = {0, 0};
    m_EMStatus = {0, 0};
  }

  ATH_CHECK(m_eventInfoKey.initialize());

  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_centroidEventValidKey, ".centroidEventValid"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_centroidStatusKey, ".centroidStatus"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_RPDChannelSubtrAmpKey, ".RPDChannelSubtrAmp"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_RPDSubtrAmpSumKey, ".RPDSubtrAmpSum"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_xCentroidPreGeomCorPreAvgSubtrKey, ".xCentroidPreGeomCorPreAvgSubtr"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_yCentroidPreGeomCorPreAvgSubtrKey, ".yCentroidPreGeomCorPreAvgSubtr"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_xCentroidPreAvgSubtrKey, ".xCentroidPreAvgSubtr"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_yCentroidPreAvgSubtrKey, ".yCentroidPreAvgSubtr"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_xCentroidKey, ".xCentroid"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_yCentroidKey, ".yCentroid"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_xRowCentroidKey, ".xRowCentroid"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_yColCentroidKey, ".yColCentroid"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_reactionPlaneAngleKey, ".reactionPlaneAngle"));
  ATH_CHECK(initializeKey(m_ZDCSumContainerName, m_cosDeltaReactionPlaneAngleKey, ".cosDeltaReactionPlaneAngle"));

  if (m_writeAux && !m_auxSuffix.empty()) {
    ATH_MSG_DEBUG("suffix string = " << m_auxSuffix);
  }

  m_initialized = true;

  return StatusCode::SUCCESS;
}

void RpdSubtractCentroidTool::reset() {
  m_eventValid = false;
  for (auto& status : m_centroidStatus) {
    status.reset();
    status.set(ValidBit, true);
  }
  RPDUtils::helpZero(m_subtrAmp);
  RPDUtils::helpZero(m_subtrAmpRowSum);
  RPDUtils::helpZero(m_subtrAmpColSum);
  RPDUtils::helpZero(m_subtrAmpSum);
  RPDUtils::helpZero(m_xCentroidPreGeomCorPreAvgSubtr);
  RPDUtils::helpZero(m_yCentroidPreGeomCorPreAvgSubtr);
  RPDUtils::helpZero(m_xCentroidPreAvgSubtr);
  RPDUtils::helpZero(m_yCentroidPreAvgSubtr);
  RPDUtils::helpZero(m_xCentroid);
  RPDUtils::helpZero(m_yCentroid);
  RPDUtils::helpZero(m_xRowCentroid);
  RPDUtils::helpZero(m_yColCentroid);
  RPDUtils::helpZero(m_reactionPlaneAngle);
  m_cosDeltaReactionPlaneAngle = 0;
}

RpdSubtractCentroidTool::SubstepStatus RpdSubtractCentroidTool::readAOD(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) {
  // initialize read handles from read handle keys
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) {
    return SubstepStatus::Failure;
  }
  // RPD decorations are always needed
  if (eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR)) {
    ATH_MSG_WARNING("RPD decoding error found - abandoning RPD centroid reco!");
    return SubstepStatus::SkipEvent;
  }
  // ZDC decorations are sometimes needed
  if (m_readZDCDecorations && eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::ZDCDECODINGERROR)) {
    ATH_MSG_WARNING("ZDC decoding error found - abandoning RPD centroid reco!");
    return SubstepStatus::SkipEvent;
  }

  // nominally, aux suffix is added to read key, since upstream tools were probably configured to write decorations with the same suffix
  // however, certain decorations remain unchanged in a reprocessing (e.g., those written by ZdcRecChannelToolLucrod), in which case the suffix should NOT be added
  static SG::ConstAccessor<float> const xposRelAcc("xposRel");
  static SG::ConstAccessor<float> const yposRelAcc("yposRel");
  static SG::ConstAccessor<unsigned short> const rowAcc("row");
  static SG::ConstAccessor<unsigned short> const colAcc("col");
  
  static SG::ConstAccessor<float> const rpdChannelSumAdcAcc("RPDChannelAmplitude" + m_auxSuffix);
  static SG::ConstAccessor<float> const rpdChannelSumAdcCalibAcc("RPDChannelAmplitudeCalib" + m_auxSuffix);
  static SG::ConstAccessor<float> const rpdChannelMaxADCAcc("RPDChannelMaxADC" + m_auxSuffix);
  static SG::ConstAccessor<float> const rpdChannelMaxADCCalibAcc("RPDChannelMaxADCCalib" + m_auxSuffix);
  static SG::ConstAccessor<float> const rpdChannelPileupFracAcc("RPDChannelPileupFrac" + m_auxSuffix);
  static SG::ConstAccessor<unsigned int> const rpdChannelStatusAcc("RPDChannelStatus" + m_auxSuffix);
  static SG::ConstAccessor<unsigned int> const rpdSideStatusAcc("RPDStatus" + m_auxSuffix);
  static auto const zdcModuleCalibEnergyAcc = m_readZDCDecorations
    ? std::optional<SG::ConstAccessor<float>>("CalibEnergy" + m_auxSuffix)
    : std::nullopt;
  static auto const zdcModuleStatusAcc = m_readZDCDecorations
    ? std::optional<SG::ConstAccessor<unsigned int>>("Status" + m_auxSuffix)
    : std::nullopt;
  static auto const zdcFinalEnergyAcc = m_readZDCDecorations
    ? std::optional<SG::ConstAccessor<float>>("FinalEnergy" + m_auxSuffix)
    : std::nullopt;
  static auto const zdcStatusAcc = m_readZDCDecorations
    ? std::optional<SG::ConstAccessor<unsigned int>>("Status" + m_auxSuffix)
    : std::nullopt;

  ATH_MSG_DEBUG("Processing modules");

  for (auto const * const zdcModule : moduleContainer) {
    unsigned int const side = RPDUtils::ZDCSideToSideIndex(zdcModule->zdcSide());
    if (zdcModule->zdcType() == RPDUtils::ZDCModuleZDCType && zdcModule->zdcModule() == RPDUtils::ZDCModuleEMModule) {
      // this is a ZDC module and this is an EM module
      if (m_readZDCDecorations) {
        m_EMCalibEnergy->at(side) = (*zdcModuleCalibEnergyAcc)(*zdcModule);
        m_EMStatus->at(side) = (*zdcModuleStatusAcc)(*zdcModule);
      }
    } else if (zdcModule->zdcType() == RPDUtils::ZDCModuleRPDType) {
      // this is a Run 3 RPD module
      // (it is assumed that this tool will not be invoked otherwise)
      //
      if (zdcModule->zdcChannel() < 0 || static_cast<unsigned int>(zdcModule->zdcChannel()) > RPDUtils::nChannels - 1) {
        ATH_MSG_ERROR("Invalid RPD channel found on side " << side << ": channel number = " << zdcModule->zdcChannel());
      }
      // channel numbers are fixed in mapping in ZdcConditions, numbered 0-15
      auto const channel = zdcModule->zdcChannel();
      auto const& row = rowAcc(*zdcModule);
      auto const& col = colAcc(*zdcModule);
      m_RPDChannelData.at(side).at(row).at(col).channel = static_cast<unsigned int>(channel);
      m_RPDChannelData.at(side).at(row).at(col).xposRel = xposRelAcc(*zdcModule);
      m_RPDChannelData.at(side).at(row).at(col).yposRel = yposRelAcc(*zdcModule);
      m_RPDChannelData.at(side).at(row).at(col).row = rowAcc(*zdcModule);
      m_RPDChannelData.at(side).at(row).at(col).col = colAcc(*zdcModule);
      if (m_useRPDSumAdc) {
        if (m_useCalibDecorations) {
          m_RPDChannelData.at(side).at(row).at(col).amp = rpdChannelSumAdcCalibAcc(*zdcModule);
        } else {
          m_RPDChannelData.at(side).at(row).at(col).amp = rpdChannelSumAdcAcc(*zdcModule);
        }
      } else {
        if (m_useCalibDecorations) {
          m_RPDChannelData.at(side).at(row).at(col).amp = rpdChannelMaxADCCalibAcc(*zdcModule);
        } else {
          m_RPDChannelData.at(side).at(row).at(col).amp = rpdChannelMaxADCAcc(*zdcModule);
        }
      }
      m_RPDChannelData.at(side).at(row).at(col).pileupFrac = rpdChannelPileupFracAcc(*zdcModule);
      m_RPDChannelData.at(side).at(row).at(col).status = rpdChannelStatusAcc(*zdcModule);
    }
  }

  for (auto const * const zdcSum: moduleSumContainer) {
    if (zdcSum->zdcSide() == RPDUtils::ZDCSumsGlobalZDCSide) {
      // skip global sum (it's like the side between sides)
      continue;
    }
    unsigned int const side = RPDUtils::ZDCSideToSideIndex(zdcSum->zdcSide());
    m_RPDSideStatus.at(side) = rpdSideStatusAcc(*zdcSum);
    if (m_ZDCSideStatus) m_ZDCSideStatus->at(side) = (*zdcStatusAcc)(*zdcSum);
    if (m_ZDCFinalEnergy) m_ZDCFinalEnergy->at(side) = (*zdcFinalEnergyAcc)(*zdcSum);
  }

  return SubstepStatus::Success;
}

bool RpdSubtractCentroidTool::checkZdcRpdValidity(unsigned int side) {
  if (m_readZDCDecorations) {
    if (m_ZDCSideStatus->at(side) == 0) {
      // zdc bad
      m_centroidStatus.at(side).set(ZDCInvalidBit, true);
      m_centroidStatus.at(side).set(ValidBit, false);
    } else {
      // zdc good
      if (m_ZDCFinalEnergy->at(side) < m_minZDCEnergy.at(side)) {
        m_centroidStatus.at(side).set(InsufficientZDCEnergyBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
      if (m_ZDCFinalEnergy->at(side) > m_maxZDCEnergy.at(side)) {
        m_centroidStatus.at(side).set(ExcessiveZDCEnergyBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
    }

    if (m_EMStatus->at(side)[ZDCPulseAnalyzer::FailBit]) {
      // em bad
      m_centroidStatus.at(side).set(EMInvalidBit, true);
      m_centroidStatus.at(side).set(ValidBit, false);
    } else {
      // em good
      if (m_EMCalibEnergy->at(side) < m_minEMEnergy.at(side)) {
        m_centroidStatus.at(side).set(InsufficientEMEnergyBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
      if (m_EMCalibEnergy->at(side) > m_maxEMEnergy.at(side)) {
        m_centroidStatus.at(side).set(ExcessiveEMEnergyBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
    }
  }

  if (m_RPDSideStatus.at(side)[RPDDataAnalyzer::OutOfTimePileupBit]) {
    m_centroidStatus.at(side).set(PileupBit, true);
  }

  for (unsigned int row = 0; row < RPDUtils::nRows; row++) {
    for (unsigned int col = 0; col < RPDUtils::nCols; col++) {
      if (m_RPDChannelData.at(side).at(row).at(col).pileupFrac > m_pileupMaxFrac.at(side)) {
        m_centroidStatus.at(side).set(ExcessivePileupBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
    }
  }

  if (!m_RPDSideStatus.at(side)[RPDDataAnalyzer::ValidBit]) {
    m_centroidStatus.at(side).set(RPDInvalidBit, true);
    m_centroidStatus.at(side).set(ValidBit, false);
    return false;
  }

  return true;
}

bool RpdSubtractCentroidTool::subtractRpdAmplitudes(unsigned int side) {
  for (unsigned int row = 0; row < RPDUtils::nRows; row++) {
    for (unsigned int col = 0; col < RPDUtils::nCols; col++) {
      float subtrAmp {};
      if (row == RPDUtils::nRows - 1) {
        // top row -> nothing to subtract
        subtrAmp = m_RPDChannelData.at(side).at(row).at(col).amp;
      } else {
        // other rows -> subtract the tile above this one
        subtrAmp = m_RPDChannelData.at(side).at(row).at(col).amp - m_RPDChannelData.at(side).at(row + 1).at(col).amp;
      }
      m_RPDChannelData.at(side).at(row).at(col).subtrAmp = subtrAmp;
      m_subtrAmp.at(side).at(m_RPDChannelData.at(side).at(row).at(col).channel) = subtrAmp;
      m_subtrAmpRowSum.at(side).at(row) += subtrAmp;
      m_subtrAmpColSum.at(side).at(col) += subtrAmp;
      m_subtrAmpSum.at(side) += subtrAmp;
    }
  }

  if (m_subtrAmpSum.at(side) <= 0) {
    m_centroidStatus.at(side).set(ZeroSumBit, true);
    m_centroidStatus.at(side).set(ValidBit, false);
    return false;
  }

  for (unsigned int row = 0; row < RPDUtils::nRows; row++) {
    for (unsigned int col = 0; col < RPDUtils::nCols; col++) {
      const float &subtrAmp = m_RPDChannelData.at(side).at(row).at(col).subtrAmp;
      if (subtrAmp < 0 && -subtrAmp/m_subtrAmpSum.at(side) > m_maximumNegativeSubtrAmpFrac.at(side)) {
        m_centroidStatus.at(side).set(ExcessiveSubtrUnderflowBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
    }
  }

  return true;
}

void RpdSubtractCentroidTool::calculateDetectorCentroid(unsigned int side) {
  for (unsigned int col = 0; col < RPDUtils::nCols; col++) {
    m_xCentroidPreGeomCorPreAvgSubtr.at(side) += m_subtrAmpColSum.at(side).at(col)*m_RPDChannelData.at(side).at(0).at(col).xposRel/m_subtrAmpSum.at(side);
  }

  for (unsigned int row = 0; row < RPDUtils::nRows; row++) {
    m_yCentroidPreGeomCorPreAvgSubtr.at(side) += m_subtrAmpRowSum.at(side).at(row)*m_RPDChannelData.at(side).at(row).at(0).yposRel/m_subtrAmpSum.at(side);
  }

  for (unsigned int row = 0; row < RPDUtils::nRows; row++) {
    if (m_subtrAmpRowSum.at(side).at(row) <= 0) continue;
    for (unsigned int col = 0; col < RPDUtils::nCols; col++) {
      m_xRowCentroid.at(side).at(row) += m_RPDChannelData.at(side).at(row).at(col).subtrAmp*m_RPDChannelData.at(side).at(row).at(col).xposRel/m_subtrAmpRowSum.at(side).at(row);
    }
    m_centroidStatus.at(side).set(Row0ValidBit + row, true);
  }

  for (unsigned int col = 0; col < RPDUtils::nCols; col++) {
    if (m_subtrAmpColSum.at(side).at(col) <= 0) continue;
    for (unsigned int row = 0; row < RPDUtils::nRows; row++) {
      m_yColCentroid.at(side).at(col) += m_RPDChannelData.at(side).at(row).at(col).subtrAmp*m_RPDChannelData.at(side).at(row).at(col).yposRel/m_subtrAmpColSum.at(side).at(col);
    }
    m_centroidStatus.at(side).set(Col0ValidBit + col, true);
  }
}

void RpdSubtractCentroidTool::geometryCorrection(unsigned int side) {
  m_xCentroidPreAvgSubtr.at(side) = m_xCentroidPreGeomCorPreAvgSubtr.at(side) - m_alignmentXOffset.at(side);
  m_yCentroidPreAvgSubtr.at(side) = m_yCentroidPreGeomCorPreAvgSubtr.at(side) - m_alignmentYOffset.at(side);
  /** ROTATIONS CORRECTIONS GO HERE */
}

void RpdSubtractCentroidTool::subtractAverageCentroid(unsigned int side) {
  m_xCentroid.at(side) = m_xCentroidPreAvgSubtr.at(side) - m_avgXCentroid.at(side);
  m_yCentroid.at(side) = m_yCentroidPreAvgSubtr.at(side) - m_avgYCentroid.at(side);
}

void RpdSubtractCentroidTool::calculateReactionPlaneAngle(unsigned int side) {
  auto angle = std::atan2(m_yCentroid.at(side), m_xCentroid.at(side));
  // our angles are now simply the angle of the centroid in ATLAS coordinates on either side
  // however, we expect correlated deflection, so we want the difference between the angles
  // to be small when the centroids are in opposite quadrants of the two RPDs
  // therefore, we add pi to side A (chosen arbitrarily)
  if (side == RPDUtils::sideA) angle += std::numbers::pi_v<float>;
  // also, restrict to [-pi, pi)
  // we choose this rather than (-pi, pi] for ease of binning the edge case +/- pi
  if (angle >= std::numbers::pi) angle -= 2*std::numbers::pi_v<float>;
  m_reactionPlaneAngle.at(side) = angle;
}

void RpdSubtractCentroidTool::writeAOD(xAOD::ZdcModuleContainer const& moduleSumContainer) const {
  if (!m_writeAux) return;
  ATH_MSG_DEBUG("Adding variables with suffix = " + m_auxSuffix);

  // initialize write handles from write handle keys
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, char> centroidEventValidHandle(m_centroidEventValidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, unsigned int> centroidStatusHandle(m_centroidStatusKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> rpdChannelSubtrAmpHandle(m_RPDChannelSubtrAmpKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> rpdSubtrAmpSumHandle(m_RPDSubtrAmpSumKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xCentroidPreGeomCorPreAvgSubtrHandle(m_xCentroidPreGeomCorPreAvgSubtrKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yCentroidPreGeomCorPreAvgSubtrHandle(m_yCentroidPreGeomCorPreAvgSubtrKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xCentroidPreAvgSubtrHandle(m_xCentroidPreAvgSubtrKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yCentroidPreAvgSubtrHandle(m_yCentroidPreAvgSubtrKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> xCentroidHandle(m_xCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> yCentroidHandle(m_yCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> xRowCentroidHandle(m_xRowCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, std::vector<float>> yColCentroidHandle(m_yColCentroidKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> reactionPlaneAngleHandle(m_reactionPlaneAngleKey);
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, float> cosDeltaReactionPlaneAngleHandle(m_cosDeltaReactionPlaneAngleKey);

  for (auto const * const zdcSum: moduleSumContainer) {
    if (zdcSum->zdcSide() == RPDUtils::ZDCSumsGlobalZDCSide) {
      // global sum container
      // event status is bool, but stored as char to save disk space
      centroidEventValidHandle(*zdcSum) = static_cast<char>(m_eventValid);
      cosDeltaReactionPlaneAngleHandle(*zdcSum) = m_cosDeltaReactionPlaneAngle;
      continue;
    }
    unsigned int const side = RPDUtils::ZDCSideToSideIndex(zdcSum->zdcSide());
    centroidStatusHandle(*zdcSum) = static_cast<unsigned int>(m_centroidStatus.at(side).to_ulong());
    rpdChannelSubtrAmpHandle(*zdcSum) = m_subtrAmp.at(side);
    rpdSubtrAmpSumHandle(*zdcSum) = m_subtrAmpSum.at(side);
    xCentroidPreGeomCorPreAvgSubtrHandle(*zdcSum) = m_xCentroidPreGeomCorPreAvgSubtr.at(side);
    yCentroidPreGeomCorPreAvgSubtrHandle(*zdcSum) = m_yCentroidPreGeomCorPreAvgSubtr.at(side);
    xCentroidPreAvgSubtrHandle(*zdcSum) = m_xCentroidPreAvgSubtr.at(side);
    yCentroidPreAvgSubtrHandle(*zdcSum) = m_yCentroidPreAvgSubtr.at(side);
    xCentroidHandle(*zdcSum) = m_xCentroid.at(side);
    yCentroidHandle(*zdcSum) = m_yCentroid.at(side);
    xRowCentroidHandle(*zdcSum) = m_xRowCentroid.at(side);
    yColCentroidHandle(*zdcSum) = m_yColCentroid.at(side);
    reactionPlaneAngleHandle(*zdcSum) = m_reactionPlaneAngle.at(side);
  }
}

StatusCode RpdSubtractCentroidTool::recoZdcModules(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer) {
  if (moduleContainer.empty()) {
    // no modules - do nothing
    return StatusCode::SUCCESS;
  }
  reset();
  switch (readAOD(moduleContainer, moduleSumContainer)) {
    case SubstepStatus::Success:
      // do nothing - proceed
      break;
    case SubstepStatus::Failure:
      // stop and propagate error to Athena
      return StatusCode::FAILURE;
    case SubstepStatus::SkipEvent:
      // stop and tell Athena the event was a success
      return StatusCode::SUCCESS;
  }
  for (auto const side : RPDUtils::sides) {
    if (!checkZdcRpdValidity(side)) continue; // rpd invalid -> don't calculate centroid
    if (!subtractRpdAmplitudes(side)) continue; // bad total sum -> don't calculate centroid
    calculateDetectorCentroid(side);
    geometryCorrection(side);
    subtractAverageCentroid(side);
    calculateReactionPlaneAngle(side);
    m_centroidStatus.at(side).set(HasCentroidBit, true);
  }
  if (m_centroidStatus.at(RPDUtils::sideC)[HasCentroidBit] && m_centroidStatus.at(RPDUtils::sideA)[HasCentroidBit]) {
    m_cosDeltaReactionPlaneAngle = std::cos(m_reactionPlaneAngle.at(RPDUtils::sideC) - m_reactionPlaneAngle.at(RPDUtils::sideA));
  }
  if (m_centroidStatus.at(RPDUtils::sideC)[ValidBit] && m_centroidStatus.at(RPDUtils::sideA)[ValidBit]) {
    m_eventValid = true; // event is good for analysis
  }
  writeAOD(moduleSumContainer);
  ATH_MSG_DEBUG("Finishing event processing");
  return StatusCode::SUCCESS;
}

StatusCode RpdSubtractCentroidTool::reprocessZdc() {
  if (!m_initialized) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Trying to retrieve " << m_ZDCModuleContainerName);
  xAOD::ZdcModuleContainer const* zdcModules = nullptr;
  ATH_CHECK(evtStore()->retrieve(zdcModules, m_ZDCModuleContainerName));
  xAOD::ZdcModuleContainer const* zdcSums = nullptr;
  ATH_CHECK(evtStore()->retrieve(zdcSums, m_ZDCSumContainerName));
  ATH_CHECK(recoZdcModules(*zdcModules, *zdcSums));
  return StatusCode::SUCCESS;
}

} // namespace ZDC
