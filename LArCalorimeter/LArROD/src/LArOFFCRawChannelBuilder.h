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
#include "CaloIdentifier/CaloCell_ID.h"
#include "CxxUtils/checker_macros.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Event classes
class LArDigitContainer;
class LArRawChannelContainer;
class LArOnlineID;
class CaloCell_ID;

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

  // PER-LAYER CONFIGURATION. The scalars above remain the global fallback,
  // so a job setting nothing new behaves as before. FilterThreshold is in
  // ADC, a different energy in every layer -- the useful threshold sits near
  // 3 sigma of local noise, 25 ADC in EMB/2 but 100 in EMEC-OW/1.
  // Keys are "<REGION>/<LAYER>": EMB, EMEC-OW, EMEC-IW, HEC, FCAL, and the
  // sampling (FCAL module).
  Gaudi::Property<std::map<std::string, double>> m_filterThresholdByLayer{
      this, "FilterThresholdByLayer", {},
      "Per-layer FilterThreshold, keyed <REGION>/<LAYER>; unlisted layers "
      "fall back to FilterThreshold"};
  Gaudi::Property<std::map<std::string, double>> m_q3CutByLayer{
      this, "Q3CutByLayer", {},
      "Per-layer Q3Cut; unlisted layers fall back to Q3Cut"};
  Gaudi::Property<std::map<std::string, double>> m_q3OffsetByLayer{
      this, "Q3OffsetByLayer", {},
      "Per-layer Q3Offset; unlisted layers fall back to Q3Offset"};
  Gaudi::Property<std::map<std::string, int>> m_nPulseByLayer{
      this, "NPulseByLayer", {},
      "Per-layer NPulse; unlisted layers fall back to NPulse"};

  /// Layers the correction may run in; empty means all. A layer left out gets
  /// an unreachable FilterThreshold, so nothing is subtracted and the output
  /// is bit-identical to the plain OF -- no separate code path.
  Gaudi::Property<std::vector<std::string>> m_enabledLayers{
      this, "EnabledLayers", {},
      "Layers where the correction may fire; empty means all"};

  /// Resolved parameters for one layer.
  struct LayerParams {
    double q3Cut = 0.1;
    double q3Offset = 2.0;
    double filterThreshold = 0.0;
    int nPulse = 0;
    double belowThreshold = 0.0;
    int belowTillReset = 0;
  };

  /// Five regions x at most four samplings. m_layerParams holds s_nSlots+1:
  /// the last entry is the global fallback for channels outside those.
  static constexpr size_t s_nSlots = 20;
  std::vector<LayerParams> m_layerParams;

  /// Slot for a region code (0=EMB..4=FCAL) and sampling/module;
  /// s_nSlots if out of range.
  static size_t slotOf(int region, int layer);
  /// Parse "<REGION>/<LAYER>" into a slot. Returns s_nSlots if unparseable.
  static size_t slotOfKey(const std::string& key);
  static std::string keyOfSlot(size_t slot);

  /// online hash -> slot, built on the first event: it needs the cabling,
  /// which is conditions data with an IOV.
  mutable std::vector<uint8_t> m_slotByHash ATLAS_THREAD_SAFE;
  mutable std::once_flag m_slotOnce ATLAS_THREAD_SAFE;
  mutable StatusCode m_slotStatus ATLAS_THREAD_SAFE{StatusCode::SUCCESS};
  StatusCode buildLayerMap(const EventContext& ctx) const;

  // Identifier helpers
  const LArOnlineID* m_onlineId = nullptr;
  const CaloCell_ID* m_caloId = nullptr;

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
                     const ILArShape::ShapeRef_t& shape, double pedestal,
                     const LayerParams& par) const;
};

#endif
