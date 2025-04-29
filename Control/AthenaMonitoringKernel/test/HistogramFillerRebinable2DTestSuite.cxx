/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Unit test for 2D rebinable histograms
 *
 * Only specific features to 2D histograms (rebinning of Y-axis) is tested here.
 * The main functionality of the rebinning is tested in the "1DTestSuite".
 */

#define BOOST_TEST_MODULE HistogramFillerRebinable2D
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "TestTools/initGaudi.h"

#include "TH2D.h"
#include "AthenaMonitoringKernel/HistogramFiller.h"
#include "AthenaMonitoringKernel/MonitoredScalar.h"
#include "../src/HistogramFiller/HistogramFillerRebinable.h"
#include "mocks/MockHistogramProvider.h"

using namespace Monitored;

/// Test fixture (run before each test)
class TestFixture {
public:
  TestFixture() {
    m_histogramDef.type = "TH1F";
    m_histogramDef.xbins = 1;
    m_histogramDef.kAddBinsDynamically = true;
    m_histogramProvider.reset(new MockHistogramProvider());
    m_histogram.reset(new TH2D("MockHistogram", "Mock Histogram", 8, 1.0, 3.0, 5, 1.0, 5.0));
    m_testObj.reset(new HistogramFillerRebinable2D(m_histogramDef, m_histogramProvider));

    m_histogramProvider->mock_histogram = [this]() { return m_histogram.get(); };
  }

protected:
  HistogramDef m_histogramDef;
  std::shared_ptr<MockHistogramProvider> m_histogramProvider;
  std::shared_ptr<TH2D> m_histogram;
  std::shared_ptr<HistogramFillerRebinable2D> m_testObj;
};


// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE( HistogramFillerRebinable2D,
                          TestFixture,
                          * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("GenericMonMinimal.txt")) )

BOOST_AUTO_TEST_CASE( test_shouldKeepNumberOfBinsForValueInHistogramsRange ) {
  Monitored::Scalar<double> var1("var1", 2.9);
  Monitored::Scalar<double> var2("var2", 4.9);
  HistogramFiller::VariablesPack vars({&var1, &var2});

  auto check = [&](){
    BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
    BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
    BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );
    BOOST_TEST( m_histogram->GetYaxis()->GetNbins() == 5 );
    BOOST_TEST( m_histogram->GetYaxis()->GetXmin() == 1.0 );
    BOOST_TEST( m_histogram->GetYaxis()->GetXmax() == 5.0 );
  };
  check();
  m_testObj->fill(vars);
  check();
  BOOST_TEST( m_histogram->GetBinContent(8,5) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldDoubleNumberOfBinsForValueOutsideRange ) {
  Monitored::Scalar<double> var1("var1", 3.0);
  Monitored::Scalar<double> var2("var2", 5.0);
  HistogramFiller::VariablesPack vars({&var1, &var2});

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 16 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );
  BOOST_TEST( m_histogram->GetYaxis()->GetNbins() == 10 );
  BOOST_TEST( m_histogram->GetYaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetYaxis()->GetXmax() == 9.0 );
  BOOST_TEST( m_histogram->GetBinContent(9,6) == 1.0 );
}


BOOST_AUTO_TEST_SUITE_END()
