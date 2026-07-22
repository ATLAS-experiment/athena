/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SecVtxValidationPlots.h"
#include "TVector3.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "AthenaKernel/Units.h"
#include <cmath>

using Athena::Units::GeV;

SecVtxValidationPlots::SecVtxValidationPlots(PlotBase* pParent, 
                                             const std::string& sDir) 
  : PlotBase(pParent, sDir) 
{
  // position
  m_vertex_x = Book1D("x", "Vertex_x;x [mm];Entries;", 300, -300, 300, false);       
  m_vertex_y = Book1D("y", "Vertex_y;y [mm];Entries;", 300, -300, 300, false);       
  m_vertex_z = Book1D("z", "Vertex_z;z [mm];Entries;", 300, -300, 300, false);       
  m_vertex_r = Book1D("r", "Vertex_r;r [mm];Entries;", 300, 0, 300, false);       

  // four vector
  m_vertex_pt  = Book1D("pt", "Vertex_pt;p_{T} [GeV];Entries;", 100, 0, 100, false);
  m_vertex_eta = Book1D("eta", "Vertex_eta;#eta;Entries;", 50, -2.5, 2.5, false);
  m_vertex_phi = Book1D("phi", "Vertex_phi;#phi;Entries;", 64, -3.2, 3.2, false);
  m_vertex_m   = Book1D("m", "Vertex_m;m [GeV];Entries;", 100, 0, 100, false);

  // misc
  m_vertex_ntrk   = Book1D("ntrk", "Vertex_ntrk;n_{trk};Entries;", 20, 0, 20, false);
  m_vertex_chi2   = Book1D("chi2", "Vertex_chi2;#chi^{2}/n_{DoF};Entries;", 50, 0, 10, false);
  m_vertex_charge = Book1D("charge", "Vertex_charge;charge;Entries;", 20, -10, 10, false);
  m_vertex_mind0  = Book1D("mind0", "Vertex_mind0;d_{0,min} [mm];Entries;", 100, 0, 10, false);
  m_vertex_maxd0  = Book1D("maxd0", "Vertex_maxd0;d_{0,max} [mm];Entries;", 300, 0, 300, false);
}

void SecVtxValidationPlots::fill(const xAOD::Vertex* secVtx){

  TVector3 vtx_pos(secVtx->x(), secVtx->y(), secVtx->z());
  float vtx_r = vtx_pos.Perp();

  xAOD::Vertex::ConstAccessor<float> Chi2("chiSquared");
  xAOD::Vertex::ConstAccessor<float> nDoF("numberDoF");
  xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");

  float minD0 = 1.0 * 1.e4;
  float maxD0 = 0.0;
  int charge = 0;

  const xAOD::Vertex::TrackParticleLinks_t & trkParts = trkAcc( *secVtx );

  size_t ntrk = trkParts.size();
  TLorentzVector sumP4(0,0,0,0);
  //accessors used in loop; declare before entering
  xAOD::TrackParticle::ConstAccessor< float > ptAcc( "pt_wrtSV" );
  xAOD::TrackParticle::ConstAccessor< float > etaAcc( "eta_wrtSV" );
  xAOD::TrackParticle::ConstAccessor< float > phiAcc( "phi_wrtSV" );
  for (const auto &trklink : trkParts) {
    if (!trklink.isValid()) {
      continue;
    }
    const xAOD::TrackParticle & trk = **trklink;

    double trk_d0 = std::abs(trk.definingParameters()[0]);
    if(trk_d0 < minD0){ minD0 = trk_d0; }
    if(trk_d0 > maxD0){ maxD0 = trk_d0; }

    TLorentzVector trk_P4;
    trk_P4.SetPtEtaPhiM(ptAcc(trk), etaAcc(trk), phiAcc(trk), trk.p4().M());

    sumP4 += trk_P4;
    charge += trk.charge();
  }

  m_vertex_x->Fill(secVtx->x());
  m_vertex_y->Fill(secVtx->y());
  m_vertex_z->Fill(secVtx->z());
  m_vertex_r->Fill(vtx_r);
  m_vertex_pt->Fill(sumP4.Pt() / GeV); // GeV 
  m_vertex_eta->Fill(sumP4.Eta());
  m_vertex_phi->Fill(sumP4.Phi());
  m_vertex_m->Fill(sumP4.M() / GeV); // GeV
  m_vertex_ntrk->Fill(ntrk);
  m_vertex_chi2->Fill(Chi2(*secVtx)/nDoF(*secVtx));
  m_vertex_charge->Fill(charge);
  m_vertex_mind0->Fill(minD0);
  m_vertex_maxd0->Fill(maxD0);

}
