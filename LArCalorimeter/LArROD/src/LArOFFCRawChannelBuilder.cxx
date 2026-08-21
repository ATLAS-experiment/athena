/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArOFFCRawChannelBuilder.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArElecCalib/LArProvenance.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <span>
#include <vector>

namespace {
// Lags entering the Q3 test, relative to the candidate peak. Lag 0 is left
// out: its residual is identically zero, A being the corrected value at the
// peak and the template 1 there. Nothing above lag +1 either, that is the
// newest value available when the peak is tested.
constexpr std::array<int, 3> q3Lags{-2, -1, 1};
static_assert(std::ranges::is_sorted(q3Lags) && q3Lags.back() <= 1);

// Entry of a pulse response that a pulse is found at. Pulses are identified as
// local maxima of the filter output, so that is where the OFC window sits on
// the shape. Undefined for an empty response, callers check.
int anchorIndex(const std::vector<double>& response) {
  return std::distance(response.begin(), std::ranges::max_element(response));
}
}  // namespace

std::vector<double> LArOFFCRawChannelBuilder::pulseResponse(
    const ILArShape::ShapeRef_t& shape, const ILArOFC::OFCRef_t& ofc) const {
  // Filter output for a unit-amplitude pulse: same correlation as the
  // filtering loop in computeOFFC, samples replaced by the shape. Writing it
  // as a convolution.

  const int shapeSize = shape.size();
  const int ofcSize = ofc.size();

  auto shapeVal = [&](int k) {
    return (k >= 0 && k < shapeSize) ? shape[k] : 0.0f;
  };

  // response[k] is the filter output for a pulse offset by k-(ofcSize-1)
  // samples from the OFC window, tabulated over the whole range where the two
  // overlap. Where the in-time position falls inside it is up to the OFCs.
  std::vector<double> response(std::max(0, shapeSize + ofcSize - 1), 0.0);

  for (int k = 0; k < static_cast<int>(response.size()); ++k) {
    const int offset = k - (ofcSize - 1);
    for (int j = 0; j < ofcSize; ++j)
      response[k] += shapeVal(offset + j) * ofc[j];
  }

  return response;
}

