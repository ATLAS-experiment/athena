/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcAnalysis/RpdSubtractCentroidTool.h"
#include "ZdcAnalysis/RPDDataAnalyzer.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "TMath.h"

namespace ZDC
{

RpdSubtractCentroidTool::RpdSubtractCentroidTool(const std::string& name) :
  asg::AsgTool(name),
  m_name(name),
  m_init(false)
{
  #ifndef XAOD_STANDALONE
    declareInterface<IZdcAnalysisTool>(this);
  #endif

  declareProperty("ZdcModuleContainerName", m_zdcModuleContainerName = "ZdcModules", "Location of ZDC processed data");
  declareProperty("ZdcSumContainerName", m_zdcSumContainerName = "ZdcSums", "Location of ZDC processed sums");
  declareProperty("WriteAux", m_writeAux = true);
  declareProperty("AuxSuffix", m_auxSuffix = "");
  declareProperty("MinZdcEnergy", m_minZdcEnergy = {-1.0, -1.0}, "Minimum (calibrated) ZDC energy for valid centriod (negative to disable)");
  declareProperty("MaxZdcEnergy", m_maxZdcEnergy = {-1.0, -1.0}, "Maximum (calibrated) ZDC energy for valid centriod (negative to disable)");
  declareProperty("MinEmEnergy", m_minEmEnergy = {-1.0, -1.0}, "Minimum (calibrated) EM energy for valid centriod (negative to disable)");
  declareProperty("MaxEmEnergy", m_maxEmEnergy = {-1.0, -1.0}, "Minimum (calibrated) EM energy for valid centriod (negative to disable)");
  declareProperty("PileupMaxFrac", m_pileupMaxFrac = {1.0, 1.0}, "Maximum fractional pileup allowed in an RPD channel for valid centroid");
  declareProperty("ExcessiveSubtrUnderflowFrac", m_maximumNegativeSubtrAmpFrac = {1.0, 1.0}, "If any RPD channel subtracted amplitude is negative and its fraction of subtracted amplitude sum is greater than or equal to this number, the centroid is invalid");
  declareProperty("UseRpdSumAdc", m_useRpdSumAdc = true, "If true, use RPD channel sum ADC for centroid calculation, else use RPD channel max ADC");
  declareProperty("UseCalibDecorations", m_useCalibDecorations = true, "If true, use reconstruction-calibrated RPD channel sum/max ADC decorations, else use uncalibrated");
}

RpdSubtractCentroidTool::~RpdSubtractCentroidTool()
{
  ATH_MSG_DEBUG("Deleting RpdSubtractCentroidTool named " << m_name);
}

StatusCode RpdSubtractCentroidTool::initialize()
{
  for (unsigned int side : {0, 1}) {
    if (m_minZdcEnergy[side] < 0) m_minZdcEnergy[side] = -std::numeric_limits<float>::infinity();
    if (m_maxZdcEnergy[side] < 0) m_maxZdcEnergy[side] = std::numeric_limits<float>::infinity();
    if (m_minEmEnergy[side] < 0) m_minEmEnergy[side] = -std::numeric_limits<float>::infinity();
    if (m_maxEmEnergy[side] < 0) m_maxEmEnergy[side] = std::numeric_limits<float>::infinity();
  }

  ATH_CHECK(m_eventInfoKey.initialize());

  // zdc modules read keys
  m_xposRelKey = m_zdcModuleContainerName + ".xposRel";
  ATH_CHECK(m_xposRelKey.initialize());
  m_yposRelKey = m_zdcModuleContainerName + ".yposRel";
  ATH_CHECK(m_yposRelKey.initialize());
  m_rowKey = m_zdcModuleContainerName + ".row";
  ATH_CHECK(m_rowKey.initialize());
  m_colKey = m_zdcModuleContainerName + ".col";
  ATH_CHECK(m_colKey.initialize());
  m_ZDCModuleCalibEnergyKey = m_zdcModuleContainerName + ".CalibEnergy" + m_auxSuffix;
  ATH_CHECK(m_ZDCModuleCalibEnergyKey.initialize());
  m_ZDCModuleStatusKey = m_zdcModuleContainerName + ".Status" + m_auxSuffix;
  ATH_CHECK(m_ZDCModuleStatusKey.initialize());
  m_RPDChannelAmplitudeKey = m_zdcModuleContainerName + ".RPDChannelAmplitude" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelAmplitudeKey.initialize());
  m_RPDChannelAmplitudeCalibKey = m_zdcModuleContainerName + ".RPDChannelAmplitudeCalib" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelAmplitudeCalibKey.initialize());
  m_RPDChannelMaxADCKey = m_zdcModuleContainerName + ".RPDChannelMaxADC" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelMaxADCKey.initialize());
  m_RPDChannelMaxADCCalibKey = m_zdcModuleContainerName + ".RPDChannelMaxADCCalib" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelMaxADCCalibKey.initialize());
  m_RPDChannelPileupFracKey = m_zdcModuleContainerName + ".RPDChannelPileupFrac" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelPileupFracKey.initialize());
  m_RPDChannelStatusKey = m_zdcModuleContainerName + ".RPDChannelStatus" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelStatusKey.initialize());

  // zdc sums read keys
  m_RPDSideStatusKey = m_zdcSumContainerName + ".RPDStatus" + m_auxSuffix;
  ATH_CHECK(m_RPDSideStatusKey.initialize());
  m_ZDCFinalEnergyKey = m_zdcSumContainerName + ".FinalEnergy" + m_auxSuffix;
  ATH_CHECK(m_ZDCFinalEnergyKey.initialize());
  m_ZDCStatusKey = m_zdcSumContainerName + ".Status" + m_auxSuffix;
  ATH_CHECK(m_ZDCStatusKey.initialize());

  // zdc sums write keys
  m_centroidEventValidKey = m_zdcSumContainerName + ".centroidEventValid" + m_auxSuffix;
  ATH_CHECK(m_centroidEventValidKey.initialize());
  m_centroidStatusKey = m_zdcSumContainerName + ".centroidStatus" + m_auxSuffix;
  ATH_CHECK(m_centroidStatusKey.initialize());
  m_RPDChannelSubtrAmpKey = m_zdcSumContainerName + ".RPDChannelSubtrAmp" + m_auxSuffix;
  ATH_CHECK(m_RPDChannelSubtrAmpKey.initialize());
  m_RPDSubtrAmpSumKey = m_zdcSumContainerName + ".RPDSubtrAmpSum" + m_auxSuffix;
  ATH_CHECK(m_RPDSubtrAmpSumKey.initialize());
  m_xCentroidPreGeomCorPreAvgSubtrKey = m_zdcSumContainerName + ".xCentroidPreGeomCorPreAvgSubtr" + m_auxSuffix;
  ATH_CHECK(m_xCentroidPreGeomCorPreAvgSubtrKey.initialize());
  m_yCentroidPreGeomCorPreAvgSubtrKey = m_zdcSumContainerName + ".yCentroidPreGeomCorPreAvgSubtr" + m_auxSuffix;
  ATH_CHECK(m_yCentroidPreGeomCorPreAvgSubtrKey.initialize());
  m_xCentroidPreAvgSubtrKey = m_zdcSumContainerName + ".xCentroidPreAvgSubtr" + m_auxSuffix;
  ATH_CHECK(m_xCentroidPreAvgSubtrKey.initialize());
  m_yCentroidPreAvgSubtrKey = m_zdcSumContainerName + ".yCentroidPreAvgSubtr" + m_auxSuffix;
  ATH_CHECK(m_yCentroidPreAvgSubtrKey.initialize());
  m_xCentroidKey = m_zdcSumContainerName + ".xCentroid" + m_auxSuffix;
  ATH_CHECK(m_xCentroidKey.initialize());
  m_yCentroidKey = m_zdcSumContainerName + ".yCentroid" + m_auxSuffix;
  ATH_CHECK(m_yCentroidKey.initialize());
  m_xRowCentroidKey = m_zdcSumContainerName + ".xRowCentroid" + m_auxSuffix;
  ATH_CHECK(m_xRowCentroidKey.initialize());
  m_yColCentroidKey = m_zdcSumContainerName + ".yColCentroid" + m_auxSuffix;
  ATH_CHECK(m_yColCentroidKey.initialize());
  m_reactionPlaneAngleKey = m_zdcSumContainerName + ".reactionPlaneAngle" + m_auxSuffix;
  ATH_CHECK(m_reactionPlaneAngleKey.initialize());
  m_cosDeltaReactionPlaneAngleKey = m_zdcSumContainerName + ".cosDeltaReactionPlaneAngle" + m_auxSuffix;
  ATH_CHECK(m_cosDeltaReactionPlaneAngleKey.initialize());

  if (m_writeAux && m_auxSuffix != "") {
    ATH_MSG_DEBUG("suffix string = " << m_auxSuffix);
  }

  m_init = true;

  return StatusCode::SUCCESS;
}

