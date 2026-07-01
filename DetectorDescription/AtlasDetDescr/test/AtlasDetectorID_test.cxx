/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AtlasDetDescr/test/AtlasDetectorID_test.cxx
 * @author Shaun Roe
 * @date April 2026
 * @brief Some tests for AtlasDetectorID
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE AtlasDetDescr

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictParser/IdDictParser.h"
#include "Identifier/ExpandedIdentifier.h"
#include "Identifier/IdContext.h"
#include "Identifier/IdentifierHash.h"

#include <boost/test/unit_test.hpp>

#include <array>
#include <string>
#include <iomanip> //for std::quoted

namespace {
  const std::string atlasDictFilename{"IdDictParser/ATLAS_IDS.xml"};
}

class TestAtlasDetectorID : public AtlasDetectorID {
public:
  TestAtlasDetectorID(const std::string& name, const std::string& group)
    : AtlasDetectorID(name, group) {}

  using AtlasDetectorID::indet_exp;
  using AtlasDetectorID::pixel_exp;
  using AtlasDetectorID::sct_exp;
  using AtlasDetectorID::trt_exp;
  using AtlasDetectorID::hgtd_exp;
  using AtlasDetectorID::lumi_exp;
  using AtlasDetectorID::lar_exp;
  using AtlasDetectorID::lar_em_exp;
  using AtlasDetectorID::lar_hec_exp;
  using AtlasDetectorID::lar_fcal_exp;
  using AtlasDetectorID::tile_exp;
  using AtlasDetectorID::muon_exp;
  using AtlasDetectorID::calo_exp;
  using AtlasDetectorID::fwd_exp;
  using AtlasDetectorID::alfa_exp;
  using AtlasDetectorID::bcm_exp;
  using AtlasDetectorID::lucid_exp;
  using AtlasDetectorID::zdc_exp;
};

BOOST_AUTO_TEST_SUITE(AtlasDetectorID_Test)

BOOST_AUTO_TEST_CASE(InitializeAndMetadata){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  BOOST_TEST((atlasId.helper() == AtlasDetectorID::HelperType::Unimplemented));
  BOOST_TEST(atlasId.group() == "AtlasDetDescr");
  //this will be empty; leave commented
  //BOOST_TEST(!atlasId.dictionaryVersion().empty());

  const auto& dictNames = atlasId.dict_names();
  const auto& fileNames = atlasId.file_names();
  const auto& dictTags  = atlasId.dict_tags();

  BOOST_TEST(!dictNames.empty());
  BOOST_TEST(dictNames.size() == fileNames.size());
  BOOST_TEST(dictNames.size() == dictTags.size());

  for (const auto& s : dictNames) {
    BOOST_TEST(!s.empty());
  }
  /* don't test these for now, but leave them commented
  for (const auto& s : fileNames) {
    BOOST_TEST(!s.empty());
  }
  for (const auto& s : dictTags) {
    BOOST_TEST(!s.empty());
  }
  */
}

BOOST_AUTO_TEST_CASE(Flags){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  const bool initialChecks = atlasId.do_checks();
  atlasId.set_do_checks(!initialChecks);
  BOOST_TEST(atlasId.do_checks() == !initialChecks);
  atlasId.set_do_checks(initialChecks);
  BOOST_TEST(atlasId.do_checks() == initialChecks);

  const bool initialNeighbours = atlasId.do_neighbours();
  atlasId.set_do_neighbours(!initialNeighbours);
  BOOST_TEST(atlasId.do_neighbours() == !initialNeighbours);
  atlasId.set_do_neighbours(initialNeighbours);
  BOOST_TEST(atlasId.do_neighbours() == initialNeighbours);
}

BOOST_AUTO_TEST_CASE(TopLevelIdentifierCreationAndClassification){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  const Identifier indetId = atlasId.indet();
  const Identifier larId   = atlasId.lar();
  const Identifier tileId  = atlasId.tile();
  const Identifier muonId  = atlasId.muon();
  const Identifier caloId  = atlasId.calo();

  BOOST_TEST(indetId.is_valid());
  BOOST_TEST(larId.is_valid());
  BOOST_TEST(tileId.is_valid());
  BOOST_TEST(muonId.is_valid());
  BOOST_TEST(caloId.is_valid());

  BOOST_TEST(indetId != larId);
  BOOST_TEST(indetId != tileId);
  BOOST_TEST(indetId != muonId);
  BOOST_TEST(indetId != caloId);

  BOOST_TEST(atlasId.is_indet(indetId));
  BOOST_TEST(atlasId.is_lar(larId));
  BOOST_TEST(atlasId.is_tile(tileId));
  BOOST_TEST(atlasId.is_muon(muonId));
  BOOST_TEST(atlasId.is_calo(caloId));

  BOOST_TEST(!atlasId.is_lar(indetId));
  BOOST_TEST(!atlasId.is_tile(indetId));
  BOOST_TEST(!atlasId.is_muon(indetId));
  BOOST_TEST(!atlasId.is_calo(indetId));
}

