/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Unit tests and documentation examples
 *
 * Note that the snippets marked with:
 * //! [Example1]
 * ...
 * //! [Example1]
 * are used in the Doxygen documentation of the various classes.
 * If you change them make sure to check the resulting Doxygen.
 */

#define BOOST_TEST_MODULE GenericMonFilling
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <thread>

#include "TestTools/initGaudi.h"
#include "GaudiKernel/ITHistSvc.h"
#include "CxxUtils/ubsan_suppress.h"

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "TTree.h"
#include "THashList.h"
#include "TInterpreter.h"


/// Test fixture (run before each test)
struct TestFixture {
  TestFixture() :
    histSvc("THistSvc", "GenericMonFilling"),
    monTool("GenericMonitoringTool/MonTool")
  {
    BOOST_TEST( histSvc.retrieve() );
    BOOST_TEST( monTool.retrieve() );
  }

  ServiceHandle<ITHistSvc> histSvc;
  ToolHandle<GenericMonitoringTool> monTool;
};

struct GaudiFixture : Athena_test::InitGaudi {
  GaudiFixture() :
    Athena_test::InitGaudi("GenericMon.txt")
  {
    // Need to start the services in order to register histograms
    BOOST_TEST( svcLoc.as<ISvcManager>()->start() );
  }

  ~GaudiFixture() {
    svcLoc.as<ISvcManager>()->stop().ignore();
  }
};


// Helpers
const TH1* getHist( ServiceHandle<ITHistSvc>& histSvc, const std::string& histName ) {
  TH1* h( nullptr );
  BOOST_TEST( histSvc->getHist( histName, h ) );
  return h;
}

TTree* getTree( ServiceHandle<ITHistSvc>& histSvc, const std::string& treeName ) {
  TTree* t( nullptr );
  BOOST_TEST( histSvc->getTree( treeName, t ) );
  return t;
}

void resetHist( ServiceHandle<ITHistSvc>& histSvc, const std::string& histName ) {
  TH1* h ATLAS_THREAD_SAFE = const_cast<TH1*>(getHist( histSvc, histName ));
  h->Reset();
  THashList* labels = h->GetXaxis()->GetLabels();
  if (labels) labels->Clear();
  labels = h->GetYaxis()->GetLabels();
  if (labels) labels->Clear();
}

void resetHists( ServiceHandle<ITHistSvc>& histSvc ) {
  for (const std::string& name : histSvc->getHists()) {
    resetHist( histSvc, name );
  }
  for (const std::string& name : histSvc->getTrees()) {
    getTree( histSvc, name )->Reset();
  }
}

double contentInBin1DHist( ServiceHandle<ITHistSvc>& histSvc, const std::string& histName, int bin ) {
  const TH1* h = getHist( histSvc, histName );
  // this are in fact securing basic correctness of the tests
  BOOST_TEST( h != nullptr );
  BOOST_TEST( bin >= 1 );
  BOOST_TEST( bin <= h->GetXaxis()->GetNbins()+1 );
  return h->GetBinContent( bin );
}

double contentInBin2DHist( ServiceHandle<ITHistSvc>& histSvc, const std::string& histName, int bin1, int bin2 ) {
  TH2* h( nullptr );
  BOOST_TEST( histSvc->getHist( histName, h ) );
  // this are in fact securing basic correctness of the tests
  BOOST_TEST( h != nullptr );
  BOOST_TEST( bin1 >= 1 );
  BOOST_TEST( bin1 <= h->GetXaxis()->GetNbins()+1 );
  BOOST_TEST( bin2 >= 1 );
  BOOST_TEST( bin2 <= h->GetYaxis()->GetNbins()+1 );
  return h->GetBinContent( bin1, bin2 );
}


/**
 * Launch `nthreads` each calling `func` `nfills` times.
 * @return Total number of fills performed
 */
