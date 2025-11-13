/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileDigitsFlxMonitorAlgorithm.h"
#include "TileIdentifier/TileHWID.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

// Athena includes
#include "StoreGate/ReadHandle.h"

#include <cmath>
#include <algorithm>


StatusCode TileDigitsFlxMonitorAlgorithm::initialize() {

  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_MSG_INFO("in initialize()");

  ATH_CHECK( detStore()->retrieve(m_tileHWID) );
  ATH_CHECK( m_digitsContainerKeyLegacy.initialize() );
  ATH_CHECK( m_digitsContainerKeyFlx.initialize() );

  std::ostringstream os;

  if ( m_fragIDsToCompare.size() != 0) {
    std::sort(m_fragIDsToCompare.begin(), m_fragIDsToCompare.end());
    for (int fragID : m_fragIDsToCompare) {
      unsigned int ros    = fragID >> 8;
      unsigned int drawer = fragID & 0xFF;
      std::string module = TileCalibUtils::getDrawerString(ros, drawer);
      os << " " << module << "/0x" << std::hex << fragID << std::dec;
    }
  } else {
    os << "NONE";
  }
  m_firstFelix = static_cast<unsigned int>(m_firstSample) + m_felixOffset;
  m_nSamples = (m_lastSample >= m_firstSample) ? 1 + m_lastSample - m_firstSample : 0;

  ATH_MSG_INFO("Monitored modules/frag ID:" << os.str());

  return StatusCode::SUCCESS;
}


