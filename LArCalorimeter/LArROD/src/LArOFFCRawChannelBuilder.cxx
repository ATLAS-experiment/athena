/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArOFFCRawChannelBuilder.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArSimEvent/LArHit.h"
#include "LArSimEvent/LArHitContainer.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"

#include <cmath>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <ranges>

std::vector<double> LArOFFCRawChannelBuilder::convolvePulse(
    const ILArShape::ShapeRef_t& shape, const ILArOFC::OFCRef_t& ofc) const {
  // Convolve pulse shape with time-reversed OFC to build forward-correction
  // template

  const int shapeSize = shape.size();
  const int ofcSize = ofc.size();

  std::vector<double> ofcRev(ofcSize);
  for (int i = 0; i < ofcSize; ++i)
    ofcRev[i] = ofc[ofcSize - 1 - i];

  const int fullLen = shapeSize + ofcSize - 1;
  std::vector<double> fullConv(fullLen, 0.0);

  for (int i = 0; i < shapeSize; ++i)
    for (int j = 0; j < ofcSize; ++j)
      fullConv[i + j] += shape[i] * ofcRev[j];

  // Drop early samples to align correction with trigger window
  const int startIndex = 2;
  if (startIndex >= fullLen)
    return {};

  return std::vector<double>(fullConv.begin() + startIndex, fullConv.end());
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

  // Pedestal subtraction
  std::vector<double> samp_no_ped(nSamples);
  for (int i = 0; i < nSamples; ++i)
    samp_no_ped[i] = samples[i] - pedestal;

  std::vector<double> filtered(nSamples, 0.0);
  std::vector<double> reco(nSamples, 0.0);

  // Precompute convolved pulse
  std::vector<double> convPulse = convolvePulse(shape, ofc);
  const int convSize = convPulse.size();
  std::vector<double> cache(convSize, 0.0);

  // Index of pulse maximum in reference shape
  auto it = std::ranges::max_element(shape);
  int shapemax = std::distance(shape.begin(), it);

  // Determine how many pulses stored
  std::vector<int> context(m_nPulse, 0);
  int belowCounter = 0;

  const int loopEnd = std::max(0, nSamples - ofcLen);

  for (int i = 0; i < loopEnd; ++i) {

    // Reset correction cache after extended quiet region
    if (std::abs(samp_no_ped[i]) < m_belowThreshold) {
      if (++belowCounter == m_belowTillReset) {
        belowCounter = 0;
        std::fill(cache.begin(), cache.end(), 0.0);
      }
    } else {
      belowCounter = 0;
    }

    // Standard OF filtering
    for (int j = 0; j < ofcLen; ++j)
      filtered[i] += samp_no_ped[i + j] * ofc[j];

    if (i > 4) {
      // Apply  forward correction
      reco[i - 1] = filtered[i - 1] + cache[2];
      const double A = reco[i - 1];

      // Local maximum + amplitude cut
      if (A > m_filterThreshold && filtered[i - 1] > filtered[i - 2] &&
          filtered[i - 1] > filtered[i]) {

        // Shape-consistency (Q3) test
        auto shapeVal = [&](int k) {
          return (k >= 0 && k < (int)shape.size()) ? shape[k] : 0.0;
        };

        double Q3 = 0.0;
        Q3 += std::abs(filtered[i - 2] - A * shapeVal(shapemax - 1));
        Q3 += std::abs(filtered[i - 1] - A * shapeVal(shapemax));
        Q3 += std::abs(filtered[i] - A * shapeVal(shapemax + 1));
        Q3 += std::abs(filtered[i - 3] - A * shapeVal(shapemax - 2));

        // Accept pulse and subtract its forward correction
        if (Q3 < m_Q3cut) {
          for (int& c : context) {
            if (c == 0) {
              for (int k = 0; k < convSize; ++k)
                cache[k] -= convPulse[k] * A;
              c = convSize;
              break;
            }
          }
        }
      }
    }

    // Advance correction cache in time
    std::rotate(cache.begin(), cache.begin() + 1, cache.end());
    cache.back() = 0.0;

    for (int& c : context)
      if (c > 0)
        --c;
  }

  // Return reconstructed amplitude at requested sample
  return reco[firstSample + 1];
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

  if (m_firstSample < 5) { 
    ATH_MSG_ERROR("firstSample must be >= 5 to allow for OFFC processing");
    return StatusCode::FAILURE;
  }

  const std::string cutmsg = m_absECutFortQ.value() ? " fabs(E) < " : " E < ";
  ATH_MSG_INFO("Energy cut for time and quality computation: "
               << cutmsg << " taken from COOL folder "
               << m_run1DSPThresholdsKey.key() << " (run1) "
               << m_run2DSPThresholdsKey.key() << " (run2) ");

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

    size_t firstSample = m_firstSample;

    const HWIdentifier id = digit->hardwareID();

    const bool connected = cabling->isOnlineConnected(id);

    const std::vector<short>& samples = digit->samples();
    const int gain = digit->gain();
    const float p = peds->pedestal(id, gain);

    // The following autos will resolve either into vectors or vector-proxies
    const auto& ofca = ofcs->OFC_a(id, gain);
    const auto& adc2mev = adc2MeVs->ADC2MEV(id, gain);
    const size_t nOFC = ofca.size();

    // Sanity check on input conditions data:
    //  ensure that the size of the samples vector is compatible with ofc_a size
    //  when preceeding samples are saved
    const size_t nSamples = samples.size() - firstSample;
    if (nSamples < nOFC) {
      ATH_MSG_ERROR("effective sample size: "
                    << nSamples << ", must be >= OFC_a size: " << ofca.size());
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

    uint16_t prov = 0xa5;  // Means all constants from DB
    if (saturated)
      prov |= 0x0400;

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
      prov |= 0x2000;  //  fill bit in provenance that time+quality information
                       //  are available

      // Get time by applying OFC-b coefficients:
      const auto& ofcb = ofcs->OFC_b(id, gain);
      double At = 0;
      for (size_t i = 0; i < nOFC; ++i) {
        At += static_cast<double>(samp_no_ped[i]) * ofcb[i];
      }

      // At = m_offcTool->compute(samples, firstSample, ofcb, fullShape,
      // ecutadc, p);
      // Divide A*t/A to get time
      tau = (std::fabs(A) > 0.1) ? At / A : 0.0;

      // Get Q-factor
      // fixing HEC to move +1 in case of 4 samples and firstSample 0 (copied
      // from old LArRawChannelBuilder)
      const size_t nSamples = samples.size();
      if (fullShape.size() > nSamples && nSamples == 4 && m_firstSample == 0) {
        if (m_onlineId->isHECchannel(id)) {
          firstSample = 1;
        }
      }

      if (ATH_UNLIKELY(fullShape.size() < nOFC + firstSample)) {
        if (!connected)
          continue;  // No conditions for disconnected channel, who cares?
        ATH_MSG_ERROR("No valid shape for channel "
                      << m_onlineId->channel_name(id) << " gain " << gain);
        ATH_MSG_ERROR("Got size " << fullShape.size() << ", expected at least "
                                  << nSamples + firstSample);
        return StatusCode::FAILURE;
      }

      std::span<const float> shape(fullShape.data() + firstSample, fullShape.size() - firstSample);

      double q = 0;
      if (m_useShapeDer) {
        const auto& fullshapeDer = shapes->ShapeDer(id, gain);
        if (ATH_UNLIKELY(fullshapeDer.size() < nOFC + firstSample)) {
          ATH_MSG_ERROR("No valid shape derivative for channel "
                        << m_onlineId->channel_name(id) << " gain " << gain);
          ATH_MSG_ERROR("Got size " << fullshapeDer.size()
                                    << ", expected at least "
                                    << nOFC + firstSample);
          return StatusCode::FAILURE;
        }

        std::span<const float> shapeDer(fullshapeDer.data() + firstSample, fullshapeDer.size() - firstSample);

        
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

      int iqua = std::min(static_cast<int>(q), 0xFFFF);

      iquaShort = static_cast<uint16_t>(iqua & 0xFFFF);

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
