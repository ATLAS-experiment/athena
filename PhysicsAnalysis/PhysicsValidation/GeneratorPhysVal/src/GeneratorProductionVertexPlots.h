/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeneratorPhysValVtxPlots_GeneratorProductionVertexPlots_H
#define GeneratorPhysValVtxPlots_GeneratorProductionVertexPlots_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODBase/IParticle.h"
#include "xAODTruth/TruthVertex.h"
namespace GeneratorPhysVal
{
  class GeneratorProductionVertexPlots : public PlotBase
  {
  public:
    TH1* prod_r = nullptr;
    TH1* prod_x = nullptr;
    TH1* prod_y = nullptr;
    TH1* prod_z = nullptr;
    GeneratorProductionVertexPlots(PlotBase* pParent, const std::string& sDir, const std::string& sType = ""): PlotBase(pParent, sDir), m_sType(sType)
    {
      prod_r = Book1D("prod_r", "prod_r" + m_sType + ";r;Entries", 600, -300., 300.);
      prod_x = Book1D("prod_x", "prod_x" + m_sType + ";x;Entries", 100, -50., 50.);
      prod_y = Book1D("prod_y", "prod_y" + m_sType + ";y;Entries", 100, -50., 50.);
      prod_z = Book1D("prod_z", "prod_z" + m_sType + ";z;Entries", 2000, -1000., 1000.);

    } 

    void fillProdVtx(const xAOD::TruthVertex* vtx)
    {
      prod_r->Fill(vtx->perp());
      prod_x->Fill(vtx->x());
      prod_y->Fill(vtx->y());
      prod_z->Fill(vtx->z());
    }



  private:
    std::string m_sType = "";
  };

}

#endif
