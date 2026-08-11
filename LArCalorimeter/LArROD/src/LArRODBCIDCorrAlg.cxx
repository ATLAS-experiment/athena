/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file LArROD/src/LArRODBCIDCorrAlg.cxx
 * @date May 2026
 * @brief Subtract the BCID-dependent pile-up offset from the LArDigit samples before  digital filterings.
 */

#include "LArRODBCIDCorrAlg.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace {

/// Conversion from mu per bunch to luminosity in 10**30 units per bunch, for MC
/// only: 25ns * nBCID * 71mb * 10**30 = 25e-9 * 3564 * 71e-27 * 1e30 = 6.31, so
/// the factor is 1/6.31. This is the same constant as in CaloBCIDLumi.
constexpr float xlumiMC = 0.158478605f;

/// Minimum intensity for a bunch to count as filled. The same threshold as in
/// BunchCrossingCondAlg.
constexpr float minBunchIntensity = 0.001f;

/// Smallest ADC to MeV slope for which the correction can be scaled reliably.
constexpr float minAdc2MeVSlope = 1e-9f;

/// Number of digit samples for which the HEC shape shift applies.
constexpr size_t hecShiftNSamples = 4;

} // anonymous namespace

StatusCode LArRODBCIDCorrAlg::initialize()
{
  ATH_CHECK( m_digitKey.initialize() );
  ATH_CHECK( m_digitCorrKey.initialize() );
  ATH_CHECK( m_adc2MeVKey.initialize() );
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_shapeKey.initialize() );
  ATH_CHECK( m_minBiasAvgKey.initialize() );
  ATH_CHECK( m_eventInfoKey.initialize() );

  ATH_CHECK( detStore()->retrieve (m_onlineId, "LArOnlineID") );

  if (m_minBunchCrossing > m_maxBunchCrossing) {
    ATH_MSG_ERROR( "MinBunchCrossing (" << m_minBunchCrossing
                   << ") is larger than MaxBunchCrossing ("
                   << m_maxBunchCrossing << ")" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( fillLumi() );

  ATH_MSG_INFO( "firstSample = " << m_firstSample
                << ", so digit sample 0 corresponds to bunch crossing "
                << m_firstSample << " relative to the triggered one" );

  return StatusCode::SUCCESS;
}

StatusCode LArRODBCIDCorrAlg::execute (const EventContext& ctx) const
{
  ATH_MSG_DEBUG( "Executing LArRODBCIDCorrAlg" );

  SG::ReadHandle<LArDigitContainer> digitHandle (m_digitKey, ctx);
  if (!digitHandle.isValid()) {
    ATH_MSG_ERROR( "Failed to retrieve LArDigitContainer "
                   << m_digitKey.key() );
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl (m_cablingKey, ctx);
  if (!cablingHdl.isValid()) {
    ATH_MSG_ERROR( "Invalid cabling conditions handle" );
    return StatusCode::FAILURE;
  }
  const LArOnOffIdMapping* cabling = *cablingHdl;

  SG::ReadCondHandle<ILArShape> shapesHdl (m_shapeKey, ctx);
  if (!shapesHdl.isValid()) {
    ATH_MSG_ERROR( "Invalid shape conditions handle" );
    return StatusCode::FAILURE;
  }
  const ILArShape* shapes = *shapesHdl;

  SG::ReadCondHandle<LArADC2MeV> adc2MeVHdl (m_adc2MeVKey, ctx);
  if (!adc2MeVHdl.isValid()) {
    ATH_MSG_ERROR( "Invalid ADC2MeV conditions handle" );
    return StatusCode::FAILURE;
  }
  const LArADC2MeV* adc2MeVs = *adc2MeVHdl;

  SG::ReadCondHandle<ILArMinBiasAverage> minBiasAvgHdl (m_minBiasAvgKey, ctx);
  if (!minBiasAvgHdl.isValid()) {
    ATH_MSG_ERROR( "Invalid MinBiasAverage conditions handle" );
    return StatusCode::FAILURE;
  }
  const ILArMinBiasAverage* minBiasAvg = minBiasAvgHdl.cptr();

  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
  if (!eventInfo.isValid()) {
    ATH_MSG_ERROR( "Failed to retrieve EventInfo" );
    return StatusCode::FAILURE;
  }

  const int bcid = static_cast<int> (eventInfo->bcid());
  if (bcid < 0 || bcid >= s_nBCID) {
    ATH_MSG_ERROR( "BCID " << bcid << " is outside [0, " << s_nBCID << ")" );
    return StatusCode::FAILURE;
  }
  const float intperBC = eventInfo->averageInteractionsPerCrossing();

  SG::WriteHandle<LArDigitContainer> outHandle (m_digitCorrKey, ctx);
  auto output = std::make_unique<LArDigitContainer> (SG::OWN_ELEMENTS);
  LArDigitContainer* outputPtr = output.get();
  outputPtr->reserve (digitHandle->size());
  ATH_CHECK( outHandle.record (std::move (output)) );

  // Declared outside the loop so that the buffers are reused across channels.
  std::vector<float> pileupADC;
  std::vector<short> correctedSamples;
  size_t nClamped = 0;

  for (const LArDigit* digit : *digitHandle) {

    const HWIdentifier id = digit->hardwareID();
    const CaloGain::CaloGain gain = digit->gain();
    const std::vector<short>& rawSamples = digit->samples();

    float slope = 0;
    float mbEnergy = 0;
    ILArShape::ShapeRef_t pulseShape;
    bool correctable = cabling->isOnlineConnected (id) && !rawSamples.empty();
    if (correctable) {
      const auto& adc2mev = adc2MeVs->ADC2MEV (id, gain);
      correctable = adc2mev.size() >= 2 &&
                    std::abs (adc2mev[1]) > minAdc2MeVSlope;
      if (correctable) {
        slope = adc2mev[1];
        pulseShape = shapes->Shape (id, gain);
        correctable = !pulseShape.empty();
      }
      if (correctable) {
        // A non-positive average means no correction, as in CaloBCIDCoeffs.
        // This also catches ILArMinBiasAverage::ERRORCODE.
        mbEnergy = minBiasAvg->minBiasAverage (id);
        correctable = mbEnergy > 0;
      }
    }

    // Channels for which no correction can be computed are copied through
    // unchanged, so that the output container always mirrors the input.
    if (!correctable) {
      outputPtr->push_back (std::make_unique<LArDigit> (id, gain, rawSamples));
      continue;
    }

    // LArHitEMapToDigitAlg shifts the HEC shape by one sample in the four
    // sample readout, so reproduce that here to stay aligned with the digits.
    const bool hecShift = rawSamples.size() == hecShiftNSamples &&
                          m_firstSample == 0 &&
                          m_onlineId->isHECchannel (id);

    pileupADC.assign (rawSamples.size(), 0);
    computePileupADCCorrection (mbEnergy, pulseShape,
                                 bcid, slope, intperBC, hecShift, pileupADC);

    // The corrected value is clamped to the range of short rather than to the
    // ADC range -> the raw channel builders treat samples of 0 and 4096 as
    // saturation markers, so clipping there would flag saturation spuriously.

    correctedSamples.resize (rawSamples.size());
    for (size_t i = 0; i < rawSamples.size(); ++i) {
      const long corrected =
        std::lround (static_cast<double> (rawSamples[i]) -
                     static_cast<double> (pileupADC[i]));
      const long clamped =
        std::clamp (corrected,
                    static_cast<long> (std::numeric_limits<short>::lowest()),
                    static_cast<long> (std::numeric_limits<short>::max()));
      if (clamped != corrected) {
        ++nClamped;
      }
      correctedSamples[i] = static_cast<short> (clamped);
    }

    outputPtr->push_back
      (std::make_unique<LArDigit> (id, gain, correctedSamples));
  }

  if (nClamped > 0) {
    ATH_MSG_WARNING( nClamped << " corrected sample(s) fell outside the range "
                     "of short and were clamped" );
  }

  ATH_MSG_DEBUG( "Wrote " << outputPtr->size() << " digits to "
                 << m_digitCorrKey.key() );

  return StatusCode::SUCCESS;
}

StatusCode LArRODBCIDCorrAlg::fillLumi()
{
  const std::vector<float>& pattern = m_beamIntensityPattern;
  if (pattern.empty()) {
    ATH_MSG_ERROR( "BeamIntensityPattern is empty. It must be set from "
                   "flags.Digitization.PU.BeamIntensityPattern." );
    return StatusCode::FAILURE;
  }

  const int nPattern = static_cast<int> (pattern.size());
  m_lumi.assign (s_nBCID, 0);

  // Reproduce BunchCrossingCondAlg, so that the set of filled bunch crossings
  // used here is identical to the one in BunchCrossingCondData, which is what
  // the energy level correction uses.
  if (s_nBCID % nPattern != 0) {
    ATH_MSG_INFO( "Bunch pattern of length " << nPattern
                  << " does not fit into " << s_nBCID );
    // As in BunchCrossingCondAlg, the loop deliberately stops short of the
    // half orbit so as not to produce an odd pattern half way round.
    const int nGuard = 20;
    for (int i = 0; i < s_nBCID / 2 - nGuard; ++i) {
      const int pos1 = i % nPattern;
      const int pos2 = nPattern - 1 - (i % nPattern);
      if (pattern[pos1] > minBunchIntensity) {
        m_lumi[i] = xlumiMC;
      }
      if (pattern[pos2] > minBunchIntensity) {
        m_lumi[s_nBCID - 1 - i] = xlumiMC;
      }
    }
  }
  else {
    ATH_MSG_INFO( "Bunch pattern of length " << nPattern << " fits into "
                  << s_nBCID );
    for (int i = 0; i < s_nBCID; ++i) {
      if (pattern[i % nPattern] > minBunchIntensity) {
        m_lumi[i] = xlumiMC;
      }
    }
  }

  const int nFilled = std::count_if (m_lumi.begin(), m_lumi.end(),
                                     [] (float lumi) { return lumi > 0; });
  ATH_MSG_INFO( "Filled " << nFilled << " of " << s_nBCID
                << " bunch crossings" );
  if (nFilled == 0) {
    ATH_MSG_ERROR( "No filled bunch crossing found in the beam intensity "
                   "pattern, so the correction would be identically zero." );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

void
LArRODBCIDCorrAlg::computePileupADCCorrection (float mbEnergy,
                                               const ILArShape::ShapeRef_t& pulseShape,
                                               int bcid,
                                               float adc2MeVSlope,
                                               float intperBC,
                                               bool hecShift,
                                               std::vector<float>& pileupADC) const
{
  const int nShape = static_cast<int> (pulseShape.size());
  const int nSamples = static_cast<int> (pileupADC.size());
  const int firstSample = m_firstSample + (hecShift ? 1 : 0);

  for (int i = 0; i < nSamples; ++i) {
    // Digit sample i is fed by a pulse from bunch crossing bc through shape
    // index j, with j = i - bc + firstSample.
    double sum = 0;
    for (int j = 0; j < nShape; ++j) {
      const int bc = i + firstSample - j;
      if (bc < m_minBunchCrossing || bc > m_maxBunchCrossing) {
        continue;
      }
      int idx = (bcid + bc) % s_nBCID;
      if (idx < 0) {
        idx += s_nBCID;
      }
      sum += static_cast<double> (m_lumi[idx]) * pulseShape[j];
    }
    pileupADC[i] =
      static_cast<float> (mbEnergy * sum * intperBC / adc2MeVSlope);
  }
}
