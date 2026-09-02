/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArOFFCRawChannelBuilder.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArElecCalib/LArProvenance.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
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
#include <limits>
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

namespace {
// Region codes: |barrel_ec|-1 gives 0 = EMB, 1 = EMEC outer wheel,
// 2 = EMEC inner wheel; HEC is 3 and FCAL 4. Written once because getting
// it wrong attaches a channel to a plausible but wrong configuration,
// which reads as a resolution problem rather than a lookup bug.
constexpr std::array<const char*, 5> regionNames{"EMB", "EMEC-OW", "EMEC-IW",
                                                 "HEC", "FCAL"};
constexpr int maxLayer = 4;  // samplings 0..3; FCAL modules 1..3
}  // namespace

size_t LArOFFCRawChannelBuilder::slotOf(int region, int layer) {
  if (region < 0 || region >= static_cast<int>(regionNames.size()))
    return s_nSlots;
  if (layer < 0 || layer >= maxLayer) return s_nSlots;
  return static_cast<size_t>(region) * maxLayer + static_cast<size_t>(layer);
}

size_t LArOFFCRawChannelBuilder::slotOfKey(const std::string& key) {
  const auto slash = key.rfind('/');
  if (slash == std::string::npos || slash + 1 >= key.size()) return s_nSlots;
  const std::string region = key.substr(0, slash);
  int layer = -1;
  try {
    layer = std::stoi(key.substr(slash + 1));
  } catch (...) {
    return s_nSlots;
  }
  for (size_t r = 0; r < regionNames.size(); ++r)
    if (region == regionNames[r]) return slotOf(static_cast<int>(r), layer);
  return s_nSlots;
}

std::string LArOFFCRawChannelBuilder::keyOfSlot(size_t slot) {
  if (slot >= s_nSlots) return "?";
  return std::string(regionNames[slot / maxLayer]) + "/" +
         std::to_string(slot % maxLayer);
}

