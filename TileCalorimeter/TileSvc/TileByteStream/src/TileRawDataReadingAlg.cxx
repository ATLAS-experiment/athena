/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileRawDataReadingAlg.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileEvent/TileMutableDigitsContainer.h"
#include "TileEvent/TileMutableRawChannelContainer.h"
#include "TileEvent/TileMutableBeamElemContainer.h"
#include "TileIdentifier/TileTBFrag.h"

// Athena includes
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "ByteStreamCnvSvcBase/ROBDataProviderSvc.h"

#include <functional>

StatusCode TileRawDataReadingAlg::initialize() {

  m_doDigits = !m_digitsContainerKey.empty();
  ATH_CHECK(m_digitsContainerKey.initialize(m_doDigits));

  m_doMuRcvDigits = !m_muRcvDigitsContainerKey.empty();
  ATH_CHECK(m_muRcvDigitsContainerKey.initialize(m_doMuRcvDigits));

  m_doFlxDigits = !m_flxDigitsContainerKey.empty();
  ATH_CHECK(m_flxDigitsContainerKey.initialize(m_doFlxDigits));

  m_doRawChannels = !m_rawChannelContainerKey.empty();
  ATH_CHECK(m_rawChannelContainerKey.initialize(m_doRawChannels));

  m_doMuRcvRawChannels = !m_muRcvRawChannelContainerKey.empty();
  ATH_CHECK(m_muRcvRawChannelContainerKey.initialize(m_doMuRcvRawChannels));

  m_doBeamElements = !m_beamElemContainerKey.empty();
  ATH_CHECK(m_beamElemContainerKey.initialize(m_doBeamElements));

  m_doLaserObject = !m_laserObjectKey.empty();
  ATH_CHECK(m_laserObjectKey.initialize(m_doLaserObject));

  m_doMuonReceiver = !m_muonReceiverContainerKey.empty();
  ATH_CHECK(m_muonReceiverContainerKey.initialize(m_doMuonReceiver));

  m_doL2 = !m_l2ContainerKey.empty();
  ATH_CHECK(m_l2ContainerKey.initialize(m_doL2));


  ATH_MSG_DEBUG("Initialize Tile raw data reading: "
                << ((m_doDigits) ? m_digitsContainerKey.key() + " " : "")
                << ((m_doMuRcvDigits) ? m_muRcvDigitsContainerKey.key() + " " : "")
                << ((m_doFlxDigits) ? m_flxDigitsContainerKey.key() + " " : "")
                << ((m_doRawChannels) ? m_rawChannelContainerKey.key() + " " : "")
                << ((m_doMuRcvRawChannels) ? m_muRcvRawChannelContainerKey.key() + " " : "")
                << ((m_doBeamElements) ? m_beamElemContainerKey.key() + " " : "")
                << ((m_doLaserObject) ? m_laserObjectKey.key() + " " : "")
                << ((m_doMuonReceiver) ? m_muonReceiverContainerKey.key() + " " : "")
                << ((m_doL2) ? m_l2ContainerKey.key() + " " : ""));

  ATH_CHECK(m_hid2RESrcIDKey.initialize());

  ATH_CHECK(m_robSvc.retrieve());
  ATH_CHECK(m_cablingSvc.retrieve());
  ATH_CHECK(m_decoder.retrieve());

  return StatusCode::SUCCESS;
}     
  
