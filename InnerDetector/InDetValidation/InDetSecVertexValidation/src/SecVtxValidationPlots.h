/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VERTEXVALIDATIONPLOTS_H
#define VERTEXVALIDATIONPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODTracking/Vertex.h"
#include <string>

class TH1;

class SecVtxValidationPlots : public PlotBase {

  public:

    /** Standard Constructor */
    SecVtxValidationPlots(PlotBase* pParent, const std::string& sDir);
    virtual ~SecVtxValidationPlots() = default;

    /** fill the histograms */
    void fill(const xAOD::Vertex* secVtx);

  private:

    // position
    TH1* m_vertex_x {};
    TH1* m_vertex_y {};
    TH1* m_vertex_z {};
    TH1* m_vertex_r {};

    // four vector
    TH1* m_vertex_pt {};
    TH1* m_vertex_eta {};
    TH1* m_vertex_phi {};
    TH1* m_vertex_m {};

    // misc
    TH1* m_vertex_ntrk {};
    TH1* m_vertex_chi2 {};
    TH1* m_vertex_charge {};
    TH1* m_vertex_mind0 {};
    TH1* m_vertex_maxd0 {};


};
#endif /* VERTEXVALIDATIONPLOTS_H */
