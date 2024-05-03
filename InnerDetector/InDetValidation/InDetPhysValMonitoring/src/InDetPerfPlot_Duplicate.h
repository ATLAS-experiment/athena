/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_INDETPERFPLOT_DUPLICATE
#define INDETPHYSVALMONITORING_INDETPERFPLOT_DUPLICATE

#include "InDetPlotBase.h"
#include "xAODTruth/TruthParticle.h"

#include "TEfficiency.h"
#include "TProfile.h"

class InDetPerfPlot_Duplicate: public InDetPlotBase {

 public:
  InDetPerfPlot_Duplicate(InDetPlotBase* pParent, const std::string& dirName);

  void fill(const xAOD::TruthParticle& truth, unsigned int ntracks, float weight);

 private:
  TEfficiency* m_rate_vs_pt{};
  TEfficiency* m_rate_vs_eta{};

  TProfile* m_number_vs_pt{};
  TProfile* m_number_vs_eta{};

  TProfile* m_number_nonzero_vs_pt{};
  TProfile* m_number_nonzero_vs_eta{};

  // plot base has nop default implementation of this; we use it to book the histos
  void initializePlots();
  void finalizePlots(){}
  
};

#endif
