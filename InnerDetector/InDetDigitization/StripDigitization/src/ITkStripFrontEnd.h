/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SCT_DIGITIZATION_ITkStripFrontEnd_H
#define SCT_DIGITIZATION_ITkStripFrontEnd_H
// Inheritance
#include "AthenaBaseComps/AthAlgTool.h"
#include "SiDigitization/IFrontEnd.h"
// Athena
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "SiDigitization/IAmplifier.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
// STL
#include <string>

class SCT_ID;

namespace InDetDD {
  class SCT_DetectorManager;
}

namespace CLHEP {
  class HepRandomEngine;
}
/**
 * @brief simulation of the ITk Strips front-end electronics
 * working as a SiPreDigitsProcessor
 * models response of ABCstar chip amplifiers to 
 * collected charges, also does cross-talk, offset
 * variation and gain variation, in a correlated way
 */

struct ITkStripFrontEndData {
  std::vector<float> m_GainFactor; //!< generate gain per channel  (added to the gain per chip from calib data)
  std::array<std::vector<float>, 3> m_Analogue{};  //!< To hold the noise and amplifier response
  std::vector<int> m_StripHitsOnWafer; //!< Info about which strips are above threshold
};


class  ITkStripFrontEnd : public extends<AthAlgTool, IFrontEnd> {
 public:
  /**  constructor */
  ITkStripFrontEnd(const std::string& type, const std::string& name, const IInterface* parent);
  /** Destructor */
  virtual ~ITkStripFrontEnd() = default;  
  /** AlgTool initialize */
  virtual StatusCode initialize() override;
  /**use the baseclass default finalize */
  //
  /**
   * process the collection of pre digits: needed to go through all single-strip pre-digits to calculate
   * the amplifier response add noise (this could be moved elsewhere later) apply threshold do clustering
   * stripMax is for benefit of ITkStrips which can have different numbers of strips for each module 
   */
  virtual void process(SiChargedDiodeCollection& collection, CLHEP::HepRandomEngine* rndmEngine) const override;
  void doSignalChargeForHits(SiChargedDiodeCollection& collectione, ITkStripFrontEndData& data, const int& stripMax) const;
  void doThresholdCheckForRealHits(SiChargedDiodeCollection& collectione, ITkStripFrontEndData& data, const int& stripMax) const;
  // StatusCode doThresholdCheckForCrosstalkHits(SiChargedDiodeCollection& collection, ITkStripFrontEndData& data, const int& stripMax) const;
  // StatusCode doClustering(SiChargedDiodeCollection& collection, ITkStripFrontEndData& data, const int& stripMax) const;
  // StatusCode prepareGainAndOffset(SiChargedDiodeCollection& collection, const Identifier& moduleId, CLHEP::HepRandomEngine* rndmEngine, ITkStripFrontEndData& data, const int& stripMax) const;
  // StatusCode randomNoise(SiChargedDiodeCollection& collection, const Identifier& moduleId, CLHEP::HepRandomEngine* rndmEngine, ITkStripFrontEndData& data, const int& stripMax) const;
  // StatusCode addNoiseDiode(SiChargedDiodeCollection& collection, int strip, int tbin) const;
  static float meanValue(std::vector<float>& calibDataVect) ;
  void initVectors(int strips, ITkStripFrontEndData& data) const;

  
 private:

  enum CompressionMode { Level_X1X=1, Edge_01X=2, AnyHit_1XX_X1X_XX1=3 }; // Used for m_data_compression_mode (DataCompressionMode)
  enum ReadOutMode { Condensed=0, Expanded=1 }; // Used for m_data_readout_mode (DataReadOutMode)

  FloatProperty m_NoiseBarrel{this, "NoiseBarrel", 1500.0, "Noise factor, Barrel  (in the case of no use of calibration data)"};
  FloatProperty m_NoiseBarrel3{this, "NoiseBarrel3", 1541.0, "Noise factor, Barrel3  (in the case of no use of calibration data)"};
  FloatProperty m_NoiseInners{this, "NoiseInners", 1090.0, "Noise factor, EC Inners  (in the case of no use of calibration data)"};
  FloatProperty m_NoiseMiddles{this, "NoiseMiddles", 1557.0, "Noise factor, EC Middles (in the case of no use of calibration data)"};
  FloatProperty m_NoiseShortMiddles{this, "NoiseShortMiddles", 940.0, "Noise factor, EC Short Middles (in the case of no use of calibration data)"};
  FloatProperty m_NoiseOuters{this, "NoiseOuters", 1618.0, "Noise factor, Ec Outers  (in the case of no use of calibration data)"};
  DoubleProperty m_NOBarrel{this, "NOBarrel", 1.5e-5, "Noise factor, Barrel  (in the case of no use of calibration data)"};
  DoubleProperty m_NOBarrel3{this, "NOBarrel3", 2.1e-5, "Noise factor, Barrel3  (in the case of no use of calibration data)"};
  DoubleProperty m_NOInners{this, "NOInners", 5.0e-9, "Noise Occupancy, EC Inners  (in the case of no use of calibration data)"};
  DoubleProperty m_NOMiddles{this, "NOMiddles", 2.7e-5, "Noise Occupancy, EC Middles (in the case of no use of calibration data)"};
  DoubleProperty m_NOShortMiddles{this, "NOShortMiddles", 2.0e-9, "Noise Occupancy, EC Short Middles (in the case of no use of calibration data)"};
  DoubleProperty m_NOOuters{this, "NOOuters", 3.5e-5, "Noise Occupancy, Ec Outers  (in the case of no use of calibration data)"};
  BooleanProperty m_NoiseOn{this, "NoiseOn", true, "To know if noise is on or off when using calibration data"};
  BooleanProperty m_analogueNoiseOn{this, "AnalogueNoiseOn", true, "To know if analogue noise is on or off"};
  FloatProperty m_GainRMS{this, "GainRMS", 0.031, "Gain spread parameter within the strips for a given Chip gain"};
  FloatProperty m_Ospread{this, "Ospread", 0.0001, "offset spread within the strips for a given Chip offset"};
  FloatProperty m_OGcorr{this, "OffsetGainCorrelation", 0.00001, "Gain/offset correlation for the strips"};
  FloatProperty m_Threshold{this, "Threshold", 1.0, "Threshold"};
  FloatProperty m_timeOfThreshold{this, "TimeOfThreshold", 30.0, "Threshold time"};
  ShortProperty m_data_compression_mode{this, "DataCompressionMode", Edge_01X, "Front End Data Compression Mode: 1 is level mode X1X (default), 2 is edge mode 01X, 3 is any hit mode (1XX|X1X|XX1)"};
  ShortProperty m_data_readout_mode{this, "DataReadOutMode", Condensed, "Front End Data Read out mode Mode: 0 is condensed mode and 1 is expanded mode"};
  ToolHandle<IAmplifier> m_strip_amplifier{this, "ITkStripAmp", "ITkStripAmp", "Handle the Amplifier tool"}; //!< Handle the Amplifier tool


  const InDetDD::SCT_DetectorManager* m_ITkStripMgr{nullptr}; //!< Handle to SCT detector manager, also valid for ITkStrips
  const SCT_ID* m_ITkStripId{nullptr}; //!< Handle to SCT ID helper  also valid for ITkStrips
  StringProperty m_detMgrName{this, "DetectorManager", "SCT", "Name of DetectorManager to retrieve"};
};




#endif //ITkStripFrontEnd_H