void RpdSubtractCentroidTool::reset()
{
  m_eventStatus = false;
  for (auto& status : m_centroidStatus) {
    status.reset();
    status.set(ValidBit, true);
  }
  for (auto& sidev : m_subtrAmp) {
    std::fill(sidev.begin(), sidev.end(), 0.0);
  }
  for (auto& sidev : m_subtrAmpRowSum) {
    std::fill(sidev.begin(), sidev.end(), 0.0);
  }
  for (auto& sidev : m_subtrAmpColSum) {
    std::fill(sidev.begin(), sidev.end(), 0.0);
  }
  m_subtrAmpSum = {0.0, 0.0};
  m_xCentroidPreGeomCorPreAvgSubtr = {0.0, 0.0};
  m_yCentroidPreGeomCorPreAvgSubtr = {0.0, 0.0};
  m_xCentroidPreAvgSubtr = {0.0, 0.0};
  m_yCentroidPreAvgSubtr = {0.0, 0.0};
  m_xCentroid = {0.0, 0.0};
  m_yCentroid = {0.0, 0.0};
  for (auto& sidev : m_xRowCentroid) {
    std::fill(sidev.begin(), sidev.end(), 0.0);
  }
  for (auto& sidev : m_yColCentroid) {
    std::fill(sidev.begin(), sidev.end(), 0.0);
  }
  m_reactionPlaneAngle = {0.0, 0.0};
  m_cosDeltaReactionPlaneAngle = 0;
}

