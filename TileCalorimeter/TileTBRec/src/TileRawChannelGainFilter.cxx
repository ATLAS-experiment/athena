/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileRawChannelGainFilter.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileIdentifier/TileHWID.h"

// Atlas includes
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"


StatusCode TileRawChannelGainFilter::initialize() {

  CHECK( detStore()->retrieve(m_tileHWID) );

  ATH_CHECK( m_inputContainerKey.initialize() );
  ATH_CHECK( m_outputContainerKey.initialize() );

  ATH_MSG_INFO( "Input raw channel container: '" << m_inputContainerKey
                << "'  output container: '" << m_outputContainerKey << "'" );


  return StatusCode::SUCCESS;
}


StatusCode TileRawChannelGainFilter::execute(const EventContext& ctx) const {

  SG::ReadHandle<TileRawChannelContainer> inputContainer(m_inputContainerKey, ctx);
  ATH_CHECK( inputContainer.isValid() );

  TileRawChannelUnit::UNIT rawChanUnit = inputContainer->get_unit();
  TileFragHash::TYPE rawChanType = inputContainer->get_type();

  // Create new container for filtered raw channels
  auto outputContainer = std::make_unique<TileRawChannelContainer>(false, rawChanType, rawChanUnit, SG::VIEW_ELEMENTS);
  outputContainer->set_bsflags(inputContainer->get_bsflags());

  const TileRawChannel* rawChannels[2][48] = {{0}}; // 2 gains and 48 channels

  TileRawChannelContainer::const_iterator collIt = inputContainer->begin();
  TileRawChannelContainer::const_iterator collEnd = inputContainer->end();
  for (; collIt != collEnd; ++collIt) {
    const TileRawChannelCollection* rawChannelCollection = *collIt;

    memset(rawChannels, 0, sizeof(rawChannels));

    for (const TileRawChannel* rawChannel : *rawChannelCollection) {
      HWIdentifier adcId = rawChannel->adc_HWID();
      int channel = m_tileHWID->channel(adcId);
      int gain = m_tileHWID->adc(adcId);
      rawChannels[gain][channel] = rawChannel;
    }

    auto outCollection = std::make_unique<ConstDataVector<TileRawChannelCollection>>(SG::VIEW_ELEMENTS, rawChannelCollection->identify());
    outCollection->reserve(TileCalibUtils::MAX_CHAN);

    for (unsigned int channel = 0; channel < TileCalibUtils::MAX_CHAN; ++channel) {

      const TileRawChannel* rawChannelLG = rawChannels[0][channel];
      const TileRawChannel* rawChannelHG = rawChannels[1][channel];

      if (rawChannelHG && rawChannelLG) {
        if ((rawChannelHG->pedestal() > OVERFLOW_PEDESTAL_MIN) && (rawChannelHG->pedestal() < OVERFLOW_PEDESTAL_MAX)) {
          ATH_MSG_VERBOSE("Overflowed HG: " << (std::string) *rawChannelHG << ", ped = " << rawChannelHG->pedestal());
          ATH_MSG_VERBOSE("Save LG: " << (std::string) *rawChannelLG << ", ped = " << rawChannelLG->pedestal());
          outCollection->push_back(rawChannelLG);
        } else {
          ATH_MSG_VERBOSE("Save HG: " << (std::string) *rawChannelHG << ", ped = " << rawChannelHG->pedestal());
          outCollection->push_back(rawChannelHG);
        }
      } else if (rawChannelLG) {
        ATH_MSG_VERBOSE("Save LG (only available): " << (std::string) *rawChannelLG << ", ped = " << rawChannelLG->pedestal());
        outCollection->push_back(rawChannelLG);
      } else if (rawChannelHG) {
        ATH_MSG_VERBOSE("Save HG (only available): " << (std::string) *rawChannelHG << ", ped = " << rawChannelHG->pedestal());
        outCollection->push_back(rawChannelHG);
      }

    }

    ATH_CHECK( outputContainer->addCollection(outCollection.release()->asDataVector(), collIt.hashId()) );
  }

  SG::WriteHandle<TileRawChannelContainer> outputRawChannelContainer(m_outputContainerKey, ctx);
  ATH_CHECK( outputRawChannelContainer.record(std::move(outputContainer)) );

  return StatusCode::SUCCESS;
}


StatusCode TileRawChannelGainFilter::finalize() {

  ATH_MSG_DEBUG( "in finalize()" );

  return StatusCode::SUCCESS;
}

