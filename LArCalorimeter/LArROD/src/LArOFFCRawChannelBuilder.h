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

#include <atomic>
#include <vector>

// Event classes
class LArDigitContainer;
class LArRawChannelContainer;
class LArOnlineID;

class LArOFFCRawChannelBuilder : public AthReentrantAlgorithm {

 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;

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
  /// Index of the digit sample the OFC window starts at, i.e. the amplitude is
  /// that of sum_j (samples[firstSample+j]-pedestal)*OFC_a[j]. Not an offset
  /// into the reference shape: the digitisation writes shape index
  /// k-nPreceedingSamples into digit sample k.
  Gaudi::Property<int> m_firstSample{
      this, "firstSample", 0,
      "Index of the digit sample the OFC window starts at"};

  /**
   * The OFFC extends optimal filtering by finding pulses in the preceding
   * samples and subtracting their expected filter response from the later
   * ones. At every position of the pedestal-subtracted digit it applies:
   *  - the optimal filter,
   *  - a local-maximum pulse search on the corrected filter output,
   *  - a shape-consistency (Q3) cut against that same response,
   *  - and a forward subtraction of the response of each accepted pulse.
   */

  // OFFC algorithm configuration
  /// ADC threshold below which samples are considered noise.
  /// Consecutive samples below this threshold increment a counter used
  /// to decide when to reset the forward-subtraction cache.
  Gaudi::Property<double> m_belowThreshold{
      this, "BelowThreshold", 0,
      "ADC threshold below which samples are treated as noise"};

  /// Number of consecutive below-threshold samples after which the pending
  /// corrections are dropped, so that stale ones cannot persist through a
  /// quiet region. Either this or BelowThreshold at zero disables the reset.
  Gaudi::Property<int> m_belowTillReset{
      this, "BelowTillReset", 0,
      "Number of consecutive noise samples before cache reset (<=0 disables)"};

  /// Maximum number of pulse corrections in flight at once. A slot is held
  /// until the pulse response has been fully subtracted, which for a typical
  /// shape outlasts the digit: in practice this caps the pulses per digit.
  Gaudi::Property<int> m_nPulse{
      this, "NPulse", 0, "Maximum number of pulse corrections in flight"};

  /// Quality cut for pulse acceptance. Q3 sums the absolute residuals of the
  /// corrected filter output against the expected response at lags -2, -1 and
  /// +1 from the candidate peak, and is zero for a clean pulse of any
  /// amplitude. The cut is applied to Q3/A, so it is a fractional mismatch and
  /// does not have to be rescaled with the pulse size. Pulses below it are
  /// accepted and subtracted.
  Gaudi::Property<double> m_Q3cut{
      this, "Q3Cut", 0.1,
      "Shape-consistency cut for pulse acceptance, on Q3/A"};

  /// Absolute term of the Q3 cut, in ADC. Q3 has a noise floor of a few times
  /// the sample noise whatever the amplitude, so a purely relative cut would
  /// reject small pulses on noise alone. Set this comparable to
  /// FilterThreshold, the smallest amplitude worth looking at.
  Gaudi::Property<double> m_Q3Offset{
      this, "Q3Offset", 2.0,
      "Absolute term of the Q3 cut in ADC, covering the noise floor"};

  /// Minimum pile-up corrected amplitude required to accept a pulse peak.
  /// This suppresses spurious pulse finding due to noise fluctuations.
  Gaudi::Property<double> m_filterThreshold{
      this, "FilterThreshold", 0,
      "Minimum corrected amplitude for pulse finding"};

  // Identifier helper
  const LArOnlineID* m_onlineId = nullptr;

  /// Accepted pulses, and those NPulse left no room to subtract. Only touched
  /// on the rare accepted-pulse branch, summarised in finalize().
  mutable std::atomic<unsigned long> m_nSubtracted{0};
  mutable std::atomic<unsigned long> m_nDropped{0};

  // --- Member functions ---
  /// Filter output for a unit-amplitude pulse, tabulated over every offset of
  /// the shape against the OFC window at which the two overlap. Entry k is the
  /// response for an offset of k-(ofc.size()-1) samples.
  std::vector<double> pulseResponse(const ILArShape::ShapeRef_t& shape,
                                    const ILArOFC::OFCRef_t& ofc) const;

  double computeOFFC(const std::vector<short>& samples, int firstSample,
                     const ILArOFC::OFCRef_t& ofc,
                     const ILArShape::ShapeRef_t& shape, double pedestal) const;
};

#endif
