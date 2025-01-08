/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE HistogramFillerFactory
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <memory>

#include "TestTools/initGaudi.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

#include "AthenaMonitoringKernel/HistogramDef.h"

#include "../src/HistogramFiller/StaticHistogramProvider.h"
#include "../src/HistogramFiller/LumiblockHistogramProvider.h"
#include "../src/HistogramFiller/HistogramFiller1D.h"
#include "../src/HistogramFiller/HistogramFillerEfficiency.h"
#include "../src/HistogramFiller/CumulativeHistogramFiller1D.h"
#include "../src/HistogramFiller/VecHistogramFiller1D.h"
#include "../src/HistogramFiller/HistogramFillerRebinable.h"
#include "../src/HistogramFiller/HistogramFillerProfile.h"
#include "../src/HistogramFiller/HistogramFiller2D.h"
#include "../src/HistogramFiller/HistogramFiller2DProfile.h"
#include "../src/HistogramFiller/HistogramFillerFactory.h"

using namespace Monitored;


class HistogramProviderGetter : public HistogramFiller {
  public: 
    HistogramProviderGetter(const HistogramFiller& hf) 
      : HistogramFiller(hf) {}

  virtual unsigned fill( const HistogramFiller::VariablesPack& ) const { return 0; }
    virtual HistogramFiller* clone() const { return nullptr; }

    std::shared_ptr<IHistogramProvider> histogramProvider() { return m_histogramProvider; }
};


/// Test fixture (run before each test)
class TestFixture {
public:
  TestFixture() :
    m_gmTool("GenericMonitoringTool/MonTool")
  {
    BOOST_TEST( m_gmTool.retrieve() );
    m_histDef.reset(new Monitored::HistogramDef());
    m_histDef->alias = "Hist" + std::to_string(++m_histCount);
    m_histDef->xbins = 1;
    m_histDef->ybins = 1;
  }

  template<class FillerType, class ProviderType>
  void performCreateFillerAndVerify() {
    HistogramFillerFactory testObj(m_gmTool.get(), "HistogramFillerFactoryTestSuite");
    HistogramFiller* const result = testObj.create(*m_histDef);
    BOOST_TEST( dynamic_cast<FillerType*>(result) != nullptr );

    HistogramProviderGetter providerGetter(*result);
    IHistogramProvider* const provider = providerGetter.histogramProvider().get();
    BOOST_TEST( dynamic_cast<ProviderType*>(provider) != nullptr );
  }

protected:
  static inline std::atomic<int> m_histCount{0};
  ToolHandle<GenericMonitoringTool> m_gmTool;
  std::shared_ptr<HistogramDef> m_histDef;
};

// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE( HistogramFillerFactory,
                          TestFixture,
                          * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("GenericMonMinimal.txt")) )

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticHistogramFiller1D ) {
  m_histDef->type = "TH1F";
  performCreateFillerAndVerify<HistogramFiller1D, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticCumulativeHistogramFiller1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kCumulative = true;
  performCreateFillerAndVerify<CumulativeHistogramFiller1D, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticVecHistogramFiller1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kVec = true;
  performCreateFillerAndVerify<VecHistogramFiller1D, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticHistogramFillerRebinable1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kAddBinsDynamically = true;
  performCreateFillerAndVerify<HistogramFillerRebinable1D, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticHistogramFiller2D ) {
  m_histDef->type = "TH2D";
  performCreateFillerAndVerify<HistogramFiller2D, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticHistogramFillerProfile ) {
  m_histDef->type = "TProfile";
  performCreateFillerAndVerify<HistogramFillerProfile, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticHistogramFiller2DProfile ) {
  m_histDef->type = "TProfile2D";
  performCreateFillerAndVerify<HistogramFiller2DProfile, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateStaticHistogramFillerEfficiency ) {
  m_histDef->type = "TEfficiency";
  performCreateFillerAndVerify<HistogramFillerEfficiency, StaticHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockHistogramFiller1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<HistogramFiller1D, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockCumulativeHistogramFiller1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kCumulative = true;
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<CumulativeHistogramFiller1D, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockVecHistogramFiller1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kVec = true;
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<VecHistogramFiller1D, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockHistogramFillerRebinable1D ) {
  m_histDef->type = "TH1F";
  m_histDef->kAddBinsDynamically = true;
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<HistogramFillerRebinable1D, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockHistogramFiller2D ) {
  m_histDef->type = "TH2D";
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<HistogramFiller2D, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockHistogramFillerProfile ) {
  m_histDef->type = "TProfile";
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<HistogramFillerProfile, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockHistogramFiller2DProfile ) {
  m_histDef->type = "TProfile2D";
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<HistogramFiller2DProfile, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_CASE( test_shouldCreateLumiblockHistogramFillerEfficiency ) {
  m_histDef->type = "TEfficiency";
  m_histDef->kLBNHistoryDepth = 10;
  performCreateFillerAndVerify<HistogramFillerEfficiency, LumiblockHistogramProvider>();
}

BOOST_AUTO_TEST_SUITE_END()
