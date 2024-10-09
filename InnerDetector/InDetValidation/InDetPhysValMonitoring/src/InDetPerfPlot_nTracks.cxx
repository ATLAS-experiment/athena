/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_nTracks.cxx
 * @author shaun roe
 **/

#include "InDetPerfPlot_nTracks.h"


InDetPerfPlot_nTracks::InDetPerfPlot_nTracks(InDetPlotBase* pParent, const std::string& sDir) : InDetPlotBase(pParent, sDir) {
  // nop
}

void
InDetPerfPlot_nTracks::initializePlots() {
  const bool prependDirectory(false);
  SingleHistogramDefinition hd = retrieveDefinition("ntrack");
  m_counters[ALLRECO] = Book1D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, prependDirectory);
  hd = retrieveDefinition("ntracksel");
  m_counters[SELECTEDRECO] = Book1D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, prependDirectory);
  hd = retrieveDefinition("nparticle");
  m_counters[ALLTRUTH] = Book1D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, prependDirectory);
  hd = retrieveDefinition("nparticlesel");
  m_counters[SELECTEDTRUTH] = Book1D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, prependDirectory);
  hd = retrieveDefinition("num_truthmatch_match");
  m_counters[MATCHEDRECO] =
    Book1D(hd.name, hd.allTitles, hd.nBinsX, hd.xAxis.first, hd.xAxis.second, prependDirectory);

  book(m_ntracks_vs_truthMu, "ntracks_vs_truthMu");
  book(m_ntracks_vs_truthMu_absEta_0_2p5, "ntracks_vs_truthMu_absEta_0_2p5");
  book(m_ntracks_vs_truthMu_pT_1GeV, "ntracks_vs_truthMu_pT_1GeV");
  book(m_ntracks_vs_actualMu, "ntracks_vs_actualMu");
  book(m_ntracks_vs_actualMu_absEta_0_2p5, "ntracks_vs_actualMu_absEta_0_2p5");
  book(m_ntracks_vs_actualMu_pT_1GeV, "ntracks_vs_actualMu_pT_1GeV");
  book(m_ntracks_vs_nvertices, "ntracks_vs_nvertices");
  book(m_ntracks_vs_nvertices_absEta_0_2p5, "ntracks_vs_nvertices_absEta_0_2p5");
  book(m_ntracks_vs_nvertices_pT_1GeV, "ntracks_vs_nvertices_pT_1GeV");

}

void
InDetPerfPlot_nTracks::fill(const unsigned int freq, const CounterCategory counter, float weight) {
  if (counter < N_COUNTERS) {
    fillHisto((m_counters[counter]), freq, weight);
  }
}

void InDetPerfPlot_nTracks::fill
(const unsigned int ntracksFull, const unsigned int ntracksCentral,
 const unsigned int ntracksPt1GeV, const unsigned int truthMu,
 const float actualMu, const unsigned int nvertices, const float weight) {

  fillHisto(m_ntracks_vs_truthMu, truthMu, ntracksFull, weight);
  fillHisto(m_ntracks_vs_actualMu, actualMu, ntracksFull, weight);
  fillHisto(m_ntracks_vs_nvertices, nvertices, ntracksFull, weight);
  fillHisto(m_ntracks_vs_truthMu_absEta_0_2p5, truthMu, ntracksCentral, weight);
  fillHisto(m_ntracks_vs_actualMu_absEta_0_2p5, actualMu, ntracksCentral, weight);
  fillHisto(m_ntracks_vs_nvertices_absEta_0_2p5, nvertices, ntracksCentral, weight);
  fillHisto(m_ntracks_vs_truthMu_pT_1GeV, truthMu, ntracksPt1GeV, weight);
  fillHisto(m_ntracks_vs_actualMu_pT_1GeV, actualMu, ntracksPt1GeV, weight);
  fillHisto(m_ntracks_vs_nvertices_pT_1GeV, nvertices, ntracksPt1GeV, weight);

}