template <typename F>
size_t fill_mt(const F& func)
{
  const size_t nthreads = 10;
  const size_t nfills = 1000;

  // Create threads
  std::vector<std::thread> threads;
  std::vector<size_t> fills(nthreads, 0);
  threads.reserve(nthreads);
  for (size_t i = 0; i < nthreads; ++i) {
    threads.push_back(std::thread([&, i]() {
      for (size_t j = 0; j < nfills; ++j) fills[i] += func();
    }));
  }
  // Launch and wait
  for (auto& t : threads) t.join();

  // Return total number of fills performed
  return std::accumulate(fills.begin(), fills.end(), 0);
}


// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE( GenericMonFilling,
                          TestFixture,
                          * boost::unit_test::fixture<GaudiFixture>()
                          * boost::unit_test::tolerance(1.e-6) )


BOOST_AUTO_TEST_CASE( emptyMonTool ) {
  // we need to test what happens to the monitoring when tool is not valid
  ToolHandle<GenericMonitoringTool> emptyMon("");
  BOOST_TEST( !emptyMon.isEnabled() );

  auto x = Monitored::Scalar( "x", -99.0 );
  auto group = Monitored::Group( emptyMon, x );
}


BOOST_AUTO_TEST_CASE( fillFromScalar ) {

  auto fill = [&]() {
    //! [fillFromScalar]
    auto eta = Monitored::Scalar<double>( "Eta" ); //explicit double
    auto phi = Monitored::Scalar( "Phi", -99.0 );  //deduced double
    auto group = Monitored::Group( monTool, eta, phi );
    phi = 0.1;
    eta = -0.2;
    //! [fillFromScalar]
    return 1;
  };

  auto check = [&](size_t N) {
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 1 ) == 0 );
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == N );

    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 1 ) == N );
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 0 );

    auto tree = getTree( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta_Tree" );
    BOOST_TEST( tree->GetEntries() == N );
    std::vector<float> tmpvec;
    Float_t tmp;
    tree->GetBranch("Phi")->SetObject(&tmpvec);
    tree->GetBranch("Eta")->SetAddress(&tmp);
    for (int i=0; i<tree->GetEntries(); ++i) {
      tree->GetEntry(i);
      BOOST_TEST( tmp == -0.2 );
      BOOST_TEST( (const float&) tmpvec.at(0) == 0.1 );
    }
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));
}


BOOST_AUTO_TEST_CASE( fillFromScalarTrf ) {

  auto fill = [&]() {
    //! [fillFromScalarTrf]
    // Take absolute value before filling
    auto phi = Monitored::Scalar<double>( "Phi", -3.0, [](double d){ return fabs(d); } );
    //! [fillFromScalarTrf]
    auto group = Monitored::Group( monTool, phi );
    return 1;
  };
  auto check = [&](size_t N) {
    BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi" )->GetEntries() == N);
    BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi" )->GetMean() == 3.0);
  };

  resetHists( histSvc ); check(fill());
}


BOOST_AUTO_TEST_CASE( fillFromScalarIndependentScopes ) {
  resetHists( histSvc );
  //! [fillFromScalarIndependentScopes]
  // The variable are declared in an outer scope
  auto phi = Monitored::Scalar( "Phi", -99.0 );
  auto eta = Monitored::Scalar<double>( "Eta", -99 );

  for ( size_t i =0; i < 3; ++i ) {
    // Inside another scope the Group is instantiated and destroyed resulting in multiple histogram fills.
    auto group = Monitored::Group( monTool, phi );
    phi = 0.1;
    eta = -0.2;
  }
  //! [fillFromScalarIndependentScopes]

  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 1 ) == 0 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == 3 );

  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 1 ) == 0 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 0 );

  for ( size_t i =0; i < 10; ++i ) {
    auto group = Monitored::Group( monTool, eta );
    phi = 0.1;
    eta = -0.2;
  }

  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 1 ) == 0 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == 3 );

  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 1 ) == 10 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 0 );
}


