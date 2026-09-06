/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LArROD/src/LArRODBCIDCorrAlg.h
 * @date May 2026
 * @brief Subtract the BCID-dependent pile-up offset from the LArDigit samples before  digital filterings.
 */

#ifndef LARROD_LARRODBCIDCORRALG_H
#define LARROD_LARRODBCIDCORRALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArElecCalib/ILArMinBiasAverage.h"
#include "LArElecCalib/ILArShape.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

#include <vector>

class LArDigitContainer;
class LArOnlineID;

/**
 * @brief Subtract the BCID-dependent pile-up offset from the LArDigit samples.
 *
 * This is the ADC-level equivalent of the energy-level correction performed by
 * CaloBCIDAvgAlg and CaloBCIDCoeffs (Calo.Cell.doPileupOffsetBCIDCorr). The two
 * must not both be enabled, or the offset is subtracted twice.
 *
 * The sample filling in LArHitEMapToDigitAlg::ConvertHits2Samples is
 *
 *     j = i - ishift + firstSample;  sampleList[i] += Shape[j] * energy
 *
 * with @c ishift the bunch crossing of the energy deposit relative to the
 * triggered one, and @c firstSample the digitisation firstSample (that is,
 * -nPreceedingSamples; see LArDigitizationConfig.py). Averaging over minimum
 * bias events therefore gives, for digit sample @c i,
 *
 *     ADC_pileup[i] = mbEnergy * mu / adc2MeV
 *                   * sum_j Shape[j] * lumi[(bcid + i + firstSample - j) % 3564]
 *
 * which for firstSample = 0 is exactly the convolution implemented by
 * CaloBCIDCoeffs. Note that the sample index @c i and the shape index @c j
 * enter the luminosity lookup with opposite signs.
 */
class LArRODBCIDCorrAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute (const EventContext& ctx) const override;

private:
  /// Number of bunch crossings in one LHC orbit.
  static constexpr int s_nBCID = 3564;

  SG::ReadHandleKey<LArDigitContainer> m_digitKey
  { this, "LArDigitKey", "LArDigitContainer_MC",
    "SG key of the input LArDigitContainer" };

  SG::WriteHandleKey<LArDigitContainer> m_digitCorrKey
  { this, "OutputDigitKey", "LArDigitContainer_PileupCorrected",
    "SG key of the output LArDigitContainer" };

  SG::ReadCondHandleKey<LArADC2MeV> m_adc2MeVKey
  { this, "ADC2MeVKey", "LArADC2MeV",
    "SG key of the ADC2MeV conditions object" };

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey
  { this, "CablingKey", "LArOnOffIdMap",
    "SG key of the LArOnOffIdMapping object" };

  SG::ReadCondHandleKey<ILArShape> m_shapeKey
  { this, "ShapeKey", "LArShape",
    "SG key of the LArShape conditions object" };

  SG::ReadCondHandleKey<ILArMinBiasAverage> m_minBiasAvgKey
  { this, "MinBiasAvgKey", "LArPileupAverageSym",
    "SG key of the LArMinBiasAverage object" };

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
  { this, "EventInfo", "EventInfo", "SG key of the EventInfo object" };

  Gaudi::Property<std::vector<float> > m_beamIntensityPattern
  { this, "BeamIntensityPattern", {}, "LHC beam intensity pattern" };

  Gaudi::Property<int> m_firstSample
  { this, "firstSample", 0,
    "Digitisation firstSample: the shape index seen by digit sample 0. Must be "
    "set exactly as LArHitEMapToDigitAlg.firstSample, that is "
    "-nPreceedingSamples, or LAr.ROD.FirstSample when that is zero." };

  Gaudi::Property<int> m_minBunchCrossing
  { this, "MinBunchCrossing", -s_nBCID,
    "Earliest bunch crossing, relative to the triggered one, included in the "
    "correction. Set this to the earliest crossing actually overlaid by the "
    "digitisation to avoid over-subtracting in the preceding samples." };

  Gaudi::Property<int> m_maxBunchCrossing
  { this, "MaxBunchCrossing", s_nBCID,
    "Latest bunch crossing, relative to the triggered one, included in the "
    "correction." };

  /// Per-BCID luminosity, indexed by absolute BCID in [0, s_nBCID). Filled once
  /// in initialize() and only read afterwards, so it is safe to share between
  /// threads.
  std::vector<float> m_lumi;

  /// Online identifier helper, used to recognise HEC channels.
  const LArOnlineID* m_onlineId = nullptr;

  /**
   * @brief Fill @c m_lumi from the beam intensity pattern.
   *
   * Uses the same pattern-to-BCID mapping as BunchCrossingCondAlg, so that the
   * set of filled bunch crossings matches the one CaloBCIDLumi would see.
   */
  StatusCode fillLumi();

  /**
   * @brief Compute the mean pile-up ADC offset per sample for one channel.
   * @param mbEnergy Average minimum bias energy for this channel, in MeV.
   * @param pulseShape Pulse shape for this channel and gain.
   * @param bcid Bunch crossing identifier of the current event.
   * @param adc2MeVSlope Linear ADC to MeV conversion factor for this gain.
   * @param intperBC Average number of interactions per bunch crossing.
   * @param hecShift Whether to apply the one sample shift that
   *                 LArHitEMapToDigitAlg applies to HEC channels when there are
   *                 four samples and firstSample is zero.
   * @param[out] pileupADC Offset per sample. Must be sized to the number of
   *                       digit samples on entry.
   *
   * The result is returned through @c pileupADC rather than by value so that
   * the caller can reuse one buffer across all channels of an event.
   */
  void computePileupADCCorrection (float mbEnergy,
                                   const ILArShape::ShapeRef_t& pulseShape,
                                   int bcid,
                                   float adc2MeVSlope,
                                   float intperBC,
                                   bool hecShift,
                                   std::vector<float>& pileupADC) const;
};

#endif // LARROD_LARRODBCIDCORRALG_H
