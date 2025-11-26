/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTop/JetMSVAugmentation.h"

#include "xAODBTagging/SecVtxHelper.h"
#include "xAODTracking/Vertex.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"
#include "StoreGate/WriteDecorHandle.h"


namespace DerivationFramework {


JetMSVAugmentation::JetMSVAugmentation(const std::string& t, const std::string& n, const IInterface* p):
  base_class(t,n,p)
{
}



JetMSVAugmentation::~JetMSVAugmentation() = default;



StatusCode JetMSVAugmentation::initialize(){
  ATH_MSG_DEBUG("Initialize " );
  ATH_CHECK(m_jetCollectionName.initialize());

  return StatusCode::SUCCESS;

}



StatusCode JetMSVAugmentation::addBranches(const EventContext& ctx) const{

  SG::ReadHandle<xAOD::JetContainer> jets{m_jetCollectionName, ctx};
  if ( !jets.isValid() ) {
    ATH_MSG_ERROR ("Couldn't retrieve jets with key: " << m_jetCollectionName );
    return StatusCode::FAILURE;
  }


  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxmass(m_dec_vtxmass, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxpt(m_dec_vtxpt, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxeta(m_dec_vtxeta, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxphi(m_dec_vtxphi, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxefrac(m_dec_vtxefrac, ctx);

  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxx(m_dec_vtxx, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxy(m_dec_vtxy, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxz(m_dec_vtxz, ctx);

  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<int>> dec_vtxntrk(m_dec_vtxntrk, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> dec_vtxdls(m_dec_vtxdls, ctx);


  for (auto jet : *jets) {
    const xAOD::BTagging* bjet = xAOD::BTaggingUtilities::getBTagging( *jet );
    if (!bjet) {
      ATH_MSG_WARNING("btagging information not available" );
      continue;
    }

    std::vector< ElementLink< xAOD::VertexContainer > > msvVertices;
    bjet->variable<std::vector<ElementLink<xAOD::VertexContainer> > >(m_vtxAlgName, "vertices", msvVertices);

    std::vector<float> vtx_mass;
    std::vector<float> vtx_pt;
    std::vector<float> vtx_eta;
    std::vector<float> vtx_phi;
    std::vector<float> vtx_efrac;
    std::vector<float> vtx_x;
    std::vector<float> vtx_y;
    std::vector<float> vtx_z;
    std::vector<int> vtx_ntrk;
    std::vector<float> vtx_dls;


    for (auto vtx : msvVertices) {//loop in vertices

      int   ntrk = xAOD::SecVtxHelper::VtxNtrk(*vtx);
      float mass = xAOD::SecVtxHelper::VertexMass(*vtx);
      float efrc = xAOD::SecVtxHelper::EnergyFraction(*vtx);
      float pt   = xAOD::SecVtxHelper::Vtxpt(*vtx);
      float eta  = xAOD::SecVtxHelper::Vtxeta(*vtx);
      float phi  = xAOD::SecVtxHelper::Vtxphi(*vtx);
      float dls  = xAOD::SecVtxHelper::VtxnormDist(*vtx);
      float xp   = (*vtx)->x();
      float yp   = (*vtx)->y();
      float zp   = (*vtx)->z();
      // float chi  = (*vtx)->chiSquared();
      // float ndf  = (*vtx)->numberDoF();

      TLorentzVector p;
      p.SetPtEtaPhiM(pt,eta,phi,mass);

      vtx_mass.push_back(mass);
      vtx_pt.push_back(pt);
      vtx_eta.push_back(eta);
      vtx_phi.push_back(phi);
      vtx_efrac.push_back(efrc);
      vtx_x.push_back(xp);
      vtx_y.push_back(yp);
      vtx_z.push_back(zp);
      vtx_ntrk.push_back(ntrk);
      vtx_dls.push_back(dls);
    }

    dec_vtxmass(*bjet)=vtx_mass;
    dec_vtxpt(*bjet)=vtx_pt;
    dec_vtxeta(*bjet)=vtx_eta;
    dec_vtxphi(*bjet)=vtx_phi;
    dec_vtxefrac(*bjet)=vtx_efrac;
    dec_vtxx(*bjet)=vtx_x;
    dec_vtxy(*bjet)=vtx_y;
    dec_vtxz(*bjet)=vtx_z;
    dec_vtxdls(*bjet)=vtx_dls;
    dec_vtxntrk(*bjet)=vtx_ntrk;

  }

  return StatusCode::SUCCESS;

}



} /// namespace
