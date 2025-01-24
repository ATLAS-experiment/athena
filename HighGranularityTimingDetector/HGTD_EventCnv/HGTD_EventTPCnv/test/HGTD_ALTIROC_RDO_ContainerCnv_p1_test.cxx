/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventTPCnv/test/HGTD_ALTIROC_RDO_ContainerCnv_p1_test.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @brief
 */

#include "GaudiKernel/MsgStream.h"
#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_ContainerCnv_p1.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"
#include "IdDictParser/IdDictParser.h"
#include "Identifier/Identifier.h"
#include "SGTools/TestStore.h"
#include "StoreGate/StoreGateSvc.h"
#include "TestTools/initGaudi.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

std::unique_ptr<HGTD_ALTIROC_RDO> createRDO(int id, uint64_t word) {
  std::cout << "createRDO\n";

  Identifier identifier(id);

  return std::make_unique<HGTD_ALTIROC_RDO>(identifier, word);
}

void compare(const HGTD_ALTIROC_RDO& p1, const HGTD_ALTIROC_RDO& p2) {
  std::cout << "compare HGTD_ALTIROC_RDO\n";
  BOOST_CHECK(p1.identify() == p2.identify());
  BOOST_CHECK(p1.getToA() == p2.getToA());
  BOOST_CHECK(p1.getToT() == p2.getToT());
  BOOST_CHECK(p1.getBCID() == p2.getBCID());
  BOOST_CHECK(p1.getL1ID() == p2.getL1ID());
  BOOST_CHECK(p1.getCRC() == p2.getCRC());
  BOOST_CHECK(p1.getWord() == p2.getWord());
  std::cout << "compare HGTD_ALTIROC_RDO done\n";
}

void compare(const HGTD_ALTIROC_RDO_Container& p1,
             const HGTD_ALTIROC_RDO_Container& p2) {
  std::cout << "start HGTD_ALTIROC_RDO_Container comparison\n";
  HGTD_ALTIROC_RDO_Container::const_iterator it1 = p1.begin();
  HGTD_ALTIROC_RDO_Container::const_iterator it1e = p1.end();
  HGTD_ALTIROC_RDO_Container::const_iterator it2 = p2.begin();
  HGTD_ALTIROC_RDO_Container::const_iterator it2e = p2.end();
  while (it1 != it1e && it2 != it2e) {
    BOOST_CHECK(it1.hashId() == it2.hashId());
    const HGTD_ALTIROC_RDO_Collection& coll1 = **it1;
    const HGTD_ALTIROC_RDO_Collection& coll2 = **it2;
    BOOST_CHECK(coll1.size() == coll2.size());
    for (size_t j = 0; j < coll1.size(); j++) {
      compare(*coll1.at(j), *coll2.at(j));
    }
    ++it1;
    ++it2;
  }
  BOOST_CHECK(it1 == it1e && it2 == it2e);
}

std::unique_ptr<const HGTD_ALTIROC_RDO_Container> makeRDOContainer(const HGTD_ID* hgtd_idhelper) {
  auto container = std::make_unique<HGTD_ALTIROC_RDO_Container>(5);

  for (int hash = 2; hash <= 3; hash++) {
    // create a collection
    auto collection =
        std::make_unique<HGTD_ALTIROC_RDO_Collection>(IdentifierHash(hash));
    // fill it with RDOs
    for (unsigned int i = 1; i < 10; i++) {

      Identifier id = hgtd_idhelper->wafer_id(2, 1, hash, i);

      std::unique_ptr<HGTD_ALTIROC_RDO> rdo =
          createRDO(id.get_compact(), 0x42855003);
      collection->push_back(std::move(rdo));
    }

    BOOST_CHECK(collection->identifierHash() == IdentifierHash(hash));

    BOOST_CHECK(container->addCollection(collection.release(), hash).isSuccess());
  }
  return container;
}

void registerIDHelperAndDetManager() {
  IdDictParser parser;
  parser.register_external_entity(
      "InnerDetector", "InDetIdDictFiles/IdDictInnerDetector_ITK_HGTD_23.xml");
  IdDictMgr& idd = parser.parse("IdDictParser/ATLAS_IDS.xml");

  auto hgtd_id{std::make_unique<HGTD_ID>()};
  hgtd_id->initialize_from_dictionary(idd);

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  SmartIF<StoreGateSvc> sg{svcLoc->service("DetectorStore")};
  BOOST_CHECK(sg.isValid());
  BOOST_CHECK(sg->record(std::move(hgtd_id), "HGTD_ID"));
}

HGTD_ID* retrieve() {
  std::cout << "retrieve \n";

  // Get Storegate, ID helpers, and so on
  ISvcLocator* svc_locator = Gaudi::svcLocator();
  // get StoreGate service

  SmartIF<StoreGateSvc> storegate{svc_locator->service("StoreGateSvc")};
  if (!storegate) {
    std::cout << "StoreGate service not found !\n";
    return nullptr;
  }

  // get DetectorStore service
  SmartIF<StoreGateSvc> detStore{svc_locator->service("DetectorStore")};
  if (!detStore) {
    std::cout << "DetectorStore service not found !\n";
    return nullptr;
  }

  // Get the sct helper from the detector store
  HGTD_ID* hgtd_idhelper = nullptr;
  StatusCode sc = detStore->retrieve(hgtd_idhelper, "HGTD_ID");
  if (sc.isFailure()) {
    std::cout << "Could not get HGTD_ID helper !\n";
    return nullptr;
  } else {
    std::cout << "[retrieve] HGTD_ID initialised\n";
  }

  return hgtd_idhelper;
}

BOOST_AUTO_TEST_CASE(HGTD_ALTIROC_RDO_ContainerCnv_p1_test) {

  std::cout << "start HGTD_ALTIROC_RDO_ContainerCnv_p1_test\n";

  // initialise Gaudi for testing
  ISvcLocator* pSvcLoc;
  BOOST_REQUIRE(Athena_test::initGaudi("HGTD_EventTPCnv_test.txt", pSvcLoc));

  registerIDHelperAndDetManager();

  HGTD_ID* hgtd_idhelper = retrieve();
  BOOST_CHECK(hgtd_idhelper != nullptr);

  hgtd_idhelper->set_do_checks(true);

  std::unique_ptr<const HGTD_ALTIROC_RDO_Container> trans_container =
      makeRDOContainer(hgtd_idhelper);
  std::cout << "HGTD_ALTIROC_RDO_Container created\n";
  // otherwise there is nothing to test
  BOOST_REQUIRE(!trans_container->empty());

  MsgStream log(nullptr, "test");
  HGTD_ALTIROC_RDO_ContainerCnv_p1 cnv;

  HGTD_ALTIROC_RDO_Container_p1 pers;

  std::cout << "HGTD_ALTIROC_RDO_Container transToPers\n";
  cnv.transToPers(trans_container.get(), &pers, log);
  std::cout << "HGTD_ALTIROC_RDO_Container transToPers done\n";

  HGTD_ALTIROC_RDO_Container trans(hgtd_idhelper->wafer_hash_max());

  std::cout << "HGTD_ALTIROC_RDO_Container persToTrans\n";
  cnv.persToTrans(&pers, &trans, log);
  std::cout << "HGTD_ALTIROC_RDO_Container persToTrans done\n";

  compare(*trans_container, trans);

  std::cout << "done\n";
}
