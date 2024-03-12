/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_RPDSUBTRACTCENTROIDTOOL_H
#define ZDCANALYSIS_RPDSUBTRACTCENTROIDTOOL_H

#include <array>
#include <bitset>

#include "AsgTools/AsgTool.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "xAODForward/ZdcModuleContainer.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include "xAODEventInfo/EventInfo.h"

namespace ZDC
{

class RpdSubtractCentroidTool : public virtual IZdcAnalysisTool, public asg::AsgTool
{
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
    ExcessiveSubtrUnderflowBit   = 12, // a subtracted RPD amplitude on this side was negatibe and exceeded the fractional limit => analysis is invalid

    Row0ValidBit                 = 13, // row 0 x centroid is valid
    Row1ValidBit                 = 14, // row 1 x centroid is valid
    Row2ValidBit                 = 15, // row 2 x centroid is valid
    Row3ValidBit                 = 16, // row 3 x centroid is valid
    Col0ValidBit                 = 17, // column 0 y centroid is valid
    Col1ValidBit                 = 18, // column 1 y centroid is valid
    Col2ValidBit                 = 19, // column 2 y centroid is valid
    Col3ValidBit                 = 20, // column 3 y centroid is valid
  };

  RpdSubtractCentroidTool(const std::string& name);
  virtual ~RpdSubtractCentroidTool() override;

  // interface from AsgTool and IZdcAnalysisTool
  StatusCode initialize() override;
  StatusCode recoZdcModules(const xAOD::ZdcModuleContainer& moduleContainer, const xAOD::ZdcModuleContainer& moduleSumContainer) override;
  StatusCode reprocessZdc() override;

 private:
  // job properties
  //
  std::string m_zdcModuleContainerName;
  std::string m_zdcSumContainerName;
  bool m_writeAux;
  std::string m_auxSuffix;

  std::vector<float> m_minZdcEnergy;
  std::vector<float> m_maxZdcEnergy;
  std::vector<float> m_minEmEnergy;
  std::vector<float> m_maxEmEnergy;
  std::vector<float> m_pileupMaxFrac;
  std::vector<float> m_maximumNegativeSubtrAmpFrac;
  bool m_useRpdSumAdc;
  bool m_useCalibDecorations;


  //   As in the ZDC analysis, we use side C = 0, side A = 1 for indexing
  //   note that ZDC side from AOD containers is C = -1, A = 1, but
  //     side C is mapped -1 -> 0 for indexing

  // constants
  //
  unsigned int const m_nRows = 4;
  unsigned int const m_nCols = 4;
  unsigned int const m_nChannels = m_nRows*m_nCols;

  // internal properties
  //
  std::string m_name;
  bool m_init;

  // results from RPD analysis needed for centroid calculation (read from AOD)
  //
  struct RpdChannelData {
    int channel;
    float xposRel;
    float yposRel;
    unsigned short row;
    unsigned short col;
    float amp;
    float subtrAmp;
    float pileupFrac;
    unsigned int status;
  };
  std::array<std::bitset<32>, 2> m_rpdSideStatus = {0, 0}; /** RPD analysis status word on each side */
  std::array<unsigned int, 2> m_zdcSideStatus = {0, 0}; /** ZDC analysis status on each side */
  std::array<float, 2> m_zdcFinalEnergy = {0, 0}; /** ZDC final (calibrated) energy on each side */
  std::array<float, 2> m_emCalibEnergy = {0, 0}; /** EM calibrated energy on each side */
  std::array<std::bitset<32>, 2> m_emStatus = {0, 0}; /** EM modlue status word on each side */
  std::array<std::vector<std::vector<RpdChannelData>>, 2> m_rpdChannelData = {
    std::vector<std::vector<RpdChannelData>>(m_nRows, std::vector<RpdChannelData>(m_nCols)),
    std::vector<std::vector<RpdChannelData>>(m_nRows, std::vector<RpdChannelData>(m_nCols))
  }; /** RPD channel data for each channel (first index row, then index column) on each side */

