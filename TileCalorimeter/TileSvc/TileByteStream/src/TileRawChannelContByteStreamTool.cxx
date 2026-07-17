/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadCondHandle.h"

#include "TileByteStream/TileRawChannelContByteStreamTool.h"
#include "TileByteStream/TileHid2RESrcID.h"
#include "TileByteStream/TileROD_Encoder.h"

#include "TileEvent/TileRawChannelCollection.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileFastRawChannel.h"

#include "TileIdentifier/TileHWID.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileConditions/TileCablingService.h"
#include "TileByteStream/TileROD_Decoder.h"

#include <Gaudi/Property.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/SmartIF.h>

#include <functional>
#include <map> 
#include <stdint.h>

static const InterfaceID IID_ITileRawChannelContByteStreamTool("TileRawChannelContByteStreamTool", 1, 0);

const InterfaceID& TileRawChannelContByteStreamTool::interfaceID() {
  return IID_ITileRawChannelContByteStreamTool;
}

// default constructor

TileRawChannelContByteStreamTool::TileRawChannelContByteStreamTool(const std::string& type,
    const std::string& name, const IInterface* parent)
    : AthAlgTool(type, name, parent)
    , m_tileHWID(0)
    , m_verbose(false)
    , m_maxChannels(TileCalibUtils::MAX_CHAN)
{
  declareInterface<TileRawChannelContByteStreamTool>(this);
}

// destructor 

TileRawChannelContByteStreamTool::~TileRawChannelContByteStreamTool() {
}

StatusCode TileRawChannelContByteStreamTool::initialize() {

  ATH_MSG_INFO ("Initializing TileRawChannelContByteStreamTool");

  ATH_CHECK( detStore()->retrieve(m_tileHWID, "TileHWID") );

  ToolHandle<TileROD_Decoder> dec("TileROD_Decoder");
  ATH_CHECK( dec.retrieve() );

  const Gaudi::Details::PropertyBase& demoFragIDsProperty = SmartIF<IProperty>(dec.get())->getProperty("DemoFragIDs");
  const IntegerArrayProperty& demoFragIDs = dynamic_cast<const IntegerArrayProperty&>(demoFragIDsProperty);

  m_demoFragIDs.reserve(demoFragIDs.size());
  for (const auto fragID : demoFragIDs.value()) {
    m_demoFragIDs.push_back(fragID);
  }

  if ( !m_demoFragIDs.empty() ) {
    std::ostringstream os;
    std::sort(m_demoFragIDs.begin(),m_demoFragIDs.end());
    os << " (frag IDs):";
    for (int fragID : m_demoFragIDs) {
      if (fragID > 0)
        os << " 0x" << std::hex << fragID << std::dec;
      else
        os << " " << fragID;
    }
    ATH_MSG_INFO("Enable channel remapping for demonstrator modules" << os.str());
  }


  // Prepare legacy to Demonstrator channel mapping for LB and EB
  // (vice versa to the one in the Tile ROD Decoder)
  std::vector<std::pair<std::reference_wrapper<std::vector<int>>,
                        std::reference_wrapper<const std::vector<int>>>>
    legacyAndDemoChanMaps{{m_legacy2DemoChannelLB, dec->getDemoChannelMapLB()},
                          {m_legacy2DemoChannelEB, dec->getDemoChannelMapEB()}};

  for (std::pair<std::reference_wrapper<std::vector<int>>,
         std::reference_wrapper<const std::vector<int>>>
         legacyAndDemoChanMap : legacyAndDemoChanMaps) {

    std::vector<int>& legacy2DemoChannel = legacyAndDemoChanMap.first;
    const std::vector<int>& demoChanMap = legacyAndDemoChanMap.second;
    if (!demoChanMap.empty()) {
      legacy2DemoChannel.resize(m_maxChannels);
      for (unsigned int demoChannel = 0; demoChannel < demoChanMap.size(); ++demoChannel) {
        int legacyChannel = demoChanMap[demoChannel];
        if ((legacyChannel >= 0) && (legacyChannel < m_maxChannels)) {
          legacy2DemoChannel[legacyChannel] = demoChannel;
        }
      }
    }
  }


  // get TileCondToolEmscale
  ATH_CHECK( m_tileToolEmscale.retrieve() );

  // get TileBadChanTool
  ATH_CHECK( m_tileBadChanTool.retrieve() );

  ATH_CHECK( m_hid2RESrcIDKey.initialize(m_initializeForWriting) );

  m_maxChannels = TileCablingService::getInstance()->getMaxChannels();

  return StatusCode::SUCCESS;
}

StatusCode TileRawChannelContByteStreamTool::finalize() {
  ATH_MSG_INFO ("Finalizing TileRawChannelContByteStreamTool successfuly");
  return StatusCode::SUCCESS;
}