StatusCode TileRawDataReadingAlg::execute(const EventContext& ctx) const {

  SG::ReadCondHandle<TileHid2RESrcID> hid2re(m_hid2RESrcIDKey, ctx);
  ATH_CHECK(hid2re.isValid());

  if (m_doDigits) {
    ATH_CHECK(readDigits(m_digitsContainerKey, ctx, *hid2re,
                         std::mem_fn(&TileHid2RESrcID::getRobFromFragID),
                           [] (const TileROD_Decoder* decoder, const ROBDataProviderSvc::ROBF* rob, TileDigitsCollection& digitsCollection) {
                           decoder->fillCollection(rob, digitsCollection);
                         }));
  }

  if (m_doRawChannels) {
    ATH_CHECK(readRawChannels(m_rawChannelContainerKey, ctx, *hid2re,
                              std::mem_fn(&TileHid2RESrcID::getRobFromFragID),
                              [] (const TileROD_Decoder* decoder, const ROBDataProviderSvc::ROBF* rob,
                                  TileRawChannelCollection& rawChannelCollection, TileRawChannelContainer* container) {
                                decoder->fillCollection(rob, rawChannelCollection, container);
                              }));
  }

  if (m_doMuonReceiver) {
    ATH_CHECK(readMuonReceiver(m_muonReceiverContainerKey, ctx));
  }

  if (m_doMuRcvDigits) {
    ATH_CHECK(readDigits(m_muRcvDigitsContainerKey, ctx, *hid2re,
                         std::mem_fn(&TileHid2RESrcID::getRobFromTileMuRcvFragID),
                         std::mem_fn(&TileROD_Decoder::fillCollection_TileMuRcv_Digi)));
  }

  if (m_doMuRcvRawChannels) {
    ATH_CHECK(readRawChannels(m_muRcvRawChannelContainerKey, ctx, *hid2re,
                              std::mem_fn(&TileHid2RESrcID::getRobFromTileMuRcvFragID),
                              [] (const TileROD_Decoder* decoder, const ROBDataProviderSvc::ROBF* rob,
                                  TileRawChannelCollection& rawChannelCollection, TileRawChannelContainer*) {
                                decoder->fillCollection_TileMuRcv_RawChannel(rob, rawChannelCollection);
                              }, TileFragHash::MF));
  }

  if (m_doBeamElements) {
    ATH_CHECK(readBeamElements(m_beamElemContainerKey, ctx, *hid2re));
  }

  if (m_doLaserObject) {
    ATH_CHECK(readLaserObject(m_laserObjectKey, ctx, *hid2re));
  }

  if (m_doFlxDigits) {
    ATH_CHECK(readDigits(m_flxDigitsContainerKey, ctx, *hid2re,
                         std::mem_fn(&TileHid2RESrcID::getRobFromFragID),
                         std::mem_fn(&TileROD_Decoder::fillCollection_FELIX_Digi),
                         TileCalibUtils::FELIX_FRAGID_OFFSET));
  }

  if (m_doL2) {
    ATH_CHECK(readL2(m_l2ContainerKey, ctx));
  }

  return StatusCode::SUCCESS;
}


template <class GetRobOperation, class FillCollOperation>
StatusCode TileRawDataReadingAlg::readDigits(const SG::WriteHandleKey<TileDigitsContainer>& digitsKey,
                                             const EventContext& ctx, const TileHid2RESrcID* hid2re,
                                             GetRobOperation getRobFromFragID, FillCollOperation fillCollection,
                                             unsigned int offsetID) const {

  uint32_t newrob = 0x0;
  std::vector<uint32_t> robid{0};
  std::vector<const ROBDataProviderSvc::ROBF*> robf;
  auto digitsContainer = std::make_unique<TileMutableDigitsContainer>(true, TileFragHash::Digitizer);
  ATH_CHECK( digitsContainer->status() );

  // Iterate over all collections in a container and fill them
  for (IdentifierHash hash : digitsContainer->GetAllCurrentHashes()) {
    TileDigitsCollection* digitsCollection = digitsContainer->indexFindPtr (hash);
    TileDigitsCollection::ID collID = digitsCollection->identify();

    // Find ROB
    newrob = getRobFromFragID(hid2re, collID + offsetID);
    if (newrob != robid[0]) {
      robid[0] = newrob;
      robf.clear();
      m_robSvc->getROBData(robid, robf);
    }

    if (robf.size() > 0 ) {
      fillCollection(&*m_decoder, robf[0], *digitsCollection);
    } else {
      uint32_t status = TileROD_Decoder::NO_ROB;
      digitsCollection->setFragBCID(0xDEAD | (status << 16));
      ATH_MSG_DEBUG( "Status [" << digitsKey.key() << "] for " << "drawer 0x" << MSG::hex
                     << collID << " in Digi frag is 0x" << status << MSG::dec);
    }
  }

  ATH_MSG_DEBUG( "Creating digits container: " << digitsKey.key() );

  SG::WriteHandle<TileDigitsContainer> digitsCnt(digitsKey, ctx);
  ATH_CHECK( digitsCnt.record(std::move(digitsContainer)) );

  return StatusCode::SUCCESS;
}

