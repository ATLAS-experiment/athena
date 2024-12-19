/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDSUBTRACTCENTROIDTOOL_H
#define ZDCANALYSIS_RPDSUBTRACTCENTROIDTOOL_H


#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "ZdcAnalysis/RPDDataAnalyzer.h"
#include "ZdcAnalysis/ZDCPulseAnalyzer.h"
#include "ZdcUtils/RPDUtils.h"

#include "AsgTools/AsgTool.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include <vector>
#include <array>
#include <bitset>
#include <optional>

namespace ZDC {

class RpdSubtractCentroidTool : public virtual IZdcAnalysisTool, public asg::AsgTool {
  ASG_TOOL_CLASS(RpdSubtractCentroidTool, ZDC::IZdcAnalysisTool)
 public:
  enum {
    ValidBit                     =  0, // analysis and output are valid
    HasCentroidBit               =  1, // centroid was calculated but analysis is invalid
    ZDCInvalidBit                =  2, // ZDC analysis on this side failed => analysis is invalid
    InsufficientZDCEnergyBit     =  3, // ZDC energy on this side is below minimum => analysis is invalid
    ExcessiveZDCEnergyBit        =  4, // ZDC energy on this side is above maximum => analysis is invalid
    EMInvalidBit                 =  5, // EM analysis on this side failed => analysis is invalid
    InsufficientEMEnergyBit      =  6, // EM energy on this side is below minimum => analysis is invalid
    ExcessiveEMEnergyBit         =  7, // EM energy on this side is above maximum => analysis is invalid
    RPDInvalidBit                =  8, // RPD analysis on this side was invalid => calculation stopped and analysis is invalid
    PileupBit                    =  9, // pileup was detected in RPD on this side
    ExcessivePileupBit           = 10, // pileup was detected in RPD on this side and a channel exceeded the fractional limit => analysis is invalid
    ZeroSumBit                   = 11, // sum of subtracted RPD amplitudes on this side was not positive => calculation stopped and analysis is invalid
    ExcessiveSubtrUnderflowBit   = 12, // a subtracted RPD amplitude on this side was negative and exceeded the fractional limit => analysis is invalid

    Row0ValidBit                 = 13, // row 0 x centroid is valid
    Row1ValidBit                 = 14, // row 1 x centroid is valid
    Row2ValidBit                 = 15, // row 2 x centroid is valid
    Row3ValidBit                 = 16, // row 3 x centroid is valid
    Col0ValidBit                 = 17, // column 0 y centroid is valid
    Col1ValidBit                 = 18, // column 1 y centroid is valid
    Col2ValidBit                 = 19, // column 2 y centroid is valid
    Col3ValidBit                 = 20, // column 3 y centroid is valid

    N_STATUS_BITS
  };

  explicit RpdSubtractCentroidTool(const std::string& name);
  ~RpdSubtractCentroidTool() override = default;

  // this tool should never be copied or moved
  RpdSubtractCentroidTool(RpdSubtractCentroidTool const&) = delete;
  RpdSubtractCentroidTool& operator=(RpdSubtractCentroidTool const&) = delete;
  RpdSubtractCentroidTool(RpdSubtractCentroidTool &&) = delete;
  RpdSubtractCentroidTool& operator=(RpdSubtractCentroidTool &&) = delete;

  // interface from AsgTool and IZdcAnalysisTool
  StatusCode initialize() override;
  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

 private:
  // tool properties to be configured in Python
  std::string m_configuration;
  std::string m_ZDCModuleContainerName;
  std::string m_ZDCSumContainerName;
  bool m_writeAux{};
  std::string m_auxSuffix;
  // these are optional and null by default; if their values are set, they will overwrite the bulk configuration from config string
  RPDUtils::OptionalToolProperty<std::vector<float>> m_forceMinZDCEnergy;
  RPDUtils::OptionalToolProperty<std::vector<float>> m_forceMaxZDCEnergy;
  RPDUtils::OptionalToolProperty<std::vector<float>> m_forceMinEMEnergy;
  RPDUtils::OptionalToolProperty<std::vector<float>> m_forceMaxEMEnergy;
  RPDUtils::OptionalToolProperty<std::vector<float>> m_forcePileupMaxFrac;
  RPDUtils::OptionalToolProperty<std::vector<float>> m_forceMaximumNegativeSubtrAmpFrac;
  RPDUtils::OptionalToolProperty<bool> m_forceUseRPDSumAdc;
  RPDUtils::OptionalToolProperty<bool> m_forceUseCalibDecorations;

  // these are the final reconstruction parameters
  std::vector<float> m_minZDCEnergy;
  std::vector<float> m_maxZDCEnergy;
  std::vector<float> m_minEMEnergy;
  std::vector<float> m_maxEMEnergy;
  std::vector<float> m_pileupMaxFrac;
  std::vector<float> m_maximumNegativeSubtrAmpFrac;
  bool m_useRPDSumAdc{};
  bool m_useCalibDecorations{};

