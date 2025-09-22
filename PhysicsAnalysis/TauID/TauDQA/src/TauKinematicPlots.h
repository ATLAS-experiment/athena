/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_TAUKINEMATICPLOTS_H
#define TAUDQA_TAUKINEMATICPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODBase/IParticle.h"

namespace Tau{

class TauKinematicPlots:public PlotBase {
   public:      
      TauKinematicPlots(PlotBase *pParent, const std::string& sDir, std::string sParticleType);
      ~TauKinematicPlots();
      void fill(const xAOD::IParticle& prt, float weight);
      
      TH1* eta{};
      TH1* phi{};
      TH1* pt{};
      
      TH2* eta_phi{};
      TH2* eta_pt{};
      
   private:
      void initializePlots();
      std::string m_sParticleType;
};

}

#endif // TAUDQA_TAUKINEMATICPLOTS_H