StatusCode TileDigitsFlxMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  SG::ReadHandle<TileDigitsContainer> digitsContainerLegacy(m_digitsContainerKeyLegacy, ctx);
  ATH_CHECK( digitsContainerLegacy.isValid() );

  SG::ReadHandle<TileDigitsContainer> digitsContainerFlx(m_digitsContainerKeyFlx, ctx);
  ATH_CHECK( digitsContainerFlx.isValid() );

  std::array<std::string, 2> gainName{"_LG", "_HG"};

  std::vector<float>digitsLegacy[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  std::vector<float>digitsFlx[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];
  int found[TileCalibUtils::MAX_CHAN][TileCalibUtils::MAX_GAIN];

  const TileFragHash& hashFunc = digitsContainerFlx->hashFunc();

  for (int fragID : m_fragIDsToCompare) {

    memset(found,0,sizeof(found));

    IdentifierHash hash = static_cast<IdentifierHash>(hashFunc(fragID));
    unsigned int drawer = (fragID & 0x3F);
    unsigned int ros = fragID >> 8;
    std::string module = TileCalibUtils::getDrawerString(ros, drawer);
    std::array<std::string, 2> moduleName{module+gainName[0], module+gainName[1]};

    // Legacy digits
    const TileDigitsCollection* digitsCollectionLegacy = digitsContainerLegacy->indexFindPtr(hash);
    for (const TileDigits* tile_digits : *digitsCollectionLegacy) {

      ATH_MSG_VERBOSE((std::string) *tile_digits);

      unsigned int nSamples = tile_digits->nsamples();
      if (m_firstSample < nSamples) {

        HWIdentifier adcId = tile_digits->adc_HWID();
        int channel = m_tileHWID->channel(adcId);
        int gain = m_tileHWID->adc(adcId);

        std::string sampleName = module + "_ch_" + std::to_string(channel) + gainName[gain] +  "_samples";
        auto channelSample = Monitored::Scalar<float>(sampleName, 0.0F);

        std::vector<float> digits_monitored;
        auto it = tile_digits->samples().begin() + m_firstSample;
        auto end = tile_digits->samples().end();
        for(auto i = m_nSamples; it != end && i-->0; ++it) {
          float sample = (*it);
          channelSample = sample;
          fill("TileLegacyMonSamples", channelSample);
          digits_monitored.push_back(sample);
        }

        nSamples = digits_monitored.size();
        if (nSamples>0) {

          auto monitoredChannel = Monitored::Scalar<float>(moduleName[gain] + "_channel", channel);
          auto pedestal = Monitored::Scalar<float>(moduleName[gain] + "_Pedestal", digits_monitored[0]);
          fill("TileLegacyMonPed", monitoredChannel, pedestal);

          if (nSamples>1) {

            double sampleSum = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0);

            double sampleRMS = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0, [] (double acc, float sample) {
                  return acc + sample * sample;
                });
            sampleRMS -= sampleSum * sampleSum / nSamples;
            sampleRMS = (sampleRMS > 0.0) ? std::sqrt(sampleRMS / (nSamples - 1)) : 0.0;

            auto hfn = Monitored::Scalar<float>(moduleName[gain] + "_HFN", sampleRMS);
            fill("TileLegacyMonHFN", monitoredChannel, hfn);
          }

          found[channel][gain] |= 1;
          digitsLegacy[channel][gain].swap(digits_monitored);
        }
      }
    }

    // FELIX digits
    const TileDigitsCollection* digitsCollectionFlx = digitsContainerFlx->indexFindPtr(hash);
    for (const TileDigits* tile_digits : *digitsCollectionFlx) {

      ATH_MSG_VERBOSE((std::string) *tile_digits);

      unsigned int nSamples = tile_digits->nsamples();
      if (m_firstFelix < nSamples) {

        HWIdentifier adcId = tile_digits->adc_HWID();
        int channel = m_tileHWID->channel(adcId);
        int gain = m_tileHWID->adc(adcId);

        std::string sampleName = module + "_ch_" + std::to_string(channel) + gainName[gain] +  "_samples";
        auto channelSample = Monitored::Scalar<float>(std::move(sampleName), 0.0F);

        std::vector<float> digits_monitored;
        auto it = tile_digits->samples().begin() + m_firstFelix;
        auto end = tile_digits->samples().end();
        
        for(auto i = m_nSamples; it != end && i-->0; ++it) {
          float sample = (*it);
          channelSample = sample;
          fill("TileFlxMonSamples", channelSample);
          digits_monitored.push_back(sample);
        }

        nSamples = digits_monitored.size();
        if (nSamples>0) {

          auto monitoredChannel = Monitored::Scalar<float>(moduleName[gain] + "_channel", channel);
          auto pedestal = Monitored::Scalar<float>(moduleName[gain] + "_Pedestal", digits_monitored[0]);
          fill("TileFlxMonPed", monitoredChannel, pedestal);

          if (nSamples>1) {

            double sampleSum = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0);

            double sampleRMS = std::accumulate( digits_monitored.begin(), digits_monitored.end(), 0.0, [] (double acc, float sample) {
                  return acc + sample * sample;
                });
            sampleRMS -= sampleSum * sampleSum / nSamples;
            sampleRMS = (sampleRMS > 0.0) ? std::sqrt(sampleRMS / (nSamples - 1)) : 0.0;

            auto hfn = Monitored::Scalar<float>(moduleName[gain] + "_HFN", sampleRMS);
            fill("TileFlxMonHFN", monitoredChannel, hfn);
          }

          found[channel][gain] |= 2;
          digitsFlx[channel][gain].swap(digits_monitored);
        }
      }
    }

    // Compare digits from Legacy and FELIX
    for(unsigned int gain = 0; gain<TileCalibUtils::MAX_GAIN; ++gain) {
      auto monitoredChannel = Monitored::Scalar<float>(moduleName[gain] + "_channel", 0.0F);
      auto moduleSampleDiff = Monitored::Scalar<float>(moduleName[gain] + "_samples_diff", 0.0F);

      for(unsigned int channel = 0; channel<TileCalibUtils::MAX_CHAN; ++channel) {

        if (found[channel][gain] == 3) {

          monitoredChannel = channel;

          std::string sampleName = module + "_ch_" + std::to_string(channel) + gainName[gain] +  "_samples_diff";
          auto channelSampleDiff = Monitored::Scalar<float>(std::move(sampleName), 0.0F);

          for (auto it1=digitsLegacy[channel][gain].begin(), it2=digitsFlx[channel][gain].begin();
               it1!=digitsLegacy[channel][gain].end() && it2!=digitsFlx[channel][gain].end(); ++it1, ++it2) {

            float diff = (*it2)- (*it1)*m_felixScale;
            channelSampleDiff = diff;
            moduleSampleDiff = diff;

            fill("TileChannelAllSamplesDiff", channelSampleDiff);
            fill("TileModuleAllSamplesDiff", moduleSampleDiff);
            fill("TileModuleSamplesDiff", monitoredChannel, moduleSampleDiff);
          }
        }
      }
    }
  }

  fill("TileDigitsFlxMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}
