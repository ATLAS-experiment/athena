/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE HistogramProviderTestSuite
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <memory>

#include "TH1F.h"

#include "AthenaMonitoringKernel/HistogramDef.h"
#include "TestTools/initGaudi.h"

#include "mocks/MockGenericMonitoringTool.h"
#include "mocks/MockHistogramFactory.h"
#include "../src/HistogramFiller/StaticHistogramProvider.h"
#include "../src/HistogramFiller/LumiblockHistogramProvider.h"
#include "../src/HistogramFiller/OfflineHistogramProvider.h"
#include "../src/HistogramFiller/LiveHistogramProvider.h"

using std::make_tuple;
using namespace Monitored;


/// Test fixture (run before each test)
struct TestFixture {
  TestFixture() {
    ServiceHandle<IToolSvc> toolSvc("ToolSvc", "LumiblockHistogramProviderTestSuite");

    BOOST_TEST( toolSvc.retrieve().isSuccess() );
    m_gmTool.reset(new MockGenericMonitoringTool(toolSvc.get()));
    m_histogramFactory.reset(new MockHistogramFactory());
  }

  std::shared_ptr<MockGenericMonitoringTool> m_gmTool;
  std::shared_ptr<MockHistogramFactory> m_histogramFactory;
};


// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE( HistogramProviderTestSuite,
                          TestFixture,
                          * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("GenericMonMinimal.txt")) )


BOOST_AUTO_TEST_CASE(test_shouldCreateAndReturnJustOneHistogram) {
  TNamed histogram;
  HistogramDef histogramDef;
  m_histogramFactory->mock_create = [&histogram, &histogramDef](const HistogramDef& def) mutable {
    BOOST_TEST( &def == &histogramDef );
    return &histogram;
  };

  StaticHistogramProvider testObj(m_histogramFactory, histogramDef);
  TNamed* firstResult = testObj.histogram();
  TNamed* secondResult = testObj.histogram();

  BOOST_TEST( firstResult == &histogram );
  BOOST_TEST( secondResult == &histogram );
}


BOOST_AUTO_TEST_CASE(test_shouldThrowExceptionWhen_kLBNHistoryDepth_isNonPositive) {
  HistogramDef histogramDef;
  histogramDef.kLBNHistoryDepth = -1;
  LumiblockHistogramProvider testObj(m_gmTool.get(), m_histogramFactory, histogramDef);
  BOOST_CHECK_THROW( testObj.histogram(), HistogramException );
}


BOOST_AUTO_TEST_CASE(test_shouldNotThrowExceptionWhen_kLBNHistoryDepth_isDefinedAsNumber) {
  HistogramDef histogramDef;
  histogramDef.kLBNHistoryDepth = 12345;
  LumiblockHistogramProvider testObj(m_gmTool.get(), m_histogramFactory, histogramDef);
}


BOOST_AUTO_TEST_CASE(test_shouldCreateNewHistogramWithUpdatedAlias) {
  auto expectedFlow = {
    make_tuple(0, "alias_LB0_2"),
    make_tuple(1, "alias_LB0_2"),
    make_tuple(2, "alias_LB0_2"),
    make_tuple(3, "alias_LB3_5"),
    make_tuple(4, "alias_LB3_5"),
    make_tuple(5, "alias_LB3_5"),
    make_tuple(6, "alias_LB6_8"),
    make_tuple(7, "alias_LB6_8"),
    make_tuple(8, "alias_LB6_8"),
    make_tuple(9, "alias_LB9_11"),
  };

  TH1F histogram("h", "h", 1, 0, 1);
  HistogramDef histogramDef;
  histogramDef.alias = "alias";
  histogramDef.path = "HistogramProviderTestSuite";
  histogramDef.kLBNHistoryDepth = 3;

  LumiblockHistogramProvider testObj(m_gmTool.get(), m_histogramFactory, histogramDef);

  for (auto input : expectedFlow) {
    const unsigned lumiBlock = get<0>(input);
    const std::string expectedAlias = get<1>(input);

    m_gmTool->mock_lumiBlock = [lumiBlock]() { return lumiBlock; };
    m_histogramFactory->mock_create = [&](const HistogramDef& def) mutable {
      BOOST_TEST( def.alias == expectedAlias );
      std::cout << "Registering: " << def.alias << std::endl;
      m_gmTool->histogramService()->regHist(m_histogramFactory->getFullName(def),
                                            static_cast<TH1F*>(histogram.Clone())).ignore();
      return &histogram;
    };
    m_histogramFactory->mock_remove = [&](const Monitored::HistogramDef& def) {
      std::cout << "Deregistering: " << def.alias << std::endl;
      m_gmTool->histogramService()->deReg(m_histogramFactory->getFullName(def)).ignore();
      return nullptr;
    };

    TNamed* const result = testObj.histogram();
    BOOST_TEST( result == &histogram );
  }
  // We keep histograms active for the last 5 LBs. That means on LB 9 we have
  // histograms covering LBs 5-9 registered, i.e. the last 3 from expectedFlow.
  BOOST_TEST( m_gmTool->histogramService()->getHists().size() == 3 );
}


