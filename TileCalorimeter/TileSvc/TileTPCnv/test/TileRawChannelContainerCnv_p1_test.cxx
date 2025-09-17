/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TileTPCnv/test/TileRawChannelContainerCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2019
 * @brief Tests for TileRawChannelContainerCnv_p1.
 */


#include "Identifier/HWIdentifier.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileRawChannelCollection.h"
#include "TileEvent/TileRawChannelContainer.h"
#include <memory>
#undef NDEBUG
#include "TileTPCnv/TileRawChannelContainerCnv_p1.h"
#include "TileConditions/TileCablingService.h"
#include "TileIdentifier/TileHWID.h"
#include "IdDictParser/IdDictParser.h"
#include "TestTools/initGaudi.h"
#include "TestTools/leakcheck.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <memory>


class TileCablingSvc
{
public:
  TileCablingSvc (IdDictParser& parser) ATLAS_NOT_THREAD_SAFE
  {
    tileid.set_do_neighbours (false);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    hwid.set_quiet (true);
    tbid.set_quiet (true);
    tileid.set_quiet (true);
    assert (hwid.initialize_from_dictionary (idd) == 0);
    assert (tbid.initialize_from_dictionary (idd) == 0);
    assert (tileid.initialize_from_dictionary (idd) == 0);
    TileCablingService* svc = TileCablingService::getInstance_nc();
    svc->setTileHWID (&hwid);
    svc->setTileTBID (&tbid);
    svc->setTileID (&tileid);
  }

  TileHWID hwid;
  TileTBID tbid;
  TileID   tileid;
};


void compare (const TileRawChannel& p1,
              const TileRawChannel& p2)
{
  assert (p1.identify() == p2.identify());

  assert (p1.size() == p2.size());
  for (int i=0; i < p1.size(); i++)
    assert (p1.amplitude(i) == p2.amplitude(i));
  assert (p1.sizeTime() == p2.sizeTime());
  for (int i=0; i < p1.sizeTime(); i++)
    assert (p1.time(i) == p2.time(i));
  assert (p1.sizeQuality() == p2.sizeQuality());
  for (int i=0; i < p1.sizeQuality(); i++)
    assert (p1.quality(i) == p2.quality(i));
  assert (p1.pedestal() == p2.pedestal());
}


void compare (const TileRawChannelCollection& p1,
              const TileRawChannelCollection& p2)
{
  assert (p1.identify() == p2.identify());
  // These aren't saved.
  //assert (p1.getLvl1Id() == p2.getLvl1Id());
  //assert (p1.getLvl1Type() == p2.getLvl1Type());
  //assert (p1.getDetEvType() == p2.getDetEvType());
  //assert (p1.getRODBCID() == p2.getRODBCID());
  assert (p1.size() == p1.size());
  for (size_t j = 0; j < p1.size(); j++) {
    compare (*p1[j], *p2[j]);
  }
}


void compare (const TileRawChannelContainer& p1,
              const TileRawChannelContainer& p2)
{
  assert (p1.get_hashType() == p2.get_hashType());
  assert (p1.get_unit() == p2.get_unit());

  TileRawChannelContainer::const_iterator it1 = p1.begin();
  TileRawChannelContainer::const_iterator it1e = p1.end();
  TileRawChannelContainer::const_iterator it2 = p2.begin();
  TileRawChannelContainer::const_iterator it2e = p2.end();
  while (it1 != it1e && it2 != it2e) {
    assert (it1.hashId() == it2.hashId());
    compare (**it1, **it2);
    ++it1;
    ++it2;
  }
  assert (it1 == it1e && it2 == it2e);
}


