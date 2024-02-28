/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// JpsiPlusPsiCascade.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/JpsiPlusPsiCascade.h"
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

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;

  StatusCode JpsiPlusPsiCascade::initialize() {
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
    if(m_mass_jpsi < 0.) m_mass_jpsi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_mass_jpsi2 < 0.) m_mass_jpsi2 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_mass_psi < 0.) m_mass_psi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::psi_2S);

    if(m_vtx1Daug1MassHypo < 0.) m_vtx1Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx1Daug2MassHypo < 0.) m_vtx1Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx1Daug3MassHypo < 0.) m_vtx1Daug3MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx1Daug4MassHypo < 0.) m_vtx1Daug4MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx2Daug1MassHypo < 0.) m_vtx2Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx2Daug2MassHypo < 0.) m_vtx2Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiPlusPsiCascade::addBranches() const {
    constexpr int topoN = 3;
    std::array<std::unique_ptr<xAOD::VertexContainer>, topoN> VtxWriteHandles;
    std::array<std::unique_ptr<xAOD::VertexAuxContainer>, topoN> VtxWriteHandlesAux;
    if(m_cascadeOutputsKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of VtxContainers");
      return StatusCode::FAILURE;
    }
    
    if (m_vtx1Daug_num != 3 && m_vtx1Daug_num != 4) {
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

    // Decorators for the vertices
    SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks"); 
    SG::AuxElement::Decorator<VertexLinkVector> PsiLinksDecor("PsiVertexLinks"); 
    SG::AuxElement::Decorator<VertexLinkVector> JpsiLinksDecor("JpsiVertexLinks"); 
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

    // Get the containers and identify the input Jpsi and Psi
    const xAOD::VertexContainer *psiContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(psiContainer, m_vertexPsiContainerKey));
    const xAOD::VertexContainer *jpsiContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jpsiContainer, m_vertexContainerKey));

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

      // Identify the input Jpsi
      xAOD::Vertex* jpsiVertex = BPhysPVCascadeTools::FindVertex<2>(jpsiContainer, cascadeVertices[1]);
      // Identify the input Psi
      xAOD::Vertex* psiVertex(0);
      if(m_vtx1Daug_num==4) psiVertex = BPhysPVCascadeTools::FindVertex<4>(psiContainer, cascadeVertices[0]);
      else psiVertex = BPhysPVCascadeTools::FindVertex<3>(psiContainer, cascadeVertices[0]);

      // Set links to input vertices
      std::vector<xAOD::Vertex*> jpsiVerticestoLink;
      if(jpsiVertex) jpsiVerticestoLink.push_back(jpsiVertex);
      else ATH_MSG_WARNING("Could not find linking Jpsi");
      if(!BPhysPVCascadeTools::LinkVertices(JpsiLinksDecor, jpsiVerticestoLink, jpsiContainer, mainVertex)) ATH_MSG_ERROR("Error decorating with Jpsi vertex");

      std::vector<xAOD::Vertex*> psiVerticestoLink;
      if(psiVertex) psiVerticestoLink.push_back(psiVertex);
      else ATH_MSG_WARNING("Could not find linking Psi");
      if(!BPhysPVCascadeTools::LinkVertices(PsiLinksDecor, psiVerticestoLink, psiContainer, mainVertex)) ATH_MSG_ERROR("Error decorating with Psi vertex");

      // set hypotheses for output vertices
      for(size_t i=0; i<m_vertexJpsiHypoNames.size(); i++) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexJpsiHypoNames[i]);
        if(flagAcc.isAvailable(*jpsiVertex) && flagAcc(*jpsiVertex)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+m_vertexJpsiHypoNames[i]);
	  flagDec(*cascadeVertices[1]) = true;
        }
      }

      for(size_t i=0; i<m_vertexPsiHypoNames.size(); i++) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsiHypoNames[i]);
        if(flagAcc.isAvailable(*psiVertex) && flagAcc(*psiVertex)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+m_vertexPsiHypoNames[i]);
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
      BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[2],cascade_info->getCovariance()[2])) );
      // pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
      Pt_decor(*mainVertex)       = m_CascadeTools->pT(moms[2]);
      PtErr_decor(*mainVertex)    = m_CascadeTools->pTError(moms[2],cascade_info->getCovariance()[2]);
      // chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
      chi2_decor(*mainVertex)     = cascade_info->fitChi2();
      ndof_decor(*mainVertex)     = cascade_info->nDoF();
      chi2_nc_decor(*mainVertex)  = cascade_info_noConstr ? cascade_info_noConstr->fitChi2() : -999999.;
      ndof_nc_decor(*mainVertex)  = cascade_info_noConstr ? cascade_info_noConstr->nDoF() : -1;

      // decorate the Psi vertex
      chi2_SV1_decor(*cascadeVertices[0])    = m_V0Tools->chisq(cascadeVertices[0]);
      chi2_nc_SV1_decor(*cascadeVertices[0]) = cascade_info_noConstr ? m_V0Tools->chisq(cascade_info_noConstr->vertices()[0]) : -999999.;
      chi2_V1_decor(*cascadeVertices[0])     = m_V0Tools->chisq(psiVertex);
      ndof_V1_decor(*cascadeVertices[0])     = m_V0Tools->ndof(psiVertex);
      lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
      lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
      a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
      a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);

      // decorate the Jpsi vertex
      chi2_SV2_decor(*cascadeVertices[1])    = m_V0Tools->chisq(cascadeVertices[1]);
      chi2_nc_SV2_decor(*cascadeVertices[1]) = cascade_info_noConstr ? m_V0Tools->chisq(cascade_info_noConstr->vertices()[1]) : -999999.;
      chi2_V2_decor(*cascadeVertices[1])     = m_V0Tools->chisq(jpsiVertex);
      ndof_V2_decor(*cascadeVertices[1])     = m_V0Tools->ndof(jpsiVertex);
      lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
      lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
      a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
      a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);

      double Mass_Moth = m_CascadeTools->invariantMass(moms[2]); //size=2
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer, refPvContainer, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info, 2, Mass_Moth, vtx));
    }

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

  JpsiPlusPsiCascade::JpsiPlusPsiCascade(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexContainerKey(""),
    m_vertexPsiContainerKey(""),
    m_cascadeOutputsKeys{ "JpsiPlusPsiCascadeVtx1", "JpsiPlusPsiCascadeVtx2", "JpsiPlusPsiCascadeVtx3" },
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(20000.0),
    m_diTrackMassLower(-1.0),
    m_diTrackMassUpper(-1.0),
    m_psiMassLower(0.0),
    m_psiMassUpper(25000.0),
    m_jpsi2MassLower(0.0),
    m_jpsi2MassUpper(20000.0),
    m_MassLower(0.0),
    m_MassUpper(31000.0),
    m_vtx1Daug_num(4),
    m_vtx1Daug1MassHypo(-1),
    m_vtx1Daug2MassHypo(-1),
    m_vtx1Daug3MassHypo(-1),
    m_vtx1Daug4MassHypo(-1),
    m_vtx2Daug1MassHypo(-1),
    m_vtx2Daug2MassHypo(-1),
    m_mass_jpsi(-1),
    m_mass_diTrk(-1),
    m_mass_psi(-1),
    m_mass_jpsi2(-1),
    m_constrPsi(false),
    m_constrJpsi(false),
    m_constrDiTrk(false),
    m_constrJpsi2(false),
    m_chi2cut_Psi(-1.0),
    m_chi2cut_Jpsi(-1.0),
    m_chi2cut(-1.0),
    m_maxCandidates(0),
    m_beamSpotSvc("BeamCondSvc",name),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_CascadeTools("DerivationFramework::CascadeTools")
  {
    declareProperty("JpsiVertices",               m_vertexContainerKey);
    declareProperty("PsiVertices",                m_vertexPsiContainerKey);
    declareProperty("JpsiVtxHypoNames",           m_vertexJpsiHypoNames);
    declareProperty("PsiVtxHypoNames",            m_vertexPsiHypoNames);
    declareProperty("VxPrimaryCandidateName",     m_VxPrimaryCandidateName);
    declareProperty("RefPVContainerName",         m_refPVContainerName = "RefittedPrimaryVertices");
    declareProperty("JpsiMassLowerCut",           m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",           m_jpsiMassUpper);
    declareProperty("DiTrackMassLower",           m_diTrackMassLower); // only effective when m_vtx1Daug_num=4
    declareProperty("DiTrackMassUpper",           m_diTrackMassUpper); // only effective when m_vtx1Daug_num=4
    declareProperty("PsiMassLowerCut",            m_psiMassLower);
    declareProperty("PsiMassUpperCut",            m_psiMassUpper);
    declareProperty("Jpsi2MassLowerCut",          m_jpsi2MassLower);
    declareProperty("Jpsi2MassUpperCut",          m_jpsi2MassUpper);
    declareProperty("MassLowerCut",               m_MassLower);
    declareProperty("MassUpperCut",               m_MassUpper);
    declareProperty("HypothesisName",             m_hypoName = "TQ");
    declareProperty("NumberOfPsiDaughters",       m_vtx1Daug_num); // 3 or 4 only
    declareProperty("Vtx1Daug1MassHypo",          m_vtx1Daug1MassHypo);
    declareProperty("Vtx1Daug2MassHypo",          m_vtx1Daug2MassHypo);
    declareProperty("Vtx1Daug3MassHypo",          m_vtx1Daug3MassHypo);
    declareProperty("Vtx1Daug4MassHypo",          m_vtx1Daug4MassHypo);
    declareProperty("Vtx2Daug1MassHypo",          m_vtx2Daug1MassHypo);
    declareProperty("Vtx2Daug2MassHypo",          m_vtx2Daug2MassHypo);
    declareProperty("JpsiMass",                   m_mass_jpsi);
    declareProperty("DiTrackMass",                m_mass_diTrk);
    declareProperty("PsiMass",                    m_mass_psi);
    declareProperty("Jpsi2Mass",                  m_mass_jpsi2);
    declareProperty("ApplyJpsiMassConstraint",    m_constrJpsi);
    declareProperty("ApplyDiTrackMassConstraint", m_constrDiTrk); // only effective when m_vtx1Daug_num=4
    declareProperty("ApplyPsiMassConstraint",     m_constrPsi);
    declareProperty("ApplyJpsi2MassConstraint",   m_constrJpsi2);
    declareProperty("Chi2CutPsi",                 m_chi2cut_Psi);
    declareProperty("Chi2CutJpsi",                m_chi2cut_Jpsi);
    declareProperty("Chi2Cut",                    m_chi2cut);
    declareProperty("MaxCandidates",              m_maxCandidates);
    declareProperty("RefitPV",                    m_refitPV         = true);
    declareProperty("MaxnPV",                     m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",             m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",               m_DoVertexType    = 7);
    declareProperty("TrkVertexFitterTool",        m_iVertexFitter);
    declareProperty("PVRefitter",                 m_pvRefitter);
    declareProperty("V0Tools",                    m_V0Tools);
    declareProperty("CascadeTools",               m_CascadeTools);
    declareProperty("CascadeVertexCollections",   m_cascadeOutputsKeys);
  }

  StatusCode JpsiPlusPsiCascade::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer_noConstr) const {
    ATH_MSG_DEBUG( "JpsiPlusPsiCascade::performSearch" );
    assert(cascadeinfoContainer!=nullptr && cascadeinfoContainer_noConstr!=nullptr);

    // Get TrackParticle container (for setting links to the original tracks)
    const xAOD::TrackParticleContainer  *trackContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(trackContainer, "InDetTrackParticles"));

    std::vector<const xAOD::TrackParticle*> tracksJpsi;
    std::vector<const xAOD::TrackParticle*> tracksDiTrk;
    std::vector<const xAOD::TrackParticle*> tracksPsi;
    std::vector<const xAOD::TrackParticle*> tracksJpsi2;
    std::vector<double> massesPsi;
    massesPsi.push_back(m_vtx1Daug1MassHypo);
    massesPsi.push_back(m_vtx1Daug2MassHypo);
    massesPsi.push_back(m_vtx1Daug3MassHypo);
    if(m_vtx1Daug_num==4) massesPsi.push_back(m_vtx1Daug4MassHypo);
    std::vector<double> massesJpsi2;
    massesJpsi2.push_back(m_vtx2Daug1MassHypo);
    massesJpsi2.push_back(m_vtx2Daug2MassHypo);

    // Get Psi container
    const xAOD::VertexContainer  *psiContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(psiContainer, m_vertexPsiContainerKey));

    // Get Jpsi container
    const xAOD::VertexContainer  *jpsiContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jpsiContainer, m_vertexContainerKey));

    // Select the J/psi candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedJpsiCandidates;
    for(auto vxcItr=jpsiContainer->cbegin(); vxcItr!=jpsiContainer->cend(); ++vxcItr) {
      // Check the passed flag first
      xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexJpsiHypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexJpsiHypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexJpsiHypoNames.size() && !passed) continue;

      // Check Jpsi candidate invariant mass and skip if need be
      double mass_jpsi2 = m_V0Tools->invariantMass(*vxcItr, massesJpsi2);
      if (mass_jpsi2 < m_jpsi2MassLower || mass_jpsi2 > m_jpsi2MassUpper) continue;

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_Jpsi>0 && chi2DOF>m_chi2cut_Jpsi) continue;

      selectedJpsiCandidates.push_back(*vxcItr);
    }
    if(selectedJpsiCandidates.size()==0) return StatusCode::SUCCESS;

    // Select the Psi candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedPsiCandidates;
    for(auto vxcItr=psiContainer->cbegin(); vxcItr!=psiContainer->cend(); ++vxcItr) {
      // Check the passed flag first
      xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexPsiHypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsiHypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexPsiHypoNames.size() && !passed) continue;

      // Check Psi candidate invariant mass and skip if need be
      double mass_psi = m_V0Tools->invariantMass(*vxcItr,massesPsi);
      if(mass_psi < m_psiMassLower || mass_psi > m_psiMassUpper) continue;

      // Add loose cut on Jpsi mass from Psi -> Jpsi pi+ pi-, or on phi mass from Ds+ -> phi pi+
      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( (*vxcItr)->trackParticle(0)->pt(),
			   (*vxcItr)->trackParticle(0)->eta(),
			   (*vxcItr)->trackParticle(0)->phi(), m_vtx1Daug1MassHypo);
      p4_mu2.SetPtEtaPhiM( (*vxcItr)->trackParticle(1)->pt(),
			   (*vxcItr)->trackParticle(1)->eta(),
			   (*vxcItr)->trackParticle(1)->phi(), m_vtx1Daug2MassHypo);
      double mass_jpsi = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi < m_jpsiMassLower || mass_jpsi > m_jpsiMassUpper) continue;

      if(m_vtx1Daug_num==4 && m_diTrackMassLower>=0 && m_diTrackMassUpper>=0) {
	TLorentzVector p4_trk1, p4_trk2;
	p4_trk1.SetPtEtaPhiM( (*vxcItr)->trackParticle(2)->pt(),
			      (*vxcItr)->trackParticle(2)->eta(),
			      (*vxcItr)->trackParticle(2)->phi(), m_vtx1Daug3MassHypo);
	p4_trk2.SetPtEtaPhiM( (*vxcItr)->trackParticle(3)->pt(),
			      (*vxcItr)->trackParticle(3)->eta(),
			      (*vxcItr)->trackParticle(3)->phi(), m_vtx1Daug4MassHypo);
	double mass_diTrk = (p4_trk1 + p4_trk2).M();
	if (mass_diTrk < m_diTrackMassLower || mass_diTrk > m_diTrackMassUpper) continue;
      }

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_Psi>0 && chi2DOF>m_chi2cut_Psi) continue;

      selectedPsiCandidates.push_back(*vxcItr);
    }
    if(selectedPsiCandidates.size()==0) return StatusCode::SUCCESS;

    std::vector<std::pair<const xAOD::Vertex*, const xAOD::Vertex*> > candidatePairs;
    for(auto jpsiItr=selectedJpsiCandidates.cbegin(); jpsiItr!=selectedJpsiCandidates.cend(); ++jpsiItr) {
      tracksJpsi2.clear();
      for(size_t i=0; i<(*jpsiItr)->nTrackParticles(); i++) tracksJpsi2.push_back((*jpsiItr)->trackParticle(i));
      for(auto psiItr=selectedPsiCandidates.cbegin(); psiItr!=selectedPsiCandidates.cend(); ++psiItr) {
	bool skip = false;
	for(size_t j=0; j<(*psiItr)->nTrackParticles(); j++) {
	  if(std::find(tracksJpsi2.cbegin(), tracksJpsi2.cend(), (*psiItr)->trackParticle(j)) != tracksJpsi2.cend()) { skip = true; break; }
	}
	if(skip) continue;
	candidatePairs.push_back(std::pair<const xAOD::Vertex*, const xAOD::Vertex*>(*jpsiItr,*psiItr));
      }
    }

    std::sort( candidatePairs.begin(), candidatePairs.end(), [](std::pair<const xAOD::Vertex*, const xAOD::Vertex*> a, std::pair<const xAOD::Vertex*, const xAOD::Vertex*> b) { return a.first->chiSquared()/a.first->numberDoF()+a.second->chiSquared()/a.second->numberDoF() < b.first->chiSquared()/b.first->numberDoF()+b.second->chiSquared()/b.second->numberDoF(); } );
    if(m_maxCandidates>0 && candidatePairs.size()>m_maxCandidates) {
      candidatePairs.erase(candidatePairs.begin()+m_maxCandidates, candidatePairs.end());
    }

    for(size_t ic=0; ic<candidatePairs.size(); ic++) {
      const xAOD::Vertex* jpsiVertex = candidatePairs[ic].first;
      const xAOD::Vertex* psiVertex = candidatePairs[ic].second;

      tracksJpsi2.clear();
      for(size_t it=0; it<jpsiVertex->nTrackParticles(); it++) tracksJpsi2.push_back(jpsiVertex->trackParticle(it));
      if (tracksJpsi2.size() != 2 || massesJpsi2.size() != 2) {
	ATH_MSG_ERROR("Problems with Jpsi input: number of tracks or track mass inputs is not 2!");
      }
      tracksPsi.clear();
      for(size_t it=0; it<psiVertex->nTrackParticles(); it++) tracksPsi.push_back(psiVertex->trackParticle(it));
      if (tracksPsi.size() != massesPsi.size()) {
	ATH_MSG_ERROR("Problems with Psi input: number of tracks or track mass inputs is not correct!");
      }
      
      tracksJpsi.clear();
      tracksJpsi.push_back(psiVertex->trackParticle(0));
      tracksJpsi.push_back(psiVertex->trackParticle(1));
      tracksDiTrk.clear();
      if(m_vtx1Daug_num==4) {
	tracksDiTrk.push_back(psiVertex->trackParticle(2));
	tracksDiTrk.push_back(psiVertex->trackParticle(3));
      }

      TLorentzVector p4_moth;
      TLorentzVector tmp;
      for(size_t it=0; it<jpsiVertex->nTrackParticles(); it++) {
	tmp.SetPtEtaPhiM(jpsiVertex->trackParticle(it)->pt(),jpsiVertex->trackParticle(it)->eta(),jpsiVertex->trackParticle(it)->phi(),massesJpsi2[it]);
	p4_moth += tmp;
      }
      for(size_t it=0; it<psiVertex->nTrackParticles(); it++) {
	tmp.SetPtEtaPhiM(psiVertex->trackParticle(it)->pt(),psiVertex->trackParticle(it)->eta(),psiVertex->trackParticle(it)->phi(),massesPsi[it]);
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
      // Psi vertex
      Trk::VertexID vID1;
      // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
      if (m_constrPsi) {
	vID1 = m_iVertexFitter->startVertex(tracksPsi,massesPsi,m_mass_psi);
      } else {
	vID1 = m_iVertexFitter->startVertex(tracksPsi,massesPsi);
      }
      vrtList.push_back(vID1);
      // Jpsi vertex
      Trk::VertexID vID2;
      if (m_constrJpsi2) {
	vID2 = m_iVertexFitter->nextVertex(tracksJpsi2,massesJpsi2,m_mass_jpsi2);
      } else {
	vID2 = m_iVertexFitter->nextVertex(tracksJpsi2,massesJpsi2);
      }
      vrtList.push_back(vID2);
      // Mother vertex including Jpsi and Psi
      std::vector<const xAOD::TrackParticle*> tp; tp.clear();
      std::vector<double> tp_masses; tp_masses.clear();
      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
      if (m_constrJpsi) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID1,tracksJpsi,cnstV,m_mass_jpsi).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	}
      }
      if (m_constrDiTrk && m_vtx1Daug_num==4 && m_mass_diTrk>0) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID1,tracksDiTrk,cnstV,m_mass_diTrk).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for DiTrk failed");
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
	if(m_constrJpsi || m_constrPsi || m_constrJpsi2 || (m_constrDiTrk && m_vtx1Daug_num==4 && m_mass_diTrk>0)) {
	  m_iVertexFitter->setDefault();
	  m_iVertexFitter->setRobustness(robustness);
	  std::vector<Trk::VertexID> vrtList_nc;
	  // Psi vertex
	  Trk::VertexID vID1_nc = m_iVertexFitter->startVertex(tracksPsi,massesPsi);
	  vrtList_nc.push_back(vID1_nc);
	  Trk::VertexID vID2_nc = m_iVertexFitter->nextVertex(tracksJpsi2,massesJpsi2);
	  vrtList_nc.push_back(vID2_nc);
	  // Mother vertex including Jpsi and Psi
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