BOOST_AUTO_TEST_CASE(test_shouldCreateNewHistogramWithUpdatedLumiBlock) {
  auto expectedFlow = {
    make_tuple(100, 100000, "/run_100000/lowStat_LB81-100/"),
    make_tuple(125, 200000, "/run_200000/lowStat_LB121-140/"),
  };

  TNamed histogram;
  HistogramDef histogramDef;
  histogramDef.convention = "OFFLINE:lowStat";
  histogramDef.runmode = HistogramDef::RunMode::Offline;
  histogramDef.runperiod = HistogramDef::RunPeriod::LowStat;

  OfflineHistogramProvider testObj(m_gmTool.get(), m_histogramFactory, histogramDef);

  for (auto input : expectedFlow) {
    const unsigned lumiBlock = get<0>(input);
    const unsigned runNumber = get<1>(input);
    const std::string expectedTld = get<2>(input);

    m_gmTool->mock_lumiBlock = [lumiBlock]() { return lumiBlock; };
    m_gmTool->mock_runNumber = [runNumber]() { return runNumber; };
    m_histogramFactory->mock_create = [&histogram, &expectedTld](const HistogramDef& def) mutable {
      BOOST_TEST( def.tld == expectedTld );
      return &histogram;
    };

    TNamed* const result = testObj.histogram();
    BOOST_TEST( result == &histogram );
  }
}


BOOST_AUTO_TEST_CASE(test_shouldCreateNewHistogramWithLatestLumiBlocks) {
  auto expectedFlow = {
    // tuple elements are: (LB, xbins, xmin, xmax)
    make_tuple(1, 10, 0.5, 10.5),
    make_tuple(2, 10, 0.5, 10.5),
    make_tuple(10, 10, 0.5, 10.5),
    make_tuple(11, 10, 1.5, 11.5),
  };

  TH1F histogram("histogram", "", 10, 0.5, 10.5);

  HistogramDef histogramDef;
  histogramDef.type = "TH1F";
  histogramDef.xbins = 10;
  histogramDef.xmin = 0.5;
  histogramDef.xmax = 10.5;
  histogramDef.kLive = 10;

  LiveHistogramProvider provider(m_gmTool.get(), m_histogramFactory, histogramDef);

  for (auto input : expectedFlow) {
    const unsigned lumiBlock = get<0>(input);
    const float expected_xbins = get<1>(input);
    const float expected_xmin = get<2>(input);
    const float expected_xmax = get<3>(input);

    m_gmTool->mock_lumiBlock = [lumiBlock]() { return lumiBlock; };

    m_histogramFactory->mock_create = [&](const HistogramDef& def) mutable {
      BOOST_TEST( def.xbins == expected_xbins );
      BOOST_TEST( def.xmin == expected_xmin );
      BOOST_TEST( def.xmax == expected_xmax );
      return &histogram;
    };

    TNamed* result = provider.histogram();
    BOOST_TEST( result == &histogram );
  }
}


BOOST_AUTO_TEST_CASE(test_shouldCreateNewEfficiencyWithLatestLumiBlocks) {
  auto expectedFlow = {
    // tuple elements are: (LB, xbins, xmin, xmax)
    make_tuple(1, 10, 0.5, 10.5),
    make_tuple(2, 10, 0.5, 10.5),
    make_tuple(10, 10, 0.5, 10.5),
    make_tuple(11, 10, 1.5, 11.5),
  };

  TEfficiency efficiency("efficiency", "", 10, 0.5, 10.5);

  HistogramDef histogramDef;
  histogramDef.type = "TEfficiency";
  histogramDef.xbins = 10;
  histogramDef.xmin = 0.5;
  histogramDef.xmax = 10.5;
  histogramDef.kLive = 10;

  LiveHistogramProvider provider(m_gmTool.get(), m_histogramFactory, histogramDef);

  for (auto input : expectedFlow) {
    const unsigned lumiBlock = get<0>(input);
    const float expected_xbins = get<1>(input);
    const float expected_xmin = get<2>(input);
    const float expected_xmax = get<3>(input);

    m_gmTool->mock_lumiBlock = [lumiBlock]() { return lumiBlock; };

    m_histogramFactory->mock_create = [&](const HistogramDef& def) mutable {
      BOOST_TEST( def.xbins == expected_xbins );
      BOOST_TEST( def.xmin == expected_xmin );
      BOOST_TEST( def.xmax == expected_xmax );
      return &efficiency;
    };

    TNamed* result = provider.histogram();
    BOOST_TEST( result == &efficiency );
  }
}

BOOST_AUTO_TEST_SUITE_END()
