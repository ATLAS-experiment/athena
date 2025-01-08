/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE HistogramFillerRebinable1D
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <memory>

#include "TestTools/initGaudi.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

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
    m_histogram.reset(new TH1D("MockHistogram", "Mock Histogram", 8, 1.0, 3.0));
    m_testObj.reset(new HistogramFillerRebinable1D(m_histogramDef, m_histogramProvider));

    m_histogramProvider->mock_histogram = [this]() { return m_histogram.get(); };
  }

protected:
  HistogramDef m_histogramDef;
  std::shared_ptr<MockHistogramProvider> m_histogramProvider;
  std::shared_ptr<TH1D> m_histogram;
  std::shared_ptr<HistogramFillerRebinable1D> m_testObj;
};


// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE( HistogramFillerRebinable1D,
                          TestFixture,
                          * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("GenericMonMinimal.txt")) )

BOOST_AUTO_TEST_CASE( test_shouldKeepNumberOfBinsForValueInHistogramsRange ) {
  Monitored::Scalar<double> var("var", 2.9);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );
  BOOST_TEST( m_histogram->GetBinContent(8) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldDoubleNumberOfBinsForBoundaryValueOf3 ) {
  Monitored::Scalar<double> var("var", 3.0);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 16 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );
  BOOST_TEST( m_histogram->GetBinContent(9) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldDoubleNumberOfBinsForValueSlightlySmallerThan5 ) {
  Monitored::Scalar<double> var("var", 4.9);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 16 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );
  BOOST_TEST( m_histogram->GetBinContent(16) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldQuadrupleNumberOfBinsForBoundaryValueOf5 ) {
  Monitored::Scalar<double> var("var", 5.0);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 32 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 9.0 );
  BOOST_TEST( m_histogram->GetBinContent(17) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldQuadrupleNumberOfBinsForValueSlightlyBiggerThan5 ) {
  Monitored::Scalar<double> var("var", 5.1);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 32 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 9.0 );
  BOOST_TEST( m_histogram->GetBinContent(17) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldQuadrupleNumberOfBinsForValueSlightlySmallerThan9 ) {
  Monitored::Scalar<double> var("var", 8.9);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 32 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 9.0 );
  BOOST_TEST( m_histogram->GetBinContent(32) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldOctupleNumberOfBinsForBoundaryValueOf9 ) {
  Monitored::Scalar<double> var("var", 9.0);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 64 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 17.0 );
  BOOST_TEST( m_histogram->GetBinContent(33) == 1.0 );
}

BOOST_AUTO_TEST_CASE( test_shouldOctupleNumberOfBinsForValueSlightlyBiggerThan9 ) {
  Monitored::Scalar<double> var("var", 9.1);
  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 8 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 3.0 );

  m_testObj->fill(vars);

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 64 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 1.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 17.0 );
  BOOST_TEST( m_histogram->GetBinContent(33) == 1.0 );
}

BOOST_AUTO_TEST_SUITE_END()