BOOST_AUTO_TEST_CASE(SubdetectorIdentifierCreationAndClassification){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  const Identifier pixelId   = atlasId.pixel();
  const Identifier sctId     = atlasId.sct();
  const Identifier trtId     = atlasId.trt();
  //unused for now, leave commented
  //const Identifier hgtdId    = atlasId.hgtd();
  const Identifier lumiId    = atlasId.lumi();

  const Identifier larEmId   = atlasId.lar_em();
  const Identifier larHecId  = atlasId.lar_hec();
  const Identifier larFcalId = atlasId.lar_fcal();

  const Identifier larLvl1Id = atlasId.lar_lvl1();
  const Identifier larDmId   = atlasId.lar_dm();
  const Identifier tileDmId  = atlasId.tile_dm();

  BOOST_TEST(pixelId.is_valid());
  BOOST_TEST(sctId.is_valid());
  BOOST_TEST(trtId.is_valid());
  BOOST_TEST(larEmId.is_valid());
  BOOST_TEST(larHecId.is_valid());
  BOOST_TEST(larFcalId.is_valid());
  BOOST_TEST(larLvl1Id.is_valid());
  BOOST_TEST(larDmId.is_valid());
  BOOST_TEST(tileDmId.is_valid());

  BOOST_TEST(atlasId.is_indet(pixelId));
  BOOST_TEST(atlasId.is_pixel(pixelId));
  BOOST_TEST(!atlasId.is_sct(pixelId));
  BOOST_TEST(!atlasId.is_trt(pixelId));
  BOOST_TEST(!atlasId.is_hgtd(pixelId));
  BOOST_TEST(!atlasId.is_lumi(pixelId));

  BOOST_TEST(atlasId.is_indet(sctId));
  BOOST_TEST(atlasId.is_sct(sctId));
  BOOST_TEST(!atlasId.is_pixel(sctId));
  BOOST_TEST(!atlasId.is_trt(sctId));

  BOOST_TEST(atlasId.is_indet(trtId));
  BOOST_TEST(atlasId.is_trt(trtId));
  BOOST_TEST(!atlasId.is_pixel(trtId));
  BOOST_TEST(!atlasId.is_sct(trtId));

  // These will depend on dictionary contents / current implementation
  /* comment failing test out for now
  if (hgtdId.is_valid()) {
    BOOST_TEST(atlasId.is_indet(hgtdId));
    BOOST_TEST(atlasId.is_hgtd(hgtdId));
  }
  */

  if (lumiId.is_valid()) {
    BOOST_TEST(atlasId.is_indet(lumiId));
    BOOST_TEST(atlasId.is_lumi(lumiId));
  }

  BOOST_TEST(atlasId.is_lar(larEmId));
  BOOST_TEST(atlasId.is_lar_em(larEmId));
  BOOST_TEST(!atlasId.is_lar_hec(larEmId));
  BOOST_TEST(!atlasId.is_lar_fcal(larEmId));

  BOOST_TEST(atlasId.is_lar(larHecId));
  BOOST_TEST(atlasId.is_lar_hec(larHecId));
  BOOST_TEST(!atlasId.is_lar_em(larHecId));
  BOOST_TEST(!atlasId.is_lar_fcal(larHecId));

  BOOST_TEST(atlasId.is_lar(larFcalId));
  BOOST_TEST(atlasId.is_lar_fcal(larFcalId));
  BOOST_TEST(!atlasId.is_lar_em(larFcalId));
  BOOST_TEST(!atlasId.is_lar_hec(larFcalId));

  BOOST_TEST(atlasId.is_calo(larLvl1Id));
  BOOST_TEST(atlasId.is_lvl1_trig_towers(larLvl1Id));

  BOOST_TEST(atlasId.is_calo(larDmId));
  BOOST_TEST(atlasId.is_lar_dm(larDmId));

  BOOST_TEST(atlasId.is_calo(tileDmId));
  BOOST_TEST(atlasId.is_tile_dm(tileDmId));
}