void testit (const TileRawChannelContainer& trans1)
{
  MsgStream log (0, "test");
  TileRawChannelContainerCnv_p1 cnv;
  TileRawChannelContainer_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  TileRawChannelContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


std::unique_ptr<const TileRawChannelContainer>
makecont (const TileID& tileid)
{
  auto cont = std::make_unique<TileRawChannelContainer> (false,
                                                       TileFragHash::Default,
                                                       TileRawChannelUnit::ADCcounts);

  std::vector<std::unique_ptr<TileRawChannelCollection> > colls (100);
  for (int hi=2; hi <= 3; hi++) {
    auto coll = std::make_unique<TileRawChannelCollection>(IdentifierHash(hi));
    coll->setLvl1Id (hi + 10);
    coll->setLvl1Type (hi + 20);
    coll->setDetEvType (hi + 30);
    coll->setRODBCID (hi + 40);

    for (int i=0; i < 10; i++) {
      int offs = i*10 + hi*100;

      Identifier id = tileid.adc_id (1, hi*2 - 5, 2, i, 0, 0, 0, true);

      auto elt = std::make_unique<TileRawChannel>
        (TileCablingService::getInstance()->s2h_adc_id (id),
         std::vector<float> {offs + 1.5f},
         std::vector<float> {offs + 2.5f, offs + 3.5f},
         std::vector<float> {offs + 4.5f, offs + 5.5f, offs + 6.5f},
         offs + 7.5);

      int hash = cont->hashFunc().hash(elt->frag_ID());
      if (hash >= static_cast<int>(colls.size())) {
        colls.resize (hash+1);
      }
      if (!colls[hash]) {
        auto coll = std::make_unique<TileRawChannelCollection>(IdentifierHash(elt->frag_ID()));
        coll->setLvl1Id (hash + 10);
        coll->setLvl1Type (hash + 20);
        coll->setDetEvType (hash + 30);
        coll->setRODBCID (hash + 40);
        colls[hash] = std::move(coll);
      }

      colls[hash]->push_back (std::move (elt));
    }
  }

  for (size_t hash = 0; hash < colls.size(); hash++) {
    if (colls[hash]) {
      cont->addCollection (colls[hash].release(), hash).ignore();
    }
  }
  return cont;
}


void test1 ATLAS_NOT_THREAD_SAFE (const TileID& tileid)
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  std::unique_ptr<const TileRawChannelContainer> trans1 = makecont (tileid);

  testit (*trans1);
}


std::unique_ptr<TileRawChannelCollection>
getCollection(const TileHWID& tileHWID, unsigned int bcid, unsigned int frag, unsigned int mask)
{
  auto coll = std::make_unique<TileRawChannelCollection>(frag);
  coll->setFragDSPBCID(bcid);
  coll->setFragBCID(mask);
  coll->setFragRODChipMask(~mask);
  HWIdentifier hwid = tileHWID.adc_id(tileHWID.drawer_id(frag), 0, 0);
  coll->push_back(std::make_unique<TileRawChannel>(hwid, std::vector<float>(1, frag),
                                                   std::vector<float>(1, frag + 1000.0),
                                                   std::vector<float>(1, frag + 2000.0)));
  return coll;
}


std::unique_ptr<TileRawChannelContainer>
getContainerDSP(const TileHWID& tileHWID)
{
  auto container = std::make_unique<TileRawChannelContainer>(false,
                                                             TileFragHash::OptFilterDsp,
                                                             TileRawChannelUnit::ADCcounts);

  unsigned int goodBCID = 1234;
  std::map<unsigned int, unsigned int> fragGoodMask{{0x100, 0}, {0x300, 0xC300}, {0x30E, 0xC301}};
  for (const auto& fragAndMask : fragGoodMask) {
    std::unique_ptr<TileRawChannelCollection> coll = getCollection(tileHWID, goodBCID, fragAndMask.first, fragAndMask.second);
    int hash = container->hashFunc()(coll->identify());
    container->addCollection(coll.release(), hash).ignore();
  }

  return container;
}

void checkGoodMetaData(const TileRawChannelContainer& container, unsigned int goodBCID, int exludeFrag = -1)
{
  for (const TileRawChannelCollection* coll : container) {
    if (coll->identify() == exludeFrag) continue;
    assert (coll->getFragDSPBCID() == goodBCID);
    assert (coll->getFragGlobalCRC() == 0);
    assert (coll->getFragBCID() == 0);
    assert (coll->getFragMemoryPar() == 0);
    assert (coll->getFragHeaderBit() == 0);
    assert (coll->getFragHeaderPar() == 0);
    assert (coll->getFragSampleBit() == 0);
    assert (coll->getFragSamplePar() == 0);
    assert (coll->getFragFEChipMask() == 0xFFFF);
    assert (coll->getFragRODChipMask() == 0xFFFF);
  }
}

void testCollectionMetaDataNotDSP ATLAS_NOT_THREAD_SAFE (const TileID& tileid)
{
  // Test the case when Tile raw channel container does not come from DSP
  std::cout << "testCollectionMetaDataNotDSP\n";
  Athena_test::Leakcheck check;

  std::unique_ptr<const TileRawChannelContainer> trans1 = makecont(tileid);

  MsgStream log (0, "test");
  TileRawChannelContainerCnv_p1 cnv;

  TileRawChannelContainer_p1 pers;
  cnv.transToPers (trans1.get(), &pers, log);
  // No collection metadata should be saved if Tile raw channel container does not come from DSP
  // Only container metadata packed into the first element
  assert(pers.getParam().size() == 1);
}