BOOST_AUTO_TEST_CASE( fill2D ) {

  auto fill = [&]() {
    //! [fill2D_correct]
    // For 2D histogram to be filled the two histogrammed variables need to be grouped.
    // This code will cause the 2D histogram fill
    auto phi = Monitored::Scalar( "Phi", -99.0 );
    auto eta = Monitored::Scalar( "Eta", -99.0 );
    
    auto group = Monitored::Group( monTool, eta, phi );
    eta = 0.2;
    phi = -0.1;
    //! [fill2D_correct]
    return 1;
  };
  auto check = [&](size_t N) {
    BOOST_TEST( contentInBin2DHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta", 1, 1 ) == 0 );
    BOOST_TEST( contentInBin2DHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta", 1, 2 ) == 0 );
    BOOST_TEST( contentInBin2DHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta", 2, 1 ) == N );
    BOOST_TEST( contentInBin2DHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta", 2, 2 ) == 0 );
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == N ); // counts also visible in 1 D
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 1 ) == N );
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));

  // 2D Hist fill should not affect 1D
  resetHists( histSvc );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 0 );
  {
    // This code will NOT cause a 2D histogram fill but instead
    // It will cause though 1D histogram fill if one is defined for this quantity.
    auto eta = Monitored::Scalar( "Eta", -99.0 );
    auto group = Monitored::Group( monTool, eta );
    eta = 0.2;
  }
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 0 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 1 );

  {
    auto phi = Monitored::Scalar( "Phi", -99.0 );
    auto group = Monitored::Group( monTool, phi );
    phi = -0.1;
  }
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 0 ); // still no entries as scope used above is not having both needed variables
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 1 ); // no increase of counts
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 1 ) == 1 );
}


BOOST_AUTO_TEST_CASE( fillProfile ) {

  auto fill = [&]() {
    auto pt = Monitored::Scalar<double>( "pt", 3.0 );
    auto eta = Monitored::Scalar( "Eta", 0.2 );
    auto group = Monitored::Group( monTool, eta, pt );
    return 1;
  };

  auto check = [&](size_t N) {
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/pt_vs_Eta", 1 ) == 0 );
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/pt_vs_Eta", 2 ) == 3 );
    BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/pt_vs_Eta" )->GetEntries() == N );
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));
}


BOOST_AUTO_TEST_CASE( fillExplicitly ) {
  resetHists( histSvc );
  //! [fillExplicitly_noop]
  auto phi = Monitored::Scalar( "Phi", -99.0 );
  auto eta = Monitored::Scalar( "Eta", -99.0 );
  {
    auto group = Monitored::Group( monTool, eta, phi );
    group.setAutoFill( false ); // results in no histogram fills
    eta = 0.2;
    phi = -0.1;
  }
  //! [fillExplicitly_noop]
  
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 0 ); //  auto filling was disabled so no entries
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Eta" )->GetEntries() == 0 ); //  auto filling was disabled so no entries
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi" )->GetEntries() == 0 ); //  auto filling was disabled so no entries

  // Check explicit fill in loops
  {
    //! [fillExplicitly_fill]
    auto group = Monitored::Group( monTool, eta, phi );
    for ( size_t i = 0; i < 3; ++i ) {
      group.fill();   // this will fill and disable AutoFill
    }
    //! [fillExplicitly_fill]
  }
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 3 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Eta" )->GetEntries() == 3 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi" )->GetEntries() == 3 );

  // Check explicit one-time fill via temporary Group instance
  {
    Monitored::Group( monTool, eta, phi ).fill();
  }
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 4 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Eta" )->GetEntries() == 4 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi" )->GetEntries() == 4 );
}