StatusCode LArOFFCRawChannelBuilder::buildLayerMap(
    const EventContext& ctx) const {
  // Layer is an IDENTIFIER quantity, not a geometric one, so this needs no
  // CaloDetDescrManager and has no matching tolerance to get wrong: a
  // channel resolves to exactly one layer or to none.
  const LArOnOffIdMapping* cabling{};
  ATH_CHECK(SG::get(cabling, m_cablingKey, ctx));

  const LArEM_ID* emId = m_caloId->em_idHelper();
  const LArHEC_ID* hecId = m_caloId->hec_idHelper();
  const LArFCAL_ID* fcalId = m_caloId->fcal_idHelper();

  m_slotByHash.assign(m_onlineId->channelHashMax(),
                      static_cast<uint8_t>(s_nSlots));
  size_t nMapped = 0, nUnmapped = 0;
  for (auto it = m_onlineId->channel_begin(); it != m_onlineId->channel_end();
       ++it) {
    const HWIdentifier hw = *it;
    if (!cabling->isOnlineConnected(hw)) continue;
    const Identifier cid = cabling->cnvToIdentifier(hw);
    int region = -1, layer = -1;
    if (m_caloId->is_em(cid)) {
      region = std::abs(emId->barrel_ec(cid)) - 1;
      layer = emId->sampling(cid);
    } else if (m_caloId->is_hec(cid)) {
      region = 3;
      layer = hecId->sampling(cid);
    } else if (m_caloId->is_fcal(cid)) {
      region = 4;
      layer = fcalId->module(cid);
    } else {
      continue;
    }
    const size_t slot = slotOf(region, layer);
    if (slot >= s_nSlots) { ++nUnmapped; continue; }
    m_slotByHash[m_onlineId->channel_Hash(hw)] = static_cast<uint8_t>(slot);
    ++nMapped;
  }
  ATH_MSG_INFO("layer map: " << nMapped << " channels resolved, " << nUnmapped
                             << " outside the known regions (these fall back "
                                "to the global settings)");
  return StatusCode::SUCCESS;
}

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
                                             double pedestal,
                                             const LayerParams& par) const {
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
  std::vector<int> slotFreeAt(par.nPulse, 0);
  int belowCounter = 0;

  const int loopEnd = std::max(0, nSamples - ofcLen + 1);

  for (int i = 0; i < loopEnd; ++i) {

    // Reset the correction cache after an extended quiet region
    if (std::abs(samp_no_ped[i]) < par.belowThreshold)
      ++belowCounter;
    else
      belowCounter = 0;

    if (par.belowTillReset > 0 && belowCounter >= par.belowTillReset) {
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
      const double A = reco.at(peak);

      // Local maximum + amplitude cut on the corrected waveform: a pulse
      // riding on the tail of one already subtracted need not be a local
      // maximum of the raw filter output at all.
      if (A > par.filterThreshold && A > reco.at(peak - 1) && A > recoCurrent) {

        auto responseVal = [&](int lag) {
          const int k = lagZero + lag;
          return (k >= 0 && k < responseSize) ? response[k] : 0.0;
        };
        // reco[] is final for the peak and everything before it; the sample
        // being filtered is not stored yet, and no later lag can occur here.
        auto recoVal = [&](int lag) {
          return lag <= 0 ? reco.at(peak + lag) : recoCurrent;
        };

        double Q3 = 0.0;
        for (int lag : q3Lags)
          Q3 += std::abs(recoVal(lag) - A * responseVal(lag));

        // Accept pulse and subtract its forward correction, if NPulse leaves
        // room for it. Shape mismatch scales with the amplitude while the
        // noise floor does not, so the cut carries one term of each: written
        // as a product rather than a ratio to avoid dividing, and A>0 here for
        // any sensible FilterThreshold.
        if (Q3 < par.q3Offset + par.q3Cut * A) {
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
  ATH_CHECK(detStore()->retrieve(m_caloId, "CaloCell_ID"));

  // Resolve the per-layer table once. Every slot starts at the global values,
  // so a job that sets no per-layer property behaves exactly as before.
  m_layerParams.assign(s_nSlots + 1,
                       LayerParams{m_Q3cut, m_Q3Offset, m_filterThreshold,
                                   m_nPulse, m_belowThreshold,
                                   m_belowTillReset});
  auto applyD = [&](const std::map<std::string, double>& m, const char* what,
                    double LayerParams::*field) -> StatusCode {
    for (const auto& [key, val] : m) {
      const size_t slot = slotOfKey(key);
      if (slot >= s_nSlots) {
        ATH_MSG_ERROR(what << " has key '" << key
                           << "' which is not <REGION>/<LAYER> with REGION in "
                              "EMB, EMEC-OW, EMEC-IW, HEC, FCAL and LAYER 0-3");
        return StatusCode::FAILURE;
      }
      m_layerParams[slot].*field = val;
    }
    return StatusCode::SUCCESS;
  };
  ATH_CHECK(applyD(m_filterThresholdByLayer, "FilterThresholdByLayer",
                   &LayerParams::filterThreshold));
  ATH_CHECK(applyD(m_q3CutByLayer, "Q3CutByLayer", &LayerParams::q3Cut));
  ATH_CHECK(applyD(m_q3OffsetByLayer, "Q3OffsetByLayer",
                   &LayerParams::q3Offset));
  for (const auto& [key, val] : m_nPulseByLayer) {
    const size_t slot = slotOfKey(key);
    if (slot >= s_nSlots) {
      ATH_MSG_ERROR("NPulseByLayer has unparseable key '" << key << "'");
      return StatusCode::FAILURE;
    }
    if (val < 1) {
      ATH_MSG_ERROR("NPulseByLayer['" << key << "'] is " << val
                                      << ", must be >= 1");
      return StatusCode::FAILURE;
    }
    m_layerParams[slot].nPulse = val;
  }

  // Disabling is an unreachable threshold, not a separate branch: no
  // amplitude satisfies A > filterThreshold, so nothing is subtracted and
  // the output is bit-identical to the plain OF (verified to 0 ADC).
  if (!m_enabledLayers.empty()) {
    std::vector<bool> on(s_nSlots + 1, false);
    for (const std::string& key : m_enabledLayers.value()) {
      const size_t slot = slotOfKey(key);
      if (slot >= s_nSlots) {
        ATH_MSG_ERROR("EnabledLayers contains unparseable key '" << key << "'");
        return StatusCode::FAILURE;
      }
      on[slot] = true;
    }
    size_t nOff = 0;
    for (size_t slot = 0; slot <= s_nSlots; ++slot) {
      if (slot < s_nSlots && on[slot]) continue;
      m_layerParams[slot].filterThreshold =
          std::numeric_limits<double>::max();
      ++nOff;
    }
    ATH_MSG_INFO("forward correction enabled in "
                 << m_enabledLayers.size() << " layers; " << nOff
                 << " slots left at the Optimal Filter");
  }
  for (size_t slot = 0; slot < s_nSlots; ++slot) {
    const LayerParams& p = m_layerParams[slot];
    if (p.filterThreshold == std::numeric_limits<double>::max()) continue;
    ATH_MSG_DEBUG(keyOfSlot(slot) << ": Q3Cut=" << p.q3Cut << " Q3Offset="
                                  << p.q3Offset << " FilterThreshold="
                                  << p.filterThreshold << " NPulse="
                                  << p.nPulse);
  }

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
      if (!run2DSPThresh->good()) [[unlikely]] {
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
  // Built once on the first event, not in initialize(): it needs the
  // cabling, which is conditions data with an IOV.
  std::call_once(m_slotOnce, [&]() {
    m_slotStatus = this->buildLayerMap(ctx);
  });
  ATH_CHECK(m_slotStatus);

  for (const LArDigit* digit : *inputContainer) {

    const size_t firstSample = m_firstSample;

    const HWIdentifier id = digit->hardwareID();

    const bool connected = cabling->isOnlineConnected(id);

    // Per-layer parameters. The last entry is the global fallback, used for
    // any channel outside the five known regions.
    const IdentifierHash hash = m_onlineId->channel_Hash(id);
    const size_t slot = (hash < m_slotByHash.size())
                            ? static_cast<size_t>(m_slotByHash[hash])
                            : s_nSlots;
    const LayerParams& par = m_layerParams[slot];

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

    if (p == ILArPedestal::ERRORCODE) [[unlikely]] {
      if (!connected)
        continue;  // No conditions for disconencted channel, who cares?
      ATH_MSG_ERROR("No valid pedestal for connected channel "
                    << m_onlineId->channel_name(id) << " gain " << gain);
      return StatusCode::FAILURE;
    }

    if (adc2mev.size() < 2) [[unlikely]] {
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

    double A = computeOFFC(samples, firstSample, ofca, fullShape, p, par);

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

      if (shapeShift < 0 || fullShape.size() < nOFC + shapeShift) [[unlikely]] {
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
        if (fullshapeDer.size() < nOFC + shapeShift) [[unlikely]] {
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