bool RpdSubtractCentroidTool::readAOD(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer)
{
  // initialize read handles from read handle keys
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) {
    return false;
  }
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> xposRelHandle(m_xposRelKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> yposRelHandle(m_yposRelKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned short> rowHandle(m_rowKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned short> colHandle(m_colKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibEnergyHandle(m_ZDCModuleCalibEnergyKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcModuleStatusHandle(m_ZDCModuleStatusKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelSumAdcHandle(m_RPDChannelAmplitudeKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelSumAdcCalibHandle(m_RPDChannelAmplitudeCalibKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelMaxADCHandle(m_RPDChannelMaxADCKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelMaxADCCalibHandle(m_RPDChannelMaxADCCalibKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> rpdChannelPileupFracHandle(m_RPDChannelPileupFracKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> rpdChannelStatusHandle(m_RPDChannelStatusKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> rpdSideStatusHandle(m_RPDSideStatusKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcFinalEnergyHandle(m_ZDCFinalEnergyKey);
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, unsigned int> zdcStatusHandle(m_ZDCStatusKey);

  ATH_MSG_DEBUG("Processing modules");

  for (const auto zdcModule : moduleContainer) {
    int side = -1;
    if (zdcModule->zdcSide() == -1) {
      side = 0;
    } else if (zdcModule->zdcSide() == 1) {
      side = 1;
    } else {
      continue;
    }
    if (zdcModule->zdcType() == 0) {
      // this is a ZDC module
      if (zdcModule->zdcModule() == 0) {
        // this is an EM module
        m_emCalibEnergy.at(side) = zdcModuleCalibEnergyHandle(*zdcModule);
        m_emStatus.at(side) = zdcModuleStatusHandle(*zdcModule);
      }
    } else if (zdcModule->zdcType() == 1) {
      // this is a Run 3 RPD module
      // (it is assumed that this tool will not be invoked otherwise)
      //
      unsigned int const& rpdChannel = zdcModule->zdcChannel();
      if (rpdChannel > 15) {
        ATH_MSG_WARNING("Invalid RPD channel found on side " << side << ", channel number = " << rpdChannel << ", skipping this module");
        continue;
      } else {
        unsigned short const& row = rowHandle(*zdcModule);
        unsigned short const& col = colHandle(*zdcModule);
        m_rpdChannelData.at(side).at(row).at(col).channel = rpdChannel;
        m_rpdChannelData.at(side).at(row).at(col).xposRel = xposRelHandle(*zdcModule);
        m_rpdChannelData.at(side).at(row).at(col).yposRel = yposRelHandle(*zdcModule);
        m_rpdChannelData.at(side).at(row).at(col).row = rowHandle(*zdcModule);
        m_rpdChannelData.at(side).at(row).at(col).col = colHandle(*zdcModule);
        if (m_useRpdSumAdc) {
          if (m_useCalibDecorations) {
            m_rpdChannelData.at(side).at(row).at(col).amp = rpdChannelSumAdcCalibHandle(*zdcModule);
          } else {
            m_rpdChannelData.at(side).at(row).at(col).amp = rpdChannelSumAdcHandle(*zdcModule);
          }
        } else {
          if (m_useCalibDecorations) {
            m_rpdChannelData.at(side).at(row).at(col).amp = rpdChannelMaxADCCalibHandle(*zdcModule);
          } else {
            m_rpdChannelData.at(side).at(row).at(col).amp = rpdChannelMaxADCHandle(*zdcModule);
          }
        }
        m_rpdChannelData.at(side).at(row).at(col).pileupFrac = rpdChannelPileupFracHandle(*zdcModule);
        m_rpdChannelData.at(side).at(row).at(col).status = rpdChannelStatusHandle(*zdcModule);
      }
    }
  }

  for (const auto zdcSum: moduleSumContainer) {
    int side = -1;
    if (zdcSum->zdcSide() == -1) {
      side = 0;
    } else if (zdcSum->zdcSide() == 1) {
      side = 1;
    } else {
      continue;
    }
    m_rpdSideStatus.at(side) = rpdSideStatusHandle(*zdcSum);
    m_zdcSideStatus.at(side) = zdcStatusHandle(*zdcSum);
    m_zdcFinalEnergy.at(side) = zdcFinalEnergyHandle(*zdcSum);
  }

  return true;
}

bool RpdSubtractCentroidTool::checkZdcRpdValidity(unsigned int side)
{
  if (!m_zdcSideStatus.at(side)) {
    // zdc bad
    m_centroidStatus.at(side).set(ZDCInvalidBit, true);
    m_centroidStatus.at(side).set(ValidBit, false);
  } else {
    // zdc good
    if (m_zdcFinalEnergy.at(side) < m_minZdcEnergy[side]) {
      m_centroidStatus.at(side).set(InsufficientZDCEnergyBit, true);
      m_centroidStatus.at(side).set(ValidBit, false);
    }
    if (m_zdcFinalEnergy.at(side) > m_maxZdcEnergy[side]) {
      m_centroidStatus.at(side).set(ExcessiveZDCEnergyBit, true);
      m_centroidStatus.at(side).set(ValidBit, false);
    }
  }

  if (m_emStatus.at(side)[ZDCPulseAnalyzer::FailBit]) {
    // em bad
    m_centroidStatus.at(side).set(EMInvalidBit, true);
    m_centroidStatus.at(side).set(ValidBit, false);
  } else {
    // em good
    if (m_emCalibEnergy.at(side) < m_minEmEnergy[side]) {
      m_centroidStatus.at(side).set(InsufficientEMEnergyBit, true);
      m_centroidStatus.at(side).set(ValidBit, false);
    }
    if (m_emCalibEnergy.at(side) > m_maxEmEnergy[side]) {
      m_centroidStatus.at(side).set(ExcessiveEMEnergyBit, true);
      m_centroidStatus.at(side).set(ValidBit, false);
    }
  }

  if (m_rpdSideStatus.at(side)[RPDDataAnalyzer::OutOfTimePileupBit]) {
    m_centroidStatus.at(side).set(PileupBit, true);
  }

  for (unsigned int row = 0; row < m_nRows; row++) {
    for (unsigned int col = 0; col < m_nCols; col++) {
      if (m_rpdChannelData.at(side).at(row).at(col).pileupFrac > m_pileupMaxFrac[side]) {
        m_centroidStatus.at(side).set(ExcessivePileupBit, true);
        m_centroidStatus.at(side).set(ValidBit, false);
      }
    }
  }

  if (!m_rpdSideStatus.at(side)[RPDDataAnalyzer::ValidBit]) {
    m_centroidStatus.at(side).set(RPDInvalidBit, true);
    m_centroidStatus.at(side).set(ValidBit, false);
    return false;
  }

  return true;
}

bool RpdSubtractCentroidTool::subtractRpdAmplitudes(unsigned int side)
{
  for (unsigned int row = 0; row < m_nRows; row++) {
    for (unsigned int col = 0; col < m_nCols; col++) {
      float subtrAmp;
      if (row == m_nRows - 1) {
        // top row -> nothing to subtract
        subtrAmp = m_rpdChannelData.at(side).at(row).at(col).amp;
      } else {
        // other rows -> subtract the tile above this one
        subtrAmp = m_rpdChannelData.at(side).at(row).at(col).amp - m_rpdChannelData.at(side).at(row + 1).at(col).amp;
      }
      m_rpdChannelData.at(side).at(row).at(col).subtrAmp = subtrAmp;
      m_subtrAmp.at(side).at(m_rpdChannelData.at(side).at(row).at(col).channel) = subtrAmp;
      m_subtrAmpRowSum.at(side).at(row) += subtrAmp;
      m_subtrAmpColSum.at(side).at(col) += subtrAmp;
      m_subtrAmpSum.at(side) += subtrAmp;
    }
  }

  if (m_subtrAmpSum.at(side) <= 0) {
    m_centroidStatus.at(side).set(ZeroSumBit, true);
    m_centroidStatus.at(side) &= ~(1 << ValidBit);
    return false;
  }

  for (unsigned int row = 0; row < m_nRows; row++) {
    for (unsigned int col = 0; col < m_nCols; col++) {
      const float &subtrAmp = m_rpdChannelData.at(side).at(row).at(col).subtrAmp;
      if (subtrAmp < 0 && -subtrAmp/m_subtrAmpSum.at(side) > m_maximumNegativeSubtrAmpFrac[side]) {
        m_centroidStatus.at(side).set(ExcessiveSubtrUnderflowBit, true);
        m_centroidStatus.at(side) &= ~(1 << ValidBit);
      }
    }
  }

  return true;
}

void RpdSubtractCentroidTool::calculateDetectorCentroid(unsigned int side)
{
  for (unsigned int col = 0; col < m_nCols; col++) {
    m_xCentroidPreGeomCorPreAvgSubtr.at(side) += m_subtrAmpColSum.at(side).at(col)*m_rpdChannelData.at(side).at(0).at(col).xposRel/m_subtrAmpSum.at(side);
  }

  for (unsigned int row = 0; row < m_nRows; row++) {
    m_yCentroidPreGeomCorPreAvgSubtr.at(side) += m_subtrAmpRowSum.at(side).at(row)*m_rpdChannelData.at(side).at(row).at(0).yposRel/m_subtrAmpSum.at(side);
  }

  for (unsigned int row = 0; row < m_nRows; row++) {
    if (m_subtrAmpRowSum.at(side).at(row) <= 0) continue;
    for (unsigned int col = 0; col < m_nCols; col++) {
      m_xRowCentroid.at(side).at(row) += m_rpdChannelData.at(side).at(row).at(col).subtrAmp*m_rpdChannelData.at(side).at(row).at(col).xposRel/m_subtrAmpRowSum.at(side).at(row);
    }
    m_centroidStatus.at(side).set(Row0ValidBit + row, true);
  }

  for (unsigned int col = 0; col < m_nCols; col++) {
    if (m_subtrAmpColSum.at(side).at(col) <= 0) continue;
    for (unsigned int row = 0; row < m_nRows; row++) {
      m_yColCentroid.at(side).at(col) += m_rpdChannelData.at(side).at(row).at(col).subtrAmp*m_rpdChannelData.at(side).at(row).at(col).yposRel/m_subtrAmpColSum.at(side).at(col);
    }
    m_centroidStatus.at(side).set(Col0ValidBit + col, true);
  }
}

void RpdSubtractCentroidTool::geometryCorrection(unsigned int side)
{
  m_xCentroidPreAvgSubtr.at(side) = m_xCentroidPreGeomCorPreAvgSubtr.at(side) - m_alignmentXOffset.at(side);
  m_yCentroidPreAvgSubtr.at(side) = m_yCentroidPreGeomCorPreAvgSubtr.at(side) - m_alignmentYOffset.at(side);
  /** ROTATIONS CORRECTIONS GO HERE */
}

void RpdSubtractCentroidTool::subtractAverageCentroid(unsigned int side)
{
  m_xCentroid.at(side) = m_xCentroidPreAvgSubtr.at(side) - m_avgXCentroid.at(side);
  m_yCentroid.at(side) = m_yCentroidPreAvgSubtr.at(side) - m_avgYCentroid.at(side);
}

void RpdSubtractCentroidTool::calculateReactionPlaneAngle(unsigned int side)
{
  double angle = TMath::ATan2(m_yCentroid.at(side), m_xCentroid.at(side));
  // our angles are now simply the angle of the centroid in ATLAS coordinates on either side
  // however, we expect correlated deflection, so we want the difference between the angles
  // to be small when the centroids are in opposite quadrants of the two RPDs
  // therefore, we add pi to side A (chosen arbitrarily)
  if (side == 1) angle += TMath::Pi();
  // also, restrict to [-pi, pi)
  // we choose this rather than (-pi, pi] for ease of binning the edge case +/- pi
  if (angle >= TMath::Pi()) angle -= TMath::TwoPi();
  m_reactionPlaneAngle.at(side) = angle;
}

void RpdSubtractCentroidTool::writeAOD(xAOD::ZdcModuleContainer const& moduleSumContainer) const
{
  ATH_MSG_DEBUG("Adding variables with suffix=" + m_auxSuffix);

  // initialize write handles from write handle keys
  SG::WriteDecorHandle<xAOD::ZdcModuleContainer, bool> centroidEventValidHandle(m_centroidEventValidKey);
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

  for (const auto zdcSum: moduleSumContainer) {
    int side = -1;
    if (zdcSum->zdcSide() == -1) {
      side = 0;
    } else if (zdcSum->zdcSide() == 1) {
      side = 1;
    } else {
      // global sum container
      centroidEventValidHandle(*zdcSum) = m_eventStatus;
      cosDeltaReactionPlaneAngleHandle(*zdcSum) = m_cosDeltaReactionPlaneAngle;
      continue;
    }
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


StatusCode RpdSubtractCentroidTool::recoZdcModules(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer)
{
  if (moduleContainer.size() == 0) {
    // no modules - do nothing
    return StatusCode::SUCCESS;
  }

  reset();
  if (!readAOD(moduleContainer, moduleSumContainer)) return StatusCode::FAILURE;

  for (unsigned int side : {0, 1}) {
    if (!checkZdcRpdValidity(side)) continue; // rpd invalid -> don't calculate centroid
    if (!subtractRpdAmplitudes(side)) continue; // bad total sum -> don't calculate centroid
    calculateDetectorCentroid(side);
    geometryCorrection(side);
    subtractAverageCentroid(side);
    calculateReactionPlaneAngle(side);
    m_centroidStatus.at(side).set(HasCentroidBit, true);
  }

  if (m_centroidStatus.at(0)[HasCentroidBit] && m_centroidStatus.at(1)[HasCentroidBit]) {
    m_cosDeltaReactionPlaneAngle = TMath::Cos(m_reactionPlaneAngle.at(0) - m_reactionPlaneAngle.at(1));
  }

  if (m_centroidStatus.at(0)[ValidBit] && m_centroidStatus.at(1)[ValidBit]) {
    m_eventStatus = true; // event is good for analysis
  }

  writeAOD(moduleSumContainer);

  ATH_MSG_DEBUG("Finishing event processing");

  return StatusCode::SUCCESS;
}

StatusCode RpdSubtractCentroidTool::reprocessZdc()
{
  if (!m_init) {
    ATH_MSG_WARNING("Tool not initialized!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Trying to retrieve " << m_zdcModuleContainerName);

  xAOD::ZdcModuleContainer const* zdcModules = nullptr;
  ATH_CHECK(evtStore()->retrieve(zdcModules, m_zdcModuleContainerName));

  xAOD::ZdcModuleContainer const* zdcSums = nullptr;
  ATH_CHECK(evtStore()->retrieve(zdcSums, m_zdcSumContainerName));

  ATH_CHECK(recoZdcModules(*zdcModules, *zdcSums));

  return StatusCode::SUCCESS;
}

} // namespace ZDC
