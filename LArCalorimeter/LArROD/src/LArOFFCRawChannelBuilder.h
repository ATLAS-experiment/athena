/* -*-c++-*- */
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARROD_LAROFFCRAWCHANNELBUILDER_H
#define LARROD_LAROFFCRAWCHANNELBUILDER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArElecCalib/ILArShape.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "LArRawConditions/LArDSPThresholdsComplete.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Event classes
class LArDigitContainer;
class LArRawChannelContainer;
class LArOnlineID;

class LArOFFCRawChannelBuilder : public AthReentrantAlgorithm {

 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  // Event input
  SG::ReadHandleKey<LArDigitContainer> m_digitKey{
      this, "LArDigitKey", "FREE", "SG Key of LArDigitContainer"};

  // Event output
  SG::WriteHandleKey<LArRawChannelContainer> m_rawChannelKey{
      this, "LArRawChannelKey", "LArRawChannels",
      "SG key of the output LArRawChannelContainer"};

  // Conditions input
  SG::ReadCondHandleKey<ILArPedestal> m_pedestalKey{
      this, "PedestalKey", "LArPedestal",
      "SG Key of Pedestal conditions object"};
  SG::ReadCondHandleKey<LArADC2MeV> m_adc2MeVKey{
      this, "ADC2MeVKey", "LArADC2MeV", "SG Key of ADC2MeV conditions object"};
  SG::ReadCondHandleKey<ILArOFC> m_ofcKey{this, "OFCKey", "LArOFC",
                                          "SG Key of OFC conditions object"};
  SG::ReadCondHandleKey<ILArShape> m_shapeKey{
      this, "ShapeKey", "LArShape", "SG Key of Shape conditions object"};

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{
      this, "CablingKey", "LArOnOffIdMap",
      "SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArDSPThresholdsComplete> m_run1DSPThresholdsKey{
      this, "Run1DSPThresholdsKey", "",
      "SG Key for thresholds to compute time and quality, run 1"};
  SG::ReadCondHandleKey<AthenaAttributeList> m_run2DSPThresholdsKey{
      this, "Run2DSPThresholdsKey", "",
      "SG Key for thresholds to compute time and quality, run 2"};

  // Other jobOptions
  Gaudi::Property<float> m_eCutFortQ{this, "ECutFortQ", 256.0,
                                     "Time and Quality will be computed only "
                                     "for channels with E above this value"};
  Gaudi::Property<bool> m_absECutFortQ{
      this, "absECut", true, "Cut on fabs(E) for Q and t computation"};
  Gaudi::Property<bool> m_useShapeDer{
      this, "useShapeDer", true,
      "Use shape derivative in Q-factor computation"};
  Gaudi::Property<bool> m_useDBFortQ{this, "useDB", true,
                                     "Use DB for cut on t,Q"};
  Gaudi::Property<int> m_firstSample{
      this, "firstSample", 0,
      "First of the 32 samples of the MC shape to be used"};

  /**
   * The OFFC algorithm extends the standard optimal filtering by identifying
   * in-time pulses and subtracting their expected contribution from future
   * samples. This mitigates out-of-time pileup by iteratively removing pulse
   * shapes that are consistent with the detector response.
   *
   * The algorithm operates on pedestal-subtracted ADC samples and applies:
   *  - an optimal filter convolution,
   *  - a local-maximum pulse-finding criterion,
   *  - an optional shape-consistency (Q3) quality cut,
   *  - and a forward subtraction using the OFC–shape convolution.
   *
   * The parameters below control noise suppression, pulse acceptance,
   * and the handling of overlapping pulses.
   */

  // OFFC algorithm configuration
  /// ADC threshold below which samples are considered noise.
  /// Consecutive samples below this threshold increment a counter used
  /// to decide when to reset the forward-subtraction cache.
  Gaudi::Property<double> m_belowThreshold{
      this, "BelowThreshold", 0,
      "ADC threshold below which samples are treated as noise"};

  /// Number of consecutive below-threshold samples required before
  /// the forward-subtraction cache is reset.
  /// This prevents stale pulse contributions from persisting indefinitely
  /// in quiet regions of the readout.
  Gaudi::Property<int> m_belowTillReset{
      this, "BelowTillReset", 0,
      "Number of consecutive noise samples before cache reset"};

  /// Maximum number of overlapping pulses that can be tracked simultaneously.
  /// Each accepted pulse occupies a context slot for the duration of the
  /// OFC–shape convolution window.
  Gaudi::Property<int> m_nPulse{
      this, "NPulse", 0, "Maximum number of overlapping pulses to subtract"};

  /// Quality cut used for pulse acceptance.
  /// Q3 is computed as the sum of absolute residuals between the filtered
  /// samples and the expected pulse shape around the peak.
  /// Pulses with Q3 below this threshold are accepted and subtracted.
  Gaudi::Property<double> m_Q3cut{
      this, "Q3Cut", 0, "Shape-consistency quality cut for pulse acceptance"};

  /// Minimum filtered amplitude required to consider a sample as a pulse peak.
  /// This suppresses spurious pulse finding due to noise fluctuations.
  Gaudi::Property<double> m_filterThreshold{
      this, "FilterThreshold", 0,
      "Minimum filtered amplitude for pulse finding"};

  // Identifier helper
  const LArOnlineID* m_onlineId = nullptr;

  // --- Member functions ---
  std::vector<double> convolvePulse(const ILArShape::ShapeRef_t& shape,
                                    const ILArOFC::OFCRef_t& ofc) const;

  double computeOFFC(const std::vector<short>& samples, int firstSample,
                     const ILArOFC::OFCRef_t& ofc,
                     const ILArShape::ShapeRef_t& shape, double pedestal) const;
};

#endif