BOOST_AUTO_TEST_CASE(ExpandedIdentifierClassification, * boost::unit_test::expected_failures(2)){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  TestAtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  const ExpandedIdentifier indetExp   = atlasId.indet_exp();
  const ExpandedIdentifier pixelExp   = atlasId.pixel_exp();
  const ExpandedIdentifier sctExp     = atlasId.sct_exp();
  const ExpandedIdentifier trtExp     = atlasId.trt_exp();
  const ExpandedIdentifier hgtdExp    = atlasId.hgtd_exp();
  const ExpandedIdentifier lumiExp    = atlasId.lumi_exp();
  const ExpandedIdentifier larExp     = atlasId.lar_exp();
  const ExpandedIdentifier larEmExp   = atlasId.lar_em_exp();
  const ExpandedIdentifier larHecExp  = atlasId.lar_hec_exp();
  const ExpandedIdentifier larFcalExp = atlasId.lar_fcal_exp();
  const ExpandedIdentifier tileExp    = atlasId.tile_exp();
  const ExpandedIdentifier muonExp    = atlasId.muon_exp();
  const ExpandedIdentifier caloExp    = atlasId.calo_exp();

  BOOST_TEST(atlasId.is_indet(indetExp));
  BOOST_TEST(atlasId.is_pixel(pixelExp));
  BOOST_TEST(atlasId.is_sct(sctExp));
  BOOST_TEST(atlasId.is_trt(trtExp));

  BOOST_TEST(atlasId.is_lar(larExp));
  BOOST_TEST(atlasId.is_lar_em(larEmExp));
  BOOST_TEST(atlasId.is_lar_hec(larHecExp));
  BOOST_TEST(atlasId.is_lar_fcal(larFcalExp));

  BOOST_TEST(atlasId.is_tile(tileExp));
  BOOST_TEST(atlasId.is_muon(muonExp));
  BOOST_TEST(atlasId.is_calo(caloExp));

  if (hgtdExp.fields() > 1) {
    BOOST_TEST(atlasId.is_hgtd(hgtdExp));
  }
  if (lumiExp.fields() > 1) {
    BOOST_TEST(atlasId.is_lumi(lumiExp));
  }

  BOOST_TEST(!atlasId.is_sct(pixelExp));
  BOOST_TEST(!atlasId.is_trt(pixelExp));//buggy
  BOOST_TEST(!atlasId.is_pixel(sctExp));
  BOOST_TEST(!atlasId.is_trt(sctExp));//buggy
  BOOST_TEST(!atlasId.is_pixel(trtExp));
  BOOST_TEST(!atlasId.is_sct(trtExp));
}

BOOST_AUTO_TEST_CASE(Contexts){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  const IdContext detContext    = atlasId.detsystem_context();
  const IdContext subdetContext = atlasId.subdet_context();

  BOOST_TEST(detContext.begin_index() == 0);
  BOOST_TEST(subdetContext.begin_index() == 0);
  BOOST_TEST(detContext.end_index() <= subdetContext.end_index());
}

BOOST_AUTO_TEST_CASE(PrintMethods){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  const std::array<Identifier, 5> ids{
    atlasId.indet(),
    atlasId.pixel(),
    atlasId.lar_em(),
    atlasId.tile(),
    atlasId.calo()
  };

  for (const Identifier id : ids) {
    const std::string shown = atlasId.show_to_string(id);
    BOOST_TEST(!shown.empty());
    BOOST_TEST_MESSAGE("atlasId.show_to_string(id) output:");
    BOOST_TEST_MESSAGE(std::quoted(shown));
    // Current implementation may return an empty string here.
    const std::string printed = atlasId.print_to_string(id);
    BOOST_TEST_MESSAGE("atlasId.print_to_string(id) output:");
    BOOST_TEST_MESSAGE(std::quoted(printed));
  }

  BOOST_TEST(atlasId.show_to_string(Identifier{}) == "[INVALID]");
}

BOOST_AUTO_TEST_CASE(HashMethods_CurrentBehaviour){
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  IdentifierHash hash{};
  Identifier id{};

  // Keep this minimal unless get_hash/get_id are fully implemented.
  BOOST_TEST(atlasId.get_hash(atlasId.pixel(), hash, nullptr) == 0);
  BOOST_TEST(atlasId.get_id(hash, id, nullptr) == 0);
}

BOOST_AUTO_TEST_CASE(MuonCompactCreators_CurrentBehaviour)
{
  IdDictParser parser;
  IdDictMgr& idd = parser.parse(atlasDictFilename);

  AtlasDetectorID atlasId{"AtlasDetectorID", "AtlasDetDescr"};
  BOOST_TEST(atlasId.initialize_from_dictionary(idd) == 0);

  BOOST_TEST(!atlasId.mdt().is_valid());
  BOOST_TEST(!atlasId.csc().is_valid());
  BOOST_TEST(!atlasId.rpc().is_valid());
  BOOST_TEST(!atlasId.tgc().is_valid());
  BOOST_TEST(!atlasId.stgc().is_valid());
  BOOST_TEST(!atlasId.mm().is_valid());
}

BOOST_AUTO_TEST_SUITE_END()