BOOST_AUTO_TEST_CASE( fillWithCutMask ) {

  auto fill1 = [&]() {
    //! [fillWithCutMask]
    for (int ctr = 0; ctr < 10; ++ctr) {
      auto eta = Monitored::Scalar<double>( "Eta", -99 );
      auto cutMask = Monitored::Scalar<bool>( "CutMask", (ctr % 2) == 0); // filter every second entry
      auto group = Monitored::Group( monTool, eta, cutMask );
      eta = -0.2;
      //! [fillWithCutMask]
    }
    return 5;
  };
  auto check1 = [&](size_t N) {
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta_CutMask", 1 ) == N );
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta_CutMask", 2 ) == 0 );
  };
  resetHists( histSvc ); check1(fill1());
  resetHists( histSvc ); check1(fill_mt(fill1));

  auto fill2 = [&]() {
    //! [fillWithCutMask_collection]
    std::vector<float> etaVec{-0.2, 0.2, -0.4, 0.4, -0.6};
    std::vector<char> cutMaskVec{0, 1, 1, 1, 0};
    auto eta = Monitored::Collection( "Eta", etaVec );
    auto cutMask = Monitored::Collection( "CutMask", cutMaskVec );
    auto group = Monitored::Group( monTool, eta, cutMask );
    //! [fillWithCutMask_collection]
    return 3;
  };
  auto check2 = [&](size_t N) {
    BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Eta_CutMask" )->GetEntries() == N );
  };
  resetHists( histSvc ); check2(fill2());
  resetHists( histSvc ); check2(fill_mt(fill2));
}


BOOST_AUTO_TEST_CASE( fillWithWeight ) {

  auto fill = [&]() {
    //! [fillWithWeight]
    auto pt = Monitored::Scalar<double>( "pt", 3.0 );
    auto weight = Monitored::Scalar<float>( "Weight", 0.5 );
    auto group = Monitored::Group( monTool, pt, weight );
    //! [fillWithWeight]
    return 1;
  };
  auto check = [&](size_t N) {
    BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/pt", 4 ) == 0.5*N );
  };
  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));

  auto fill2 = [&]() {
    //! [fillWithWeight_collection]
    std::vector<double> ptvec{1.0, 2.0, 3.0 ,4.0, 5.0};
    std::vector<float>   wvec{0.0, 0.5, 0.5, 1.0, 2.0};
    auto pt = Monitored::Collection( "pt", ptvec );
    auto weight = Monitored::Collection( "Weight", wvec );
    auto group = Monitored::Group( monTool, pt, weight );
    //! [fillWithWeight_collection]
    return 5;
  };
  auto check2 = [&](size_t N) {
    BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/pt" )->GetEntries() == N );
    BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/pt" )->GetMean() == 4.125 );
  };
  resetHists( histSvc ); check2(fill2());
  resetHists( histSvc ); check2(fill_mt(fill2));
}


/// Example of custom scalar class
class Scalar {
public:
    Scalar() : m_value( 0 ) { }
    Scalar( double value ) : m_value( value ) { }

    void operator=( double value ) { m_value = value; }
    operator double() const { return m_value; }
private:
    double m_value;
};

/// Example track class
class Track {
public:
    Track() : m_eta( 0 ), m_phi( 0 ) {}
    Track( double e, double p ) : m_eta( e ), m_phi( p ) {}
    double eta() const { return m_eta; }
    double phi() const { return m_phi; }
private:
    double m_eta, m_phi;
};


