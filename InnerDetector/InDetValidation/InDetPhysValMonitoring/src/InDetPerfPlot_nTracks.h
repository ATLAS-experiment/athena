/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_INDETPERFPLOT_NTRACKS
#define INDETPHYSVALMONITORING_INDETPERFPLOT_NTRACKS
/**
 *  @file InDetPerfPlot_nTracks.h
 *  @author shaun roe
 **/

// std includes
#include <string>

// local includes
#include "InDetPlotBase.h"
class TH1;
class TH2;

///Class to hold various counters used in RTT code
class InDetPerfPlot_nTracks: public InDetPlotBase {
public:
  InDetPerfPlot_nTracks(InDetPlotBase* pParent, const std::string& dirName);
  enum CounterCategory {
    ALLRECO, SELECTEDRECO, ALLTRUTH, SELECTEDTRUTH, ALLASSOCIATEDTRUTH, MATCHEDRECO, N_COUNTERS
  };
  void fill(const unsigned int freq, const CounterCategory counter, float weight=1.0);
  void fill(const unsigned int ntracksFull, const unsigned int ntracksCentral,
	    const unsigned int ntracksPt1GeV, const unsigned int truthMu,
	    const float actualMu, const unsigned int nvertices, const float weight=1.0);

private:
  TH1* m_counters[N_COUNTERS]{nullptr};
  TH2* m_ntracks_vs_truthMu{};
  TH2* m_ntracks_vs_truthMu_absEta_0_2p5{};
  TH2* m_ntracks_vs_truthMu_pT_1GeV{};
  TH2* m_ntracks_vs_actualMu{};
  TH2* m_ntracks_vs_actualMu_absEta_0_2p5{};
  TH2* m_ntracks_vs_actualMu_pT_1GeV{};
  TH2* m_ntracks_vs_nvertices{};
  TH2* m_ntracks_vs_nvertices_absEta_0_2p5{};
  TH2* m_ntracks_vs_nvertices_pT_1GeV{};
  void initializePlots();
};

#endif
