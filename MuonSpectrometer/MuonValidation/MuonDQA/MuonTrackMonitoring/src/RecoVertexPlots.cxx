/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackMonitoring/RecoVertexPlots.h"
#include "AthContainers/ConstAccessor.h"


RecoVertexPlots::RecoVertexPlots(PlotBase *pParent, const std::string& sDir):PlotBase(pParent, sDir),
	m_nMDT(nullptr),
	m_nRPC(nullptr),
	m_nTGC(nullptr),
	m_nTracklets(nullptr),
	m_VertexEtaPhi(nullptr)
{}

void RecoVertexPlots::initializePlots(){

    m_nMDT = Book1D("m_MSVx_nMDT", "m MSVx nMDT: M MSVx MDT Hits;;N_{MDT}", 100,  0.0,  3000.);
    m_nRPC = Book1D("m_MSVx_nRPC", "m MSVx nRPC: M MSVx RPC Hits;;N_{RPC}", 100,  0.0,  1000.);
    m_nTGC = Book1D("m_MSVx_nTGC", "m MSVx nTGC: M MSVx TGC Hits;;N_{TGC}", 100,  0.0,  1000.);

    m_nTracklets = Book1D("m_MSVx_nTracklets", "MSVx nTracklets: M MSVx Number of Tracklets; N_{trackletS}", 20,  0.0,  20.);   
    m_VertexEtaPhi   = Book2D("m_VertexEtaPhi", "Vertex Eta Phi: M MSVx #eta vs. #phi; #eta; #phi", 25, -2.5, 25, 32, -3.2, 3.2);    
 
}

void RecoVertexPlots::fill(const xAOD::Vertex& msVx){    
    TVector3 tmp_vx; tmp_vx.SetXYZ(msVx.x(),msVx.y(),msVx.z());
    static const SG::ConstAccessor< int > nMDTAcc( "nMDT" );
    static const SG::ConstAccessor< int > nRPCAcc( "nRPC" );
    static const SG::ConstAccessor< int > nTGCAcc( "nTGC" );
    m_nMDT->Fill(nMDTAcc(msVx));
    m_nRPC->Fill(nRPCAcc(msVx));
    m_nTGC->Fill(nTGCAcc(msVx));
    m_nTracklets->Fill(msVx.numberDoF()+1.);
    m_VertexEtaPhi->Fill(tmp_vx.Eta(),tmp_vx.Phi());
}
