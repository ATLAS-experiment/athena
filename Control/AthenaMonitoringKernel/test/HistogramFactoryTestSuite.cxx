/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE HistogramProviderTestSuite
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <memory>

#include "TestTools/initGaudi.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

#include "AthenaMonitoringKernel/HistogramDef.h"
#include "../src/HistogramFiller/HistogramFactory.h"

using namespace Monitored;


/// Test fixture (run before each test)
class TestFixture {
public:
  TestFixture() :
    m_histSvc("THistSvc", "HistogramFactoryTestSuite")
  {
    BOOST_TEST( m_histSvc.retrieve() );
    m_testObj.reset(new HistogramFactory(m_histSvc, "/HistogramFactoryTestSuite"));
  }

  ~TestFixture() {
    clearHistogramService();
  }

  HistogramDef defaultHistogramDef(const std::string& histogramType) {
    HistogramDef result;

    result.path = "DEFAULT";
    result.type = histogramType;
    result.alias = histogramType;
    result.title = histogramType;
    result.xbins = 1;
    result.ybins = 1;
    result.zbins = 0;

    return result;
  }

  template <class HistogramType>
  HistogramType* createHistogram(const std::string& histogramType) {
    HistogramDef histogramDef = defaultHistogramDef(histogramType);
    return static_cast<HistogramType*>(m_testObj->create(histogramDef));
  }

  TEfficiency* createEfficiency() {
    HistogramDef histogramDef = defaultHistogramDef("TEfficiency");
    histogramDef.ybins = 0;
    histogramDef.zbins = 0;
    return static_cast<TEfficiency*>(m_testObj->create(histogramDef));
  }

  void clearHistogramService() {
    for (const std::string& name : m_histSvc->getHists()) {
      BOOST_TEST( m_histSvc->deReg(name) );
    }

    for (const std::string& name : m_histSvc->getTrees()) {
      BOOST_TEST( m_histSvc->deReg(name) );
    }

    for (const std::string& name : m_histSvc->getGraphs()) {
      BOOST_TEST( m_histSvc->deReg(name) );
    }

    for (const std::string& name : m_histSvc->getEfficiencies()) {
      BOOST_TEST( m_histSvc->deReg(name) );
    }
  }

protected:
  ServiceHandle<ITHistSvc> m_histSvc;
  std::shared_ptr<HistogramFactory> m_testObj;
};


// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE( HistogramProviderTestSuite,
                          TestFixture,
                          * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("GenericMonMinimal.txt")) )

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTH1FHistogram ) {
  TH1F* const histogram = createHistogram<TH1F>("TH1F");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TH1F") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTH1DHistogram ) {
  TH1D* const histogram = createHistogram<TH1D>("TH1D");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TH1D") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTH1IHistogram ) {
  TH1I* const histogram = createHistogram<TH1I>("TH1I");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TH1I") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTH2FHistogram ) {
  TH2F* const histogram = createHistogram<TH2F>("TH2F");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TH2F") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTH2DHistogram ) {
  TH2D* const histogram = createHistogram<TH2D>("TH2D");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TH2D") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTH2IHistogram ) {
  TH2I* const histogram = createHistogram<TH2I>("TH2I");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TH2I") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTProfileHistogram ) {
  TProfile* const histogram = createHistogram<TProfile>("TProfile");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TProfile") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTProfile2DHistogram ) {
  TProfile2D* const histogram = createHistogram<TProfile2D>("TProfile2D");
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/TProfile2D") );
  BOOST_TEST( histogram );
}

BOOST_AUTO_TEST_CASE( test_shouldRegisterAndReturnTEfficiencyHistogram ) {
  TEfficiency* const efficiency = createEfficiency();
  BOOST_TEST( m_histSvc->existsEfficiency("/HistogramFactoryTestSuite/TEfficiency") );
  BOOST_TEST( efficiency );
}

BOOST_AUTO_TEST_CASE( test_shouldThrowExceptionForUnknownHistogramType ) {
  BOOST_CHECK_THROW( createHistogram<TH1F>("UnknownType"), HistogramException );
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/UnknownType") == false );
}

BOOST_AUTO_TEST_CASE( test_shouldProperlyFormatPathForOnlineHistograms ) {
  const std::vector<std::pair<std::string, std::string>> possibleCases = {
    {"EXPERT", "/EXPERT/HistogramFactoryTestSuite/onlineHistAlias"},
    {"SHIFT", "/SHIFT/HistogramFactoryTestSuite/onlineHistAlias"},
    {"DEBUG", "/DEBUG/HistogramFactoryTestSuite/onlineHistAlias"},
    {"RUNSTAT", "/RUNSTAT/HistogramFactoryTestSuite/onlineHistAlias"},
    {"EXPRESS", "/EXPRESS/HistogramFactoryTestSuite/onlineHistAlias"}
  };
  
  for (const auto& [onlinePath, expectedPath] : possibleCases) {
    HistogramDef histogramDef = defaultHistogramDef("TH1F");
    histogramDef.path = onlinePath;
    histogramDef.alias = "onlineHistAlias";
    m_testObj->create(histogramDef);
    BOOST_TEST( m_histSvc->exists(expectedPath) );
  }
}

BOOST_AUTO_TEST_CASE( test_shouldProperlyFormatPathForDefaultHistograms ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.path = "DEFAULT";
  histogramDef.alias = "/defaultAlias";
  m_testObj->create(histogramDef);
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/defaultAlias") );
}