  // alignment (geometry) and crossing angle correction, to be read from ZdcConditions
  //
  std::array<float, 2> m_alignmentXOffset = {0.0, 0.0}; /** geometry + crossing angle correction in x (ATLAS coordinates) */
  std::array<float, 2> m_alignmentYOffset = {0.0, 0.0}; /** geometry + crossing angle correction in y (ATLAS coordinates) */
  /** ROTATIONS GO HERE */

  // average centroids, to be read from monitoring histograms
  //
  std::array<float, 2> m_avgXCentroid = {0.0, 0.0}; /** average x centroid */
  std::array<float, 2> m_avgYCentroid = {0.0, 0.0}; /** average y centroid */

  // centroid calculation results (reset each event)
  //
  bool m_eventStatus; /** event status */
  std::array<std::bitset<32>, 2> m_centroidStatus = {1 << ValidBit, 1 << ValidBit}; /** centroid status (valid by default) on each side */
  std::array<std::vector<float>, 2> m_subtrAmp = {
    std::vector<float>(m_nChannels, 0.0),
    std::vector<float>(m_nChannels, 0.0)
  }; /** subtracted amplitude for each channel on each side */
  std::array<std::vector<float>, 2> m_subtrAmpRowSum = {
    std::vector<float>(m_nRows, 0.0),
    std::vector<float>(m_nRows, 0.0)
  }; /** subtracted amplitude for each row on each side */
  std::array<std::vector<float>, 2> m_subtrAmpColSum = {
    std::vector<float>(m_nCols, 0.0),
    std::vector<float>(m_nCols, 0.0)
  }; /** subtracted amplitude for each column on each side */
  std::array<float, 2> m_subtrAmpSum = {0.0, 0.0}; /** subtracted amplitude sum on each side */
  std::array<float, 2> m_xCentroidPreGeomCorPreAvgSubtr = {0.0, 0.0};  /** x centroid before geomerty correction and before average subtraction (RPD detector coordinates) on each side */
  std::array<float, 2> m_yCentroidPreGeomCorPreAvgSubtr = {0.0, 0.0};  /** y centroid before geomerty correction and before average subtraction (RPD detector coordinates) on each side */
  std::array<float, 2> m_xCentroidPreAvgSubtr = {0.0, 0.0};  /** x centroid after geomerty correction and before average subtraction on each side */
  std::array<float, 2> m_yCentroidPreAvgSubtr = {0.0, 0.0};  /** y centroid after geomerty correction and before average subtraction on each side */
  std::array<float, 2> m_xCentroid = {0.0, 0.0};  /** x centroid after geomerty correction and after average subtraction on each side */
  std::array<float, 2> m_yCentroid = {0.0, 0.0};  /** y centroid after geomerty correction and after average subtraction on each side */
  std::array<std::vector<float>, 2> m_xRowCentroid = {
    std::vector<float>(m_nRows, 0.0),
    std::vector<float>(m_nRows, 0.0)
  }; /** the x centroid for each row on each side */
  std::array<std::vector<float>, 2> m_yColCentroid = {
    std::vector<float>(m_nCols, 0.0),
    std::vector<float>(m_nCols, 0.0)
  }; /** the y centroid for each column on each side */
  std::array<float, 2> m_reactionPlaneAngle = {0.0, 0.0}; /** reaction plane angle on each side */
  float m_cosDeltaReactionPlaneAngle = 0; /** cosine of difference between reaction plane angles of the two sides */

  // methods used for centroid calculation
  //
  void reset();
  bool readAOD(xAOD::ZdcModuleContainer const& moduleContainer, xAOD::ZdcModuleContainer const& moduleSumContainer);
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
    "Row X centroids after geometry corrections and after average centroid subtraction"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_yColCentroidKey {
    this, "yColCentroidKey", "",
    "Column Y centroids after geometry corrections and after average centroid subtraction"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_reactionPlaneAngleKey {
    this, "reactionPlaneAngleKey", "",
    "Reaction plane angle in [-pi, pi) from the positive x axis (angle of centorid on side C, angle of centroid + pi on side A)"
  };
  SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_cosDeltaReactionPlaneAngleKey {
    this, "cosDeltaReactionPlaneAngleKey", "",
    "Cosine of the difference between the reaction plane angles of the two sides"
  };

};
} // namespace ZDC

#endif
