/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE HistogramFillerUtils
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../src/HistogramFiller/HistogramFillerUtils.h"
#include "TestTools/expect.h"

#include "TH1F.h"

using Monitored::detail::fillWillRebinHistogram;


BOOST_AUTO_TEST_CASE( checkRebin ) {
  TH1F h("h", "", 3, 0, 3);
  BOOST_TEST( !fillWillRebinHistogram(h.GetXaxis(), 1.0) );
  BOOST_TEST( fillWillRebinHistogram(h.GetXaxis(), -1.0) );
  BOOST_TEST( fillWillRebinHistogram(h.GetXaxis(), 4.0) );
}

BOOST_AUTO_TEST_CASE( checkRebin_string ) {
  TH1F h("h", "", 3, 0, 3);
  BOOST_TEST( !fillWillRebinHistogram(h.GetXaxis(), "a") );
  h.Fill("a", 1.0);
  BOOST_TEST( !fillWillRebinHistogram(h.GetXaxis(), "a") );
  h.Fill("b", 1.0);
  h.Fill("c", 1.0);
  // Now all the available bins have labels
  BOOST_TEST( !fillWillRebinHistogram(h.GetXaxis(), "c") );
  BOOST_TEST( fillWillRebinHistogram(h.GetXaxis(), "d") );
  h.Fill("d", 1.0); // adds more than one bin so we have free labels again
  // But this should still be considered as rebinning (ADHI-4881)
  BOOST_TEST( fillWillRebinHistogram(h.GetXaxis(), "e") );
}
