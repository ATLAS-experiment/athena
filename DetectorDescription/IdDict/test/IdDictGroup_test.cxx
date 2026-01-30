/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file IdDict/test/IdDitGroup_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2025
 * @brief Tests for IdDictGroup (incomplete).
 */


#undef NDEBUG

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>

#include "IdDict/IdDictGroup.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictLabel.h"
#include <memory>
#include <initializer_list>
#include <cstdlib>


BOOST_AUTO_TEST_SUITE(IdDictDictionaryTest)


const std::string group_name = "lar_fcal";


std::unique_ptr<IdDictRange> make_range (const std::string& field_name,
                                         const std::string& label)
{
  return std::make_unique<IdDictRange>(field_name, label);
}

std::unique_ptr<IdDictRange> make_range (const std::string& field_name,
                                         std::initializer_list<std::string> labels)
{
  std::vector<std::string> vlabels (labels);
  return std::make_unique<IdDictRange> (field_name, vlabels);
}

 
std::unique_ptr<IdDictRange> make_range (const std::string& field_name,
                                         const IdDictDictionary& d)
{
  const IdDictField* f = d.find_field (field_name);
  if (!f) std::abort();
  std::vector<std::string> labels;
  for (size_t i = 0; i < f->get_label_number(); ++i) {
    labels.push_back (f->get_label (i));
  }
  return std::make_unique<IdDictRange>(field_name, labels);
}


std::unique_ptr<IdDictRange> make_range (const std::string& field_name,
                                         int minvalue,
                                         int maxvalue)
{
  return std::make_unique<IdDictRange>(field_name, minvalue, maxvalue);
}


std::unique_ptr<IdDictField> make_field (const std::string& field_name,
                                         std::initializer_list<std::string> labels,
                                         std::initializer_list<int> values)
{
  auto f = std::make_unique<IdDictField>(field_name);

  std::vector<std::string> vlabels (labels);
  std::vector<int> vvalues (values);
  if (vlabels.size() != vvalues.size()) std::abort();
  for (size_t i = 0; i < vlabels.size(); ++i) {
    f->add_label (std::make_unique<IdDictLabel> (vlabels[i], vvalues[i]));
  }
  return f;
}


std::unique_ptr<IdDictRegion> make_region (const std::string& name,
                                           const std::string& modlab)
{
  auto r = std::make_unique<IdDictRegion>(name, group_name, "");
  r->add_entry (make_range ("subdet", "LArCalorimeter"));
  r->add_entry (make_range ("part", "LArFCAL"));
  r->add_entry (make_range ("barrel-endcap",
                            { "negative-endcap-outer-wheel", "positive-endcap-outer-wheel"}));
  r->add_entry (make_range ("module", modlab.substr(0, 1)));

  if (modlab == "1") {
    r->add_entry (make_range ("eta-fcal", 0, 62));
    r->add_entry (make_range ("phi-fcal", 0, 15));
  }
  else if (modlab == "2a") {
    r->add_entry (make_range ("eta-fcal", 0, 29));
    r->add_entry (make_range ("phi-fcal", {"0", "7", "8", "15"}));
  }
  else if (modlab == "2b") {
    r->add_entry (make_range ("eta-fcal", 0, 30));
    r->add_entry (make_range ("phi-fcal", {"3", "4", "11", "12"}));
  }
  else if (modlab == "2c") {
    r->add_entry (make_range ("eta-fcal", 0, 31));
    r->add_entry (make_range ("phi-fcal", {"1", "2", "5", "6", "9", "10", "13", "14"}));
  }
  else if (modlab == "3a") {
    r->add_entry (make_range ("eta-fcal", 0, 14));
    r->add_entry (make_range ("phi-fcal", {"2", "10"}));
  }
  else if (modlab == "3b") {
    r->add_entry (make_range ("eta-fcal", 14, 14));
    r->add_entry (make_range ("phi-fcal", { "0", "1", "3", "4", "5", "6", "7", "8", "9", "11", "12", "13", "14", "15"}));
  }
  else if (modlab == "3c") {
    r->add_entry (make_range ("eta-fcal", 15, 15));
    r->add_entry (make_range ("phi-fcal", { "0", "1", "2", "3", "4", "6", "7", "8", "9", "10", "11", "12", "14", "15"}));
  }

  r->add_entry (make_range ("is-slar-fcal", "cell"));
  return r;
}


bool check_unpack (const IdDictDictionary& dictionary,
                   std::initializer_list<size_t> indices,
                   std::initializer_list<int> exp_id,
                   const std::string& exp_s)
{
  Identifier::value_type val = 0;
  const IdDictRegion* r = dictionary.find_region ("dummy");
  if (not r){
    std::cerr << "region 'dummy' not found in IdDictGroup_test\n";
    return false;
  }
  std::vector<size_t> vindices (indices);
  size_t nimpl = r->n_implementation();
  for (size_t ifield = 0; ifield < nimpl; ++ifield)
  {
    const IdDictFieldImplementation& impl = r->implementation(ifield);
    if (ifield >= vindices.size()) break;
    val |= (vindices[ifield] << impl.shift());
  }

  Identifier id (val);
  ExpandedIdentifier pref;
  ExpandedIdentifier unpacked;
  int ret = dictionary.unpack (group_name, id, pref, vindices.size(), unpacked);
  if (ret) return false;

  ExpandedIdentifier exp;
  for (int x : exp_id) {
    exp << x;
  }

  if (exp != unpacked) {
    std::cerr << "Identifier 0x" << std::hex << val << std::dec
              << " unpacked as:\n";
    std::cerr << "  " << unpacked <<"\n";
    std::cerr << "expected:\n";
    std::cerr << "  " << exp <<"\n";
    return false;
  }

  std::string unpacked_s;
  ret = dictionary.unpack (group_name, id, pref, vindices.size(),
                           " ", unpacked_s);
  if (ret) return false;

  if (unpacked_s != exp_s) {
    std::cerr << "Identifier 0x" << std::hex << val << std::dec
              << " unpacked as:\n";
    std::cerr << "  " << unpacked_s <<"\n";
    std::cerr << "expected:\n";
    std::cerr << "  " << exp_s <<"\n";
    return false;
  }

  return true;
}


