/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeneratorPhysValPlots_GeneratorPlots_H
#define GeneratorPhysValPlots_GeneratorPlots_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODBase/IParticle.h"

namespace GeneratorPhysVal
{
  class GeneratorPlots : public PlotBase
  {
  public:
    TH1* pT = nullptr;
    TH1* eta = nullptr;
    TH1* phi = nullptr;
    
    GeneratorPlots(PlotBase* pParent, const std::string& sDir, const std::string& sType = ""): PlotBase(pParent, sDir), m_sType(sType)
    {
      pT = Book1D("pT", "p_{T} of " + m_sType + ";pT(GeV);Entries", 15, 0., 15.);
      eta = Book1D("eta", "#eta of " + m_sType + ";#eta; Events ", 400, -10, 10.);
      phi = Book1D("phi", "#varphi of " + m_sType + ";#varphi;Events ", 128, -3.2, 3.2);
    }

    void fill(const xAOD::IParticle* part)
    {
      pT->Fill(0.001 * part->pt());
      eta->Fill(part->eta());
      phi->Fill(part->phi());
    }

    void fill(const std::vector< TLorentzVector >& part)
    {
      for(const auto &parti : part ){
        pT->Fill(0.001 * parti.Pt());
        eta->Fill(parti.Eta());
        phi->Fill(parti.Phi());
      }
    }
    


  private:
    std::string m_sType = "";
  };

}

#endif