StatusCode TileRawChannelContByteStreamTool::convert(CONTAINER* rawChannelContainer, FullEventAssembler<TileHid2RESrcID> *fea) const
{
  bool isTMDB = evtStore()->proxy(rawChannelContainer)->name() == "MuRcvRawChCnt";

  TileFragHash::TYPE contType = rawChannelContainer->get_type();
  TileRawChannelUnit::UNIT inputUnit = rawChannelContainer->get_unit();
  TileRawChannelUnit::UNIT outputUnit = inputUnit; 

  bool oflCont = (inputUnit < TileRawChannelUnit::OnlineOffset);

  FullEventAssembler<TileHid2RESrcID>::RODDATA* theROD;
  SG::ReadCondHandle<TileHid2RESrcID> hid2re{m_hid2RESrcIDKey};

  ATH_MSG_DEBUG( " Number of raw channel collections... " << rawChannelContainer->size() << " " << evtStore()->proxy(rawChannelContainer)->name());

  std::map<uint32_t, TileROD_Encoder> mapEncoder;
  std::vector<TileFastRawChannel> channels;
  channels.reserve (m_tileHWID->channel_hash_max());

  uint32_t reid = 0x0;

  for (const TileRawChannelCollection* rawChannelCollection : *rawChannelContainer) {

    TileRawChannelCollection::ID frag_id = rawChannelCollection->identify();

    if (isTMDB) reid = hid2re->getRodTileMuRcvID(frag_id);
    else reid = hid2re->getRodID(frag_id);

    TileROD_Encoder& encoder = mapEncoder[reid];

    encoder.setTileHWID(m_tileHWID, m_verbose, 4);
    encoder.setTypeAndUnit(contType, outputUnit);
    encoder.setMaxChannels(m_maxChannels);

    HWIdentifier drawer_id = m_tileHWID->drawer_id(frag_id);

    int ros = m_tileHWID->ros(drawer_id);
    int drawer = m_tileHWID->drawer(drawer_id);
    int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);

    const std::vector<uint32_t> & drawer_info = hid2re->getDrawerInfo(frag_id);
    int drawer_type = drawer_info.size() > 2 ? static_cast<int>(drawer_info[2]) : -1;
    bool remap = (drawer_type > 0) || std::binary_search(m_demoFragIDs.begin(), m_demoFragIDs.end(), frag_id);
    const std::vector<int>& legacy2DemoChannel = (ros < 3) ? m_legacy2DemoChannelLB : m_legacy2DemoChannelEB;

    int nChannels = 0;
    std::vector<std::reference_wrapper<TileFastRawChannel>> drawerChannels;
    drawerChannels.reserve(rawChannelCollection->size());

    for (const TileRawChannel* rawChannel : *rawChannelCollection) {

      HWIdentifier adc_id = rawChannel->adc_HWID();
      int channel = m_tileHWID->channel(adc_id);
      int adc = m_tileHWID->adc(adc_id);
      float amplitude = rawChannel->amplitude();
      float time = rawChannel->time();
      float quality = rawChannel->quality();
      if (isTMDB) {
        channels.emplace_back (frag_id, channel, adc, amplitude, 0., 0.);
      } else {
        if (oflCont) {
          if (quality > 15.0) quality = 15.0;
          if (m_tileBadChanTool->getAdcStatus(drawerIdx, channel, adc).isBad()) quality += 16.;
        }
        //amplitude = m_tileToolEmscale->channelCalib(drawerIdx, channel, adc, amplitude, inputUnit, outputUnit);

        if (remap && (channel >= 0) && (channel < m_maxChannels)) {
          ATH_MSG_VERBOSE("Change channel [" << TileCalibUtils::getDrawerString(ros, drawer) << "]: "
                          << channel << " -> " << legacy2DemoChannel[channel]);
          channel = legacy2DemoChannel[channel];
        }

        channels.emplace_back (frag_id, channel, adc, amplitude, time, quality);
        drawerChannels.push_back(channels.back());
      }

      ++nChannels;
    }

    if (remap) {
      std::sort(drawerChannels.begin(), drawerChannels.end(),
                [] (const TileFastRawChannel& a, const TileFastRawChannel& b ) {
                  return a.channel() < b.channel();
                });

      }

    for (const TileFastRawChannel& channel : drawerChannels){
      // Don't need to worry about these moving due to the reserve() above.
      encoder.add(&channel);
    }

    ATH_MSG_DEBUG( " Collection " << MSG::hex << "0x" << frag_id
                  << " ROD " << "0x" << reid
                  << " number of channels " << MSG::dec << nChannels );
  }

  // TileROD_Encoder has collected all the channels, now can fill the
  // ROD block data.

  for (std::pair<const uint32_t, TileROD_Encoder>& reidAndEncoder: mapEncoder) {

    theROD = fea->getRodData(reidAndEncoder.first);
    TileROD_Encoder& theEncoder = reidAndEncoder.second;

    if ((reidAndEncoder.first & 0xf00)) {
       theEncoder.fillRODTileMuRcvRawChannel(*theROD);
    } else {
      if (m_doFragType4) theEncoder.fillROD4(*theROD);
      if (m_doFragType5) theEncoder.fillROD5(*theROD);
    }
    ATH_MSG_DEBUG( " Number of TileRawChannel words in ROD " << MSG::hex << " 0x" << reidAndEncoder.first << MSG::dec << " : " << theROD->size() );
  }

  return StatusCode::SUCCESS;
}