BOOST_AUTO_TEST_CASE( fillFromNonTrivialSources ) {
  resetHists( histSvc );
  {
    auto eta = Monitored::Scalar( "Eta", Scalar( 0.2 ) ); //works when object to number conversion defined
    auto group = Monitored::Group( monTool, eta );
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 1 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_lambda]
    std::vector<float> v{1,2,3};
    auto eta = Monitored::Scalar<int>( "Eta", [&](){ return v.size(); } );
    //! [fillFromNonTrivialSources_lambda]
    auto group = Monitored::Group( monTool, eta );
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 3 ) == 1 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_collection]
    std::vector<float> eta( {0.2, 0.1} );
    std::vector<double> phi( {-1, 1} ) ;
    auto vectorT   = Monitored::Collection( "Eta", eta );
    auto setT      = Monitored::Collection( "Phi", phi );
    auto group = Monitored::Group( monTool, vectorT, setT );
    //! [fillFromNonTrivialSources_collection]
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_collectionref]
    std::vector<float> reta( {0.2, 0.1} );
    std::vector<double> rphi( {-1, 1} ) ;
    auto eta = std::ref(reta);
    auto phi = std::ref(rphi);
    auto vectorT   = Monitored::Collection( "Eta", eta );
    auto setT      = Monitored::Collection( "Phi", phi );
    auto group = Monitored::Group( monTool, vectorT, setT );
    //! [fillFromNonTrivialSources_collectionref]
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_stdarray]
    std::array<double, 2> eta( {{0.1, 0.7}} );
    double phi[2] = {-2., -1.};
    auto arrayT = Monitored::Collection( "Eta", eta );
    auto rawArrayT = Monitored::Collection( "Phi", phi );
    auto group = Monitored::Group( monTool, arrayT, rawArrayT );
    //! [fillFromNonTrivialSources_stdarray]
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_obj_collection]
    std::vector<Track> tracks;
    auto eta = Monitored::Collection( "Eta", tracks, &Track::eta );
    auto phi = Monitored::Collection( "Phi", tracks, []( const Track& t ) { return t.phi(); } );

    auto group = Monitored::Group( monTool, eta, phi ); // this is binding to histograms

    tracks.emplace_back( 0.1, 0.9 );
    tracks.emplace_back( 1.3, 1. );
    //! [fillFromNonTrivialSources_obj_collection]
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == 2 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 2 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_obj_collectionref]
    std::vector<Track> rtracks;
    auto tracks = std::ref(rtracks);
    auto eta = Monitored::Collection( "Eta", tracks, &Track::eta );
    auto phi = Monitored::Collection( "Phi", tracks, []( const Track& t ) { return t.phi(); } );

    auto group = Monitored::Group( monTool, eta, phi ); // this is binding to histograms

    rtracks.emplace_back( 0.1, 0.9 );
    rtracks.emplace_back( 1.3, 1. );
    //! [fillFromNonTrivialSources_obj_collectionref]
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == 2 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 2 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_obj_array]
    Track tracks[2];
    auto eta = Monitored::Collection( "Eta", tracks, &Track::eta );
    auto phi = Monitored::Collection( "Phi", tracks, []( const Track& t ) { return t.phi(); } );

    auto group = Monitored::Group( monTool, eta, phi ); // this is binding to histograms

    tracks[0] = Track( 0.1, 0.9 );
    tracks[1] = Track( 1.3, 1. );
    //! [fillFromNonTrivialSources_obj_array]
  }
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == 2 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 2 );

  resetHists( histSvc );
  {
    //! [fillFromNonTrivialSources_obj_arrayref]
    Track rtracks[2];
    auto tracks = std::ref(rtracks);
    auto eta = Monitored::Collection( "Eta", tracks, &Track::eta );
    auto phi = Monitored::Collection( "Phi", tracks, []( const Track& t ) { return t.phi(); } );

    auto group = Monitored::Group( monTool, eta, phi ); // this is binding to histograms

    tracks[0] = Track( 0.1, 0.9 );
    tracks[1] = Track( 1.3, 1. );
    //! [fillFromNonTrivialSources_obj_arrayref]
  }

  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Eta", 2 ) == 2 );
  BOOST_TEST( contentInBin1DHist( histSvc, "/EXPERT/TestGroup/Phi", 2 ) == 2 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/Phi_vs_Eta" )->GetEntries() == 2 );
}

bool assign() {
  //! [assign]
  auto eta = Monitored::Scalar( "Eta", -3. );
  eta = 0.6;
  //! [assign]
  BOOST_TEST ( double( eta ) ==  0.6 );
  auto etaBis = Monitored::Scalar( "EtaBis", 0. );
  etaBis = 0.4;
  BOOST_TEST( double( etaBis ) == 0.4 );
  etaBis = double( eta );
  BOOST_TEST( double( etaBis ) == 0.6 );
  return true;
}

