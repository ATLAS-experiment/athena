/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// PsiPlusPsiCascade.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/PsiPlusPsiCascade.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetBeamSpotService/IBeamCondSvc.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "DerivationFrameworkBPhys/LocalVector.h"
#include "HepPDT/ParticleDataTable.hh"
#include <algorithm>
#include <functional>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;

  StatusCode PsiPlusPsiCascade::initialize() {
    // retrieving vertex Fitter
    ATH_CHECK( m_iVertexFitter.retrieve() );

    // retrieving the V0 tools
    ATH_CHECK( m_V0Tools.retrieve() );

    // retrieving the Cascade tools
    ATH_CHECK( m_CascadeTools.retrieve() );

    // Get the beam spot service
    ATH_CHECK( m_beamSpotSvc.retrieve() );

    IPartPropSvc* partPropSvc = nullptr;
    ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
    auto pdt = partPropSvc->PDT();

    // retrieve particle masses
    if(m_massJpsi1 < 0.) m_massJpsi1 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_massJpsi2 < 0.) m_massJpsi2 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_massPsi1 < 0.) m_massPsi1 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::psi_2S);
    if(m_massPsi2 < 0.) m_massPsi2 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::psi_2S);

    if(m_vtx1Daug1MassHypo < 0.) m_vtx1Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx1Daug2MassHypo < 0.) m_vtx1Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx1Daug3MassHypo < 0.) m_vtx1Daug3MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx1Daug4MassHypo < 0.) m_vtx1Daug4MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx2Daug1MassHypo < 0.) m_vtx2Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx2Daug2MassHypo < 0.) m_vtx2Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx2Daug3MassHypo < 0.) m_vtx2Daug3MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx2Daug4MassHypo < 0.) m_vtx2Daug4MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);

    return StatusCode::SUCCESS;
  }

  StatusCode PsiPlusPsiCascade::addBranches() const {
    constexpr int topoN = 3;
    std::array<std::unique_ptr<xAOD::VertexContainer>, topoN> VtxWriteHandles;
    std::array<std::unique_ptr<xAOD::VertexAuxContainer>, topoN> VtxWriteHandlesAux;
    if(m_cascadeOutputsKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of VtxContainers");
      return StatusCode::FAILURE;
    }
   
    if ((m_vtx1Daug_num != 3 && m_vtx1Daug_num != 4) || (m_vtx2Daug_num != 3 && m_vtx2Daug_num != 4)) {
      ATH_MSG_FATAL("Incorrect number of Psi daughters (should be 3 or 4)");
      return StatusCode::FAILURE;
    }

    for(int i=0; i<topoN; i++){
      VtxWriteHandles[i] = std::make_unique<xAOD::VertexContainer>();
      VtxWriteHandlesAux[i] = std::make_unique<xAOD::VertexAuxContainer>();
      VtxWriteHandles[i]->setStore(VtxWriteHandlesAux[i].get());
    }

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    const xAOD::VertexContainer *pvContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
    ATH_MSG_DEBUG("Found " << m_VxPrimaryCandidateName << " in StoreGate!");
    if (pvContainer->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
      return StatusCode::RECOVERABLE;
    }

    //----------------------------------------------------
    // Record refitted primary vertices
    //----------------------------------------------------
    xAOD::VertexContainer* refPvContainer = nullptr;
    xAOD::VertexAuxContainer* refPvAuxContainer = nullptr;
    if (m_refitPV) {
      refPvContainer = new xAOD::VertexContainer;
      refPvAuxContainer = new xAOD::VertexAuxContainer;
      refPvContainer->setStore(refPvAuxContainer);
      ATH_CHECK(evtStore()->record(refPvContainer   , m_refPVContainerName));
      ATH_CHECK(evtStore()->record(refPvAuxContainer, m_refPVContainerName+"Aux."));
    }

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer_noConstr;
    ATH_CHECK(performSearch(&cascadeinfoContainer,&cascadeinfoContainer_noConstr));

    BPhysPVCascadeTools helper(&(*m_CascadeTools), &m_beamSpotSvc);
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    // Decorators for the main vertex: chi2, ndf, pt and pt error, plus the V0 vertex variables
    SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> Psi1LinksDecor("Psi1VertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> Psi2LinksDecor("Psi2VertexLinks");
    SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
    SG::AuxElement::Decorator<int> ndof_decor("nDoF");
    SG::AuxElement::Decorator<float> chi2_nc_decor("ChiSquared_nc");
    SG::AuxElement::Decorator<int> ndof_nc_decor("nDoF_nc");
    SG::AuxElement::Decorator<float> Pt_decor("Pt");
    SG::AuxElement::Decorator<float> PtErr_decor("PtErr");
    SG::AuxElement::Decorator<float> chi2_SV1_decor("ChiSquared_SV1");
    SG::AuxElement::Decorator<float> chi2_nc_SV1_decor("ChiSquared_nc_SV1");
    SG::AuxElement::Decorator<float> chi2_V1_decor("ChiSquared_V1");
    SG::AuxElement::Decorator<int> ndof_V1_decor("nDoF_V1");
    SG::AuxElement::Decorator<float> lxy_SV1_decor("lxy_SV1");
    SG::AuxElement::Decorator<float> lxyErr_SV1_decor("lxyErr_SV1");
    SG::AuxElement::Decorator<float> a0xy_SV1_decor("a0xy_SV1");
    SG::AuxElement::Decorator<float> a0xyErr_SV1_decor("a0xyErr_SV1");
    SG::AuxElement::Decorator<float> a0z_SV1_decor("a0z_SV1");
    SG::AuxElement::Decorator<float> a0zErr_SV1_decor("a0zErr_SV1");
    SG::AuxElement::Decorator<float> chi2_SV2_decor("ChiSquared_SV2");
    SG::AuxElement::Decorator<float> chi2_nc_SV2_decor("ChiSquared_nc_SV2");
    SG::AuxElement::Decorator<float> chi2_V2_decor("ChiSquared_V2");
    SG::AuxElement::Decorator<int> ndof_V2_decor("nDoF_V2");
    SG::AuxElement::Decorator<float> lxy_SV2_decor("lxy_SV2");
    SG::AuxElement::Decorator<float> lxyErr_SV2_decor("lxyErr_SV2");
    SG::AuxElement::Decorator<float> a0xy_SV2_decor("a0xy_SV2");
    SG::AuxElement::Decorator<float> a0xyErr_SV2_decor("a0xyErr_SV2");
    SG::AuxElement::Decorator<float> a0z_SV2_decor("a0z_SV2");
    SG::AuxElement::Decorator<float> a0zErr_SV2_decor("a0zErr_SV2");

    // Get the container and identify the input Psi's
    const xAOD::VertexContainer *psi1Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(psi1Container, m_vertexPsi1ContainerKey));
    const xAOD::VertexContainer *psi2Container(nullptr);
    if(m_vertexPsi2ContainerKey == m_vertexPsi1ContainerKey) psi2Container = psi1Container;
    else ATH_CHECK(evtStore()->retrieve(psi2Container, m_vertexPsi2ContainerKey));

    for(unsigned int ic=0; ic<cascadeinfoContainer.size(); ic++) {
      Trk::VxCascadeInfo* cascade_info = cascadeinfoContainer[ic];
      if(cascade_info==nullptr) ATH_MSG_ERROR("CascadeInfo is null");

      Trk::VxCascadeInfo* cascade_info_noConstr = cascadeinfoContainer_noConstr[ic];

      const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info->vertices();
      if(cascadeVertices.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices");
      if(cascadeVertices[0]==nullptr || cascadeVertices[1]==nullptr || cascadeVertices[2]==nullptr) ATH_MSG_ERROR("Error null vertex");
      // Keep vertices
      for(int i=0; i<topoN; i++) VtxWriteHandles[i]->push_back(cascadeVertices[i]);

      cascade_info->getSVOwnership(false); // Prevent Container from deleting vertices
      const auto mainVertex = cascadeVertices[2]; // this is the mother vertex
      const std::vector< std::vector<TLorentzVector> > &moms = cascade_info->getParticleMoms();

      // Set links to cascade vertices
      std::vector<VertexLink> precedingVertexLinks;
      VertexLink vertexLink1;
      vertexLink1.setElement(cascadeVertices[0]);
      vertexLink1.setStorableObject(*VtxWriteHandles[0].get());
      if( vertexLink1.isValid() ) precedingVertexLinks.push_back( vertexLink1 );
      VertexLink vertexLink2;
      vertexLink2.setElement(cascadeVertices[1]);
      vertexLink2.setStorableObject(*VtxWriteHandles[1].get());
      if( vertexLink2.isValid() ) precedingVertexLinks.push_back( vertexLink2 );
      CascadeLinksDecor(*mainVertex) = precedingVertexLinks;

      // Identify the input Psi2
      xAOD::Vertex* psi2Vertex(0);
      if(m_vtx2Daug_num==4) psi2Vertex = BPhysPVCascadeTools::FindVertex<4>(psi2Container, cascadeVertices[1]);
      else psi2Vertex = BPhysPVCascadeTools::FindVertex<3>(psi2Container, cascadeVertices[1]);
      // Identify the input Psi1
      xAOD::Vertex* psi1Vertex(0);
      if(m_vtx1Daug_num==4) psi1Vertex = BPhysPVCascadeTools::FindVertex<4>(psi1Container, cascadeVertices[0]);
      else psi1Vertex = BPhysPVCascadeTools::FindVertex<3>(psi1Container, cascadeVertices[0]);

      // Set links to input vertices
      std::vector<xAOD::Vertex*> psi2VerticestoLink;
      if(psi2Vertex) psi2VerticestoLink.push_back(psi2Vertex);
      else ATH_MSG_WARNING("Could not find linking Jpsi");
      if(!BPhysPVCascadeTools::LinkVertices(Psi2LinksDecor, psi2VerticestoLink, psi2Container, mainVertex)) ATH_MSG_ERROR("Error decorating with Psi2 vertex");

      std::vector<xAOD::Vertex*> psi1VerticestoLink;
      if(psi1Vertex) psi1VerticestoLink.push_back(psi1Vertex);
      else ATH_MSG_WARNING("Could not find linking Psi1");
      if(!BPhysPVCascadeTools::LinkVertices(Psi1LinksDecor, psi1VerticestoLink, psi1Container, mainVertex)) ATH_MSG_ERROR("Error decorating with Psi1 vertex");

      // set hypotheses for output vertices
      for(size_t i=0; i<m_vertexPsi2HypoNames.size(); i++) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsi2HypoNames[i]);
        if(flagAcc.isAvailable(*psi2Vertex) && flagAcc(*psi2Vertex)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+m_vertexPsi2HypoNames[i]);
	  flagDec(*cascadeVertices[1]) = true;
        }
      }

      for(size_t i=0; i<m_vertexPsi1HypoNames.size(); i++) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsi1HypoNames[i]);
        if(flagAcc.isAvailable(*psi1Vertex) && flagAcc(*psi1Vertex)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+m_vertexPsi1HypoNames[i]);
	  flagDec(*cascadeVertices[0]) = true;
        }
      }

      // reset beamspot cache
      helper.GetBeamSpot(true);

      xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);

      // Get refitted track momenta from all vertices, charged tracks only
      BPhysPVCascadeTools::SetVectorInfo(vtx, cascade_info);
      vtx.setPass(true);

      //
      // Decorate main vertex
      //
      // mass, mass error
      // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/VxCascadeInfo.h
      BPHYS_CHECK( vtx.setMass(m_CascadeTools->invariantMass(moms[2])) );
      BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[2],cascade_info->getCovariance()[1])) );
      // pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
      Pt_decor(*mainVertex)       = m_CascadeTools->pT(moms[2]);
      PtErr_decor(*mainVertex)    = m_CascadeTools->pTError(moms[2],cascade_info->getCovariance()[1]);
      // chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
      chi2_decor(*mainVertex)     = cascade_info->fitChi2();
      ndof_decor(*mainVertex)     = cascade_info->nDoF();
      chi2_nc_decor(*mainVertex)  = cascade_info_noConstr ? cascade_info_noConstr->fitChi2() : -999999.;
      ndof_nc_decor(*mainVertex)  = cascade_info_noConstr ? cascade_info_noConstr->nDoF() : -1;

      // decorate the Psi1 vertex
      chi2_SV1_decor(*cascadeVertices[0])    = m_V0Tools->chisq(cascadeVertices[0]);
      chi2_nc_SV1_decor(*cascadeVertices[0]) = cascade_info_noConstr ? m_V0Tools->chisq(cascade_info_noConstr->vertices()[0]) : -999999.;
      chi2_V1_decor(*cascadeVertices[0])     = m_V0Tools->chisq(psi1Vertex);
      ndof_V1_decor(*cascadeVertices[0])     = m_V0Tools->ndof(psi1Vertex);
      lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
      lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
      a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
      a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);

      // decorate the Psi2 vertex
      chi2_SV2_decor(*cascadeVertices[1])    = m_V0Tools->chisq(cascadeVertices[1]);
      chi2_nc_SV2_decor(*cascadeVertices[1]) = cascade_info_noConstr ? m_V0Tools->chisq(cascade_info_noConstr->vertices()[1]) : -999999.;
      chi2_V2_decor(*cascadeVertices[1])     = m_V0Tools->chisq(psi2Vertex);
      ndof_V2_decor(*cascadeVertices[1])     = m_V0Tools->ndof(psi2Vertex);
      lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
      lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
      a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
      a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);

      double Mass_Moth = m_CascadeTools->invariantMass(moms[2]); // size=2
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer, refPvContainer, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info, 2, Mass_Moth, vtx));
    } // loop over cascadeinfoContainer

    for(int i=0; i<topoN; i++){
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandles[i])   , m_cascadeOutputsKeys[i]));
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandlesAux[i]), m_cascadeOutputsKeys[i] + "Aux."));
    }

    // Deleting cascadeinfo since this won't be stored.
    // Vertices have been kept in m_cascadeOutputs and should be owned by their container
    for (auto cascade_info : cascadeinfoContainer) delete cascade_info;
    for (auto cascade_info_noConstr : cascadeinfoContainer_noConstr) delete cascade_info_noConstr;

    return StatusCode::SUCCESS;
  }

  PsiPlusPsiCascade::PsiPlusPsiCascade(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexPsi1ContainerKey(""),
    m_vertexPsi2ContainerKey(""),
    m_cascadeOutputsKeys{ "PsiPlusPsiCascadeVtx1", "PsiPlusPsiCascadeVtx2", "PsiPlusPsiCascadeVtx3" },
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_jpsi1MassLower(0.0),
    m_jpsi1MassUpper(20000.0),
    m_jpsi2MassLower(0.0),
    m_jpsi2MassUpper(20000.0),
    m_diTrack1MassLower(-1.0),
    m_diTrack1MassUpper(-1.0),
    m_diTrack2MassLower(-1.0),
    m_diTrack2MassUpper(-1.0),
    m_psi1MassLower(0.0),
    m_psi1MassUpper(25000.0),
    m_psi2MassLower(0.0),
    m_psi2MassUpper(25000.0),
    m_MassLower(0.0),
    m_MassUpper(31000.0),
    m_vtx1Daug_num(4),
    m_vtx1Daug1MassHypo(-1),
    m_vtx1Daug2MassHypo(-1),
    m_vtx1Daug3MassHypo(-1),
    m_vtx1Daug4MassHypo(-1),
    m_vtx2Daug_num(4),
    m_vtx2Daug1MassHypo(-1),
    m_vtx2Daug2MassHypo(-1),
    m_vtx2Daug3MassHypo(-1),
    m_vtx2Daug4MassHypo(-1),
    m_massPsi1(-1),
    m_massPsi2(-1),
    m_massJpsi1(-1),
    m_massJpsi2(-1),
    m_massDiTrk1(-1),
    m_massDiTrk2(-1),
    m_constrPsi1(false),
    m_constrPsi2(false),
    m_constrJpsi1(false),
    m_constrJpsi2(false),
    m_constrDiTrk1(false),
    m_constrDiTrk2(false),
    m_chi2cut_Psi1(-1.0),
    m_chi2cut_Psi2(-1.0),
    m_chi2cut(-1.0),
    m_removeDuplicatePairs(false),
    m_maxCandidates(0),
    m_beamSpotSvc("BeamCondSvc",name),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_CascadeTools("DerivationFramework::CascadeTools")
  {
    declareProperty("Psi1Vertices",             m_vertexPsi1ContainerKey);
    declareProperty("Psi2Vertices",             m_vertexPsi2ContainerKey);
    declareProperty("Psi1VtxHypoNames",         m_vertexPsi1HypoNames);
    declareProperty("Psi2VtxHypoNames",         m_vertexPsi2HypoNames);
    declareProperty("VxPrimaryCandidateName",   m_VxPrimaryCandidateName);
    declareProperty("RefPVContainerName",       m_refPVContainerName = "RefittedPrimaryVertices");
    declareProperty("Jpsi1MassLowerCut",        m_jpsi1MassLower);
    declareProperty("Jpsi1MassUpperCut",        m_jpsi1MassUpper);
    declareProperty("Jpsi2MassLowerCut",        m_jpsi2MassLower);
    declareProperty("Jpsi2MassUpperCut",        m_jpsi2MassUpper);
    declareProperty("DiTrack1MassLower",        m_diTrack1MassLower); // only effective when m_vtx1Daug_num=4
    declareProperty("DiTrack1MassUpper",        m_diTrack1MassUpper); // only effective when m_vtx1Daug_num=4
    declareProperty("DiTrack2MassLower",        m_diTrack2MassLower); // only effective when m_vtx2Daug_num=4
    declareProperty("DiTrack2MassUpper",        m_diTrack2MassUpper); // only effective when m_vtx2Daug_num=4
    declareProperty("Psi1MassLowerCut",         m_psi1MassLower);
    declareProperty("Psi1MassUpperCut",         m_psi1MassUpper);
    declareProperty("Psi2MassLowerCut",         m_psi2MassLower);
    declareProperty("Psi2MassUpperCut",         m_psi2MassUpper);
    declareProperty("MassLowerCut",             m_MassLower);
    declareProperty("MassUpperCut",             m_MassUpper);
    declareProperty("HypothesisName",           m_hypoName = "TQ");
    declareProperty("NumberOfPsi1Daughters",    m_vtx1Daug_num); // 3 or 4 only
    declareProperty("Vtx1Daug1MassHypo",        m_vtx1Daug1MassHypo);
    declareProperty("Vtx1Daug2MassHypo",        m_vtx1Daug2MassHypo);
    declareProperty("Vtx1Daug3MassHypo",        m_vtx1Daug3MassHypo);
    declareProperty("Vtx1Daug4MassHypo",        m_vtx1Daug4MassHypo);
    declareProperty("NumberOfPsi2Daughters",    m_vtx2Daug_num); // 3 or 4 only
    declareProperty("Vtx2Daug1MassHypo",        m_vtx2Daug1MassHypo);
    declareProperty("Vtx2Daug2MassHypo",        m_vtx2Daug2MassHypo);
    declareProperty("Vtx2Daug3MassHypo",        m_vtx2Daug3MassHypo);
    declareProperty("Vtx2Daug4MassHypo",        m_vtx2Daug4MassHypo);
    declareProperty("Jpsi1Mass",                m_massJpsi1);
    declareProperty("Jpsi2Mass",                m_massJpsi2);
    declareProperty("DiTrack1Mass",             m_massDiTrk1);
    declareProperty("DiTrack2Mass",             m_massDiTrk2);
    declareProperty("Psi1Mass",                 m_massPsi1);
    declareProperty("Psi2Mass",                 m_massPsi2);
    declareProperty("ApplyPsi1MassConstraint",  m_constrPsi1);
    declareProperty("ApplyPsi2MassConstraint",  m_constrPsi2);
    declareProperty("ApplyJpsi1MassConstraint", m_constrJpsi1);
    declareProperty("ApplyJpsi2MassConstraint", m_constrJpsi2);
    declareProperty("ApplyDiTrk1MassConstraint",m_constrDiTrk1); // only effective when m_vtx1Daug_num=4
    declareProperty("ApplyDiTrk2MassConstraint",m_constrDiTrk2); // only effective when m_vtx2Daug_num=4
    declareProperty("Chi2CutPsi1",              m_chi2cut_Psi1);
    declareProperty("Chi2CutPsi2",              m_chi2cut_Psi2);
    declareProperty("Chi2Cut",                  m_chi2cut);
    declareProperty("RemoveDuplicatePairs",     m_removeDuplicatePairs); // only effective when m_vertexPsi1ContainerKey == m_vertexPsi2ContainerKey
    declareProperty("MaxCandidates",            m_maxCandidates);
    declareProperty("RefitPV",                  m_refitPV         = true);
    declareProperty("MaxnPV",                   m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",           m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",             m_DoVertexType    = 7);
    declareProperty("TrkVertexFitterTool",      m_iVertexFitter);
    declareProperty("PVRefitter",               m_pvRefitter);
    declareProperty("V0Tools",                  m_V0Tools);
    declareProperty("CascadeTools",             m_CascadeTools);
    declareProperty("CascadeVertexCollections", m_cascadeOutputsKeys);
  }

  StatusCode PsiPlusPsiCascade::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer_noConstr) const {
    ATH_MSG_DEBUG( "PsiPlusPsiCascade::performSearch" );
    assert(cascadeinfoContainer!=nullptr && cascadeinfoContainer_noConstr!=nullptr);

    // Get TrackParticle container (for setting links to the original tracks)
    const xAOD::TrackParticleContainer  *trackContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(trackContainer, "InDetTrackParticles"));

    std::vector<const xAOD::TrackParticle*> tracksJpsi1;
    std::vector<const xAOD::TrackParticle*> tracksJpsi2;
    std::vector<const xAOD::TrackParticle*> tracksDiTrk1;
    std::vector<const xAOD::TrackParticle*> tracksDiTrk2;
    std::vector<const xAOD::TrackParticle*> tracksPsi1;
    std::vector<const xAOD::TrackParticle*> tracksPsi2;
    std::vector<double> massesPsi1;
    massesPsi1.push_back(m_vtx1Daug1MassHypo);
    massesPsi1.push_back(m_vtx1Daug2MassHypo);
    massesPsi1.push_back(m_vtx1Daug3MassHypo);
    if(m_vtx1Daug_num==4) massesPsi1.push_back(m_vtx1Daug4MassHypo);
    std::vector<double> massesPsi2;
    massesPsi2.push_back(m_vtx2Daug1MassHypo);
    massesPsi2.push_back(m_vtx2Daug2MassHypo);
    massesPsi2.push_back(m_vtx2Daug3MassHypo);
    if(m_vtx2Daug_num==4) massesPsi2.push_back(m_vtx2Daug4MassHypo);
 
    // Get Psi1 container
    const xAOD::VertexContainer *psi1Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(psi1Container, m_vertexPsi1ContainerKey));

    // Get Psi2 container
    const xAOD::VertexContainer *psi2Container(nullptr);
    if(m_vertexPsi2ContainerKey == m_vertexPsi1ContainerKey) psi2Container = psi1Container;
    else ATH_CHECK(evtStore()->retrieve(psi2Container, m_vertexPsi2ContainerKey));

    // Select the Psi2 candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedPsi2Candidates;
    for(auto vxcItr=psi2Container->cbegin(); vxcItr!=psi2Container->cend(); ++vxcItr) {
      // Check the passed flag first
      xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexPsi2HypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsi2HypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexPsi2HypoNames.size() && !passed) continue;

      // Check Psi2 candidate invariant mass and skip if need be
      double mass_psi2 = m_V0Tools->invariantMass(*vxcItr, massesPsi2);
      if (mass_psi2 < m_psi2MassLower || mass_psi2 > m_psi2MassUpper) continue;

      // Add loose cut on Jpsi2 mass from Psi2 -> Jpsi2 pi+ pi-, or on phi mass from Ds+ -> phi pi+
      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( (*vxcItr)->trackParticle(0)->pt(), 
			   (*vxcItr)->trackParticle(0)->eta(),
			   (*vxcItr)->trackParticle(0)->phi(), m_vtx2Daug1MassHypo);
      p4_mu2.SetPtEtaPhiM( (*vxcItr)->trackParticle(1)->pt(), 
			   (*vxcItr)->trackParticle(1)->eta(),
			   (*vxcItr)->trackParticle(1)->phi(), m_vtx2Daug2MassHypo);
      double mass_jpsi2 = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi2 < m_jpsi2MassLower || mass_jpsi2 > m_jpsi2MassUpper) continue;

      if(m_vtx2Daug_num==4 && m_diTrack2MassLower>=0 && m_diTrack2MassUpper>=0) {
	TLorentzVector p4_trk1, p4_trk2;
	p4_trk1.SetPtEtaPhiM( (*vxcItr)->trackParticle(2)->pt(),
			      (*vxcItr)->trackParticle(2)->eta(),
			      (*vxcItr)->trackParticle(2)->phi(), m_vtx2Daug3MassHypo);
	p4_trk2.SetPtEtaPhiM( (*vxcItr)->trackParticle(3)->pt(),
			      (*vxcItr)->trackParticle(3)->eta(),
			      (*vxcItr)->trackParticle(3)->phi(), m_vtx2Daug4MassHypo);
	double mass_diTrk2 = (p4_trk1 + p4_trk2).M();
	if (mass_diTrk2 < m_diTrack2MassLower || mass_diTrk2 > m_diTrack2MassUpper) continue;
      }

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_Psi2>0 && chi2DOF>m_chi2cut_Psi2) continue;

      selectedPsi2Candidates.push_back(*vxcItr);
    }
    if(selectedPsi2Candidates.size()==0) return StatusCode::SUCCESS;

    // Select the Psi1 candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedPsi1Candidates;
    for(auto vxcItr=psi1Container->cbegin(); vxcItr!=psi1Container->cend(); ++vxcItr) {
      // Check the passed flag first
      xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexPsi1HypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsi1HypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexPsi1HypoNames.size() && !passed) continue;

      // Check Psi candidate invariant mass and skip if need be
      double mass_psi1 = m_V0Tools->invariantMass(*vxcItr,massesPsi1);
      if(mass_psi1 < m_psi1MassLower || mass_psi1 > m_psi1MassUpper) continue;

      // Add loose cut on Jpsi1 mass from Psi1 -> Jpsi1 pi+ pi-, or on phi mass from Ds+ -> phi pi+
      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( (*vxcItr)->trackParticle(0)->pt(), 
			   (*vxcItr)->trackParticle(0)->eta(),
			   (*vxcItr)->trackParticle(0)->phi(), m_vtx1Daug1MassHypo);
      p4_mu2.SetPtEtaPhiM( (*vxcItr)->trackParticle(1)->pt(), 
			   (*vxcItr)->trackParticle(1)->eta(),
			   (*vxcItr)->trackParticle(1)->phi(), m_vtx1Daug2MassHypo);
      double mass_jpsi1 = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi1 < m_jpsi1MassLower || mass_jpsi1 > m_jpsi1MassUpper) continue;

      if(m_vtx1Daug_num==4 && m_diTrack1MassLower>=0 && m_diTrack1MassUpper>=0) {
	TLorentzVector p4_trk1, p4_trk2;
	p4_trk1.SetPtEtaPhiM( (*vxcItr)->trackParticle(2)->pt(),
			      (*vxcItr)->trackParticle(2)->eta(),
			      (*vxcItr)->trackParticle(2)->phi(), m_vtx1Daug3MassHypo);
	p4_trk2.SetPtEtaPhiM( (*vxcItr)->trackParticle(3)->pt(),
			      (*vxcItr)->trackParticle(3)->eta(),
			      (*vxcItr)->trackParticle(3)->phi(), m_vtx1Daug4MassHypo);
	double mass_diTrk1 = (p4_trk1 + p4_trk2).M();
	if (mass_diTrk1 < m_diTrack1MassLower || mass_diTrk1 > m_diTrack1MassUpper) continue;
      }

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_Psi1>0 && chi2DOF>m_chi2cut_Psi1) continue;

      selectedPsi1Candidates.push_back(*vxcItr);
    }
    if(selectedPsi1Candidates.size()==0) return StatusCode::SUCCESS;

    std::vector<std::pair<const xAOD::Vertex*, const xAOD::Vertex*> > candidatePairs;
    for(auto psi1Itr=selectedPsi1Candidates.cbegin(); psi1Itr!=selectedPsi1Candidates.cend(); ++psi1Itr) {
      tracksPsi1.clear();
      for(size_t i=0; i<(*psi1Itr)->nTrackParticles(); i++) tracksPsi1.push_back((*psi1Itr)->trackParticle(i));
      for(auto psi2Itr=selectedPsi2Candidates.cbegin(); psi2Itr!=selectedPsi2Candidates.cend(); ++psi2Itr) {
	bool skip = false;
	for(size_t j=0; j<(*psi2Itr)->nTrackParticles(); j++) {
	  if(std::find(tracksPsi1.cbegin(), tracksPsi1.cend(), (*psi2Itr)->trackParticle(j)) != tracksPsi1.cend()) { skip = true; break; }
	}
	if(skip) continue;
	if(m_vertexPsi1ContainerKey == m_vertexPsi2ContainerKey) {
	  for(size_t ic=0; ic<candidatePairs.size(); ic++) {
	    const xAOD::Vertex* psi1Vertex = candidatePairs[ic].first;
	    const xAOD::Vertex* psi2Vertex = candidatePairs[ic].second;
	    if((psi1Vertex == *psi1Itr && psi2Vertex == *psi2Itr) || (psi1Vertex == *psi2Itr && psi2Vertex == *psi1Itr)) { skip = true; break; }
	  }
	}
	if(skip) continue;
	candidatePairs.push_back(std::pair<const xAOD::Vertex*, const xAOD::Vertex*>(*psi1Itr,*psi2Itr));
      }
    }

    std::sort( candidatePairs.begin(), candidatePairs.end(), [](std::pair<const xAOD::Vertex*, const xAOD::Vertex*> a, std::pair<const xAOD::Vertex*, const xAOD::Vertex*> b) { return a.first->chiSquared()/a.first->numberDoF()+a.second->chiSquared()/a.second->numberDoF() < b.first->chiSquared()/b.first->numberDoF()+b.second->chiSquared()/b.second->numberDoF(); } );
    if(m_maxCandidates>0 && candidatePairs.size()>m_maxCandidates) {
      candidatePairs.erase(candidatePairs.begin()+m_maxCandidates, candidatePairs.end());
    }

    for(size_t ic=0; ic<candidatePairs.size(); ic++) {
      const xAOD::Vertex* psi1Vertex = candidatePairs[ic].first;
      const xAOD::Vertex* psi2Vertex = candidatePairs[ic].second;

      tracksPsi1.clear();
      for(size_t it=0; it<psi1Vertex->nTrackParticles(); it++) tracksPsi1.push_back(psi1Vertex->trackParticle(it));
      if (tracksPsi1.size() != massesPsi1.size()) {
	ATH_MSG_ERROR("Problems with Psi1 input: number of tracks or track mass inputs is not correct!");
      }
      tracksPsi2.clear();
      for(size_t it=0; it<psi2Vertex->nTrackParticles(); it++) tracksPsi2.push_back(psi2Vertex->trackParticle(it));
      if (tracksPsi2.size() != massesPsi2.size()) {
	ATH_MSG_ERROR("Problems with Psi2 input: number of tracks or track mass inputs is not correct!");
      }

      tracksJpsi1.clear();
      tracksJpsi1.push_back(psi1Vertex->trackParticle(0));
      tracksJpsi1.push_back(psi1Vertex->trackParticle(1));
      tracksDiTrk1.clear();
      if(m_vtx1Daug_num==4) {
	tracksDiTrk1.push_back(psi1Vertex->trackParticle(2));
	tracksDiTrk1.push_back(psi1Vertex->trackParticle(3));
      }
      tracksJpsi2.clear();
      tracksJpsi2.push_back(psi2Vertex->trackParticle(0));
      tracksJpsi2.push_back(psi2Vertex->trackParticle(1));
      tracksDiTrk2.clear();
      if(m_vtx2Daug_num==4) {
	tracksDiTrk2.push_back(psi2Vertex->trackParticle(2));
	tracksDiTrk2.push_back(psi2Vertex->trackParticle(3));
      }

      TLorentzVector p4_moth;
      TLorentzVector tmp;
      for(size_t it=0; it<psi1Vertex->nTrackParticles(); it++) {
	tmp.SetPtEtaPhiM(psi1Vertex->trackParticle(it)->pt(),psi1Vertex->trackParticle(it)->eta(),psi1Vertex->trackParticle(it)->phi(),massesPsi1[it]);
	p4_moth += tmp;
      }
      for(size_t it=0; it<psi2Vertex->nTrackParticles(); it++) {
	tmp.SetPtEtaPhiM(psi2Vertex->trackParticle(it)->pt(),psi2Vertex->trackParticle(it)->eta(),psi2Vertex->trackParticle(it)->phi(),massesPsi2[it]);
	p4_moth += tmp;
      }
      if (p4_moth.M() < m_MassLower || p4_moth.M() > m_MassUpper) continue;
	
      // Apply the user's settings to the fitter
      // Reset
      m_iVertexFitter->setDefault();
      // Robustness: http://cdsweb.cern.ch/record/685551
      int robustness = 0;
      m_iVertexFitter->setRobustness(robustness);
      // Build up the topology
      // Vertex list
      std::vector<Trk::VertexID> vrtList;
      // Psi1 vertex
      Trk::VertexID vID1;
      // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
      if (m_constrPsi1) {
	vID1 = m_iVertexFitter->startVertex(tracksPsi1,massesPsi1,m_massPsi1);
      } else {
	vID1 = m_iVertexFitter->startVertex(tracksPsi1,massesPsi1);
      }
      vrtList.push_back(vID1);
      // Psi2 vertex
      Trk::VertexID vID2;
      if (m_constrPsi2) {
	vID2 = m_iVertexFitter->nextVertex(tracksPsi2,massesPsi2,m_massPsi2);
      } else {
	vID2 = m_iVertexFitter->nextVertex(tracksPsi2,massesPsi2);
      }
      vrtList.push_back(vID2);
      // Mother vertex including Psi1 and Psi2
      std::vector<const xAOD::TrackParticle*> tp; tp.clear();
      std::vector<double> tp_masses; tp_masses.clear();
      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
      if (m_constrJpsi1) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID1,tracksJpsi1,cnstV,m_massJpsi1).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for Jpsi1 failed");
	}
      }
      if (m_constrDiTrk1 && m_vtx1Daug_num==4 && m_massDiTrk1>0) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID1,tracksDiTrk1,cnstV,m_massDiTrk1).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for DiTrk1 failed");
	}
      }
      if (m_constrJpsi2) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJpsi2,cnstV,m_massJpsi2).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for Jpsi2 failed");
	}
      }
      if (m_constrDiTrk2 && m_vtx2Daug_num==4 && m_massDiTrk2>0) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID2,tracksDiTrk2,cnstV,m_massDiTrk2).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for DiTrk2 failed");
	}
      }
      // Do the work
      std::unique_ptr<Trk::VxCascadeInfo> result(m_iVertexFitter->fitCascade());

      bool pass = false;
      if (result != nullptr) {
	for(auto v : result->vertices()) {
	  if(v->nTrackParticles()==0) {
	    std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
	    v->setTrackParticleLinks(nullLinkVector);
	  }
	}
	// reset links to original tracks
	BPhysPVCascadeTools::PrepareVertexLinks(result.get(), trackContainer);

	// necessary to prevent memory leak
	result->getSVOwnership(true);

	// Chi2/DOF cut
	double chi2DOF = result->fitChi2()/result->nDoF();
	bool chi2CutPassed = (m_chi2cut <= 0.0 || chi2DOF < m_chi2cut);

	if(chi2CutPassed) {
	  cascadeinfoContainer->push_back(result.release());
	  pass = true;
	}
      }

      // do cascade fit again without any mass constraints
      if(pass) {
	if(m_constrJpsi1 || m_constrPsi1 || m_constrJpsi2 || m_constrPsi2 || (m_constrDiTrk1 && m_vtx1Daug_num==4 && m_massDiTrk1>0) || (m_constrDiTrk2 && m_vtx2Daug_num==4 && m_massDiTrk2>0)) {
	  m_iVertexFitter->setDefault();
	  m_iVertexFitter->setRobustness(robustness);
	  std::vector<Trk::VertexID> vrtList_nc;
	  // Psi1 vertex
	  Trk::VertexID vID1_nc = m_iVertexFitter->startVertex(tracksPsi1,massesPsi1);
	  vrtList_nc.push_back(vID1_nc);
	  // Psi2 vertex
	  Trk::VertexID vID2_nc = m_iVertexFitter->nextVertex(tracksPsi2,massesPsi2);
	  vrtList_nc.push_back(vID2_nc);
	  // Mother vertex including Psi1 and Psi2
	  std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	  std::vector<double> tp_masses; tp_masses.clear();
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList_nc);
	  // Do the work
	  std::unique_ptr<Trk::VxCascadeInfo> result_nc(m_iVertexFitter->fitCascade());

	  if (result_nc != nullptr) {
	    for(auto v : result_nc->vertices()) {
	      if(v->nTrackParticles()==0) {
		std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
		v->setTrackParticleLinks(nullLinkVector);
	      }
	    }
	    // reset links to original tracks
	    BPhysPVCascadeTools::PrepareVertexLinks(result_nc.get(), trackContainer);

	    // necessary to prevent memory leak
	    result_nc->getSVOwnership(true);
	    cascadeinfoContainer_noConstr->push_back(result_nc.release());
	  }
	  else cascadeinfoContainer_noConstr->push_back(0);
	}
	else cascadeinfoContainer_noConstr->push_back(0);
      }
    } //Iterate over candidatePairs

    return StatusCode::SUCCESS;
  }
}