template <class GetRobOperation, class FillCollOperation>
StatusCode TileRawDataReadingAlg::readRawChannels(const SG::WriteHandleKey<TileRawChannelContainer>& rawChannelsKey,
                                                  const EventContext& ctx, const TileHid2RESrcID* hid2re,
                                                  GetRobOperation getRobFromFragID, FillCollOperation fillCollection,
                                                  TileFragHash::TYPE type) const {


  uint32_t newrob = 0x0;
  std::vector<uint32_t> robid{0};
  std::vector<const ROBDataProviderSvc::ROBF*> robf;
  std::unordered_map<uint32_t, int> bsflags;
  uint32_t flags;

  auto rawChannelContainer = std::make_unique<TileMutableRawChannelContainer>(true, type);
  ATH_CHECK( rawChannelContainer->status() );

  // Iterate over all collections in a container and fill them
  for (IdentifierHash hash : rawChannelContainer->GetAllCurrentHashes()) {
    TileRawChannelCollection* rawChannelCollection = rawChannelContainer->indexFindPtr (hash);
    TileRawChannelCollection::ID collID = rawChannelCollection->identify();

    // Find ROB
    newrob = getRobFromFragID(hid2re, collID);
    if (newrob != robid[0]) {
      robid[0] = newrob;
      robf.clear();
      m_robSvc->getROBData(robid, robf);
    }

    // Unpack ROB data
    if (robf.size() > 0 ) {
      fillCollection(&*m_decoder, robf[0], *rawChannelCollection, &*rawChannelContainer);

      flags = rawChannelContainer->get_bsflags();
      auto result = bsflags.insert(std::pair<uint32_t, int>(flags, 1));
      if (result.second == false) {
        result.first->second++;
      }
    } else {
      ATH_MSG_DEBUG( "ROB [" << rawChannelsKey.key() << "] for " << "drawer 0x" << MSG::hex << collID << MSG::dec << " not found in BS" );
      uint32_t status = TileROD_Decoder::NO_ROB | TileROD_Decoder::CRC_ERR;
      rawChannelCollection->setFragGlobalCRC(status);
      ATH_MSG_DEBUG( "Status [" << rawChannelsKey.key() << "] for " << "drawer 0x" << MSG::hex << collID << " is 0x" << status << MSG::dec);
    }
  }

  if (bsflags.size() > 1) {
    int n = 0;
    for (const auto & elem : bsflags) {
      if (elem.second > n) {
        n = elem.second;
        flags = elem.first;
      }
    }

    if (flags != rawChannelContainer->get_bsflags()) {

      uint32_t unit = ((flags & 0xC0000000) >> 30);
      if ((flags & 0x30000000) < 0x30000000) {
        unit += TileRawChannelUnit::OnlineOffset; // Online units in real data
      }

      ATH_MSG_DEBUG( "Changing units [" << rawChannelsKey.key() << "] for " << "RawChannelContainer from "
                     << rawChannelContainer->get_unit() << " to " << unit << MSG::hex
                     << " and BS flags from 0x" << rawChannelContainer->get_bsflags() << " to 0x" << flags << MSG::dec);

      rawChannelContainer->set_unit((TileRawChannelUnit::UNIT)unit);
      rawChannelContainer->set_bsflags(flags);
    }
  }

  ATH_MSG_DEBUG( "Creating raw channel container: " << rawChannelsKey );

  SG::WriteHandle<TileRawChannelContainer> rawChannelsCnt(rawChannelsKey, ctx);
  ATH_CHECK( rawChannelsCnt.record(std::move(rawChannelContainer)) );

  return StatusCode::SUCCESS;
}