bool operators() {
  //! [operators_comp]
  auto count = Monitored::Scalar<float>( "Count", 0 );
  bool comparisonResult = count == count;
  //! [operators_comp]
  
  BOOST_TEST( comparisonResult );
  count += 1;
  BOOST_TEST( int(count) == 1 );
  count++;
  BOOST_TEST( int(count) == 2 );
  --count;
  BOOST_TEST( int(count) == 1 );
  count *= 3;
  BOOST_TEST( int(count) == 3 );

  //! [operators_examples]
  count += 1;
  count++;
  --count;
  count *= 3;
  //! [operators_examples]

  return true;
}


BOOST_AUTO_TEST_CASE( timerFilling ) {
  //! [timerFilling]
  // The name of the monitored timer has to start with "TIME", else runtime error.
  auto t1 = Monitored::Timer( "TIME_t1" );  // default is microseconds
  auto t2 = Monitored::Timer<std::chrono::milliseconds>( "TIME_t2" );
  {
    auto group = Monitored::Group( monTool, t1, t2 );
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  //! [timerFilling]
  // There should be one entry in the histogram with roughly 10ms.
  // But since user code can be blocked arbitrarily long we only check lower bound.
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/TIME_t1" )->GetEntries() == 1 );
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/TIME_t2" )->GetEntries() == 1 );
  double t1_value = getHist( histSvc, "/EXPERT/TestGroup/TIME_t1" )->GetMean();
  double t2_value = getHist( histSvc, "/EXPERT/TestGroup/TIME_t2" )->GetMean();
  BOOST_TEST( 8000 < t1_value );
  BOOST_TEST( 8 < t2_value );

  // Test scoped timer
  auto t3 = Monitored::Timer<std::chrono::milliseconds>( "TIME_t3" );
  {
    auto group = Monitored::Group( monTool, t3 );
    {
      Monitored::ScopedTimer timeit(t3);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
      // code that should not be included in timer
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  BOOST_TEST( getHist( histSvc, "/EXPERT/TestGroup/TIME_t3" )->GetEntries() == 1 );
  double t3_value = getHist( histSvc, "/EXPERT/TestGroup/TIME_t3" )->GetMean();
  BOOST_TEST( 8 < t3_value );
}


BOOST_AUTO_TEST_CASE( stringFilling ) {

  auto fill = [&]() {
    //! [stringFilling]
    auto det = Monitored::Scalar<std::string>( "DetID", "SCT" );
    Monitored::Group(monTool, det);
    det = "PIX";
    Monitored::Group(monTool, det);
    //! [stringFilling]
    return 2;
  };
  auto check = [&](size_t N) {
    const TH1* h = getHist( histSvc, "/EXPERT/TestGroup/DetID" );
    BOOST_TEST( h->GetEntries() == N );
    BOOST_TEST( h->GetXaxis()->GetLabels()->GetEntries() == 2 );
    const int sctBin = h->GetXaxis()->FindFixBin("SCT");
    BOOST_TEST( sctBin == 1 );
    BOOST_TEST( h->GetBinContent( sctBin ) == N/2 );
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));
}


BOOST_AUTO_TEST_CASE( stringFillingGen ) {

  auto fill = [&]() {
    auto det = Monitored::Scalar<std::string>( "DetID", [&](){return "SCT";} );
    Monitored::Group(monTool, det);
    return 1;
  };
  auto check = [&](size_t N) {
    const TH1* h = getHist( histSvc, "/EXPERT/TestGroup/DetID" );
    BOOST_TEST( h->GetEntries() == N );
    BOOST_TEST( h->GetXaxis()->FindFixBin("SCT") == 1 );
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));
}


BOOST_AUTO_TEST_CASE( stringFromCollection ) {

  auto fill = [&]() {
    //! [stringFromCollection]
    struct StringInObject {
      int layer;
      std::string name;
      const std::string& getName() const { return name; }
    };
    std::vector<StringInObject> testData({{0, "PIX"}, {1, "PIX"}, {3, "SCT"}, {1, "PIX"}});
    auto name = Monitored::Collection("DetID", testData,  [](const StringInObject& s){ return s.getName(); }); // lambda as accessor

    auto ignored1 = Monitored::Collection("ignored", testData,  &StringInObject::getName  ); // access via member function
    auto ignored2 = Monitored::Collection("ignored", testData,  [](const StringInObject& s){ return s.getName().c_str(); }  ); // accessor returning const char* is supported
    Monitored::Group(monTool, name);
    //! [stringFromCollection]
    return 4; // number of entries
  };

  auto check = [&](size_t N) {
    const TH1* h = getHist( histSvc, "/EXPERT/TestGroup/DetID" );
    BOOST_TEST( h->GetEntries() == N );
    BOOST_TEST( h->GetXaxis()->GetLabels()->GetEntries() == 2 ); // two distinct strings used in input data
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));
}


