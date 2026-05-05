/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE ALFA_ConfigParams_test
#include <boost/test/unit_test.hpp>

#include "ALFA_Geometry/ALFA_ConfigParams.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path
writeConfigFile(const std::string& contents){
  const auto path = std::filesystem::temp_directory_path() / "ALFA_ConfigParams_test.ini";

  std::ofstream out{path};
  BOOST_REQUIRE(out.good());

  out << contents;
  out.close();

  BOOST_REQUIRE(std::filesystem::exists(path));
  return path;
}

} // namespace

BOOST_AUTO_TEST_CASE(InitReadsRequestedSection){
  const auto path = writeConfigFile(
    "[FIRST]\n"
    "ignored=value\n"
    "\n"
    "[ALFA]\n"
    "beamEnergy=6500\n"
    "detector=ALFA\n"
    "; comment line ignored\n"
    "emptyLineFollows=yes\n"
    "\n"
    "[OTHER]\n"
    "beamEnergy=42\n"
  );

  ALFA_ConfigParams params;

  BOOST_CHECK_EQUAL(params.Init(path.c_str(), "[ALFA]"), 3);

  BOOST_REQUIRE(params.IsKey("beamEnergy"));
  BOOST_CHECK_EQUAL(params.GetParameter("beamEnergy"), std::string{"6500"});

  BOOST_REQUIRE(params.IsKey("detector"));
  BOOST_CHECK_EQUAL(params.GetParameter("detector"), std::string{"ALFA"});

  BOOST_REQUIRE(params.IsKey("emptyLineFollows"));
  BOOST_CHECK_EQUAL(params.GetParameter("emptyLineFollows"), std::string{"yes"});

  BOOST_CHECK(!params.IsKey("ignored"));
  BOOST_CHECK(params.GetParameter("ignored") == nullptr);

  BOOST_CHECK(!params.IsKey("missing"));
  BOOST_CHECK(params.GetParameter("missing") == nullptr);

  std::filesystem::remove(path);
}

BOOST_AUTO_TEST_CASE(InitStopsAtNextSection){
  const auto path = writeConfigFile(
    "[ALFA]\n"
    "first=1\n"
    "[NEXT]\n"
    "second=2\n"
  );

  ALFA_ConfigParams params;

  BOOST_CHECK_EQUAL(params.Init(path.c_str(), "[ALFA]"), 1);

  BOOST_REQUIRE(params.IsKey("first"));
  BOOST_CHECK_EQUAL(params.GetParameter("first"), std::string{"1"});

  BOOST_CHECK(!params.IsKey("second"));
  BOOST_CHECK(params.GetParameter("second") == nullptr);

  std::filesystem::remove(path);
}

BOOST_AUTO_TEST_CASE(MissingSectionLeavesObjectInvalid){
  const auto path = writeConfigFile(
    "[ALFA]\n"
    "first=1\n"
  );

  ALFA_ConfigParams params;

  BOOST_CHECK_EQUAL(params.Init(path.c_str(), "[MISSING]"), 0);

  BOOST_CHECK(!params.IsKey("first"));
  BOOST_CHECK(params.GetParameter("first") == nullptr);

  std::filesystem::remove(path);
}

BOOST_AUTO_TEST_CASE(UnInitializeClearsPreviouslyReadParameters){
  const auto path = writeConfigFile(
    "[ALFA]\n"
    "first=1\n"
  );

  ALFA_ConfigParams params;

  BOOST_CHECK_EQUAL(params.Init(path.c_str(), "[ALFA]"), 1);
  BOOST_CHECK(params.IsKey("first"));
  BOOST_CHECK_EQUAL(params.GetParameter("first"), std::string{"1"});

  params.UnInitialize();

  BOOST_CHECK(!params.IsKey("first"));
  BOOST_CHECK(params.GetParameter("first") == nullptr);

  std::filesystem::remove(path);
}
