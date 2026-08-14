/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include "../src/TileRawChannelNNMaker.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileMutableDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileIdentifier/TileHWID.h"
#include "TileConditions/TileCablingService.h"
#include "IdDictParser/IdDictParser.h"
#include "TestTools/initGaudi.h"
#include "GaudiKernel/ISvcLocator.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/setupStoreGate.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include "CxxUtils/checker_macros.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Four events from the v1_A1-A5_W9_concat test shards spanning the output
// range, one of them with saturated high gain. The expected codes are the
// hls4ml C-simulation outputs for exactly these samples, regenerated together
// with data/TileRawChannelNN_w9_v1.json by the training-side exporter:
// https://gitlab.cern.ch/ificuw/tilecal/-/blob/master/SignalReco_simple/export/export_athena_json.py
static const std::vector<std::vector<float> > HG_SAMPLES = {
  {3336, 1944, 489, 100, 50, 93, 63, 68, 82},
  {144, 353, 650, 485, 576, 4095, 4095, 4095, 4095},
  {0, 0, 0, 0, 185, 120, 0, 263, 737},
  {231, 249, 623, 4095, 4095, 4095, 4095, 0, 0},
};

static const std::vector<std::vector<float> > LG_SAMPLES = {
  {180, 144, 109, 99, 98, 98, 99, 100, 99},
  {99, 104, 113, 109, 111, 1297, 3710, 1864, 341},
  {52, 75, 74, 81, 101, 101, 95, 104, 115},
  {101, 103, 113, 1425, 4088, 2048, 379, 53, 46},
};

static const int64_t EXPECTED_CODE[4] = {0, 60, 188, 63812};

class TileCablingSvc {
 public:
   static void init_idhelpers ATLAS_NOT_THREAD_SAFE (IdDictParser& parser) {
     TileHWID* tileHWID = new TileHWID();
     TileID* tileID = new TileID();
     TileTBID* tileTBID = new TileTBID();
     tileID->set_do_neighbours (false);
     parser.register_external_entity ("TileCalorimeter", "IdDictTileCalorimeter.xml");
     IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
     assert (tileHWID->initialize_from_dictionary (idd) == 0);
     assert (tileTBID->initialize_from_dictionary (idd) == 0);
     assert (tileID->initialize_from_dictionary (idd) == 0);
     ServiceHandle<StoreGateSvc> detStore("DetectorStore", "");
     assert(detStore.retrieve().isSuccess());
     assert( (detStore->record(tileHWID, "TileHWID")).isSuccess() );
     assert( (detStore->record(tileTBID, "TileTBID")).isSuccess() );
     assert( (detStore->record(tileID, "TileID")).isSuccess() );
     TileCablingService* svc = TileCablingService::getInstance_nc();
     svc->setTileHWID(tileHWID);
     svc->setTileTBID(tileTBID);
     svc->setTileID(tileID);
  }
};

void test1() {
  std::cout << "test1\n";
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  ServiceHandle<StoreGateSvc> evtStore("StoreGateSvc", "");
  assert(evtStore.retrieve().isSuccess());
  ServiceHandle<StoreGateSvc> detStore("DetectorStore", "");
  assert(detStore.retrieve().isSuccess());
  EventContext ctx;
  Atlas::setExtendedEventContext (ctx, Atlas::ExtendedEventContext( evtStore.get() ) );
  TileHWID* tileHWID(nullptr);
  assert( detStore->retrieve(tileHWID).isSuccess() );
  auto alg = std::make_unique<TileRawChannelNNMaker>("TileRawChannelNNMakerTest", svcLoc);
  assert( (alg->initialize()).isSuccess() );
  unsigned int ros = 1;
  unsigned int drawer = 1;

  {
    auto digitsContainer = std::make_unique<TileMutableDigitsContainer>(true);

    for (unsigned int channel = 0; channel < HG_SAMPLES.size(); ++channel) {
      HWIdentifier loId = tileHWID->adc_id(ros, drawer, channel, TileHWID::LOWGAIN);
      HWIdentifier hiId = tileHWID->adc_id(ros, drawer, channel, TileHWID::HIGHGAIN);
      assert(digitsContainer->push_back(new TileDigits(loId, LG_SAMPLES[channel])).isSuccess());
      assert(digitsContainer->push_back(new TileDigits(hiId, HG_SAMPLES[channel])).isSuccess());
    }

    HWIdentifier loneId = tileHWID->adc_id(ros, drawer, int(HG_SAMPLES.size()), TileHWID::LOWGAIN);
    assert(digitsContainer->push_back(new TileDigits(loneId, LG_SAMPLES[0])).isSuccess());
    SG::WriteHandle<TileDigitsContainer> digitsCnt("TileDigitsCnt");
    assert(digitsCnt.record(std::move(digitsContainer)).isSuccess());
  }

  assert( (alg->execute(ctx)).isSuccess() );
  SG::ReadHandle<TileRawChannelContainer> outputContainer("TileRawChannelNN");
  assert( outputContainer.isValid() );
  const double ampPerCode = std::ldexp(4095.0, -16);
  int nRawChannels(0);

  for (const TileRawChannelCollection* collection : *outputContainer) {
    for (const TileRawChannel* rawChannel : *collection) {
      ++nRawChannels;
      HWIdentifier adcId = rawChannel->adc_HWID();
      unsigned int channel = tileHWID->channel(adcId);
      assert(unsigned(tileHWID->ros(adcId)) == ros);
      assert(unsigned(tileHWID->drawer(adcId)) == drawer);
      assert(tileHWID->adc(adcId) == TileHWID::LOWGAIN);
      assert(channel < HG_SAMPLES.size());
      assert(rawChannel->amplitude() == static_cast<float>(EXPECTED_CODE[channel] * ampPerCode));
    }
  }
  
  assert(nRawChannels == int(HG_SAMPLES.size()));
  assert( (alg->finalize()).isSuccess() );
}

//coverity[UNCAUGHT_EXCEPT]
int main ATLAS_NOT_THREAD_SAFE (int /*argc*/, char** argv) {
  Athena_test::setupStoreGate (argv[0]);
  IdDictParser parser;
  TileCablingSvc::init_idhelpers(parser);
  try{
    test1();
  } catch (std::exception & e){
    std::cerr<<"Exception "<<e.what()<<" in TileRawChannelNNMaker_test"<<std::endl;
    return 1;
  }

  return 0;
}

#include "../src/TileNNEmulator.cxx"
#include "../src/TileRawChannelNNMaker.cxx"
