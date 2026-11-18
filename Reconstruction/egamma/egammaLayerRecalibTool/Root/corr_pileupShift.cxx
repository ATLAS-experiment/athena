/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "egammaLayerRecalibTool/corr_pileupShift.h"
#include <iostream>
#include <string>
#include <utility>
#include <cmath>

#include "TFile.h"
#include "PathResolver/PathResolver.h"


corr_pileupShift::corr_pileupShift(){

  std::string filename = PathResolverFindCalibFile("egammaLayerRecalibTool/v2/layer_average.root");

  auto file = std::unique_ptr<TFile>(TFile::Open(filename.c_str()));
  if (not file or file->IsZombie()) {
    std::cerr << "FATAL: cannot open " << filename << std::endl;
  } else {
    for (int layer = 0; layer < 4; layer++) {
      char name[12];
      snprintf(name, sizeof(name), "average_%d", layer);
      auto obj = dynamic_cast<TH1D*>(file->Get(name));
      if (not obj) {
        std::cerr << "FATAL: cannot find " << name << std::endl;
      } else {
        obj->SetDirectory(0);
        m_haverage[layer].reset(obj);
      }
    }
  }
  m_runMin = 296939;
  m_runMax = 311563;
}
//===============================================================================
corr_pileupShift::~corr_pileupShift() = default;

//===============================================================================
float corr_pileupShift::getCorr(int layer, int run, float mu, float eta) const
{

   if (layer<0 || layer>3) return 0.;
   if (run<m_runMin || run>m_runMax) return 0.;
   double aeta=std::fabs(eta);
   if (aeta> 2.5) return 0.;
   if (aeta>2.37) aeta=2.37;

   int ibin = std::as_const(*m_haverage[layer]).GetXaxis()->FindFixBin(aeta);
   float corr = mu * m_haverage[layer]->GetBinContent(ibin);

   return corr;

}