BOOST_AUTO_TEST_CASE( string2DFilling ) {

  auto fill = [&]() {
    //! [string2DFilling]
    auto countID = Monitored::Scalar<std::string>( "DetID", "SCT" );
    std::vector<std::string> caloLabels( { "LAr", "LAr", "Tile" } );
    auto countCalo = Monitored::Collection<std::vector<std::string>>( "DetCalo", caloLabels );
    auto x = Monitored::Scalar("x", 1.2 );
    std::vector<double> yvalues({0.2, 2.1, 1.3});
    auto y = Monitored::Collection("y", yvalues );
    
    {
      // this will fill: (SCT,LAr) (SCT,LAr) (SCT,Tile)
      Monitored::Group(monTool, countID, countCalo);
    }
    {
      // this will fill: (LAr,0.2) (LAr,2.1) (Tile,1.3)
      Monitored::Group(monTool, countCalo, y);
    }
    {
      // this will fill: (LAr,1.2) (LAr,1.2) (Tile,1.2)
      Monitored::Group(monTool, countCalo, x);
    }
    //! [string2DFilling]
    return 1;
  };

  auto check = [&](size_t N) {
    const TH1* h = getHist( histSvc, "/EXPERT/TestGroup/DetID_vs_DetCalo" );
    int larBin = h->GetXaxis()->FindFixBin("LAr");
    int sctBin = h->GetYaxis()->FindFixBin("SCT");
    BOOST_TEST( h->GetBinContent( larBin, sctBin ) == 2*N );

    h = getHist( histSvc, "/EXPERT/TestGroup/y_vs_DetCalo" );
    larBin = h->GetXaxis()->FindFixBin("LAr");
    int tileBin = h->GetXaxis()->FindFixBin("Tile");
    BOOST_TEST( h->GetBinContent( larBin, 1 ) == N );
    BOOST_TEST( h->GetBinContent( larBin, 2 ) == 0 );
    BOOST_TEST( h->GetBinContent( larBin, 3 ) == N );
    BOOST_TEST( h->GetBinContent( tileBin, 1 ) == 0 );
    BOOST_TEST( h->GetBinContent( tileBin, 2 ) == N );

    h = getHist( histSvc, "/EXPERT/TestGroup/DetCalo_vs_x" );
    larBin = h->GetYaxis()->FindFixBin("LAr");
    tileBin = h->GetYaxis()->FindFixBin("Tile");
    BOOST_TEST( h->GetBinContent( 1, larBin) == 0 );
    BOOST_TEST( h->GetBinContent( 2, larBin) == 2*N );
    BOOST_TEST( h->GetBinContent( 1, tileBin) == 0 );
    BOOST_TEST( h->GetBinContent( 2, tileBin) == N );
  };

  resetHists( histSvc ); check(fill());
  resetHists( histSvc ); check(fill_mt(fill));
}


BOOST_AUTO_TEST_SUITE_END()
