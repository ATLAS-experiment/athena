/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE HistogramFillerVec1DTestSuite
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <memory>

#include "TH1D.h"

#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "TestTools/initGaudi.h"

#include "../src/HistogramFiller/VecHistogramFiller1D.h"
#include "mocks/MockHistogramProvider.h"

using namespace Monitored;


/// Test fixture (run before each test)
struct TestFixture {
  TestFixture(bool kVecUO = false) {
    m_histogramDef.type = "TH1F";
    m_histogramDef.xbins = 1;
    if (kVecUO) {
      m_histogramDef.kVecUO = true;
    } else {
      m_histogramDef.kVec = true;
    }
    m_histogramProvider.reset(new MockHistogramProvider());
    m_histogram.reset(new TH1D("MockHistogram", "Mock Histogram", 5, 0.0, 5.0));
    m_testObj.reset(new VecHistogramFiller1D(m_histogramDef, m_histogramProvider));
    m_histogramProvider->mock_histogram = [this]() { return m_histogram.get(); };
  }

  HistogramDef m_histogramDef;
  std::shared_ptr<MockHistogramProvider> m_histogramProvider;
  std::shared_ptr<TH1D> m_histogram;
  std::shared_ptr<VecHistogramFiller1D> m_testObj;
};

struct TestFixtureVecUO : TestFixture {
  TestFixtureVecUO() : TestFixture(true) {}
};


// Create test suite with global fixture
BOOST_AUTO_TEST_SUITE( HistogramFillerVec1DTestSuite,
                       * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("GenericMonMinimal.txt")) )


BOOST_FIXTURE_TEST_CASE( test_fillWithVector, TestFixture ) {

  using Coll = std::vector<double>;
  Coll values({1., 2., 3., 4., 5.});
  auto var = Monitored::Collection("values", values);

  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 5 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 0.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );

  m_testObj->fill(vars);

  for (unsigned i: {0, 6}) {
    BOOST_TEST( m_histogram->GetBinContent(i) == 0.0 );
  }
  for (unsigned i = 0; i != values.size(); ++ i) {
    BOOST_TEST( m_histogram->GetBinContent(i+1) == values[i] );
  }
}

BOOST_FIXTURE_TEST_CASE( test_fillWithShortVector, TestFixture ) {

  using Coll = std::vector<double>;
  Coll values({1., 2., 3.});
  auto var = Monitored::Collection("values", values);

  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 5 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 0.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );

  unsigned entriesAdded = m_testObj->fill(vars);
  BOOST_TEST( entriesAdded == 0 );

  for (unsigned i = 0; i != values.size(); ++ i) {
    BOOST_TEST( m_histogram->GetBinContent(i+1) == 0 );
  }
}

BOOST_FIXTURE_TEST_CASE( test_fillWithVectorUO, TestFixtureVecUO ) {

  using Coll = std::vector<double>;
  Coll values({1., 2., 3., 4., 5., 6., 7.});
  auto var = Monitored::Collection("values", values);

  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 5 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 0.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );

  m_testObj->fill(vars);

  for (unsigned i = 0; i != values.size(); ++ i) {
    BOOST_TEST( m_histogram->GetBinContent(i) == values[i] );
  }
}

BOOST_FIXTURE_TEST_CASE( test_fillWithShortVectorUO, TestFixtureVecUO ) {

  using Coll = std::vector<double>;
  Coll values({1., 1., 2., 3., 4., 5., 12.});
  auto var = Monitored::Collection("values", values);

  HistogramFiller::VariablesPack vars({&var});

  BOOST_TEST( m_histogram->GetXaxis()->GetNbins() == 5 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmin() == 0.0 );
  BOOST_TEST( m_histogram->GetXaxis()->GetXmax() == 5.0 );

  m_testObj->fill(vars);
  // underflow
  BOOST_TEST( m_histogram->GetBinContent(0) == 1.0 );
  // overflow
  BOOST_TEST( m_histogram->GetBinContent(7) == 12.0 );

  for (unsigned i = 0; i != values.size(); ++ i) {
    BOOST_TEST( m_histogram->GetBinContent(i) == values[i] );
  }
}

BOOST_AUTO_TEST_SUITE_END()