double LArOFFCRawChannelBuilder::computeOFFC(const std::vector<short>& samples,
                                             int firstSample,
                                             const ILArOFC::OFCRef_t& ofc,
                                             const ILArShape::ShapeRef_t& shape,
                                             double pedestal) const {
  // OFFC parameters (configured via job options)
  // belowThreshold   : ADC threshold to detect quiet regions
  // belowTillReset   : consecutive quiet samples before cache reset
  // npulse           : max number of overlapping pulses tracked
  // Q3cut            : shape-consistency cut for pulse acceptance
  // filterThreshold  : minimum filtered amplitude to seed a pulse

  const int nSamples = samples.size();
  const int ofcLen = ofc.size();

  // Checked by the caller. The filter at position i needs samples
  // i ... i+ofcLen-1, and every position it is evaluated at gets a corrected
  // value, so the last one is nSamples-ofcLen.
  if (ofcLen <= 0 || firstSample < 0 || firstSample + ofcLen > nSamples) {
    ATH_MSG_WARNING("Cannot run the OFFC on " << nSamples << " samples with "
                                              << ofcLen << " OFCs from sample "
                                              << firstSample);
    return 0.0;
  }

  // Pedestal subtraction
  std::vector<double> samp_no_ped(nSamples);
  for (int i = 0; i < nSamples; ++i)
    samp_no_ped[i] = samples[i] - pedestal;

  std::vector<double> reco(nSamples, 0.0);

  // Precompute the filter response to a unit-amplitude pulse. cache[lagZero]
  // is the correction of the sample being filtered, cache[lagZero+n] that of
  // the one n later.
  std::vector<double> response = pulseResponse(shape, ofc);
  const int responseSize = response.size();
  if (responseSize == 0) {
    ATH_MSG_WARNING("Empty pulse response, shape size " << shape.size()
                                                        << " OFC size " << ofcLen);
    return 0.0;
  }

  // Entry of response[] a pulse is found at. Pulses are identified as local
  // maxima of 'filtered', so the template has to be anchored where the
  // response peaks: that follows whatever alignment the OFCs carry, which is
  // neither fixed nor the same for every channel (see LArOFCCondAlg).
  const int lagZero = anchorIndex(response);
  if (response[lagZero] <= 0.0) {
    ATH_MSG_WARNING("Pulse response is nowhere positive, cannot subtract");
    return 0.0;
  }

  // A is the filter output at that peak, so the template must be 1 there. It
  // only already is when the response happens to peak where the OFCs were
  // matched to the shape.
  const double peakResponse = response[lagZero];
  for (double& r : response)
    r /= peakResponse;
  std::vector<double> cache(responseSize, 0.0);

  // A pulse writes cache entries lagZero ... responseSize-2, and entry k is
  // consumed k-lagZero iterations later, so that is how long a slot stays busy.
  const int correctionLength = responseSize - 2 - lagZero;

  // Sample at which each pulse slot becomes available again
  std::vector<int> slotFreeAt(m_nPulse, 0);
  int belowCounter = 0;

  const int loopEnd = std::max(0, nSamples - ofcLen + 1);

  for (int i = 0; i < loopEnd; ++i) {

    // Reset the correction cache after an extended quiet region
    if (std::abs(samp_no_ped[i]) < m_belowThreshold)
      ++belowCounter;
    else
      belowCounter = 0;

    if (m_belowTillReset > 0 && belowCounter >= m_belowTillReset) {
      belowCounter = 0;
      std::fill(cache.begin(), cache.end(), 0.0);
      // The corrections these slots were tracking have just been dropped, so
      // the slots have to be released with them
      std::fill(slotFreeAt.begin(), slotFreeAt.end(), 0);
    }

    // Standard OF filtering
    double filtered = 0.0;
    for (int j = 0; j < ofcLen; ++j)
      filtered += samp_no_ped[i + j] * ofc[j];

    // Corrected value of the sample just filtered, before anything found this
    // iteration is subtracted from it
    const double recoCurrent = filtered + cache[lagZero];

    // Candidate peak: where a pulse would be in time with the OFCs, not where
    // the pulse itself is largest. The search trails the filtering by one, a
    // maximum can only be recognised once the following sample is in.
    const int peak = i - 1;
    if (peak + q3Lags.front() >= 0) {
      const double A = reco[peak];

      // Local maximum + amplitude cut on the corrected waveform: a pulse
      // riding on the tail of one already subtracted need not be a local
      // maximum of the raw filter output at all.
      if (A > m_filterThreshold && A > reco[peak - 1] && A > recoCurrent) {

        auto responseVal = [&](int lag) {
          const int k = lagZero + lag;
          return (k >= 0 && k < responseSize) ? response[k] : 0.0;
        };
        // reco[] is final for the peak and everything before it; the sample
        // being filtered is not stored yet, and no later lag can occur here.
        auto recoVal = [&](int lag) {
          return lag <= 0 ? reco[peak + lag] : recoCurrent;
        };

        double Q3 = 0.0;
        for (int lag : q3Lags)
          Q3 += std::abs(recoVal(lag) - A * responseVal(lag));

        // Accept pulse and subtract its forward correction, if NPulse leaves
        // room for it. Shape mismatch scales with the amplitude while the
        // noise floor does not, so the cut carries one term of each: written
        // as a product rather than a ratio to avoid dividing, and A>0 here for
        // any sensible FilterThreshold.
        if (Q3 < m_Q3Offset + m_Q3cut * A) {
          const auto slot = std::ranges::find_if(
              slotFreeAt, [i](int freeAt) { return freeAt <= i; });
          if (slot == slotFreeAt.end()) {
            ++m_nDropped;
          } else {
            // The pulse peaks one sample back, so its lag n lands on the entry
            // for sample i+n-1, i.e. cache[lagZero+n-1]
            for (int k = lagZero; k + 1 < responseSize; ++k)
              cache[k] -= response[k + 1] * A;
            *slot = i + correctionLength;
            ++m_nSubtracted;
          }
        }
      }
    }

    // Corrected filter output of this sample, now including any pulse just
    // accepted one sample back
    reco[i] = filtered + cache[lagZero];

    // Advance correction cache in time
    std::rotate(cache.begin(), cache.begin() + 1, cache.end());
    cache.back() = 0.0;
  }

  // Corrected equivalent of the plain OF amplitude, which is the filter output
  // of the window starting at firstSample
  return reco[firstSample];
}