StatusCode TileRawDataReadingAlg::readBeamElements(const SG::WriteHandleKey<TileBeamElemContainer>& beamElementsKey,
                                                   const EventContext& ctx, const TileHid2RESrcID* hid2re) const {

  std::vector<uint32_t> robid{0};
  std::vector<const ROBDataProviderSvc::ROBF*> robf;

  auto beamElementsContainer = std::make_unique<TileMutableBeamElemContainer>(true);
  ATH_CHECK( beamElementsContainer->status() );

  // Iterate over all collections in a container and fill them
  for (IdentifierHash hash : beamElementsContainer->GetAllCurrentHashes()) {
    TileBeamElemCollection* beamElementsCollection = beamElementsContainer->indexFindPtr(hash);
    TileBeamElemCollection::ID collID = beamElementsCollection->identify();

    // Find ROB
    uint32_t newrob = hid2re->getRobFromFragID(collID);
    if (newrob != robid[0]) {
      robid[0] = newrob;
      robf.clear();
      m_robSvc->getROBData(robid, robf);
    }

    // Unpack ROB data
    if (robf.size() > 0 ) {
      m_decoder->fillCollection(robf[0], *beamElementsCollection);
    }
  }

  ATH_MSG_DEBUG( "Creating beam elements container: " << beamElementsKey );

  SG::WriteHandle<TileBeamElemContainer> beamElementsCnt(beamElementsKey, ctx);
  ATH_CHECK( beamElementsCnt.record(std::move(beamElementsContainer)) );

  return StatusCode::SUCCESS;
}


StatusCode TileRawDataReadingAlg::readLaserObject(const SG::WriteHandleKey<TileLaserObject>& laserObjectKey,
                                                  const EventContext& ctx, const TileHid2RESrcID* hid2re) const {

  std::vector<uint32_t> robid{0};
  std::vector<const ROBDataProviderSvc::ROBF*> robf;

  auto laserObject = std::make_unique<TileLaserObject>() ;
  m_decoder->setLaserVersion(*laserObject);

  robid[0] = hid2re->getRobFromFragID(LASER_OBJ_FRAG);
  m_robSvc->getROBData(robid, robf);

  if (robf.size() > 0 ) {
    m_decoder->fillTileLaserObj(robf[0], *laserObject);
  } else {
    ATH_MSG_DEBUG( " No LASTROD fragment in BS, TileLaserObject will be empty." );
  }

  ATH_MSG_DEBUG( "Creating laser object: " << laserObjectKey );

  SG::WriteHandle<TileLaserObject> laserObj(laserObjectKey, ctx);
  ATH_CHECK( laserObj.record(std::move(laserObject)) );

  return StatusCode::SUCCESS;
}


StatusCode TileRawDataReadingAlg::readMuonReceiver(const SG::WriteHandleKey<TileMuonReceiverContainer>& muRcvKey,
                                                   const EventContext& ctx) const {

  const RawEvent* re = m_robSvc->getEvent();
  if (!re) {
    ATH_MSG_ERROR( "Could not get raw event from ByteStreamInputSvc" );
    return StatusCode::FAILURE;
  }

  auto muonReceiverContainer = std::make_unique<TileMuonReceiverContainer>();

  if (!m_decoder->convertTMDBDecision(re, muonReceiverContainer.get()).isSuccess()) {
    ATH_MSG_WARNING( "Conversion tool returned an error. TileMuonReceiverContainer might be empty." );
  }

  SG::WriteHandle<TileMuonReceiverContainer> muRcvCnt(muRcvKey, ctx);
  ATH_CHECK( muRcvCnt.record(std::move(muonReceiverContainer)) );

  return StatusCode::SUCCESS;
}


StatusCode TileRawDataReadingAlg::readL2(const SG::WriteHandleKey<TileL2Container>& l2Key,
                                         const EventContext& ctx) const {

  const RawEvent* re = m_robSvc->getEvent();
  if (!re) {
    ATH_MSG_ERROR( "Could not get raw event from ByteStreamInputSvc" );
    return StatusCode::FAILURE;
  }

  auto l2Container = std::make_unique<TileL2Container>();
  l2Container->reserve(256);
  for(int i = 0; i < 256; ++i) {
    int collId = m_decoder->hashFunc()->identifier(i);
    l2Container->push_back (std::make_unique<TileL2>(collId));
  }

  if (!m_decoder->convert(re, l2Container.get()).isSuccess()) {
    ATH_MSG_WARNING( "Conversion tool returned an error. TileL2Container might be empty." );
  }

  SG::WriteHandle<TileL2Container> l2Cnt(l2Key, ctx);
  ATH_CHECK( l2Cnt.record(std::move(l2Container)) );

  return StatusCode::SUCCESS;
}