void testCollectionMetaDataGood ATLAS_NOT_THREAD_SAFE (const TileHWID& tileHWID)
{
  // Test the case when Tile raw channel container comes from DSP but there are no errors in collection metadata
  std::cout << "testCollectionMetaDataGood\n";
  Athena_test::Leakcheck check;

  MsgStream log (0, "test");
  TileRawChannelContainerCnv_p1 cnv;

  std::unique_ptr<TileRawChannelContainer> container1 = getContainerDSP(tileHWID);
  unsigned int goodBCID = (*(container1->begin()))->getFragDSPBCID();

  TileRawChannelContainer_p1 pers;
  cnv.transToPers(container1.get(), &pers, log);
  // If Tile raw channel container comes from DSP
  // and there are no errors in collection metadata only good BCID is saved
  // in addition to container metadata packed into the first element
  assert(pers.getParam().size() == 2);

  TileRawChannelContainer container2;
  cnv.persToTrans (&pers, &container2, log);

  checkGoodMetaData(container2, goodBCID);

  compare(*container1, container2);
}


void testCollectionMetaDataBad ATLAS_NOT_THREAD_SAFE (const TileHWID& tileHWID)
{
  // Test the case when Tile raw channel container comes from DSP but there are errors in one collection metadata
  std::cout << "testCollectionMetaDataBad\n";
  Athena_test::Leakcheck check;

  MsgStream log (0, "test");
  TileRawChannelContainerCnv_p1 cnv;

  std::unique_ptr<TileRawChannelContainer> container1 = getContainerDSP(tileHWID);
  unsigned int goodBCID = (*(container1->begin()))->getFragDSPBCID();

  int frag = 0x200;
  unsigned int wrongBCID = 4321;
  std::unique_ptr<TileRawChannelCollection> coll1 = getCollection(tileHWID, wrongBCID, frag, 0x0);

  unsigned int badCRC =                1;
  unsigned int badBCID =        (1 << 1);
  unsigned int badMemoryPar =   (1 << 2);
  unsigned int badHeaderBit =   (1 << 3);
  unsigned int badHeaderPar =   (1 << 4);
  unsigned int badSampleBit =   (1 << 5);
  unsigned int badSamplePar =   (1 << 6);
  unsigned int badFEChipMask =  (1 << 7);
  unsigned int badRODChipMask = (1 << 8);

  coll1->setFragGlobalCRC(badCRC);
  coll1->setFragBCID(badBCID);
  coll1->setFragMemoryPar(badMemoryPar);
  coll1->setFragHeaderBit(badHeaderBit);
  coll1->setFragHeaderPar(badHeaderPar);
  coll1->setFragSampleBit(badSampleBit);
  coll1->setFragSamplePar(badSamplePar);
  // The following two are inverted
  coll1->setFragFEChipMask(badFEChipMask ^ 0xFFFF);
  coll1->setFragRODChipMask(badRODChipMask ^ 0xFFFF);

  int hash = container1->hashFunc()(coll1->identify());
  container1->addCollection(coll1.release(), hash).ignore();

  TileRawChannelContainer_p1 pers;
  cnv.transToPers(container1.get(), &pers, log);
  // If Tile raw channel container comes from DSP
  // and there are errors only in one collection metadata
  // good BCID and 5 elements for problematic collection are saved
  // in addition to container metadata packed into the first element
  assert(pers.getParam().size() == 7);

  TileRawChannelContainer container2;
  cnv.persToTrans (&pers, &container2, log);

  checkGoodMetaData(container2, goodBCID, frag);

  const TileRawChannelCollection* coll2 = container2.indexFindPtr(container2.hashFunc()(frag));
  assert(coll2->getFragDSPBCID() == wrongBCID);
  assert(coll2->getFragGlobalCRC() == badCRC);
  assert(coll2->getFragBCID() == badBCID);
  // All other errors should be packed into fragment memory parity
  assert(coll2->getFragMemoryPar() == (badMemoryPar | badHeaderBit | badHeaderPar | badSampleBit | badSamplePar | badFEChipMask | badRODChipMask));
  // All other checks should be good
  assert(coll2->getFragHeaderBit() == 0);
  assert(coll2->getFragHeaderPar() == 0);
  assert(coll2->getFragSampleBit() == 0);
  assert(coll2->getFragSamplePar() == 0);
  // The following two are inverted
  assert(coll2->getFragFEChipMask() == 0xFFFF);
  assert(coll2->getFragRODChipMask() == 0xFFFF);

  compare(*container1, container2);
}



int main ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "TileTPCnv/TileRawChannelContainerCnv_p1_test\n";
  IdDictParser parser;
  try{
    auto helpers = std::make_unique<TileCablingSvc>(parser);
    test1 (helpers->tileid);

    testCollectionMetaDataNotDSP(helpers->tileid);
    testCollectionMetaDataGood(helpers->hwid);
    testCollectionMetaDataBad(helpers->hwid);
  } catch (std::exception & e){
    std::cerr<<"Exception "<<e.what()<<" in TileRawChannelContainerCnv_p1_test."<<std::endl;
    return 1;
  }

  return 0;
}
