/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#include "JpsiXPlus2V0.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtFitter/VxCascadeInfo.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "BPhysPVCascadeTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "HepPDT/ParticleDataTable.hh"
#include "VxVertex/RecVertex.h"
#include "JpsiUpsilonTools/JpsiUpsilonCommon.h"
#include "TruthUtils/HepMCHelpers.h"
#include <algorithm>
#include <functional>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;

  using Analysis::JpsiUpsilonCommon;

  JpsiXPlus2V0::JpsiXPlus2V0(const std::string& type, const std::string& name, const IInterface* parent) : base_class(type,name,parent),
    m_vertexJXContainerKey("InputJXVertices"),
    m_vertexV0ContainerKey{""},
    m_cascadeOutputKeys({"JpsiXPlus2V0_SubVtx1", "JpsiXPlus2V0_SubVtx2", "JpsiXPlus2V0_SubVtx3", "JpsiXPlus2V0_MainVtx"}),
    m_v0VtxOutputKey(""),
    m_TrkParticleCollection("InDetTrackParticles"),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_pvContainerName("PrimaryVertices"),
    m_refPVContainerName("RefittedPrimaryVertices"),
    m_eventInfo_key("EventInfo"),
    m_RelinkContainers({"InDetTrackParticles","InDetLargeD0TrackParticles"}),
    m_useImprovedMass(false),
    m_jxMassLower(0.0),
    m_jxMassUpper(30000.0),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(20000.0),
    m_diTrackMassLower(-1.0),
    m_diTrackMassUpper(-1.0),
    m_V01Hypothesis("Ks"),
    m_V02Hypothesis("Lambda"),
    m_LambdaMassLower(0.0),
    m_LambdaMassUpper(10000.0),
    m_KsMassLower(0.0),
    m_KsMassUpper(10000.0),
    m_lxyV01_cut(-999.0),
    m_lxyV02_cut(-999.0),
    m_minMass_gamma(-1.0),
    m_chi2cut_gamma(-1.0),
    m_JXV02MassLower(0.0),
    m_JXV02MassUpper(30000.0),
    m_MassLower(0.0),
    m_MassUpper(31000.0),
    m_jxDaug_num(4),
    m_jxDaug1MassHypo(-1),
    m_jxDaug2MassHypo(-1),
    m_jxDaug3MassHypo(-1),
    m_jxDaug4MassHypo(-1),
    m_massJX(-1),
    m_massJpsi(-1),
    m_massX(-1),
    m_massLd(-1),
    m_massKs(-1),
    m_massJXV02(-1),
    m_massMainV(-1),
    m_constrJX(false),
    m_constrJpsi(false),
    m_constrX(false),
    m_constrV01(false),
    m_constrV02(false),
    m_constrJXV02(false),
    m_constrMainV(false),
    m_cascadeFitWithPV(0),
    m_firstDecayAtPV(false),
    m_JXSubVtx(false),
    m_JXV02SubVtx(false),
    m_chi2cut_JX(-1.0),
    m_chi2cut_V0(-1.0),
    m_chi2cut(-1.0),
    m_useTRT(false),
    m_ptTRT(450),
    m_d0_cut(2),
    m_maxJXCandidates(0),
    m_maxV0Candidates(0),
    m_maxMainVCandidates(0),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_iV0Fitter("Trk::V0VertexFitter"),
    m_iGammaFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter", this),
    m_V0Tools("Trk::V0Tools"),
    m_trackToVertexTool("Reco::TrackToVertex"),
    m_v0TrkSelector("InDet::TrackSelectorTool"),
    m_CascadeTools("DerivationFramework::CascadeTools"),
    m_vertexEstimator("InDet::VertexPointEstimator"),
    m_extrapolator("Trk::Extrapolator/AtlasExtrapolator")
  {
    declareProperty("JXVertices",               m_vertexJXContainerKey);
    declareProperty("V0Vertices",               m_vertexV0ContainerKey);
    declareProperty("JXVtxHypoNames",           m_vertexJXHypoNames);
    declareProperty("CascadeVertexCollections", m_cascadeOutputKeys); // size is 3 or 4 only
    declareProperty("OutoutV0VtxCollection",    m_v0VtxOutputKey);
    declareProperty("TrackParticleCollection",  m_TrkParticleCollection);
    declareProperty("VxPrimaryCandidateName",   m_VxPrimaryCandidateName);
    declareProperty("PVContainerName",          m_pvContainerName);
    declareProperty("RefPVContainerName",       m_refPVContainerName);
    declareProperty("EventInfoKey",             m_eventInfo_key);
    declareProperty("RelinkTracks",             m_RelinkContainers);
    declareProperty("UseImprovedMass",          m_useImprovedMass);
    declareProperty("JXMassLowerCut",           m_jxMassLower); // only effective when m_jxDaug_num>2
    declareProperty("JXMassUpperCut",           m_jxMassUpper); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMassLowerCut",         m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",         m_jpsiMassUpper);
    declareProperty("DiTrackMassLower",         m_diTrackMassLower); // only effective when m_jxDaug_num=4
    declareProperty("DiTrackMassUpper",         m_diTrackMassUpper); // only effective when m_jxDaug_num=4
    declareProperty("V01Hypothesis",            m_V01Hypothesis); // "Ks", "Lambda" or "Lambda/Ks"
    declareProperty("V02Hypothesis",            m_V02Hypothesis); // "Ks", "Lambda" or "Lambda/Ks"
    declareProperty("LxyV01Cut",                m_lxyV01_cut);
    declareProperty("LxyV02Cut",                m_lxyV02_cut);
    declareProperty("LambdaMassLowerCut",       m_LambdaMassLower);
    declareProperty("LambdaMassUpperCut",       m_LambdaMassUpper);
    declareProperty("KsMassLowerCut",           m_KsMassLower);
    declareProperty("KsMassUpperCut",           m_KsMassUpper);
    declareProperty("MassCutGamma",             m_minMass_gamma);
    declareProperty("Chi2CutGamma",             m_chi2cut_gamma);
    declareProperty("JXV02MassLowerCut",        m_JXV02MassLower); // only effective when m_JXSubVtx=true & m_JXV02SubVtx=true
    declareProperty("JXV02MassUpperCut",        m_JXV02MassUpper); // only effective when m_JXSubVtx=true & m_JXV02SubVtx=true
    declareProperty("MassLowerCut",             m_MassLower);
    declareProperty("MassUpperCut",             m_MassUpper);
    declareProperty("HypothesisName",           m_hypoName = "TQ");
    declareProperty("NumberOfJXDaughters",      m_jxDaug_num); // 2, or 3, or 4 only
    declareProperty("JXDaug1MassHypo",          m_jxDaug1MassHypo);
    declareProperty("JXDaug2MassHypo",          m_jxDaug2MassHypo);
    declareProperty("JXDaug3MassHypo",          m_jxDaug3MassHypo);
    declareProperty("JXDaug4MassHypo",          m_jxDaug4MassHypo);
    declareProperty("JXMass",                   m_massJX); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMass",                 m_massJpsi);
    declareProperty("XMass",                    m_massX); // only effective when m_jxDaug_num=4
    declareProperty("LambdaMass",               m_massLd);
    declareProperty("KsMass",                   m_massKs);
    declareProperty("JXV02VtxMass",             m_massJXV02); // mass of JX + 2nd V0
    declareProperty("MainVtxMass",              m_massMainV);
    declareProperty("ApplyJXMassConstraint",    m_constrJX); // only effective when m_jxDaug_num>2
    declareProperty("ApplyJpsiMassConstraint",  m_constrJpsi);
    declareProperty("ApplyXMassConstraint",     m_constrX); // only effective when m_jxDaug_num=4
    declareProperty("ApplyV01MassConstraint",   m_constrV01); // first V0
    declareProperty("ApplyV02MassConstraint",   m_constrV02); // second V0
    declareProperty("ApplyJXV02MassConstraint", m_constrJXV02); // constrain JX + 2nd V0
    declareProperty("ApplyMainVMassConstraint", m_constrMainV);
    declareProperty("DoCascadeFitWithPV",       m_cascadeFitWithPV);
    declareProperty("FirstDecayAtPV",           m_firstDecayAtPV);
    declareProperty("HasJXSubVertex",           m_JXSubVtx);
    declareProperty("HasJXV02SubVertex",        m_JXV02SubVtx); // only effective when m_JXSubVtx=true
    declareProperty("Chi2CutJX",                m_chi2cut_JX);
    declareProperty("Chi2CutV0",                m_chi2cut_V0);
    declareProperty("Chi2Cut",                  m_chi2cut);
    declareProperty("UseTRT",                   m_useTRT);
    declareProperty("PtTRT",                    m_ptTRT);
    declareProperty("Trackd0Cut",               m_d0_cut);
    declareProperty("MaxJXCandidates",          m_maxJXCandidates);
    declareProperty("MaxV0Candidates",          m_maxV0Candidates);
    declareProperty("MaxMainVCandidates",       m_maxMainVCandidates);
    declareProperty("RefitPV",                  m_refitPV         = true);
    declareProperty("MaxnPV",                   m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",           m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",             m_DoVertexType    = 7);
    declareProperty("TrkVertexFitterTool",      m_iVertexFitter);
    declareProperty("V0VertexFitterTool",       m_iV0Fitter);
    declareProperty("GammaFitterTool",          m_iGammaFitter);
    declareProperty("PVRefitter",               m_pvRefitter);
    declareProperty("V0Tools",                  m_V0Tools);
    declareProperty("TrackToVertexTool",        m_trackToVertexTool);
    declareProperty("V0TrackSelectorTool",      m_v0TrkSelector);
    declareProperty("CascadeTools",             m_CascadeTools);
    declareProperty("VertexPointEstimator",     m_vertexEstimator);
    declareProperty("Extrapolator",             m_extrapolator);
  }

  StatusCode JpsiXPlus2V0::initialize() {
    if((m_V01Hypothesis != "Ks" && m_V01Hypothesis != "Lambda" && m_V01Hypothesis != "Lambda/Ks" && m_V01Hypothesis != "Ks/Lambda") ||
       (m_V02Hypothesis != "Ks" && m_V02Hypothesis != "Lambda" && m_V02Hypothesis != "Lambda/Ks" && m_V02Hypothesis != "Ks/Lambda")) {
      ATH_MSG_FATAL("Incorrect V0 container hypothesis - not recognized");
      return StatusCode::FAILURE;
    }

    if(m_jxDaug_num<2 || m_jxDaug_num>4) {
      ATH_MSG_FATAL("Incorrect number of JX daughters");
      return StatusCode::FAILURE;
    }

    if(m_vertexV0ContainerKey.key()=="" && m_v0VtxOutputKey.key()=="") {
      ATH_MSG_FATAL("Input and output V0 container names can not be both empty");
      return StatusCode::FAILURE;
    }

    // retrieving vertex Fitter
    ATH_CHECK( m_iVertexFitter.retrieve() );

    // retrieving V0 vertex Fitter
    ATH_CHECK( m_iV0Fitter.retrieve() );

    // retrieving photon conversion vertex Fitter
    ATH_CHECK( m_iGammaFitter.retrieve() );

    // retrieving primary vertex refitter
    ATH_CHECK( m_pvRefitter.retrieve() );

    // retrieving the V0 tool
    ATH_CHECK( m_V0Tools.retrieve() );

    // retrieving the TrackToVertex extrapolator tool
    ATH_CHECK( m_trackToVertexTool.retrieve() );

    // retrieving the V0 track selector tool
    ATH_CHECK( m_v0TrkSelector.retrieve() );

    // retrieving the Cascade tools
    ATH_CHECK( m_CascadeTools.retrieve() );

    // retrieving the vertex point estimator
    ATH_CHECK( m_vertexEstimator.retrieve() );

    // retrieving the extrapolator
    ATH_CHECK( m_extrapolator.retrieve() );

    ATH_CHECK( m_vertexJXContainerKey.initialize() );
    ATH_CHECK( m_vertexV0ContainerKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_VxPrimaryCandidateName.initialize() );
    ATH_CHECK( m_TrkParticleCollection.initialize() );
    ATH_CHECK( m_pvContainerName.initialize() );
    ATH_CHECK( m_refPVContainerName.initialize() );
    ATH_CHECK( m_cascadeOutputKeys.initialize() );
    ATH_CHECK( m_eventInfo_key.initialize() );
    ATH_CHECK( m_RelinkContainers.initialize() );
    ATH_CHECK( m_v0VtxOutputKey.initialize(SG::AllowEmpty) );

    ATH_CHECK( m_partPropSvc.retrieve() );
    auto pdt = m_partPropSvc->PDT();

    // https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/TruthUtils/TruthUtils/AtlasPID.h
    m_mass_e = BPhysPVCascadeTools::getParticleMass(pdt, MC::ELECTRON);
    m_mass_mu = BPhysPVCascadeTools::getParticleMass(pdt, MC::MUON);
    m_mass_pion = BPhysPVCascadeTools::getParticleMass(pdt, MC::PIPLUS);
    m_mass_proton = BPhysPVCascadeTools::getParticleMass(pdt, MC::PROTON);
    m_mass_Lambda = BPhysPVCascadeTools::getParticleMass(pdt, MC::LAMBDA0);
    m_mass_Lambdab = BPhysPVCascadeTools::getParticleMass(pdt, MC::LAMBDAB0);
    m_mass_BCPLUS = BPhysPVCascadeTools::getParticleMass(pdt, MC::BCPLUS);
    m_mass_Ks = BPhysPVCascadeTools::getParticleMass(pdt, MC::K0S);
    m_mass_Bpm = BPhysPVCascadeTools::getParticleMass(pdt, 521);
    m_mass_phi = BPhysPVCascadeTools::getParticleMass(pdt, 333);

    m_massesV0_ppi.push_back(m_mass_proton);
    m_massesV0_ppi.push_back(m_mass_pion);
    m_massesV0_pip.push_back(m_mass_pion);
    m_massesV0_pip.push_back(m_mass_proton);
    m_massesV0_pipi.push_back(m_mass_pion);
    m_massesV0_pipi.push_back(m_mass_pion);

    // retrieve particle masses
    if(m_constrJpsi && m_massJpsi<0) m_massJpsi = BPhysPVCascadeTools::getParticleMass(pdt, MC::JPSI);
    if(m_jxDaug_num>=3 && m_constrJX && m_massJX<0) m_massJX = BPhysPVCascadeTools::getParticleMass(pdt, MC::PSI2S);
    if(m_jxDaug_num==4 && m_constrX && m_massX<0) m_massX = m_mass_phi;
    if(m_constrV01 || m_constrV02) {
      if(m_massLd<0) m_massLd = m_mass_Lambda;
      if(m_massKs<0) m_massKs = m_mass_Ks;
    }
    if(m_constrJXV02 && m_massJXV02<0) m_massJXV02 = m_mass_Lambdab;
    if(m_constrMainV && m_massMainV<0) m_massMainV = m_mass_BCPLUS;

    if(m_jxDaug1MassHypo < 0.) m_jxDaug1MassHypo = m_mass_mu;
    if(m_jxDaug2MassHypo < 0.) m_jxDaug2MassHypo = m_mass_mu;
    if(m_jxDaug_num>=3 && m_jxDaug3MassHypo < 0.) m_jxDaug3MassHypo = m_mass_pion;
    if(m_jxDaug_num==4 && m_jxDaug4MassHypo < 0.) m_jxDaug4MassHypo = m_mass_pion;

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlus2V0::performSearch(std::vector<Trk::VxCascadeInfo*>& cascadeinfoContainer, const std::vector<std::pair<const xAOD::Vertex*,V0Enum> >& selectedV0Candidates, const EventContext& ctx) const {
    ATH_MSG_DEBUG( "JpsiXPlus2V0::performSearch" );
    if(selectedV0Candidates.size()==0) return StatusCode::SUCCESS;

    // Get all track containers when m_RelinkContainers is not empty
    std::vector<const xAOD::TrackParticleContainer*> trackCols;
    for(const SG::ReadHandleKey<xAOD::TrackParticleContainer>& key : m_RelinkContainers){
      SG::ReadHandle<xAOD::TrackParticleContainer> handle(key, ctx);
      ATH_CHECK( handle.isValid() );
      trackCols.push_back(handle.cptr());
    }

    // Get default PV container
    SG::ReadHandle<xAOD::VertexContainer> defaultPVContainer(m_VxPrimaryCandidateName, ctx);
    ATH_CHECK( defaultPVContainer.isValid() );

    // Get PV container
    SG::ReadHandle<xAOD::VertexContainer> pvContainer(m_pvContainerName,ctx);
    ATH_CHECK( pvContainer.isValid() );

    // Get Jpsi+X container
    SG::ReadHandle<xAOD::VertexContainer> jxContainer(m_vertexJXContainerKey, ctx);
    ATH_CHECK( jxContainer.isValid() );

    std::vector<double> massesJX{m_jxDaug1MassHypo, m_jxDaug2MassHypo};
    if(m_jxDaug_num>=3) massesJX.push_back(m_jxDaug3MassHypo);
    if(m_jxDaug_num==4) massesJX.push_back(m_jxDaug4MassHypo);

    // Select the JX candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedJXCandidates;
    for(auto vxcItr=jxContainer.ptr()->begin(); vxcItr!=jxContainer.ptr()->end(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(const std::string& name : m_vertexJXHypoNames) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed = true;
	}
      }
      if(m_vertexJXHypoNames.size() && !passed) continue;

      // Add loose cut on Jpsi mass from e.g. JX -> Jpsi pi+ pi-
      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM(vtx->trackParticle(0)->pt(),vtx->trackParticle(0)->eta(),vtx->trackParticle(0)->phi(), m_jxDaug1MassHypo);
      p4_mu2.SetPtEtaPhiM(vtx->trackParticle(1)->pt(),vtx->trackParticle(1)->eta(),vtx->trackParticle(1)->phi(), m_jxDaug2MassHypo);
      double mass_jpsi = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi < m_jpsiMassLower || mass_jpsi > m_jpsiMassUpper) continue;

      TLorentzVector p4_trk1, p4_trk2;
      if(m_jxDaug_num>=3) p4_trk1.SetPtEtaPhiM(vtx->trackParticle(2)->pt(),vtx->trackParticle(2)->eta(),vtx->trackParticle(2)->phi(), m_jxDaug3MassHypo);
      if(m_jxDaug_num==4) p4_trk2.SetPtEtaPhiM(vtx->trackParticle(3)->pt(),vtx->trackParticle(3)->eta(),vtx->trackParticle(3)->phi(), m_jxDaug4MassHypo);

      if(m_jxDaug_num==3) {
	double mass_jx = (p4_mu1 + p4_mu2 + p4_trk1).M();
	if(m_useImprovedMass && m_massJpsi>0) mass_jx += - (p4_mu1 + p4_mu2).M() + m_massJpsi;
	if(mass_jx < m_jxMassLower || mass_jx > m_jxMassUpper) continue;
      }
      else if(m_jxDaug_num==4) {
	double mass_jx = (p4_mu1 + p4_mu2 + p4_trk1 + p4_trk2).M();
	if(m_useImprovedMass && m_massJpsi>0) mass_jx += - (p4_mu1 + p4_mu2).M() + m_massJpsi;
	if(mass_jx < m_jxMassLower || mass_jx > m_jxMassUpper) continue;

	if(m_diTrackMassLower>=0 && m_diTrackMassUpper>m_diTrackMassLower) {
	  double mass_diTrk = (p4_trk1 + p4_trk2).M();
	  if(mass_diTrk < m_diTrackMassLower || mass_diTrk > m_diTrackMassUpper) continue;
	}
      }

      double chi2DOF = vtx->chiSquared()/vtx->numberDoF();
      if(m_chi2cut_JX>0 && chi2DOF>m_chi2cut_JX) continue;

      selectedJXCandidates.push_back(vtx);
    }
    if(selectedJXCandidates.size()==0) return StatusCode::SUCCESS;

    std::sort( selectedJXCandidates.begin(), selectedJXCandidates.end(), [](const xAOD::Vertex* a, const xAOD::Vertex* b) { return a->chiSquared()/a->numberDoF() < b->chiSquared()/b->numberDoF(); } );
    if(m_maxJXCandidates>0 && selectedJXCandidates.size()>m_maxJXCandidates) {
      selectedJXCandidates.erase(selectedJXCandidates.begin()+m_maxJXCandidates, selectedJXCandidates.end());
    }

    // Select JX+V0+V0 candidates
    std::vector<const xAOD::TrackParticle*> tracksJX; tracksJX.reserve(m_jxDaug_num);
    std::vector<const xAOD::TrackParticle*> tracksV01; tracksV01.reserve(2);
    for(auto jxItr=selectedJXCandidates.cbegin(); jxItr!=selectedJXCandidates.cend(); ++jxItr) {
      tracksJX.clear();
      for(size_t i=0; i<(*jxItr)->nTrackParticles(); i++) tracksJX.push_back((*jxItr)->trackParticle(i));
      for(auto V0Itr1=selectedV0Candidates.cbegin(); V0Itr1!=selectedV0Candidates.cend(); ++V0Itr1) {
	if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr1->first->trackParticle(0)) != tracksJX.cend()) continue;
	if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr1->first->trackParticle(1)) != tracksJX.cend()) continue;
	tracksV01.clear();
	for(size_t j=0; j<V0Itr1->first->nTrackParticles(); j++) tracksV01.push_back(V0Itr1->first->trackParticle(j));
	for(auto V0Itr2=V0Itr1+1; V0Itr2!=selectedV0Candidates.cend(); ++V0Itr2) {
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr2->first->trackParticle(0)) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr2->first->trackParticle(1)) != tracksJX.cend()) continue;
	  if(std::find(tracksV01.cbegin(), tracksV01.cend(), V0Itr2->first->trackParticle(0)) != tracksV01.cend()) continue;
	  if(std::find(tracksV01.cbegin(), tracksV01.cend(), V0Itr2->first->trackParticle(1)) != tracksV01.cend()) continue;

	  int numberOfVertices = 0;
	  if((m_V01Hypothesis=="Lambda/Ks" || m_V01Hypothesis=="Ks/Lambda") && (m_V02Hypothesis=="Lambda/Ks" || m_V02Hypothesis=="Ks/Lambda")) {
	    numberOfVertices = m_JXSubVtx && m_JXV02SubVtx ? 2 : 1;
	  }
	  else if((m_V01Hypothesis == "Ks" || m_V01Hypothesis == "Lambda") && (m_V02Hypothesis=="Lambda/Ks" || m_V02Hypothesis=="Ks/Lambda")) {
	    if((m_V01Hypothesis == "Ks" && V0Itr1->second==KS) ||
	       (m_V01Hypothesis == "Lambda" && (V0Itr1->second==LAMBDA || V0Itr1->second==LAMBDABAR))) numberOfVertices = 1;
	    if((m_V01Hypothesis == "Ks" && V0Itr2->second==KS) ||
	       (m_V01Hypothesis == "Lambda" && (V0Itr2->second==LAMBDA || V0Itr2->second==LAMBDABAR))) {
	      if(numberOfVertices == 1) numberOfVertices = m_JXSubVtx && m_JXV02SubVtx ? 2 : 1;
	      else numberOfVertices = -1;
	    }
	  }
	  else if((m_V02Hypothesis == "Ks" || m_V02Hypothesis == "Lambda") && (m_V01Hypothesis=="Lambda/Ks" || m_V01Hypothesis=="Ks/Lambda")) {
	    if((m_V02Hypothesis == "Ks" && V0Itr2->second==KS) ||
	       (m_V02Hypothesis == "Lambda" && (V0Itr2->second==LAMBDA || V0Itr2->second==LAMBDABAR))) numberOfVertices = 1;
	    if((m_V02Hypothesis == "Ks" && V0Itr1->second==KS) ||
	       (m_V02Hypothesis == "Lambda" && (V0Itr1->second==LAMBDA || V0Itr1->second==LAMBDABAR))) {
	      if(numberOfVertices == 1) numberOfVertices = m_JXSubVtx && m_JXV02SubVtx ? 2 : 1;
	      else numberOfVertices = -1;
	    }
	  }
	  else if(m_V01Hypothesis == "Ks" && m_V02Hypothesis == "Lambda") {
	    if(V0Itr1->second==KS && (V0Itr2->second==LAMBDA || V0Itr2->second==LAMBDABAR)) numberOfVertices = 1;
	    else if(V0Itr2->second==KS && (V0Itr1->second==LAMBDA || V0Itr1->second==LAMBDABAR)) numberOfVertices = -1;
	  }
	  else if(m_V01Hypothesis == "Lambda" && m_V02Hypothesis == "Ks") {
	    if((V0Itr1->second==LAMBDA || V0Itr1->second==LAMBDABAR) && V0Itr2->second==KS) numberOfVertices = 1;
	    else if((V0Itr2->second==LAMBDA || V0Itr2->second==LAMBDABAR) && V0Itr1->second==KS) numberOfVertices = -1;
	  }
	  else if(m_V01Hypothesis == "Ks" && m_V02Hypothesis == "Ks") {
	    if(V0Itr1->second==KS && V0Itr2->second==KS) numberOfVertices = m_JXSubVtx && m_JXV02SubVtx ? 2 : 1;
	  }
	  else if(m_V01Hypothesis == "Lambda" && m_V02Hypothesis == "Lambda") {
	    if((V0Itr1->second==LAMBDA || V0Itr1->second==LAMBDABAR) && (V0Itr2->second==LAMBDA || V0Itr2->second==LAMBDABAR)) numberOfVertices = m_JXSubVtx && m_JXV02SubVtx ? 2 : 1;
	  }

	  if(numberOfVertices==2) {
	    Trk::VxCascadeInfo* result1 = fitMainVtx(*jxItr, massesJX, V0Itr1->first, V0Itr1->second, V0Itr2->first, V0Itr2->second, trackCols, defaultPVContainer.cptr(), pvContainer.cptr());
	    if(result1) cascadeinfoContainer.push_back(result1);
	    Trk::VxCascadeInfo* result2 = fitMainVtx(*jxItr, massesJX, V0Itr2->first, V0Itr2->second, V0Itr1->first, V0Itr1->second, trackCols, defaultPVContainer.cptr(), pvContainer.cptr());
	    if(result2) cascadeinfoContainer.push_back(result2);
	  }
	  else if(numberOfVertices==1) {
	    Trk::VxCascadeInfo* result = fitMainVtx(*jxItr, massesJX, V0Itr1->first, V0Itr1->second, V0Itr2->first, V0Itr2->second, trackCols, defaultPVContainer.cptr(), pvContainer.cptr());
	    if(result) cascadeinfoContainer.push_back(result);
	  }
	  else if(numberOfVertices==-1) {
	    Trk::VxCascadeInfo* result = fitMainVtx(*jxItr, massesJX, V0Itr2->first, V0Itr2->second, V0Itr1->first, V0Itr1->second, trackCols, defaultPVContainer.cptr(), pvContainer.cptr());
	    if(result) cascadeinfoContainer.push_back(result);
	  }
	}
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlus2V0::addBranches(const EventContext& ctx) const {
    size_t topoN = 4;
    if(!m_JXSubVtx) topoN--;

    if(m_cascadeOutputKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of output cascade vertices");
      return StatusCode::FAILURE;
    }

    std::array<SG::WriteHandle<xAOD::VertexContainer>, 4> VtxWriteHandles; int ikey(0);
    for(const SG::WriteHandleKey<xAOD::VertexContainer>& key : m_cascadeOutputKeys) {
      VtxWriteHandles[ikey] = SG::WriteHandle<xAOD::VertexContainer>(key, ctx);
      ATH_CHECK( VtxWriteHandles[ikey].record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
      ikey++;
    }

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    const xAOD::Vertex* primaryVertex(nullptr);
    SG::ReadHandle<xAOD::VertexContainer> defaultPVContainer(m_VxPrimaryCandidateName, ctx);
    ATH_CHECK( defaultPVContainer.isValid() );
    if (defaultPVContainer.cptr()->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << defaultPVContainer.cptr()->size());
      return StatusCode::RECOVERABLE;
    }
    else primaryVertex = (*defaultPVContainer.cptr())[0];

    //----------------------------------------------------
    // Record refitted primary vertices
    //----------------------------------------------------
    SG::WriteHandle<xAOD::VertexContainer> refPvContainer;
    if(m_refitPV) {
      refPvContainer = SG::WriteHandle<xAOD::VertexContainer>(m_refPVContainerName, ctx);
      ATH_CHECK( refPvContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
    }

    // Get TrackParticle container (standard + LRT)
    SG::ReadHandle<xAOD::TrackParticleContainer> trackContainer(m_TrkParticleCollection, ctx);
    ATH_CHECK( trackContainer.isValid() );

    // Get all track containers when m_RelinkContainers is not empty
    std::vector<const xAOD::TrackParticleContainer*> trackCols;
    for(const SG::ReadHandleKey<xAOD::TrackParticleContainer>& key : m_RelinkContainers){
      SG::ReadHandle<xAOD::TrackParticleContainer> handle(key, ctx);
      ATH_CHECK( handle.isValid() );
      trackCols.push_back(handle.cptr());
    }

    // output V0 vertices
    SG::WriteHandle<xAOD::VertexContainer> V0OutputContainer;
    if(m_vertexV0ContainerKey.key()=="" && m_v0VtxOutputKey.key()!="") {
      V0OutputContainer = SG::WriteHandle<xAOD::VertexContainer>(m_v0VtxOutputKey, ctx);
      ATH_CHECK( V0OutputContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
    }

    // Get the input containers
    // Note: If the event does not contain a JX candidate, it is skipped and the V0 container will not be constructed if not previously available in StoreGate
    SG::ReadHandle<xAOD::VertexContainer> jxContainer(m_vertexJXContainerKey, ctx);
    ATH_CHECK( jxContainer.isValid() );
    if(jxContainer->size()==0) return StatusCode::SUCCESS;

    // Select the displaced tracks
    std::vector<const xAOD::TrackParticle*> tracksDisplaced;
    if(m_v0VtxOutputKey.key()!="") {
      for(const xAOD::TrackParticle* TP : *trackContainer.cptr()) {
	// V0 track selection (https://gitlab.cern.ch/atlas/athena/-/blob/main/InnerDetector/InDetRecTools/InDetTrackSelectorTool/src/InDetConversionTrackSelectorTool.cxx)
	if(m_v0TrkSelector->decision(*TP, primaryVertex)) {
	  uint8_t temp(0);
	  uint8_t nclus(0);
	  if(TP->summaryValue(temp, xAOD::numberOfPixelHits)) nclus += temp;
	  if(TP->summaryValue(temp, xAOD::numberOfSCTHits)  ) nclus += temp; 
	  if(!m_useTRT && nclus == 0) continue;

	  bool trk_cut = false;
	  if(nclus != 0) trk_cut = true;
	  if(nclus == 0 && TP->pt()>=m_ptTRT) trk_cut = true;
	  if(!trk_cut) continue;

	  // track is used if std::abs(d0/sig_d0) > d0_cut for PV
	  if(!d0Pass(TP,primaryVertex)) continue;

	  tracksDisplaced.push_back(TP);
	}
      }
    }

    SG::AuxElement::Accessor<std::string> mAcc_type("Type_V0Vtx");
    SG::AuxElement::Accessor<int>         mAcc_gfit("gamma_fit");
    SG::AuxElement::Accessor<float>       mAcc_gmass("gamma_mass");
    SG::AuxElement::Accessor<float>       mAcc_gchisq("gamma_chisq");
    SG::AuxElement::Accessor<int>         mAcc_gndof("gamma_ndof");

    std::vector<std::pair<const xAOD::Vertex*,V0Enum> > selectedV0Candidates;

    SG::ReadHandle<xAOD::VertexContainer> V0Container;
    if(m_vertexV0ContainerKey.key() != "") {
      V0Container = SG::ReadHandle<xAOD::VertexContainer>(m_vertexV0ContainerKey, ctx);
      ATH_CHECK( V0Container.isValid() );

      for(const xAOD::Vertex* vtx : *V0Container.cptr()) {
	std::string type_V0Vtx;
	if(mAcc_type.isAvailable(*vtx)) type_V0Vtx = mAcc_type(*vtx);

	V0Enum opt(UNKNOWN); double massV0(0);
	if(type_V0Vtx == "Lambda") {
	  opt = LAMBDA;
	  massV0 = m_V0Tools->invariantMass(vtx, m_massesV0_ppi);
	  if(massV0<m_LambdaMassLower || massV0>m_LambdaMassUpper) continue;
	}
	else if(type_V0Vtx == "Lambdabar") {
	  opt = LAMBDABAR;
	  massV0 = m_V0Tools->invariantMass(vtx, m_massesV0_pip);
	  if(massV0<m_LambdaMassLower || massV0>m_LambdaMassUpper) continue;
	}
	else if(type_V0Vtx == "Ks") {
	  opt = KS;
	  massV0 = m_V0Tools->invariantMass(vtx, m_massesV0_pipi);
	  if(massV0<m_KsMassLower || massV0>m_KsMassUpper) continue;
	}

	if(opt==UNKNOWN) continue;
	if(m_V01Hypothesis == m_V02Hypothesis) {
	  if((opt==LAMBDA || opt==LAMBDABAR) && m_V01Hypothesis == "Ks")  continue;
	  if(opt==KS && m_V01Hypothesis == "Lambda") continue;
	}

	int gamma_fit      = mAcc_gfit.isAvailable(*vtx) ? mAcc_gfit(*vtx) : 0;
	double gamma_mass  = mAcc_gmass.isAvailable(*vtx) ? mAcc_gmass(*vtx) : -1;
	double gamma_chisq = mAcc_gchisq.isAvailable(*vtx) ? mAcc_gchisq(*vtx) : 999999;
	double gamma_ndof  = mAcc_gndof.isAvailable(*vtx) ? mAcc_gndof(*vtx) : 0;
	if(gamma_fit==1 && gamma_mass<m_minMass_gamma && gamma_chisq/gamma_ndof<m_chi2cut_gamma) continue;

	selectedV0Candidates.push_back(std::pair<const xAOD::Vertex*,V0Enum>{vtx,opt});
      }
    }
    else {
      // fit V0 vertices
      fitV0Container(V0OutputContainer.ptr(), tracksDisplaced, trackCols);

      for(const xAOD::Vertex* vtx : *V0OutputContainer.cptr()) {
	std::string type_V0Vtx;
	if(mAcc_type.isAvailable(*vtx)) type_V0Vtx = mAcc_type(*vtx);

	V0Enum opt(UNKNOWN); double massV0(0);
	if(type_V0Vtx == "Lambda") {
	  opt = LAMBDA;
	  massV0 = m_V0Tools->invariantMass(vtx, m_massesV0_ppi);
	  if(massV0<m_LambdaMassLower || massV0>m_LambdaMassUpper) continue;
	}
	else if(type_V0Vtx == "Lambdabar") {
	  opt = LAMBDABAR;
	  massV0 = m_V0Tools->invariantMass(vtx, m_massesV0_pip);
	  if(massV0<m_LambdaMassLower || massV0>m_LambdaMassUpper) continue;
	}
	else if(type_V0Vtx == "Ks") {
	  opt = KS;
	  massV0 = m_V0Tools->invariantMass(vtx, m_massesV0_pipi);
	  if(massV0<m_KsMassLower || massV0>m_KsMassUpper) continue;
	}

	if(opt==UNKNOWN) continue;
	if(m_V01Hypothesis == m_V02Hypothesis) {
	  if((opt==LAMBDA || opt==LAMBDABAR) && m_V01Hypothesis == "Ks")  continue;
	  if(opt==KS && m_V01Hypothesis == "Lambda") continue;
	}

	int gamma_fit      = mAcc_gfit.isAvailable(*vtx) ? mAcc_gfit(*vtx) : 0;
	double gamma_mass  = mAcc_gmass.isAvailable(*vtx) ? mAcc_gmass(*vtx) : -1;
	double gamma_chisq = mAcc_gchisq.isAvailable(*vtx) ? mAcc_gchisq(*vtx) : 999999;
	double gamma_ndof  = mAcc_gndof.isAvailable(*vtx) ? mAcc_gndof(*vtx) : 0;
	if(gamma_fit==1 && gamma_mass<m_minMass_gamma && gamma_chisq/gamma_ndof<m_chi2cut_gamma) continue;

	selectedV0Candidates.push_back(std::pair<const xAOD::Vertex*,V0Enum>{vtx,opt});
      }
    } 

    // sort and chop the V0 candidates
    std::sort( selectedV0Candidates.begin(), selectedV0Candidates.end(), [](std::pair<const xAOD::Vertex*,V0Enum>& a, std::pair<const xAOD::Vertex*,V0Enum>& b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
    if(m_maxV0Candidates>0 && selectedV0Candidates.size()>m_maxV0Candidates) {
      selectedV0Candidates.erase(selectedV0Candidates.begin()+m_maxV0Candidates, selectedV0Candidates.end());
    }
    if(selectedV0Candidates.size()==0) return StatusCode::SUCCESS;

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
    ATH_CHECK( performSearch(cascadeinfoContainer, selectedV0Candidates, ctx) );

    // sort and chop the main candidates
    std::sort( cascadeinfoContainer.begin(), cascadeinfoContainer.end(), [](Trk::VxCascadeInfo* a, Trk::VxCascadeInfo* b) { return a->fitChi2()/a->nDoF() < b->fitChi2()/b->nDoF(); } );
    if(m_maxMainVCandidates>0 && cascadeinfoContainer.size()>m_maxMainVCandidates) {
      for(auto it=cascadeinfoContainer.begin()+m_maxMainVCandidates; it!=cascadeinfoContainer.end(); it++) delete *it;
      cascadeinfoContainer.erase(cascadeinfoContainer.begin()+m_maxMainVCandidates, cascadeinfoContainer.end());
    }

    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key, ctx);
    ATH_CHECK( evt.isValid() );
    BPhysPVCascadeTools helper(&(*m_CascadeTools), evt.cptr());
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    // Decorators for the main vertex: chi2, ndf, pt and pt error, plus the V0 vertex variables
    SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
    SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
    SG::AuxElement::Decorator<int>   ndof_decor("nDoF");
    SG::AuxElement::Decorator<float> Pt_decor("Pt");
    SG::AuxElement::Decorator<float> PtErr_decor("PtErr");

    SG::AuxElement::Decorator<float> lxy_SV1_decor("lxy_SV1");
    SG::AuxElement::Decorator<float> lxyErr_SV1_decor("lxyErr_SV1");
    SG::AuxElement::Decorator<float> a0xy_SV1_decor("a0xy_SV1");
    SG::AuxElement::Decorator<float> a0xyErr_SV1_decor("a0xyErr_SV1");
    SG::AuxElement::Decorator<float> a0z_SV1_decor("a0z_SV1");
    SG::AuxElement::Decorator<float> a0zErr_SV1_decor("a0zErr_SV1");

    SG::AuxElement::Decorator<float> lxy_SV2_decor("lxy_SV2");
    SG::AuxElement::Decorator<float> lxyErr_SV2_decor("lxyErr_SV2");
    SG::AuxElement::Decorator<float> a0xy_SV2_decor("a0xy_SV2");
    SG::AuxElement::Decorator<float> a0xyErr_SV2_decor("a0xyErr_SV2");
    SG::AuxElement::Decorator<float> a0z_SV2_decor("a0z_SV2");
    SG::AuxElement::Decorator<float> a0zErr_SV2_decor("a0zErr_SV2");

    SG::AuxElement::Decorator<float> lxy_SV3_decor("lxy_SV3");
    SG::AuxElement::Decorator<float> lxyErr_SV3_decor("lxyErr_SV3");
    SG::AuxElement::Decorator<float> a0xy_SV3_decor("a0xy_SV3");
    SG::AuxElement::Decorator<float> a0xyErr_SV3_decor("a0xyErr_SV3");
    SG::AuxElement::Decorator<float> a0z_SV3_decor("a0z_SV3");
    SG::AuxElement::Decorator<float> a0zErr_SV3_decor("a0zErr_SV3");

    SG::AuxElement::Decorator<float> chi2_V3_decor("ChiSquared_V3");
    SG::AuxElement::Decorator<int>   ndof_V3_decor("nDoF_V3");

    for(auto cascade_info : cascadeinfoContainer) {
      if(cascade_info==nullptr) {
        ATH_MSG_ERROR("CascadeInfo is null");
        continue;
      }

      const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info->vertices();
      if(cascadeVertices.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices");
      for(size_t i=0; i<topoN; i++) {
	if(cascadeVertices[i]==nullptr) ATH_MSG_ERROR("Error null vertex");
      }

      cascade_info->setSVOwnership(false); // Prevent Container from deleting vertices
      const auto mainVertex = cascadeVertices[topoN-1]; // this is the mother vertex
      const std::vector< std::vector<TLorentzVector> > &moms = cascade_info->getParticleMoms();

      // Identify the input JX
      int ijx = m_JXSubVtx ? topoN-2 : topoN-1;
      const xAOD::Vertex* jxVtx(nullptr);
      if(m_jxDaug_num==4) jxVtx = FindVertex<4>(jxContainer.ptr(), cascadeVertices[ijx]);
      else if(m_jxDaug_num==3) jxVtx = FindVertex<3>(jxContainer.ptr(), cascadeVertices[ijx]);
      else jxVtx = FindVertex<2>(jxContainer.ptr(), cascadeVertices[ijx]);

      xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);

      // Get refitted track momenta from all vertices, charged tracks only
      BPhysPVCascadeTools::SetVectorInfo(vtx, cascade_info);
      vtx.setPass(true);

      //
      // Decorate main vertex
      //
      // mass, mass error
      // https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/VxCascadeInfo.h
      BPHYS_CHECK( vtx.setMass(m_CascadeTools->invariantMass(moms[topoN-1])) );
      BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[topoN-1],cascade_info->getCovariance()[topoN-1])) );
      // pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
      Pt_decor(*mainVertex)       = m_CascadeTools->pT(moms[topoN-1]);
      PtErr_decor(*mainVertex)    = m_CascadeTools->pTError(moms[topoN-1],cascade_info->getCovariance()[topoN-1]);
      // chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
      chi2_decor(*mainVertex)     = cascade_info->fitChi2();
      ndof_decor(*mainVertex)     = cascade_info->nDoF();

      // decorate the cascade vertices
      lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
      lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
      a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
      a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);

      if(m_JXSubVtx && m_JXV02SubVtx) {
	lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
	a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],cascadeVertices[2]);
	a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
	a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
      }
      else {
	lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
	a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
	a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      }

      if(m_JXSubVtx) {
	lxy_SV3_decor(*cascadeVertices[2])     = m_CascadeTools->lxy(moms[2],cascadeVertices[2],mainVertex);
	lxyErr_SV3_decor(*cascadeVertices[2])  = m_CascadeTools->lxyError(moms[2],cascade_info->getCovariance()[2],cascadeVertices[2],mainVertex);
	a0z_SV3_decor(*cascadeVertices[2])     = m_CascadeTools->a0z(moms[2],cascadeVertices[2],mainVertex);
	a0zErr_SV3_decor(*cascadeVertices[2])  = m_CascadeTools->a0zError(moms[2],cascade_info->getCovariance()[2],cascadeVertices[2],mainVertex);
	a0xy_SV3_decor(*cascadeVertices[2])    = m_CascadeTools->a0xy(moms[2],cascadeVertices[2],mainVertex);
	a0xyErr_SV3_decor(*cascadeVertices[2]) = m_CascadeTools->a0xyError(moms[2],cascade_info->getCovariance()[2],cascadeVertices[2],mainVertex);
      }

      chi2_V3_decor(*cascadeVertices[2])     = m_V0Tools->chisq(jxVtx);
      ndof_V3_decor(*cascadeVertices[2])     = m_V0Tools->ndof(jxVtx);

      double Mass_Moth = m_CascadeTools->invariantMass(moms[topoN-1]);
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, defaultPVContainer.cptr(), m_refitPV ? refPvContainer.ptr() : 0, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info, topoN-1, Mass_Moth, vtx));

      for(size_t i=0; i<topoN; i++) {
        VtxWriteHandles[i].ptr()->push_back(cascadeVertices[i]);
      }

      // Set links to cascade vertices
      VertexLinkVector cascadeVertexLinks;
      VertexLink vertexLink1;
      vertexLink1.setElement(cascadeVertices[0]);
      vertexLink1.setStorableObject(*VtxWriteHandles[0].ptr());
      if( vertexLink1.isValid() ) cascadeVertexLinks.push_back( vertexLink1 );
      VertexLink vertexLink2;
      vertexLink2.setElement(cascadeVertices[1]);
      vertexLink2.setStorableObject(*VtxWriteHandles[1].ptr());
      if( vertexLink2.isValid() ) cascadeVertexLinks.push_back( vertexLink2 );
      if(topoN==4) {
	VertexLink vertexLink3;
	vertexLink3.setElement(cascadeVertices[2]);
	vertexLink3.setStorableObject(*VtxWriteHandles[2].ptr());
	if( vertexLink3.isValid() ) cascadeVertexLinks.push_back( vertexLink3 );
      }
      CascadeLinksDecor(*mainVertex) = cascadeVertexLinks;
    } // loop over cascadeinfoContainer

    // Deleting cascadeinfo since this won't be stored.
    for (auto cascade_info : cascadeinfoContainer) {
      if(cascade_info) delete cascade_info;
    }

    return StatusCode::SUCCESS;
  }

  bool JpsiXPlus2V0::d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const {
    bool pass = false;
    const EventContext& ctx = Gaudi::Hive::currentContext();
    std::unique_ptr<Trk::Perigee> per = m_trackToVertexTool->perigeeAtVertex(ctx, *track, PV->position());
    if(!per) return pass;
    double d0 = per->parameters()[Trk::d0];
    double sig_d0 = sqrt((*per->covariance())(0,0));
    if(std::abs(d0/sig_d0) > m_d0_cut) pass = true;
    return pass;
  }

  Trk::VxCascadeInfo* JpsiXPlus2V0::fitMainVtx(const xAOD::Vertex* JXvtx, std::vector<double>& massesJX, const xAOD::Vertex* V01vtx, const V0Enum V01, const xAOD::Vertex* V02vtx, const V0Enum V02, const std::vector<const xAOD::TrackParticleContainer*>& trackCols, const xAOD::VertexContainer* defaultPVContainer, const xAOD::VertexContainer* pvContainer) const {
    Trk::VxCascadeInfo* result(nullptr);

    std::vector<const xAOD::TrackParticle*> tracksJX;
    for(size_t i=0; i<JXvtx->nTrackParticles(); i++) tracksJX.push_back(JXvtx->trackParticle(i));
    if (tracksJX.size() != massesJX.size()) {
      ATH_MSG_ERROR("Problems with JX input: number of tracks or track mass inputs is not correct!");
      return result;
    }
    std::vector<const xAOD::TrackParticle*> tracksV01;
    for(size_t j=0; j<V01vtx->nTrackParticles(); j++) tracksV01.push_back(V01vtx->trackParticle(j));
    std::vector<const xAOD::TrackParticle*> tracksV02;
    for(size_t j=0; j<V02vtx->nTrackParticles(); j++) tracksV02.push_back(V02vtx->trackParticle(j));

    std::vector<const xAOD::TrackParticle*> tracksJpsi{tracksJX[0], tracksJX[1]};
    std::vector<const xAOD::TrackParticle*> tracksX;
    if(m_jxDaug_num>=3) tracksX.push_back(tracksJX[2]);
    if(m_jxDaug_num==4) tracksX.push_back(tracksJX[3]);

    std::vector<double> massesV01;
    if(V01==LAMBDA)         massesV01 = m_massesV0_ppi;
    else if(V01==LAMBDABAR) massesV01 = m_massesV0_pip;
    else if(V01==KS)        massesV01 = m_massesV0_pipi;
    std::vector<double> massesV02;
    if(V02==LAMBDA)         massesV02 = m_massesV0_ppi;
    else if(V02==LAMBDABAR) massesV02 = m_massesV0_pip;
    else if(V02==KS)        massesV02 = m_massesV0_pipi;

    TLorentzVector p4_moth, p4_JX, p4_v01, p4_v02, tmp;
    for(size_t it=0; it<JXvtx->nTrackParticles(); it++) {
      tmp.SetPtEtaPhiM(JXvtx->trackParticle(it)->pt(), JXvtx->trackParticle(it)->eta(), JXvtx->trackParticle(it)->phi(), massesJX[it]);
      p4_moth += tmp; p4_JX += tmp;
    }
    xAOD::BPhysHelper V01_helper(V01vtx);
    for(int it=0; it<V01_helper.nRefTrks(); it++) {
      p4_moth += V01_helper.refTrk(it,massesV01[it]);
      p4_v01 += V01_helper.refTrk(it,massesV01[it]);
    }
    xAOD::BPhysHelper V02_helper(V02vtx);
    for(int it=0; it<V02_helper.nRefTrks(); it++) {
      p4_moth += V02_helper.refTrk(it,massesV02[it]);
      p4_v02 += V02_helper.refTrk(it,massesV02[it]);
    }
    double main_mass = p4_moth.M();
    if(m_useImprovedMass) {
      if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - p4_JX.M() + m_massJpsi;
      else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - p4_JX.M() + m_massJX;
      if((V01==LAMBDA || V01==LAMBDABAR) && m_massLd>0) main_mass += - p4_v01.M() + m_massLd;
      else if(V01==KS && m_massKs>0) main_mass += - p4_v01.M() + m_massKs;
      if((V02==LAMBDA || V02==LAMBDABAR) && m_massLd>0) main_mass += - p4_v02.M() + m_massLd;
      else if(V02==KS && m_massKs>0) main_mass += - p4_v02.M() + m_massKs;
    }
    if (main_mass < m_MassLower || main_mass > m_MassUpper) return result;

    if(m_JXSubVtx && m_JXV02SubVtx) {
      double JXV02_mass = (p4_JX+p4_v02).M();
      if(m_useImprovedMass) {
	if(m_jxDaug_num==2 && m_massJpsi>0) JXV02_mass += - p4_JX.M() + m_massJpsi;
	else if(m_jxDaug_num>=3 && m_massJX>0) JXV02_mass += - p4_JX.M() + m_massJX;
	if((V02==LAMBDA || V02==LAMBDABAR) && m_massLd>0) JXV02_mass += - p4_v02.M() + m_massLd;
	else if(V02==KS && m_massKs>0) JXV02_mass += - p4_v02.M() + m_massKs;
      }
      if (JXV02_mass < m_JXV02MassLower || JXV02_mass > m_JXV02MassUpper) return result;
    }

    xAOD::BPhysHelper::pv_type pvtype = xAOD::BPhysHelper::PV_MIN_A0;
    if(m_cascadeFitWithPV==1)      pvtype = xAOD::BPhysHelper::PV_MAX_SUM_PT2;
    else if(m_cascadeFitWithPV==2) pvtype = xAOD::BPhysHelper::PV_MIN_A0;
    else if(m_cascadeFitWithPV==3) pvtype = xAOD::BPhysHelper::PV_MIN_Z0;
    else if(m_cascadeFitWithPV==4) pvtype = xAOD::BPhysHelper::PV_MIN_Z0_BA;
    xAOD::BPhysHelper JX_helper(JXvtx);
    const xAOD::Vertex* origPv_xAOD = (m_cascadeFitWithPV>=1 && m_cascadeFitWithPV<=4) ? JX_helper.origPv(pvtype) : nullptr;
    const xAOD::Vertex* pv_xAOD = (m_cascadeFitWithPV>=1 && m_cascadeFitWithPV<=4) ? JX_helper.pv(pvtype) : nullptr;
    std::unique_ptr<Trk::RecVertex> pv_AOD;
    if(pv_xAOD) pv_AOD = std::make_unique<Trk::RecVertex>(pv_xAOD->position(),pv_xAOD->covariancePosition(),pv_xAOD->numberDoF(),pv_xAOD->chiSquared());

    SG::AuxElement::Decorator<float>       chi2_V1_decor("ChiSquared_V1");
    SG::AuxElement::Decorator<int>         ndof_V1_decor("nDoF_V1");
    SG::AuxElement::Decorator<std::string> type_V1_decor("Type_V1");
    SG::AuxElement::Decorator<float>       chi2_V2_decor("ChiSquared_V2");
    SG::AuxElement::Decorator<int>         ndof_V2_decor("nDoF_V2");
    SG::AuxElement::Decorator<std::string> type_V2_decor("Type_V2");

    SG::AuxElement::Accessor<int>    mAcc_gfit("gamma_fit");
    SG::AuxElement::Accessor<float>  mAcc_gmass("gamma_mass");
    SG::AuxElement::Accessor<float>  mAcc_gmasserr("gamma_massError");
    SG::AuxElement::Accessor<float>  mAcc_gchisq("gamma_chisq");
    SG::AuxElement::Accessor<int>    mAcc_gndof("gamma_ndof");
    SG::AuxElement::Accessor<float>  mAcc_gprob("gamma_probability");

    SG::AuxElement::Decorator<int>   mDec_gfit("gamma_fit");
    SG::AuxElement::Decorator<float> mDec_gmass("gamma_mass");
    SG::AuxElement::Decorator<float> mDec_gmasserr("gamma_massError");
    SG::AuxElement::Decorator<float> mDec_gchisq("gamma_chisq");
    SG::AuxElement::Decorator<int>   mDec_gndof("gamma_ndof");
    SG::AuxElement::Decorator<float> mDec_gprob("gamma_probability");
    SG::AuxElement::Decorator< std::vector<float> > trk_pxDeco("TrackPx_V0nc");
    SG::AuxElement::Decorator< std::vector<float> > trk_pyDeco("TrackPy_V0nc");
    SG::AuxElement::Decorator< std::vector<float> > trk_pzDeco("TrackPz_V0nc");

    std::vector<float> trk_px;
    std::vector<float> trk_py;
    std::vector<float> trk_pz;

    // Apply the user's settings to the fitter
    std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
    // Robustness: http://cdsweb.cern.ch/record/685551
    int robustness = 0;
    m_iVertexFitter->setRobustness(robustness, *state);
    // Build up the topology
    // Vertex list
    std::vector<Trk::VertexID> vrtList;
    // https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
    // V01 vertex
    Trk::VertexID vID1;
    if (m_constrV01) {
      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01,*state,V01==KS?m_massKs:m_massLd);
    } else {
      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01,*state);
    }
    vrtList.push_back(vID1);
    // V02 vertex
    Trk::VertexID vID2;
    if (m_constrV02) {
      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02,*state,V02==KS?m_massKs:m_massLd);
    } else {
      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02,*state);
    }
    vrtList.push_back(vID2);
    Trk::VertexID vID3;
    if(m_JXSubVtx) {
      if(m_JXV02SubVtx) { // for e.g. Lambda_b -> Jpsi + Lambda
	// JX+V02 vertex
	std::vector<Trk::VertexID> vrtList1{vID1};
	std::vector<Trk::VertexID> vrtList2{vID2};
	if (m_constrJXV02) {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,*state,m_massJXV02);
	} else {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,*state);
	}
	vrtList1.push_back(vID3);
	if (m_constrJX && m_jxDaug_num>2) {
	  std::vector<Trk::VertexID> cnstV; cnstV.clear();
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for JX failed");
	  }
	}
	// Mother vertex including JX and two V0's
	std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	std::vector<double> tp_masses; tp_masses.clear();
	if(m_constrMainV) {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList1,*state,m_massMainV);
	} else {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList1,*state);
	}
      }
      else { // no JXV02SubVtx
	// JX vertex
	if (m_constrJX && m_jxDaug_num>2) {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state,m_massJX);
	} else {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state);
	}
	vrtList.push_back(vID3);
	// Mother vertex including JX and two V0's
	std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	std::vector<double> tp_masses; tp_masses.clear();
	if(m_constrMainV) {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,*state,m_massMainV);
	} else {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,*state);
	}
      }
    }
    else { // m_JXSubVtx=false
      // Mother vertex including JX and two V0's
      if(m_constrMainV) {
	vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state,m_massMainV);
      } else {
	vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state);
      }
      if (m_constrJX && m_jxDaug_num>2) {
	std::vector<Trk::VertexID> cnstV; cnstV.clear();
	if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for JX failed");
	}
      }
    }

    if (m_constrJpsi) {
      std::vector<Trk::VertexID> cnstV; cnstV.clear();
      if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
      }
    }
    if (m_constrX && m_jxDaug_num==4 && m_massX>0) {
      std::vector<Trk::VertexID> cnstV; cnstV.clear();
      if ( !m_iVertexFitter->addMassConstraint(vID3,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	ATH_MSG_WARNING("addMassConstraint for X failed");
      }
    }
    // Do the work
    std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state, pv_AOD.get(), pv_AOD.get() && m_firstDecayAtPV ? true : false) );

    if (fit_result != nullptr) {
      for(auto v : fit_result->vertices()) {
	if(v->nTrackParticles()==0) {
	  std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
	  v->setTrackParticleLinks(nullLinkVector);
	}
      }
      // reset links to original tracks
      BPhysPVCascadeTools::PrepareVertexLinks(fit_result.get(), trackCols);

      // necessary to prevent memory leak
      fit_result->setSVOwnership(true);

      // Chi2/DOF cut
      double chi2DOF = fit_result->fitChi2()/fit_result->nDoF();
      bool chi2CutPassed = (m_chi2cut <= 0.0 || chi2DOF < m_chi2cut);

      const std::vector<std::vector<TLorentzVector> > &moms = fit_result->getParticleMoms();
      const std::vector<xAOD::Vertex*> &cascadeVertices = fit_result->vertices();
      size_t iMoth = cascadeVertices.size()-1;
      double lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[iMoth]);
      double lxy_SV2 = (m_JXSubVtx && m_JXV02SubVtx) ? m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]) : m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[iMoth]);
      if(chi2CutPassed && lxy_SV1>m_lxyV01_cut && lxy_SV2>m_lxyV02_cut) {
	chi2_V1_decor(*cascadeVertices[0]) = V01vtx->chiSquared();
	ndof_V1_decor(*cascadeVertices[0]) = V01vtx->numberDoF();
	if(V01==LAMBDA)         type_V1_decor(*cascadeVertices[0]) = "Lambda";
	else if(V01==LAMBDABAR) type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
	else if(V01==KS)        type_V1_decor(*cascadeVertices[0]) = "Ks";
	mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V01vtx) ? mAcc_gfit(*V01vtx) : 0;
	mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V01vtx) ? mAcc_gmass(*V01vtx) : -1;
	mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V01vtx) ? mAcc_gmasserr(*V01vtx) : -1;
	mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V01vtx) ? mAcc_gchisq(*V01vtx) : 999999;
	mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V01vtx) ? mAcc_gndof(*V01vtx) : 0;
	mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V01vtx) ? mAcc_gprob(*V01vtx) : -1;
	trk_px.clear(); trk_py.clear(); trk_pz.clear();
	for(int it=0; it<V01_helper.nRefTrks(); it++) {
	  trk_px.push_back( V01_helper.refTrk(it).Px() );
	  trk_py.push_back( V01_helper.refTrk(it).Py() );
	  trk_pz.push_back( V01_helper.refTrk(it).Pz() );
	}
	trk_pxDeco(*cascadeVertices[0]) = trk_px;
	trk_pyDeco(*cascadeVertices[0]) = trk_py;
	trk_pzDeco(*cascadeVertices[0]) = trk_pz;

	chi2_V2_decor(*cascadeVertices[1]) = V02vtx->chiSquared();
	ndof_V2_decor(*cascadeVertices[1]) = V02vtx->numberDoF();
	if(V02==LAMBDA)         type_V2_decor(*cascadeVertices[1]) = "Lambda";
	else if(V02==LAMBDABAR) type_V2_decor(*cascadeVertices[1]) = "Lambdabar";
	else if(V02==KS)        type_V2_decor(*cascadeVertices[1]) = "Ks";
	mDec_gfit(*cascadeVertices[1])     = mAcc_gfit.isAvailable(*V02vtx) ? mAcc_gfit(*V02vtx) : 0;
	mDec_gmass(*cascadeVertices[1])    = mAcc_gmass.isAvailable(*V02vtx) ? mAcc_gmass(*V02vtx) : -1;
	mDec_gmasserr(*cascadeVertices[1]) = mAcc_gmasserr.isAvailable(*V02vtx) ? mAcc_gmasserr(*V02vtx) : -1;
	mDec_gchisq(*cascadeVertices[1])   = mAcc_gchisq.isAvailable(*V02vtx) ? mAcc_gchisq(*V02vtx) : 999999;
	mDec_gndof(*cascadeVertices[1])    = mAcc_gndof.isAvailable(*V02vtx) ? mAcc_gndof(*V02vtx) : 0;
	mDec_gprob(*cascadeVertices[1])    = mAcc_gprob.isAvailable(*V02vtx) ? mAcc_gprob(*V02vtx) : -1;
	trk_px.clear(); trk_py.clear(); trk_pz.clear();
	for(int it=0; it<V02_helper.nRefTrks(); it++) {
	  trk_px.push_back( V02_helper.refTrk(it).Px() );
	  trk_py.push_back( V02_helper.refTrk(it).Py() );
	  trk_pz.push_back( V02_helper.refTrk(it).Pz() );
	}
	trk_pxDeco(*cascadeVertices[1]) = trk_px;
	trk_pyDeco(*cascadeVertices[1]) = trk_py;
	trk_pzDeco(*cascadeVertices[1]) = trk_pz;

	result = fit_result.release();
      }
    }

    if(pv_xAOD && result && result->getParticleMoms().size()>0) {
      size_t index = result->getParticleMoms().size() - 1;
      const std::vector<TLorentzVector> &mom = result->getParticleMoms()[index];
      const Amg::MatrixX &cov = result->getCovariance()[index];
      const xAOD::Vertex* mainVertex = result->vertices()[index];
      xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);
      bool isInDefaultPVCont = false;
      for(const xAOD::Vertex* pvVtx : *defaultPVContainer) {
	if(pv_xAOD == pvVtx) { isInDefaultPVCont = true; break; }
      }
      if(isInDefaultPVCont) vtx.setPv( pv_xAOD, defaultPVContainer, pvtype );
      else vtx.setPv( pv_xAOD, pvContainer, pvtype );
      if(origPv_xAOD) vtx.setOrigPv( origPv_xAOD, defaultPVContainer, pvtype );
      vtx.setLxy    ( m_CascadeTools->lxy      (mom, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setLxyErr ( m_CascadeTools->lxyError (mom, cov, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setA0     ( m_CascadeTools->a0       (mom, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setA0Err  ( m_CascadeTools->a0Error  (mom, cov, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setA0xy   ( m_CascadeTools->a0xy     (mom, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setA0xyErr( m_CascadeTools->a0xyError(mom, cov, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setZ0     ( m_CascadeTools->a0z      (mom, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setZ0Err  ( m_CascadeTools->a0zError (mom, cov, vtx.vtx(), pv_xAOD), pvtype );
      vtx.setRefitPVStatus( 0, pvtype );
      // Proper decay times
      vtx.setTau( m_CascadeTools->tau(mom, vtx.vtx(), pv_xAOD), pvtype, xAOD::BPhysHypoHelper::TAU_INV_MASS );
      vtx.setTauErr( m_CascadeTools->tauError(mom, cov, vtx.vtx(), pv_xAOD), pvtype, xAOD::BPhysHypoHelper::TAU_INV_MASS );
      vtx.setTau( m_CascadeTools->tau(mom, vtx.vtx(), pv_xAOD, m_massMainV), pvtype, xAOD::BPhysHypoHelper::TAU_CONST_MASS );
      vtx.setTauErr( m_CascadeTools->tauError(mom, cov, vtx.vtx(), pv_xAOD, m_massMainV), pvtype, xAOD::BPhysHypoHelper::TAU_CONST_MASS );
    }

    return result;
  }

  void JpsiXPlus2V0::fitV0Container(xAOD::VertexContainer* V0ContainerNew, const std::vector<const xAOD::TrackParticle*>& selectedTracks, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const {
    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::AuxElement::Decorator<std::string> mDec_type("Type_V0Vtx");
    SG::AuxElement::Decorator<int>         mDec_gfit("gamma_fit");
    SG::AuxElement::Decorator<float>       mDec_gmass("gamma_mass");
    SG::AuxElement::Decorator<float>       mDec_gmasserr("gamma_massError");
    SG::AuxElement::Decorator<float>       mDec_gchisq("gamma_chisq");
    SG::AuxElement::Decorator<int>         mDec_gndof("gamma_ndof");
    SG::AuxElement::Decorator<float>       mDec_gprob("gamma_probability");

    std::vector<const xAOD::TrackParticle*> posTracks;
    std::vector<const xAOD::TrackParticle*> negTracks;
    for(const xAOD::TrackParticle* TP : selectedTracks) {
      if(TP->charge()>0) posTracks.push_back(TP);
      else negTracks.push_back(TP);
    }

    for(const xAOD::TrackParticle* TP1 : posTracks) {
      const Trk::Perigee& aPerigee1 = TP1->perigeeParameters();
      for(const xAOD::TrackParticle* TP2 : negTracks) {
	const Trk::Perigee& aPerigee2 = TP2->perigeeParameters();
	int sflag(0), errorcode(0);
	Amg::Vector3D startingPoint = m_vertexEstimator->getCirclesIntersectionPoint(&aPerigee1,&aPerigee2,sflag,errorcode);
	if (errorcode != 0) {startingPoint(0) = 0.0; startingPoint(1) = 0.0; startingPoint(2) = 0.0;}

	if (errorcode == 0 || errorcode == 5 || errorcode == 6 || errorcode == 8) {
	  Trk::PerigeeSurface perigeeSurface(startingPoint);
	  const Trk::TrackParameters* extrapolatedPerigee1 = m_extrapolator->extrapolate(ctx,TP1->perigeeParameters(), perigeeSurface).release();
	  const Trk::TrackParameters* extrapolatedPerigee2 = m_extrapolator->extrapolate(ctx,TP2->perigeeParameters(), perigeeSurface).release();
	  std::vector<std::unique_ptr<const Trk::TrackParameters> > cleanup;
	  if(!extrapolatedPerigee1) extrapolatedPerigee1 = &TP1->perigeeParameters();
	  else cleanup.push_back(std::unique_ptr<const Trk::TrackParameters>(extrapolatedPerigee1));
	  if(!extrapolatedPerigee2) extrapolatedPerigee2 = &TP2->perigeeParameters();
	  else cleanup.push_back(std::unique_ptr<const Trk::TrackParameters>(extrapolatedPerigee2));
	  if(extrapolatedPerigee1 && extrapolatedPerigee2) {
	    bool pass = false;
	    TLorentzVector v1; TLorentzVector v2;
	    if(!pass) {
	      v1.SetXYZM(extrapolatedPerigee1->momentum().x(),extrapolatedPerigee1->momentum().y(),extrapolatedPerigee1->momentum().z(),m_mass_proton);
	      v2.SetXYZM(extrapolatedPerigee2->momentum().x(),extrapolatedPerigee2->momentum().y(),extrapolatedPerigee2->momentum().z(),m_mass_pion);
	      if((v1+v2).M()>1030.0 && (v1+v2).M()<1200.0) pass = true;
	    }
	    if(!pass) {
	      v1.SetXYZM(extrapolatedPerigee1->momentum().x(),extrapolatedPerigee1->momentum().y(),extrapolatedPerigee1->momentum().z(),m_mass_pion);
	      v2.SetXYZM(extrapolatedPerigee2->momentum().x(),extrapolatedPerigee2->momentum().y(),extrapolatedPerigee2->momentum().z(),m_mass_proton);
	      if((v1+v2).M()>1030.0 && (v1+v2).M()<1200.0) pass = true;
	    }
	    if(!pass) {
	      v1.SetXYZM(extrapolatedPerigee1->momentum().x(),extrapolatedPerigee1->momentum().y(),extrapolatedPerigee1->momentum().z(),m_mass_pion);
	      v2.SetXYZM(extrapolatedPerigee2->momentum().x(),extrapolatedPerigee2->momentum().y(),extrapolatedPerigee2->momentum().z(),m_mass_pion);
	      if((v1+v2).M()>430.0 && (v1+v2).M()<565.0) pass = true;
	    }
	    if(pass) {
	      std::vector<const xAOD::TrackParticle*> tracksV0;
	      tracksV0.push_back(TP1); tracksV0.push_back(TP2);
	      std::unique_ptr<xAOD::Vertex> V0vtx = std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, startingPoint) );
	      if(V0vtx && V0vtx->chiSquared()>=0) {
		double chi2DOF = V0vtx->chiSquared()/V0vtx->numberDoF();
		if(chi2DOF>m_chi2cut_V0) continue;

		double massSig_V0_Lambda1 = std::abs(m_V0Tools->invariantMass(V0vtx.get(), m_massesV0_ppi)-m_mass_Lambda)/m_V0Tools->invariantMassError(V0vtx.get(), m_massesV0_ppi);
		double massSig_V0_Lambda2 = std::abs(m_V0Tools->invariantMass(V0vtx.get(), m_massesV0_pip)-m_mass_Lambda)/m_V0Tools->invariantMassError(V0vtx.get(), m_massesV0_pip);
		double massSig_V0_Ks = std::abs(m_V0Tools->invariantMass(V0vtx.get(), m_massesV0_pipi)-m_mass_Ks)/m_V0Tools->invariantMassError(V0vtx.get(), m_massesV0_pipi);
		if(massSig_V0_Lambda1<=massSig_V0_Lambda2 && massSig_V0_Lambda1<=massSig_V0_Ks) {
		  mDec_type(*V0vtx.get()) = "Lambda";
		}
		else if(massSig_V0_Lambda2<=massSig_V0_Lambda1 && massSig_V0_Lambda2<=massSig_V0_Ks) {
		  mDec_type(*V0vtx.get()) = "Lambdabar";
		}
		else if(massSig_V0_Ks<=massSig_V0_Lambda1 && massSig_V0_Ks<=massSig_V0_Lambda2) {
		  mDec_type(*V0vtx.get()) = "Ks";
		}

		int gamma_fit = 0; int gamma_ndof = 0; double gamma_chisq = 999999.;
		double gamma_prob = -1., gamma_mass = -1., gamma_massErr = -1.;
		std::unique_ptr<xAOD::Vertex> gammaVtx = std::unique_ptr<xAOD::Vertex>( m_iGammaFitter->fit(tracksV0, m_V0Tools->vtx(V0vtx.get())) );
		if (gammaVtx) {
		  gamma_fit     = 1;
		  gamma_mass    = m_V0Tools->invariantMass(gammaVtx.get(),m_mass_e,m_mass_e);
		  gamma_massErr = m_V0Tools->invariantMassError(gammaVtx.get(),m_mass_e,m_mass_e);
		  gamma_chisq   = m_V0Tools->chisq(gammaVtx.get());
		  gamma_ndof    = m_V0Tools->ndof(gammaVtx.get());
		  gamma_prob    = m_V0Tools->vertexProbability(gammaVtx.get());
		}
		mDec_gfit(*V0vtx.get())     = gamma_fit;
		mDec_gmass(*V0vtx.get())    = gamma_mass;
		mDec_gmasserr(*V0vtx.get()) = gamma_massErr;
		mDec_gchisq(*V0vtx.get())   = gamma_chisq;
		mDec_gndof(*V0vtx.get())    = gamma_ndof;
		mDec_gprob(*V0vtx.get())    = gamma_prob;

		xAOD::BPhysHelper V0_helper(V0vtx.get());
		V0_helper.setRefTrks(); // AOD only method

		if(not trackCols.empty()){
		  try {
		    JpsiUpsilonCommon::RelinkVertexTracks(trackCols, V0vtx.get());
		  } catch (std::runtime_error const& e) {
		    ATH_MSG_ERROR(e.what());
		    return;
		  }
		}

		V0ContainerNew->push_back(std::move(V0vtx));
	      }
	    }
	  }
	}
      }
    }
  }

  template<size_t NTracks>
  const xAOD::Vertex* JpsiXPlus2V0::FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const {
    for (const xAOD::Vertex* v1 : *cont) {
      assert(v1->nTrackParticles() == NTracks);
      std::array<const xAOD::TrackParticle*, NTracks> a1;
      std::array<const xAOD::TrackParticle*, NTracks> a2;
      for(size_t i=0; i<NTracks; i++){
	a1[i] = v1->trackParticle(i);
	a2[i] = v->trackParticle(i);
      }
      std::sort(a1.begin(), a1.end());
      std::sort(a2.begin(), a2.end());
      if(a1 == a2) return v1;
    }
    return nullptr;
  }
}