BOOST_AUTO_TEST_CASE( test_shouldProperlyFormatPathForCustomHistograms ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.path = "/custom/path/for/histogram";
  histogramDef.alias = "customAlias";
  m_testObj->create(histogramDef);
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/custom/path/for/histogram/customAlias") );
}

BOOST_AUTO_TEST_CASE( test_shouldProperlyFormatPathForOfflineHistograms ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.path = "/custom/path/for/histogram";
  histogramDef.alias = "offlineAlias";
  histogramDef.tld = "/run_XXXXXX/lbYYY/";
  m_testObj->create(histogramDef);
  BOOST_TEST( m_histSvc->exists("/HistogramFactoryTestSuite/run_XXXXXX/lbYYY/custom/path/for/histogram/offlineAlias") );
}

BOOST_AUTO_TEST_CASE( test_shouldProperlyFormatPathForTempOfflineHistograms ) {
  m_testObj.reset(new HistogramFactory(m_histSvc, "HistogramFactoryTestSuite"));
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.path = "/custom/path/for/histogram";
  histogramDef.alias = "offlineAlias";
  histogramDef.tld = "/run_XXXXXX/lbYYY/";
  m_testObj->create(histogramDef);
  BOOST_TEST( m_histSvc->exists("HistogramFactoryTestSuite/run_XXXXXX/lbYYY/custom/path/for/histogram/offlineAlias") );
}

BOOST_AUTO_TEST_CASE( test_shouldSetXAxisLabelsFor1DHistogram ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.alias = "labels1DTestAlias";
  histogramDef.xbins = 3;
  histogramDef.ybins = 0;
  histogramDef.xlabels = { "xlabel1", "xlabel2", "xlabel3" };
  histogramDef.ylabels = { "ylabel1" };
  TH1F* const histogram = static_cast<TH1F*>(m_testObj->create(histogramDef));
 
  BOOST_TEST( histogram->GetXaxis()->GetNbins() == 3);
  BOOST_TEST( std::string(histogram->GetXaxis()->GetBinLabel(1)) == "xlabel1");
  BOOST_TEST( std::string(histogram->GetXaxis()->GetBinLabel(2)) == "xlabel2");
  BOOST_TEST( std::string(histogram->GetXaxis()->GetBinLabel(3)) == "xlabel3");
  BOOST_TEST( histogram->GetYaxis()->GetNbins() == 1);
  BOOST_TEST( std::string(histogram->GetYaxis()->GetBinLabel(1)) == "ylabel1");
}

BOOST_AUTO_TEST_CASE( test_shouldSetXAndYAxisLabelsFor2DHistogram ) {
  HistogramDef histogramDef = defaultHistogramDef("TH2F");
  histogramDef.alias = "labels2DTestAlias";
  histogramDef.xbins = 3;
  histogramDef.ybins = 3;
  histogramDef.xlabels = { "xlabel1", "xlabel2", "xlabel3" };
  histogramDef.ylabels = { "ylabel1", "ylabel2", "ylabel3" };
  TH2F* const histogram = static_cast<TH2F*>(m_testObj->create(histogramDef));
 
  BOOST_TEST( histogram->GetXaxis()->GetNbins() == 3);
  BOOST_TEST( std::string(histogram->GetXaxis()->GetBinLabel(1)) == "xlabel1" );
  BOOST_TEST( std::string(histogram->GetXaxis()->GetBinLabel(2)) == "xlabel2" );
  BOOST_TEST( std::string(histogram->GetXaxis()->GetBinLabel(3)) == "xlabel3" );
  BOOST_TEST( histogram->GetYaxis()->GetNbins() == 3);
  BOOST_TEST( std::string(histogram->GetYaxis()->GetBinLabel(1)) == "ylabel1" );
  BOOST_TEST( std::string(histogram->GetYaxis()->GetBinLabel(2)) == "ylabel2" );
  BOOST_TEST( std::string(histogram->GetYaxis()->GetBinLabel(3)) == "ylabel3" );
}

BOOST_AUTO_TEST_CASE( test_shouldSetExtendAxesWhenkCanRebinIsSet ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.alias = "allAxesRebinAlias";
  histogramDef.kCanRebin = true;
  TH1F* const histogram = static_cast<TH1F*>(m_testObj->create(histogramDef));
 
  BOOST_TEST( histogram->CanExtendAllAxes() );
}

BOOST_AUTO_TEST_CASE( test_shouldNotSetExtendAxesWhenkCanRebinIsNotSet ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.alias = "noAxesRebinAlias";
  histogramDef.kCanRebin = false;
  TH1F* const histogram = static_cast<TH1F*>(m_testObj->create(histogramDef));
 
  BOOST_TEST( histogram->CanExtendAllAxes() == false );
}

BOOST_AUTO_TEST_CASE( test_shouldSetSumw2WhenSumw2IsSet ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.alias = "Sumw2ActiveAlias";
  histogramDef.Sumw2 = true;
  TH1F* const histogram = static_cast<TH1F*>(m_testObj->create(histogramDef));
 
  BOOST_TEST( histogram->GetSumw2N() == 3 );
}

BOOST_AUTO_TEST_CASE( test_shouldNotSetSumw2WhenSumw2IsNotSet ) {
  HistogramDef histogramDef = defaultHistogramDef("TH1F");
  histogramDef.alias = "Sumw2InactiveAlias";
  histogramDef.Sumw2 = false;
  TH1F* const histogram = static_cast<TH1F*>(m_testObj->create(histogramDef));
 
  BOOST_TEST( histogram->GetSumw2N() == 0 );
}


BOOST_AUTO_TEST_SUITE_END()
