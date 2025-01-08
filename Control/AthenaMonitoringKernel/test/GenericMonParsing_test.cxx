/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE GenericMonParsing
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

#include "AthenaMonitoringKernel/HistogramDef.h"

using namespace Monitored;
using json = nlohmann::json;

static const json defaultJson() {
  json j;
  j["alias"] = "var";
  j["allvars"] = json::array({"var"});
  j["convention"] = "";
  j["opt"] = "";
  j["path"] = "";
  j["title"] = "var";
  j["type"] = "TH1F";
  j["weight"] = "";
  j["cutMask"] = "";
  j["xarray"] = json::array();
  j["xbins"] = 100;
  j["xlabels"] = json::array();
  j["xmax"] = 1.0;
  j["xmin"] = 0.0;
  j["xvar"] = "var";
  j["yarray"] = json::array();
  j["ybins"] = 0;
  j["ylabels"] = json::array();
  j["ymax"] = 0.0;
  j["ymin"] = 0.0;
  j["yvar"] = "";
  j["zbins"] = 0;
  j["zlabels"] = json::array();
  j["zmax"] = 0.0;
  j["zmin"] = 0.0;
  j["zvar"] = "";
  j["Sumw2"] = false;
  j["kLBNHistoryDepth"] = 0;
  j["kAddBinsDynamically"] = false;
  j["kRebinAxes"] = false;
  j["kCanRebin"] = false;
  j["kVec"] = false;
  j["kVecUO"] = false;
  j["kCumulative"] = false;
  j["kLive"] = false;
  j["kAlwaysCreate"] = false;
  j["merge"] = "";
  j["treeDef"] = "";
  return j;
}


BOOST_AUTO_TEST_CASE( parse1D ) {
  json check = defaultJson();
  auto def = HistogramDef::parse(check.dump());

  BOOST_TEST( def.ok == true );
  BOOST_TEST( def.alias == "var" );
  BOOST_TEST( def.type == "TH1F" );
  BOOST_TEST( def.name.size() == 1 );
  BOOST_TEST( def.xvar == "var" );
  BOOST_TEST( def.xbins == 100 );
  BOOST_TEST( def.xmin == 0.0 );
  BOOST_TEST( def.xmax == 1.0 );
}


BOOST_AUTO_TEST_CASE( parse1D_options ) {
  json check = defaultJson();
  check["xbins"] = 10;
  check["xmin"] = -1.0;
  check["xmax"] = 1.0;
  check["title"] = "toptitle; xtitle; ytitle";
  check["Sumw2"] = true;
  check["kLBNHistoryDepth"] = 99;
  check["path"] = "mypath/tohistograms";
  check["type"] = "TH1D";
  check["weight"] = "myweight";
  check["cutMask"] = "mycutmask";
  auto def = HistogramDef::parse(check.dump());

  BOOST_TEST( def.ok == true );
  BOOST_TEST( def.xbins == 10 );
  BOOST_TEST( def.xmin == -1.0 );
  BOOST_TEST( def.xmax == 1.0 );
  BOOST_TEST( def.title == "toptitle; xtitle; ytitle" );
  BOOST_TEST( def.Sumw2 == true );
  BOOST_TEST( def.kLBNHistoryDepth == 99 );
  BOOST_TEST( def.path == "mypath/tohistograms" );
  BOOST_TEST( def.type == "TH1D" );
  BOOST_TEST( def.weight == "myweight" );
  BOOST_TEST( def.cutMask == "mycutmask" );
}


BOOST_AUTO_TEST_CASE( parse2D ) {
  json check = defaultJson();
  check["type"] = "TH2F";
  check["allvars"] = {"varX", "varY"};
  check["alias"] = "varX_vs_varY";
  check["xvar"] = "varX";
  check["yvar"] = "varY";
  check["ybins"] = 10;
  check["ymin"] = 0.0;
  check["ymax"] = 20.0;
  check["title"] = "X vs. Y; X [x unit]; Y [y unit]";
  auto def = HistogramDef::parse(check.dump());

  BOOST_TEST( def.ok == true );
  BOOST_TEST( def.type == "TH2F" );
  BOOST_TEST( def.name.size() == 2 );
  BOOST_TEST( std::string(def.name[0]) == "varX" );
  BOOST_TEST( std::string(def.name[1]) == "varY" );
  BOOST_TEST( def.alias == "varX_vs_varY" );
  BOOST_TEST( def.title == "X vs. Y; X [x unit]; Y [y unit]" );
  BOOST_TEST( def.xbins == 100 );
  BOOST_TEST( def.xmin == 0.0 );
  BOOST_TEST( def.xmax == 1.0 );
  BOOST_TEST( def.ybins == 10 );
  BOOST_TEST( def.ymin == 0.0 );
  BOOST_TEST( def.ymax == 20.0 );
}


BOOST_AUTO_TEST_CASE( parse3D ) {
  json check = defaultJson();
  check["type"] = "TProfile2D";
  check["allvars"] = {"varX", "varY", "varZ"};
  check["alias"] = "varX_vs_varY_vs_varZ";
  check["xvar"] = "varX";
  check["yvar"] = "varY";
  check["zvar"] = "varZ";
  check["ybins"] = 10;
  check["ymin"] = 0.0;
  check["ymax"] = 20.0;
  check["zmin"] = -1.0;
  check["zmax"] = 1.0;
  check["title"] = "X vs. Y vs. Z; X [x unit]; Y [y unit]; Z [z unit]";
  auto def = HistogramDef::parse(check.dump());

  BOOST_TEST( def.ok == true );
  BOOST_TEST( def.type == "TProfile2D" );
  BOOST_TEST( def.name.size() == 3 );
  BOOST_TEST( std::string(def.name[0]) == "varX" );
  BOOST_TEST( std::string(def.name[1]) == "varY" );
  BOOST_TEST( std::string(def.name[2]) == "varZ" );
  BOOST_TEST( def.alias == "varX_vs_varY_vs_varZ" );
  BOOST_TEST( def.title == "X vs. Y vs. Z; X [x unit]; Y [y unit]; Z [z unit]" );
  BOOST_TEST( def.xbins == 100 );
  BOOST_TEST( def.xmin == 0.0 );
  BOOST_TEST( def.xmax == 1.0 );
  BOOST_TEST( def.ybins == 10 );
  BOOST_TEST( def.ymin == 0.0 );
  BOOST_TEST( def.ymax == 20.0 );
  BOOST_TEST( def.zmin == -1.0 );
  BOOST_TEST( def.zmax == 1.0 );
}