  StatusCode initializeKey(std::string const& containerName, SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> & readHandleKey, std::string const& key);
  StatusCode initializeKey(std::string const& containerName, SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> & writeHandleKey, std::string const& key);
  static bool nonNegative(float const x) { return x >= 0; }
  static bool anyNonNegative(std::vector<float> const& v) { return std::any_of(v.begin(), v.end(), nonNegative); }

  // internal properties
  //
  bool m_initialized = false;
  bool m_readZDCDecorations = false;

  // results from RPD analysis needed for centroid calculation (read from AOD)
  //
  struct RPDChannelData {
    unsigned int channel;
    float xposRel{};
    float yposRel{};
    unsigned short row{};
    unsigned short col{};
    float amp{};
    float subtrAmp{};
    float pileupFrac{};
    unsigned int status{};
  };
  std::array<std::bitset<RPDDataAnalyzer::N_STATUS_BITS>, 2> m_RPDSideStatus {}; /** RPD analysis status word on each side */
  std::optional<std::array<unsigned int, 2>> m_ZDCSideStatus; /** ZDC analysis status on each side */
  std::optional<std::array<float, 2>> m_ZDCFinalEnergy; /** ZDC final (calibrated) energy on each side */
  std::optional<std::array<float, 2>> m_EMCalibEnergy; /** EM calibrated energy on each side */
  std::optional<std::array<std::bitset<ZDCPulseAnalyzer::N_STATUS_BITS>, 2>> m_EMStatus; /** EM modlue status word on each side */
  std::array<std::array<std::array<RPDChannelData, RPDUtils::nCols>, RPDUtils::nRows>, 2> m_RPDChannelData {}; /** RPD channel data for each channel (first index row, then index column) on each side */

  // alignment (geometry) and crossing angle correction, to be read from ZdcConditions
  //
  std::array<float, 2> m_alignmentXOffset {}; /** geometry + crossing angle correction in x (ATLAS coordinates) */
  std::array<float, 2> m_alignmentYOffset {}; /** geometry + crossing angle correction in y (ATLAS coordinates) */
  /** ROTATIONS GO HERE */

  // average centroids, to be read from monitoring histograms
  //
  std::array<float, 2> m_avgXCentroid {}; /** average x centroid */
  std::array<float, 2> m_avgYCentroid {}; /** average y centroid */

  // centroid calculation results (reset each event)
  //
  bool m_eventValid = false; /** event status */
  std::array<std::bitset<N_STATUS_BITS>, 2> m_centroidStatus {1 << ValidBit, 1 << ValidBit}; /** centroid status (valid by default) on each side */
  std::array<std::vector<float>, 2> m_subtrAmp = {
    std::vector<float>(RPDUtils::nChannels, 0.0),
    std::vector<float>(RPDUtils::nChannels, 0.0)
  }; /** subtracted amplitude for each channel on each side */
  std::array<std::array<float, RPDUtils::nRows>, 2> m_subtrAmpRowSum {}; /** subtracted amplitude for each row on each side */
  std::array<std::array<float, RPDUtils::nCols>, 2> m_subtrAmpColSum {}; /** subtracted amplitude for each column on each side */
  std::array<float, 2> m_subtrAmpSum {}; /** subtracted amplitude sum on each side */
  std::array<float, 2> m_xCentroidPreGeomCorPreAvgSubtr {};  /** x centroid before geomerty correction and before average subtraction (RPD detector coordinates) on each side */
  std::array<float, 2> m_yCentroidPreGeomCorPreAvgSubtr {};  /** y centroid before geomerty correction and before average subtraction (RPD detector coordinates) on each side */
  std::array<float, 2> m_xCentroidPreAvgSubtr {};  /** x centroid after geomerty correction and before average subtraction on each side */
  std::array<float, 2> m_yCentroidPreAvgSubtr {};  /** y centroid after geomerty correction and before average subtraction on each side */
  std::array<float, 2> m_xCentroid {};  /** x centroid after geomerty correction and after average subtraction on each side */
  std::array<float, 2> m_yCentroid {};  /** y centroid after geomerty correction and after average subtraction on each side */
  std::array<std::vector<float>, 2> m_xRowCentroid = {
    std::vector<float>(RPDUtils::nRows, 0.0),
    std::vector<float>(RPDUtils::nRows, 0.0)
  }; /** the x centroid for each row on each side */
  std::array<std::vector<float>, 2> m_yColCentroid = {
    std::vector<float>(RPDUtils::nCols, 0.0),
    std::vector<float>(RPDUtils::nCols, 0.0)
  }; /** the y centroid for each column on each side */
  std::array<float, 2> m_reactionPlaneAngle {}; /** reaction plane angle on each side */
  float m_cosDeltaReactionPlaneAngle = 0; /** cosine of difference between reaction plane angles of the two sides */