BOOST_AUTO_TEST_CASE(Unpack)
{
  IdDictDictionary dictionary;
  IdDictMgr idd;

  dictionary.add_field (make_field ("subdet",
                                    {"InnerDetector",
                                     "LArCalorimeter",
                                     "TileCaloriemter",
                                     "MuonSpectrometer",
                                     "Calorimeter",
                                     "LArHighVoltage",
                                     "LArElectrode",
                                     "ForwardDetectors"},
                                    {2, 4, 5, 7, 10, 11, 12, 13}));
  dictionary.add_field (make_field ("part",
                                    {"LArEM",
                                     "LArHEC",
                                     "LArFCAL",
                                     "LArOnline",
                                     "LArOnlineCalib",
                                     "LArEMdisc",
                                     "LArHECdisc",
                                     "LArFCALdisc"},
                                    {1, 2, 3, 4, 5, -1, -2, -3}));

  dictionary.add_field (make_field ("barrel-endcap",
                                    {"negative-endcap-outer-wheel",
                                     "positive-endcap-outer-wheel",
                                    },
                                    {-2, 2}));
  dictionary.add_field (make_field ("is-slar-fcal",
                                    {"cell", "slar"},
                                    {0, 1}));


  dictionary.add_dictentry (make_region ("LArFCAL-1", "1"));
  dictionary.add_dictentry (make_region ("LArFCAL-2a", "2a"));
  dictionary.add_dictentry (make_region ("LArFCAL-2b", "2b"));
  dictionary.add_dictentry (make_region ("LArFCAL-2c", "2c"));
  dictionary.add_dictentry (make_region ("LArFCAL-3a", "3a"));
  dictionary.add_dictentry (make_region ("LArFCAL-3b", "3b"));
  dictionary.add_dictentry (make_region ("LArFCAL-3c", "3c"));

  auto dummy = std::make_unique<IdDictRegion> ("dummy", group_name, "");
  dummy->add_entry (make_range ("subdet", dictionary));
  dummy->add_entry (make_range ("part", dictionary));
  dummy->add_entry (make_range ("barrel-endcap", dictionary));
  dummy->add_entry (make_range ("module", 1, 3));
  dummy->add_entry (make_range ("eta-fcal", 0, 63));
  dummy->add_entry (make_range ("phi-fcal", 0, 15));
  dummy->add_entry (make_range ("is-slar-fcal", dictionary));
  dictionary.add_dictentry (std::move(dummy));

  dictionary.resolve_references (idd);
  dictionary.generate_implementation (idd, "");
  dictionary.dump();

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  0, 1, 11, 15, 0},
                            {4, 3, -2, 2, 11, 15, 0},
                            "LArCalorimeter LArFCAL negative-endcap-outer-wheel module 2 eta-fcal 11 phi-fcal 15 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {2, 5,  0, 1, 11, 15, 0},
                            {},
                            "") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 4,  0, 1, 11, 15, 0},
                            {4},
                            "LArCalorimeter") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  0, 0, 62, 15, 0},
                            {4, 3, -2, 1, 62, 15, 0},
                            "LArCalorimeter LArFCAL negative-endcap-outer-wheel module 1 eta-fcal 62 phi-fcal 15 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 1, 29,  7, 0},
                            {4, 3,  2, 2, 29,  7, 0},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 2 eta-fcal 29 phi-fcal 7 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 1, 30,  3, 0},
                            {4, 3,  2, 2, 30,  3, 0},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 2 eta-fcal 30 phi-fcal 3 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 1, 31,  2, 0},
                            {4, 3,  2, 2, 31,  2, 0},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 2 eta-fcal 31 phi-fcal 2 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 1, 30,  7, 0},
                            {4, 3,  2, 2, 30},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 2 eta-fcal 30") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 2, 14, 4, 0},
                            {4, 3,  2, 3, 14, 4, 0},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 3 eta-fcal 14 phi-fcal 4 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 2, 14, 3, 0},
                            {4, 3,  2, 3, 14, 3, 0},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 3 eta-fcal 14 phi-fcal 3 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 2, 15, 6, 0},
                            {4, 3,  2, 3, 15, 6, 0},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 3 eta-fcal 15 phi-fcal 6 cell") );

  BOOST_TEST( check_unpack (dictionary,
                            {1, 5,  1, 2, 15, 6, 1},
                            {4, 3,  2, 3, 15, 6},
                            "LArCalorimeter LArFCAL positive-endcap-outer-wheel module 3 eta-fcal 15 phi-fcal 6") );
}


BOOST_AUTO_TEST_SUITE_END()
