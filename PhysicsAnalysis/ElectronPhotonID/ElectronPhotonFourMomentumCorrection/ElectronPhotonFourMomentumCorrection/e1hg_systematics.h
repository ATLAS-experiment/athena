/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef E1HG_SYSTEMATICS_H
#define E1HG_SYSTEMATICS_H
#include "TArrayD.h"
#include "TFile.h"
#include "TH1.h"

/**
  @class e1hg_Systematics
  @brief get systematics from MG/HG layer 1 miscalibration assuming MG is
  correctly re-calibrated with muons

*/

class e1hg_systematics {

 public:
  /** @brief constructor (initialization done there reading root files) */
  e1hg_systematics(const std::string& filename);
  ~e1hg_systematics();

  /**
   @brief particle_type = 0 (electrons), 1 (unconv photons), 2 (conv photons)
   @brief energy = energy in MeV
   @brief eta
   @brief return value  alpha    Ebiased =  E*(1 + alpha)
  */
  double getAlpha(int particle_type, double energy, double eta,
                  bool interpolate = false) const;

 private:
  // histograms to store parameters
  TH1D* m_helec[8];
  TH1D* m_hphot[8];
  TH1D* m_hphot2[8];
  TFile* m_file0;
  const TArrayD* m_etBins;
};

#endif
