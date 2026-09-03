/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "HLTMBTSMonitoringAlgMT.h"

HLTMBTSMonitoringAlgMT::HLTMBTSMonitoringAlgMT(const std::string &name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode HLTMBTSMonitoringAlgMT::initialize() {
  using namespace Monitored;
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_TrigT2MbtsBitsContainerKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode HLTMBTSMonitoringAlgMT::fillHistograms(const EventContext &context) const {
  using namespace Monitored;

  const auto &trigDecTool = getTrigDecisionTool();

  //==============================================================================
  // MBTS BITS INFORMATION
  //==============================================================================

  auto mbtsbits = SG::makeHandle(m_TrigT2MbtsBitsContainerKey, context);
  ATH_MSG_DEBUG("MBTS monitoring, handle validity " << mbtsbits.isValid());

  if (!mbtsbits.isValid()) {
    ATH_MSG_DEBUG("Could not retrieve mbtsbits");
    return StatusCode::SUCCESS;
  }

  if (mbtsbits->size() == 0) {
    return StatusCode::SUCCESS;
  }

  ATH_CHECK(mbtsbits->size() == 1);

  std::bitset<32> triggerWord;
  std::bitset<32> timeWord;
  static constexpr std::bitset<32> mask = 0xFFFFFFFF;

  // Parameters to be tuned to correspond to trigger threshold:
  // static constexpr double timeCut = 20.;
  // static constexpr double energyCut = 40. / 222.;
  static constexpr double timeCut = 10e-4;         // minimal cut-off : copy from TileMBTSMonitorAlg
  static constexpr double energyCut = 60. / 222.;  // copy from TileMBTSMonitorAlgorithm
  const std::string allStr{"MBTSall"};
  for (const auto &trig : m_triggerList) {
    if (not trigDecTool->isPassed(trig, TrigDefs::requireDecision)) {
      continue;
    }

    ATH_MSG_DEBUG("Chain " << trig << " is passed: YES");

    auto energyMean_A = Scalar<float>("MBTS_A_meanEnergy", 0.);
    auto energyMean_C = Scalar<float>("MBTS_C_meanEnergy", 0.);
    auto timeMean_A = Scalar<float>("MBTS_A_meanTime", 0.);
    auto timeMean_C = Scalar<float>("MBTS_C_meanTime", 0.);
    auto energyWeightedTime_A = Scalar<float>("MBTS_A_EWTime", 0.);
    auto energyWeightedTime_C = Scalar<float>("MBTS_C_EWTime", 0.);
    auto channelID = Scalar<int>("MBTS_channelID", 0);
    auto mbtsEnergy = Scalar<float>("MBTS_energy", 0.);
    auto mbtsTime = Scalar<float>("MBTS_time", 0.);
    int ebaCounters = 0;
    int ebcCounters = 0;

    auto TrigCounts = Scalar<std::string>("TrigCounts", trig);
    fill(allStr, TrigCounts);

    // There always should be only one element in this collection
    const auto mbts_itr = mbtsbits->front();

    // Energy (in pC) of signal in a counter (relative to IP), vector for all counters
    const auto& mbtsHitEnergies = mbts_itr->triggerEnergies();

    // Time of signal in counter (relative to IP), vector for all counters
    const auto& mbtsHitTimes = mbts_itr->triggerTimes();

    ATH_MSG_DEBUG("MBTS hits times container size: " << mbtsHitTimes.size());

    if (mbtsHitTimes.size() != 32)
      ATH_MSG_WARNING(
          "MBTS Cell Times are stored incorrectly. The array should have 32 elements (24 should be filled).");
    if (mbtsHitEnergies.size() != 32)
      ATH_MSG_WARNING(
          "MBTS Cell Energies are stored incorrectly. The array should have 32 elements (24 should be filled).");

    for (unsigned i = 0; i < xAOD::TrigT2MbtsBits::NUM_MBTS; i++) {
      channelID = i;
      mbtsTime = mbtsHitTimes.at(i);
      mbtsEnergy = mbtsHitEnergies.at(i);

      if (mbtsEnergy > energyCut) {
        triggerWord.set(i);
      }

      if (std::abs(mbtsTime) > timeCut) {
        timeWord.set(i);
      }

      if (not(triggerWord[i] and timeWord[i])) {
        continue;
      }

      ATH_MSG_DEBUG("MBTS channelID=" << channelID << ", mbtsEnergy=" << mbtsEnergy << ", mbtsTime=" << mbtsTime);
      fill(trig + "_shifter", channelID, mbtsTime, mbtsEnergy);

      if (i < 16) {  // A side
        energyMean_A += mbtsHitEnergies.at(i);
        timeMean_A += mbtsHitTimes.at(i);
        energyWeightedTime_A += mbtsHitEnergies.at(i) * mbtsHitTimes.at(i);

        ebaCounters++;
      } else {  // C side
        energyMean_C += mbtsHitEnergies.at(i);
        timeMean_C += mbtsHitTimes.at(i);
        energyWeightedTime_C += mbtsHitEnergies.at(i) * mbtsHitTimes.at(i);

        ebcCounters++;
      }
    }

    if (ebaCounters > 0) {
      energyMean_A /= ebaCounters;
      timeMean_A /= ebaCounters;

      if (energyMean_A > 0) {
        energyWeightedTime_A /= energyMean_A * ebaCounters;
      }

      ATH_MSG_DEBUG("Side A energyMean_A=" << energyMean_A << ", timeMean_A=" << timeMean_A
                                           << ", energyWeightedTime_A=" << energyWeightedTime_A);

      fill(trig + "_expert", energyMean_A, timeMean_A);
    } else {
      energyWeightedTime_A = -999;  // out of range
    }

    if (ebcCounters > 0) {
      energyMean_C /= ebcCounters;
      timeMean_C /= ebcCounters;

      if (energyMean_C > 0) {
        energyWeightedTime_C /= energyMean_C * ebcCounters;
      }

      ATH_MSG_DEBUG("Side C energyMean_C=" << energyMean_C << ", timeMean_C=" << timeMean_C
                                           << ", energyWeightedTime_C=" << energyWeightedTime_C);

      fill(trig + "_expert", energyMean_C, timeMean_C);
    } else {
      energyWeightedTime_C = -999;  // out of range
    }

    timeWord &= triggerWord;

    const std::bitset<8> InnerEbaBitset((triggerWord & timeWord & mask).to_ulong());
    const std::bitset<8> OuterEbaBitset((((triggerWord & timeWord) >> 8) & mask).to_ulong());
    const std::bitset<8> InnerEbcBitset((((triggerWord & timeWord) >> 16) & mask).to_ulong());
    const std::bitset<8> OuterEbcBitset((((triggerWord & timeWord) >> 24) & mask).to_ulong());

    const auto sideA_hits = InnerEbaBitset.count() + OuterEbaBitset.count();
    const auto sideC_hits = InnerEbcBitset.count() + OuterEbcBitset.count();

    auto MBTS_A_hits = Scalar<int>("MBTS_A_hits", sideA_hits);
    auto MBTS_C_hits = Scalar<int>("MBTS_C_hits", sideC_hits);
    auto MBTS_diff_timeMean = Scalar<float>("MBTS_diff_timeMean", timeMean_A - timeMean_C);
    auto MBTS_diff_weightedTimeMean =
        Scalar<float>("MBTS_diff_weightedTimeMean", energyWeightedTime_A - energyWeightedTime_C);

    fill(trig + "_shifter", MBTS_A_hits, MBTS_C_hits, energyWeightedTime_A, energyWeightedTime_C, MBTS_diff_timeMean,
         MBTS_diff_weightedTimeMean);
  }

  return StatusCode::SUCCESS;
}