  // methods used for centroid calculation
  //
  enum class SubstepStatus {
    Success, // continue to next step
    Failure, // stop (do not proceed to next step) and propagate error to Athena
    SkipEvent, // stop (do not proceed to next step) and tell Athena the event was a success
  };
  void reset();
  SubstepStatus readAOD(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer);
  bool checkZdcRpdValidity(unsigned int side);
  bool subtractRpdAmplitudes(unsigned int side);
  void calculateDetectorCentroid(unsigned int side);
  void geometryCorrection(unsigned int side);
  void subtractAverageCentroid(unsigned int side);
  void calculateReactionPlaneAngle(unsigned int side);
  void writeAOD(xAOD::ZdcModuleContainer const& moduleSumContainer) const;

  // note that we leave the keys of read/write decor handle keys unset here since they will be set
  // by this tool's initialize(); this is because container names (and suffix) are taken as tool properties
  // and cannot be used in an in-class initializer (before constructor, where properties are declared)

  // read handle keys
  //
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
    this, "EventInfoKey", "EventInfo",
    "Location of the event info"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_xposRelKey {
    this, "xposRelKey", "",
    "X position of RPD tile center relative to center of RPD active area"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_yposRelKey {
    this, "yposRelKey", "",
    "Y position of RPD tile center relative to center of RPD active area"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_rowKey {
    this, "rowKey", "",
    "Row index of RPD channel"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_colKey {
    this, "colKey", "",
    "Column index of RPD channel"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZDCModuleCalibEnergyKey {
    this, "CalibEnergyKey", "",
    "ZDC module amplitude"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZDCModuleStatusKey {
    this, "ZDCModuleStatusKey", "",
    "ZDC module status word"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeKey {
    this, "RPDChannelAmplitudeKey", "",
    "RPD channel amplitude"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelAmplitudeCalibKey {
    this, "RPDChannelAmplitudeCalibKey", "",
    "Calibrated RPD channel amplitude"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxADCKey {
    this, "RPDChannelMaxADCKey", "",
    "RPD channel max ADC"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelMaxADCCalibKey {
    this, "RPDChannelMaxADCCalibKey", "",
    "Calibrated RPD channel max ADC"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelPileupFracKey {
    this, "RPDChannelPileupFracKey", "",
    "RPD channel (out of time) pileup as a fraction of non-pileup sum"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelStatusKey {
    this, "RPDChannelStatusKey", "",
    "RPD channel status word"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDSideStatusKey {
    this, "RPDStatusKey", "",
    "RPD side status word"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZDCFinalEnergyKey {
    this, "FinalEnergyKey", "",
    "ZDC final energy"
  };
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_ZDCStatusKey {
    this, "ZDCStatusKey", "",
    "ZDC sum status word"
  };

  // write handle keys
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_centroidEventValidKey {
    this, "centroidEventValidKey", "",
    "Event status: true if both centroids are valid, else false"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_centroidStatusKey {
    this, "centroidStatusKey", "",
    "Centroid status word"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDChannelSubtrAmpKey {
    this, "RPDChannelSubtrAmpKey", "",
    "RPD channel subtracted amplitudes (tile mass) used in centroid calculation"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_RPDSubtrAmpSumKey {
    this, "RPDSubtrAmpSumKey", "",
    "Sum of RPD channel subtracted amplitudes (total mass) used in centroid calculation"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xCentroidPreGeomCorPreAvgSubtrKey {
    this, "xCentroidPreGeomCorPreAvgSubtrKey", "",
    "X centroid before geometry corrections and before average centroid subtraction (RPD detector coordinates)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yCentroidPreGeomCorPreAvgSubtrKey {
    this, "yCentroidPreGeomCorPreAvgSubtrKey", "",
    "Y centroid before geometry corrections and before average centroid subtraction (RPD detector coordinates)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xCentroidPreAvgSubtrKey {
    this, "xCentroidPreAvgSubtrKey", "",
    "X centroid after geometry corrections and before average centroid subtraction"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yCentroidPreAvgSubtrKey {
    this, "yCentroidPreAvgSubtrKey", "",
    "Y centroid after geometry corrections and before average centroid subtraction"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xCentroidKey {
    this, "xCentroidKey", "",
    "X centroid after geometry corrections and after average centroid subtraction"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yCentroidKey {
    this, "yCentroidKey", "",
    "Y centroid after geometry corrections and after average centroid subtraction"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_xRowCentroidKey {
    this, "xRowCentroidKey", "",
    "Row X centroids before geometry corrections and before average centroid subtraction (RPD detector coordinates)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yColCentroidKey {
    this, "yColCentroidKey", "",
    "Column Y centroids before geometry corrections and before average centroid subtraction (RPD detector coordinates)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_reactionPlaneAngleKey {
    this, "reactionPlaneAngleKey", "",
    "Reaction plane angle in [-pi, pi) from the positive x axis"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_cosDeltaReactionPlaneAngleKey {
    this, "cosDeltaReactionPlaneAngleKey", "",
    "Cosine of the difference between the reaction plane angles of the two sides"
  };

};
} // namespace ZDC

#endif