StatusCode LArOFFCRawChannelBuilder::initialize() {
  ATH_CHECK(m_digitKey.initialize());
  ATH_CHECK(m_rawChannelKey.initialize());
  ATH_CHECK(m_pedestalKey.initialize());
  ATH_CHECK(m_adc2MeVKey.initialize());
  ATH_CHECK(m_ofcKey.initialize());
  ATH_CHECK(m_shapeKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_run1DSPThresholdsKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_run2DSPThresholdsKey.initialize(SG::AllowEmpty));

  if (m_useDBFortQ) {
    if (m_run1DSPThresholdsKey.empty() && m_run2DSPThresholdsKey.empty()) {
      ATH_MSG_ERROR(
          "useDB requested but neither Run1... nor Run2... initialized.");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(detStore()->retrieve(m_onlineId, "LArOnlineID"));

  // The earliest testable candidate peak sits -q3Lags.front() samples into the
  // digit, so anything less leaves no room to find a pulse before the in-time
  // window and the forward correction can never contribute.
  const int minFirstSample = -q3Lags.front() + 1;
  if (m_firstSample < minFirstSample) {
    ATH_MSG_ERROR("firstSample is "
                  << m_firstSample.value() << ", must be >= " << minFirstSample
                  << " for the OFFC to find any pulse before the in-time "
                     "window (set LAr.ROD.nPreceedingSamples accordingly)");
    return StatusCode::FAILURE;
  }

  if (m_nPulse < 0) {
    ATH_MSG_ERROR("NPulse is " << m_nPulse.value() << ", must be >= 0");
    return StatusCode::FAILURE;
  }
  if (m_nPulse == 0) {
    ATH_MSG_WARNING(
        "NPulse is 0, no pulse will be subtracted and the OFFC reduces to "
        "plain optimal filtering");
  }

  const std::string cutmsg = m_absECutFortQ.value() ? "fabs(E)" : "E";
  if (m_useDBFortQ) {
    ATH_MSG_INFO("Time and quality computed for "
                 << cutmsg << " above the threshold from COOL folder "
                 << m_run1DSPThresholdsKey.key() << " (run1) "
                 << m_run2DSPThresholdsKey.key() << " (run2)");
  } else {
    ATH_MSG_INFO("Time and quality computed for " << cutmsg << " above "
                                                  << m_eCutFortQ.value());
  }

  return StatusCode::SUCCESS;
}

StatusCode LArOFFCRawChannelBuilder::finalize() {
  const unsigned long dropped = m_nDropped;
  ATH_MSG_INFO("Subtracted " << m_nSubtracted.load() << " pulses, dropped "
                             << dropped << " for want of a free slot (NPulse = "
                             << m_nPulse.value() << ")");
  if (dropped > 0)
    ATH_MSG_WARNING(dropped
                    << " accepted pulses were not subtracted: their correction "
                       "is missing from the output. Raise NPulse to keep them");
  return StatusCode::SUCCESS;
}

StatusCode LArOFFCRawChannelBuilder::execute(const EventContext& ctx) const {

  ATH_MSG_VERBOSE("Executing LArOFFCRawChannelBuilder::execute");

  // Get event inputs from read handles:
  const LArDigitContainer* inputContainer{};
  ATH_CHECK(SG::get(inputContainer, m_digitKey, ctx));

  // Write output via write handle
  auto outputContainer = std::make_unique<LArRawChannelContainer>();

  // Get Conditions input
  const ILArPedestal* peds{};
  ATH_CHECK(SG::get(peds, m_pedestalKey, ctx));

  const LArADC2MeV* adc2MeVs{};
  ATH_CHECK(SG::get(adc2MeVs, m_adc2MeVKey, ctx));

  const ILArOFC* ofcs{nullptr};
  ATH_CHECK(SG::get(ofcs, m_ofcKey, ctx));

  const ILArShape* shapes{};
  ATH_CHECK(SG::get(shapes, m_shapeKey, ctx));
  
  const LArOnOffIdMapping* cabling{};
  ATH_CHECK(SG::get(cabling, m_cablingKey, ctx));

  std::unique_ptr<LArDSPThresholdsFlat> run2DSPThresh;
  const LArDSPThresholdsComplete* run1DSPThresh = nullptr;
  ATH_CHECK(SG::get(run1DSPThresh, m_run1DSPThresholdsKey, ctx));
  if (m_useDBFortQ) {
    if (!m_run2DSPThresholdsKey.empty()) {
      SG::ReadCondHandle<AthenaAttributeList> dspThrshAttr(
          m_run2DSPThresholdsKey, ctx);
      run2DSPThresh = std::make_unique<LArDSPThresholdsFlat>(*dspThrshAttr);
      if (ATH_UNLIKELY(!run2DSPThresh->good())) {
        ATH_MSG_ERROR(
            "Failed to initialize LArDSPThresholdFlat from attribute list "
            "loaded from "
            << m_run2DSPThresholdsKey.key() << ". Aborting.");
        return StatusCode::FAILURE;
      }
    } else if (!m_run1DSPThresholdsKey.empty()) {
      SG::ReadCondHandle<LArDSPThresholdsComplete> dspThresh(
          m_run1DSPThresholdsKey, ctx);
      run1DSPThresh = dspThresh.cptr();
    } else {
      ATH_MSG_ERROR("No DSP threshold configured.");
      return StatusCode::FAILURE;
    }
  }

  // Loop over digits:
  for (const LArDigit* digit : *inputContainer) {

    const size_t firstSample = m_firstSample;

    const HWIdentifier id = digit->hardwareID();

    const bool connected = cabling->isOnlineConnected(id);

    const std::vector<short>& samples = digit->samples();
    const int gain = digit->gain();
    const float p = peds->pedestal(id, gain);

    // The following autos will resolve either into vectors or vector-proxies
    const auto& ofca = ofcs->OFC_a(id, gain);
    const auto& adc2mev = adc2MeVs->ADC2MEV(id, gain);
    const size_t nOFC = ofca.size();

    if (ATH_UNLIKELY(nOFC == 0)) {
      if (!connected)
        continue;  // No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid OFCs for connected channel "
                    << m_onlineId->channel_name(id) << " gain " << gain);
      return StatusCode::FAILURE;
    }

    // Sanity check on input conditions data: ensure the samples vector is
    // compatible with the ofc_a size when preceeding samples are saved.
    // Compared this way round because samples.size()-firstSample would wrap
    // for a short digit.
    if (samples.size() < firstSample + nOFC) {
      ATH_MSG_ERROR("digit has " << samples.size() << " samples, need at least "
                                 << firstSample + nOFC << " for firstSample "
                                 << firstSample << " and OFC_a size " << nOFC);
      return StatusCode::FAILURE;
    }

    if (ATH_UNLIKELY(p == ILArPedestal::ERRORCODE)) {
      if (!connected)
        continue;  // No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid pedestal for connected channel "
                    << m_onlineId->channel_name(id) << " gain " << gain);
      return StatusCode::FAILURE;
    }

    if (ATH_UNLIKELY(adc2mev.size() < 2)) {
      if (!connected)
        continue;  // No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid ADC2MeV for connected channel "
                    << m_onlineId->channel_name(id) << " gain " << gain);
      return StatusCode::FAILURE;
    }

    // Apply OFFC to get amplitude
    //  Evaluate sums in double-precision to get consistent results
    //  across platforms.

    bool saturated = false;
    // Check saturation AND discount pedestal
    std::vector<double> samp_no_ped(nOFC, 0.0);
    for (size_t i = 0; i < nOFC; ++i) {
      if (samples[i + firstSample] == 4096 || samples[i + firstSample] == 0)
        saturated = true;
      samp_no_ped[i] = samples[i + firstSample] - p;
    }

    uint16_t iquaShort = 0;
    float tau = 0;

    uint16_t prov = LArProv::DEFAULTRECO;  // Means all constants from DB
    if (saturated)
      prov |= LArProv::SATURATED;

    float ecut(0.);
    if (m_useDBFortQ) {
      if (run2DSPThresh) {
        ecut = run2DSPThresh->tQThr(id);
      } else if (run1DSPThresh) {
        ecut = run1DSPThresh->tQThr(id);
      } else {
        ATH_MSG_ERROR("DSP threshold problem");
        return StatusCode::FAILURE;
      }
    } else {
      ecut = m_eCutFortQ;
    }

    const auto& fullShape = shapes->Shape(id, gain);

    double A = computeOFFC(samples, firstSample, ofca, fullShape, p);

    const float E = adc2mev[0] + A * adc2mev[1];

    const float E1 = m_absECutFortQ.value() ? std::fabs(E) : E;

    if (E1 > ecut) {
      ATH_MSG_VERBOSE("Channel " << m_onlineId->channel_name(id) << " gain "
                                 << gain
                                 << " above threshold for tQ computation");
      prov |= LArProv::QTPRESENT;  // time+quality information are available

      // Get time by applying OFC-b coefficients:
      const auto& ofcb = ofcs->OFC_b(id, gain);
      double At = 0;
      for (size_t i = 0; i < nOFC; ++i) {
        At += static_cast<double>(samp_no_ped[i]) * ofcb[i];
      }

      // Divide A*t/A to get time
      tau = (std::fabs(A) > 0.1) ? At / A : 0.0;

      // Get Q-factor. The shape has to be offset by the index the OFC window
      // is matched to, which is not the digit offset: the digitisation writes
      // shape index k-nPreceedingSamples into digit sample k. Reading it back
      // from the conditions also covers the HEC shift and the fallback that
      // LArOFCCondAlg applies per channel.
      const std::vector<double> resp = pulseResponse(fullShape, ofca);
      const int shapeShift =
          resp.empty() ? -1 : anchorIndex(resp) - static_cast<int>(nOFC) + 1;

      if (ATH_UNLIKELY(shapeShift < 0 ||
                       fullShape.size() < nOFC + shapeShift)) {
        if (!connected)
          continue;  // No conditions for disconnected channel, who cares?
        ATH_MSG_ERROR("No valid shape for channel "
                      << m_onlineId->channel_name(id) << " gain " << gain);
        ATH_MSG_ERROR("Got size " << fullShape.size() << " and offset "
                                  << shapeShift << ", expected at least "
                                  << nOFC << " samples from there");
        return StatusCode::FAILURE;
      }

      std::span<const float> shape(fullShape.data() + shapeShift,
                                   fullShape.size() - shapeShift);

      double q = 0;
      if (m_useShapeDer) {
        const auto& fullshapeDer = shapes->ShapeDer(id, gain);
        if (ATH_UNLIKELY(fullshapeDer.size() < nOFC + shapeShift)) {
          ATH_MSG_ERROR("No valid shape derivative for channel "
                        << m_onlineId->channel_name(id) << " gain " << gain);
          ATH_MSG_ERROR("Got size " << fullshapeDer.size()
                                    << ", expected at least "
                                    << nOFC + shapeShift);
          return StatusCode::FAILURE;
        }

        std::span<const float> shapeDer(fullshapeDer.data() + shapeShift,
                                        fullshapeDer.size() - shapeShift);

        
        for (size_t i = 0; i < nOFC; ++i) {
          q += std::pow((A * (shape[i] - tau * shapeDer[i]) - (samp_no_ped[i])),
                        2);
        }
      }  // end if useShapeDer
      else {
        // Q-factor w/o shape derivative
        for (size_t i = 0; i < nOFC; ++i) {
          q += std::pow((A * shape[i] - (samp_no_ped[i])), 2);
        }
      }

      // Clamp before the cast, q can exceed the range of int
      iquaShort = static_cast<uint16_t>(std::min(q, 65535.0));

      tau -= ofcs->timeOffset(id, gain);
      tau *= (Gaudi::Units::nanosecond /
              Gaudi::Units::picosecond);  // Convert time to ps
    }  // end if above cut

    outputContainer->emplace_back(id, static_cast<int>(std::floor(E + 0.5)),
                                  static_cast<int>(std::floor(tau + 0.5)),
                                  iquaShort, prov, (CaloGain::CaloGain)gain);
  }

  SG::WriteHandle<LArRawChannelContainer> outputHandle(m_rawChannelKey, ctx);
  ATH_CHECK(outputHandle.record(std::move(outputContainer)));

  return StatusCode::SUCCESS;
}
