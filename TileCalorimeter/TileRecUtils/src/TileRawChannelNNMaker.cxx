/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TileRawChannelNNMaker.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileEvent/TileMutableRawChannelContainer.h"
#include "TileIdentifier/TileHWID.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>

StatusCode TileRawChannelNNMaker::initialize() {
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );
  ATH_CHECK( m_digitsContainerKey.initialize() );
  ATH_CHECK( m_rawChannelContainerKey.initialize() );
  std::string weightsPath = PathResolverFindCalibFile(m_weightsFile);
  if (weightsPath.empty()) {
    ATH_MSG_ERROR( "Weights file not found: " << m_weightsFile );
    return StatusCode::FAILURE;
  }

  std::ifstream weightsStream(weightsPath);
  std::stringstream weightsText;
  weightsText << weightsStream.rdbuf();
  std::string error;
  if (!m_nn.load(weightsText.str(), error)) {
    ATH_MSG_ERROR( "Invalid weights file " << weightsPath << ": " << error );
    return StatusCode::FAILURE;
  }

  m_ampPerCode = std::ldexp(m_nn.amplitudeScale(), -m_nn.outFracBits());
  ATH_MSG_INFO( "Input digits container: '" << m_digitsContainerKey.key()
                << "'  output container: '" << m_rawChannelContainerKey.key()
                << "'  weights: '" << m_weightsFile.value()
                << "' (" << m_nn.nSamples() << " samples)" );

  return StatusCode::SUCCESS;
}

StatusCode TileRawChannelNNMaker::execute(const EventContext& ctx) const {
  SG::ReadHandle<TileDigitsContainer> digitsContainer(m_digitsContainerKey, ctx);
  ATH_CHECK( digitsContainer.isValid() );
  auto rawChannelContainer = std::make_unique<TileMutableRawChannelContainer>(
      true, TileFragHash::Default, TileRawChannelUnit::ADCcounts);

  ATH_CHECK( rawChannelContainer->status() );
  const int nSamples = m_nn.nSamples();
  std::vector<float> s(nSamples);

  for (const TileDigitsCollection* digitsCollection : *digitsContainer) {
    const TileDigits* digits[TileCalibUtils::MAX_GAIN][TileCalibUtils::MAX_CHAN] = {{nullptr}};

    for (const TileDigits* tileDigits : *digitsCollection) {
      HWIdentifier adcId = tileDigits->adc_HWID();
      digits[m_tileHWID->adc(adcId)][m_tileHWID->channel(adcId)] = tileDigits;
    }

    for (unsigned int channel = 0; channel < TileCalibUtils::MAX_CHAN; ++channel) {
      const TileDigits* loGainDigits = digits[TileHWID::LOWGAIN][channel];
      const TileDigits* hiGainDigits = digits[TileHWID::HIGHGAIN][channel];
      if (!loGainDigits || !hiGainDigits) {
        if (loGainDigits || hiGainDigits) ++m_nMissingGain;
        continue;
      }

      if (loGainDigits->samples().size() != size_t(nSamples)
          || hiGainDigits->samples().size() != size_t(nSamples)) {
        ++m_nBadNSamples;
        continue;
      }

      for (int k = 0; k < nSamples; ++k) {
        s[k] = m_nn.sValue(hiGainDigits->samples()[k], loGainDigits->samples()[k]);
      }

      float amplitude = static_cast<float>(m_nn.run(s.data()) * m_ampPerCode);
      ATH_CHECK( rawChannelContainer->push_back(std::make_unique<TileRawChannel>(
          loGainDigits->adc_HWID(), amplitude, 0.0F, 0.0F)) );
    }
  }

  SG::WriteHandle<TileRawChannelContainer> rawChannelCnt(m_rawChannelContainerKey, ctx);
  ATH_CHECK( rawChannelCnt.record(std::move(rawChannelContainer)) );

  return StatusCode::SUCCESS;
}

StatusCode TileRawChannelNNMaker::finalize() {
  if (m_nMissingGain > 0 || m_nBadNSamples > 0) {
    ATH_MSG_WARNING( "Skipped channels: " << m_nMissingGain << " with only one gain, "
                     << m_nBadNSamples << " without " << m_nn.nSamples() << " samples" );
  }

  return StatusCode::SUCCESS;
}
