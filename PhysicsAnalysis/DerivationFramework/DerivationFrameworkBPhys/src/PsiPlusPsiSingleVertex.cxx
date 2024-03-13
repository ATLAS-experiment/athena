/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// PsiPlusPsiSingleVertex.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/PsiPlusPsiSingleVertex.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "HepPDT/ParticleDataTable.hh"
#include "TruthUtils/HepMCHelpers.h"
#include <algorithm>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;
  typedef ElementLink<xAOD::TrackParticleContainer> TrackParticleLink;
  typedef std::vector<TrackParticleLink> TrackParticleLinkVector;

  PsiPlusPsiSingleVertex::PsiPlusPsiSingleVertex(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexPsi1ContainerKey(""),
    m_vertexPsi2ContainerKey(""),
    m_outputsKeys({"Psi1Vtx", "Psi2Vtx", "MainVtx"}),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_trackContainerName("InDetTrackParticles"),
    m_eventInfo_key("EventInfo"),
    m_jpsi1MassLower(0.0),
    m_jpsi1MassUpper(20000.0),
    m_diTrack1MassLower(-1.0),
    m_diTrack1MassUpper(-1.0),
    m_psi1MassLower(0.0),
    m_psi1MassUpper(25000.0),
    m_jpsi2MassLower(0.0),
    m_jpsi2MassUpper(20000.0),
    m_diTrack2MassLower(-1.0),
    m_diTrack2MassUpper(-1.0),
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
    m_maxCandidates(0),
    m_mass_jpsi1(-1),
    m_mass_diTrk1(-1),
    m_mass_psi1(-1),
    m_mass_jpsi2(-1),
    m_mass_diTrk2(-1),
    m_mass_psi2(-1),
    m_constrJpsi1(false),
    m_constrDiTrk1(false),
    m_constrPsi1(false),
    m_constrJpsi2(false),
    m_constrDiTrk2(false),
    m_constrPsi2(false),
    m_chi2cut(-1.0),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter", this),
    m_V0Tools("Trk::V0Tools")
  {
    declareProperty("Psi1Vertices",               m_vertexPsi1ContainerKey);
    declareProperty("Psi2Vertices",               m_vertexPsi2ContainerKey);
    declareProperty("Psi1VtxHypoNames",           m_vertexPsi1HypoNames);
    declareProperty("Psi2VtxHypoNames",           m_vertexPsi2HypoNames);
    declareProperty("VxPrimaryCandidateName",     m_VxPrimaryCandidateName);
    declareProperty("TrackContainerName",         m_trackContainerName);
    declareProperty("RefPVContainerName",         m_refPVContainerName = "RefittedPrimaryVertices");
    declareProperty("Jpsi1MassLowerCut",          m_jpsi1MassLower);
    declareProperty("Jpsi1MassUpperCut",          m_jpsi1MassUpper);
    declareProperty("DiTrack1MassLower",          m_diTrack1MassLower); // only effective when m_vtx1Daug_num=4
    declareProperty("DiTrack1MassUpper",          m_diTrack1MassUpper); // only effective when m_vtx1Daug_num=4
    declareProperty("Psi1MassLowerCut",           m_psi1MassLower);
    declareProperty("Psi1MassUpperCut",           m_psi1MassUpper);
    declareProperty("Jpsi2MassLowerCut",          m_jpsi2MassLower);
    declareProperty("Jpsi2MassUpperCut",          m_jpsi2MassUpper);
    declareProperty("DiTrack2MassLower",          m_diTrack2MassLower); // only effective when m_vtx2Daug_num=4
    declareProperty("DiTrack2MassUpper",          m_diTrack2MassUpper); // only effective when m_vtx2Daug_num=4
    declareProperty("Psi2MassLowerCut",           m_psi2MassLower);
    declareProperty("Psi2MassUpperCut",           m_psi2MassUpper);
    declareProperty("MassLowerCut",               m_MassLower);
    declareProperty("MassUpperCut",               m_MassUpper);
    declareProperty("HypothesisName",             m_hypoName = "TQ");
    declareProperty("NumberOfPsi1Daughters",      m_vtx1Daug_num);
    declareProperty("Vtx1Daug1MassHypo",          m_vtx1Daug1MassHypo);
    declareProperty("Vtx1Daug2MassHypo",          m_vtx1Daug2MassHypo);
    declareProperty("Vtx1Daug3MassHypo",          m_vtx1Daug3MassHypo);
    declareProperty("Vtx1Daug4MassHypo",          m_vtx1Daug4MassHypo);
    declareProperty("NumberOfPsi2Daughters",      m_vtx2Daug_num);
    declareProperty("Vtx2Daug1MassHypo",          m_vtx2Daug1MassHypo);
    declareProperty("Vtx2Daug2MassHypo",          m_vtx2Daug2MassHypo);
    declareProperty("Vtx2Daug3MassHypo",          m_vtx2Daug3MassHypo);
    declareProperty("Vtx2Daug4MassHypo",          m_vtx2Daug4MassHypo);
    declareProperty("MaxCandidates",              m_maxCandidates);
    declareProperty("Jpsi1Mass",                  m_mass_jpsi1);
    declareProperty("DiTrack1Mass",               m_mass_diTrk1);
    declareProperty("Psi1Mass",                   m_mass_psi1);
    declareProperty("Jpsi2Mass",                  m_mass_jpsi2);
    declareProperty("DiTrack2Mass",               m_mass_diTrk2);
    declareProperty("Psi2Mass",                   m_mass_psi2);
    declareProperty("ApplyJpsi1MassConstraint",   m_constrJpsi1);
    declareProperty("ApplyDiTrk1MassConstraint",  m_constrDiTrk1); // only effective when m_vtx1Daug_num=4
    declareProperty("ApplyPsi1MassConstraint",    m_constrPsi1);
    declareProperty("ApplyJpsi2MassConstraint",   m_constrJpsi2);
    declareProperty("ApplyDiTrk2MassConstraint",  m_constrDiTrk2); // only effective when m_vtx2Daug_num=4
    declareProperty("ApplyPsi2MassConstraint",    m_constrPsi2);
    declareProperty("Chi2Cut",                    m_chi2cut);
    declareProperty("RefitPV",                    m_refitPV         = true);
    declareProperty("MaxnPV",                     m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",             m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",               m_DoVertexType    = 7);
    declareProperty("TrkVertexFitterTool",        m_iVertexFitter);
    declareProperty("PVRefitter",                 m_pvRefitter);
    declareProperty("V0Tools",                    m_V0Tools);
    declareProperty("OutputVertexCollections",    m_outputsKeys);
  }

  StatusCode PsiPlusPsiSingleVertex::initialize() {
    // retrieving vertex Fitter
    ATH_CHECK( m_iVertexFitter.retrieve() );

    // retrieve PV refitter
    ATH_CHECK( m_pvRefitter.retrieve() );

    // retrieving the V0 tools
    ATH_CHECK( m_V0Tools.retrieve() );

    ATH_CHECK( m_vertexPsi1ContainerKey.initialize() );
    ATH_CHECK( m_vertexPsi2ContainerKey.initialize() );
    ATH_CHECK( m_VxPrimaryCandidateName.initialize() );
    ATH_CHECK( m_trackContainerName.initialize() );
    ATH_CHECK( m_refPVContainerName.initialize() );
    ATH_CHECK( m_outputsKeys.initialize() );
    ATH_CHECK( m_eventInfo_key.initialize() );

    IPartPropSvc* partPropSvc = nullptr;
    ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
    auto pdt = partPropSvc->PDT();

    // retrieve particle masses
    // https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/TruthUtils/TruthUtils/AtlasPID.h
    if(m_mass_jpsi1 < 0.) m_mass_jpsi1 = BPhysPVCascadeTools::getParticleMass(pdt, MC::JPSI);
    if(m_mass_jpsi2 < 0.) m_mass_jpsi2 = BPhysPVCascadeTools::getParticleMass(pdt, MC::JPSI);
    if(m_mass_psi1 < 0.) m_mass_psi1 = BPhysPVCascadeTools::getParticleMass(pdt, MC::PSI2S);
    if(m_mass_psi2 < 0.) m_mass_psi2 = BPhysPVCascadeTools::getParticleMass(pdt, MC::PSI2S);

    if(m_vtx1Daug1MassHypo < 0.) m_vtx1Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::MUON);
    if(m_vtx1Daug2MassHypo < 0.) m_vtx1Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::MUON);
    if(m_vtx1Daug_num>=3 && m_vtx1Daug3MassHypo < 0.) m_vtx1Daug3MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::PIPLUS);
    if(m_vtx1Daug_num==4 && m_vtx1Daug4MassHypo < 0.) m_vtx1Daug4MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::PIPLUS);
    if(m_vtx2Daug1MassHypo < 0.) m_vtx2Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::MUON);
    if(m_vtx2Daug2MassHypo < 0.) m_vtx2Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::MUON);
    if(m_vtx2Daug_num>=3 && m_vtx2Daug3MassHypo < 0.) m_vtx2Daug3MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::PIPLUS);
    if(m_vtx2Daug_num==4 && m_vtx2Daug4MassHypo < 0.) m_vtx2Daug4MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, MC::PIPLUS);

    return StatusCode::SUCCESS;
  }

  StatusCode PsiPlusPsiSingleVertex::addBranches() const {
    if (m_vtx1Daug_num < 2 || m_vtx1Daug_num > 4 || m_vtx2Daug_num < 2 || m_vtx2Daug_num > 4) {
      ATH_MSG_FATAL("Incorrect number of Psi daughters");
      return StatusCode::FAILURE;
    }

    constexpr int topoN = 3;
    if(m_outputsKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of VtxContainers");
      return StatusCode::FAILURE;
    }
    std::array<SG::WriteHandle<xAOD::VertexContainer>, topoN> VtxWriteHandles; int ikey(0);
    for(const SG::WriteHandleKey<xAOD::VertexContainer>& key : m_outputsKeys) {
      VtxWriteHandles[ikey] = SG::WriteHandle<xAOD::VertexContainer>(key);
      ATH_CHECK( VtxWriteHandles[ikey].record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
      ikey++;
    }

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    SG::ReadHandle<xAOD::VertexContainer> pvContainer(m_VxPrimaryCandidateName);
    ATH_CHECK( pvContainer.isValid() );
    if (pvContainer.cptr()->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer.cptr()->size());
      return StatusCode::RECOVERABLE;
    }

    // Get TrackParticle container (for setting links to the original tracks)
    SG::ReadHandle<xAOD::TrackParticleContainer> trackContainer(m_trackContainerName);
    ATH_CHECK( trackContainer.isValid() );

    std::vector<const xAOD::TrackParticle*> tracksJpsi1;
    std::vector<const xAOD::TrackParticle*> tracksDiTrk1;
    std::vector<const xAOD::TrackParticle*> tracksPsi1;
    std::vector<const xAOD::TrackParticle*> tracksJpsi2;
    std::vector<const xAOD::TrackParticle*> tracksDiTrk2;
    std::vector<const xAOD::TrackParticle*> tracksPsi2;
    std::vector<const xAOD::TrackParticle*> inputTracks;
    std::vector<double> massesPsi1;
    massesPsi1.push_back(m_vtx1Daug1MassHypo);
    massesPsi1.push_back(m_vtx1Daug2MassHypo);
    if(m_vtx1Daug_num>=3) massesPsi1.push_back(m_vtx1Daug3MassHypo);
    if(m_vtx1Daug_num==4) massesPsi1.push_back(m_vtx1Daug4MassHypo);
    std::vector<double> massesPsi2;
    massesPsi2.push_back(m_vtx2Daug1MassHypo);
    massesPsi2.push_back(m_vtx2Daug2MassHypo);
    if(m_vtx2Daug_num>=3) massesPsi2.push_back(m_vtx2Daug3MassHypo);
    if(m_vtx2Daug_num==4) massesPsi2.push_back(m_vtx2Daug4MassHypo);
    std::vector<double> massesInputTracks;
    massesInputTracks.reserve(massesPsi1.size() + massesPsi2.size());
    for(auto mass : massesPsi1) massesInputTracks.push_back(mass);
    for(auto mass : massesPsi2) massesInputTracks.push_back(mass);

    // Get Psi1 container
    SG::ReadHandle<xAOD::VertexContainer> psi1Container(m_vertexPsi1ContainerKey);
    ATH_CHECK( psi1Container.isValid() );
    SG::ReadHandle<xAOD::VertexContainer> psi2Container(m_vertexPsi2ContainerKey);
    ATH_CHECK( psi2Container.isValid() );

    // Select the psi1 candidates before calling fit
    std::vector<const xAOD::Vertex*> selectedPsi1Candidates;
    for(auto vxcItr=psi1Container->cbegin(); vxcItr!=psi1Container->cend(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexPsi1HypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsi1HypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed = true;
	}
      }
      if(m_vertexPsi1HypoNames.size() && !passed) continue;

      // Check psi1 candidate invariant mass and skip if need be
      if(m_vtx1Daug_num>2) {
	double mass_psi1 = m_V0Tools->invariantMass(*vxcItr, massesPsi1);
	if (mass_psi1 < m_psi1MassLower || mass_psi1 > m_psi1MassUpper) continue;
      }

      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( (*vxcItr)->trackParticle(0)->pt(),
			   (*vxcItr)->trackParticle(0)->eta(),
			   (*vxcItr)->trackParticle(0)->phi(), m_vtx1Daug1MassHypo);
      p4_mu2.SetPtEtaPhiM( (*vxcItr)->trackParticle(1)->pt(),
			   (*vxcItr)->trackParticle(1)->eta(),
			   (*vxcItr)->trackParticle(1)->phi(), m_vtx1Daug2MassHypo);
      double mass_jpsi1 = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi1 < m_jpsi1MassLower || mass_jpsi1 > m_jpsi1MassUpper) continue;

      if(m_vtx1Daug_num==4 && m_diTrack1MassLower>=0 && m_diTrack1MassUpper>m_diTrack1MassLower) {
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

      selectedPsi1Candidates.push_back(*vxcItr);
    }

    // Select the psi2 candidates before calling fit
    std::vector<const xAOD::Vertex*> selectedPsi2Candidates;
    for(auto vxcItr=psi2Container->cbegin(); vxcItr!=psi2Container->cend(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexPsi2HypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexPsi2HypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed = true;
	}
      }
      if(m_vertexPsi2HypoNames.size() && !passed) continue;

      // Check psi2 candidate invariant mass and skip if need be
      if(m_vtx2Daug_num>2) {
	double mass_psi2 = m_V0Tools->invariantMass(*vxcItr,massesPsi2);
	if(mass_psi2 < m_psi2MassLower || mass_psi2 > m_psi2MassUpper) continue;
      }

      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( (*vxcItr)->trackParticle(0)->pt(),
			   (*vxcItr)->trackParticle(0)->eta(),
			   (*vxcItr)->trackParticle(0)->phi(), m_vtx2Daug1MassHypo);
      p4_mu2.SetPtEtaPhiM( (*vxcItr)->trackParticle(1)->pt(),
			   (*vxcItr)->trackParticle(1)->eta(),
			   (*vxcItr)->trackParticle(1)->phi(), m_vtx2Daug2MassHypo);
      double mass_jpsi2 = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi2 < m_jpsi2MassLower || mass_jpsi2 > m_jpsi2MassUpper) continue;

      if(m_vtx2Daug_num==4 && m_diTrack2MassLower>=0 && m_diTrack2MassUpper>m_diTrack2MassLower) {
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
      selectedPsi2Candidates.push_back(*vxcItr);
    }

    std::vector<std::pair<const xAOD::Vertex*, const xAOD::Vertex*> > candidatePairs;
    for(auto psi1Itr=selectedPsi1Candidates.cbegin(); psi1Itr!=selectedPsi1Candidates.cend(); ++psi1Itr) {
      tracksPsi1.clear();
      tracksPsi1.reserve((*psi1Itr)->nTrackParticles());
      for(size_t i=0; i<(*psi1Itr)->nTrackParticles(); i++) tracksPsi1.push_back((*psi1Itr)->trackParticle(i));
      for(auto psi2Itr=selectedPsi2Candidates.cbegin(); psi2Itr!=selectedPsi2Candidates.cend(); ++psi2Itr) {
	bool skip = false;
	for(size_t j=0; j<(*psi2Itr)->nTrackParticles(); j++) {
	  if(std::find(tracksPsi1.cbegin(), tracksPsi1.cend(), (*psi2Itr)->trackParticle(j)) != tracksPsi1.cend()) { skip = true; break; }
	}
	if(skip) continue;
	if(m_vertexPsi1ContainerKey.key() == m_vertexPsi2ContainerKey.key()) {
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
      tracksPsi1.reserve(psi1Vertex->nTrackParticles());
      for(size_t it=0; it<psi1Vertex->nTrackParticles(); it++) tracksPsi1.push_back(psi1Vertex->trackParticle(it));
      if (tracksPsi1.size() != massesPsi1.size()) {
	ATH_MSG_ERROR("Problems with Psi1 input: number of tracks or track mass inputs is not correct!");
      }
      tracksPsi2.clear();
      tracksPsi2.reserve(psi2Vertex->nTrackParticles());
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

      inputTracks.clear();
      inputTracks.push_back(psi1Vertex->trackParticle(0));
      inputTracks.push_back(psi1Vertex->trackParticle(1));
      if(m_vtx1Daug_num>=3) inputTracks.push_back(psi1Vertex->trackParticle(2));
      if(m_vtx1Daug_num==4) inputTracks.push_back(psi1Vertex->trackParticle(3));
      inputTracks.push_back(psi2Vertex->trackParticle(0));
      inputTracks.push_back(psi2Vertex->trackParticle(1));
      if(m_vtx2Daug_num>=3) inputTracks.push_back(psi2Vertex->trackParticle(2));
      if(m_vtx2Daug_num==4) inputTracks.push_back(psi2Vertex->trackParticle(3));

      // start the fit
      std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
      m_iVertexFitter->setMassInputParticles(massesInputTracks, *state);
      if (m_constrJpsi1) {
	m_iVertexFitter->setMassForConstraint(m_mass_jpsi1, std::vector<int>{1,2}, *state);
      }
      if (m_constrPsi1 && m_vtx1Daug_num>=3) {
	m_iVertexFitter->setMassForConstraint(m_mass_psi1, m_vtx1Daug_num==4 ? std::vector<int>{1,2,3,4} : std::vector<int>{1,2,3}, *state);
      }
      if (m_constrDiTrk1 && m_vtx1Daug_num==4) {
	m_iVertexFitter->setMassForConstraint(m_mass_diTrk1, std::vector<int>{3,4}, *state);
      }
      if (m_constrJpsi2) {
	m_iVertexFitter->setMassForConstraint(m_mass_jpsi2, std::vector<int>{m_vtx1Daug_num+1,m_vtx1Daug_num+2}, *state);
      }
      if (m_constrPsi2 && m_vtx2Daug_num>=3) {
	m_iVertexFitter->setMassForConstraint(m_mass_psi2, m_vtx1Daug_num==4 ? std::vector<int>{m_vtx1Daug_num+1,m_vtx1Daug_num+2,m_vtx1Daug_num+3,m_vtx1Daug_num+4} : std::vector<int>{m_vtx1Daug_num+1,m_vtx1Daug_num+2,m_vtx1Daug_num+3}, *state);
      }
      if (m_constrDiTrk2 && m_vtx2Daug_num==4) {
	m_iVertexFitter->setMassForConstraint(m_mass_diTrk2, std::vector<int>{m_vtx1Daug_num+3,m_vtx1Daug_num+4}, *state);
      }

      // Starting point
      Amg::Vector3D startingPoint((psi1Vertex->x()+psi2Vertex->x())/2,(psi1Vertex->y()+psi2Vertex->y())/2,(psi1Vertex->z()+psi2Vertex->z())/2);

      // do the fit
      std::unique_ptr<xAOD::Vertex> theResult( m_iVertexFitter->fit(inputTracks, startingPoint, *state) );

      if(theResult != nullptr){
	// Chi2/DOF cut
	double chi2DOF = theResult->chiSquared()/theResult->numberDoF();
	bool chi2CutPassed = (m_chi2cut <= 0.0 || chi2DOF < m_chi2cut);
	if(chi2CutPassed) {
	  TrackParticleLinkVector tpLinkVector;
	  for(size_t i=0; i<theResult->trackParticleLinks().size(); i++) {
	    TrackParticleLink mylink = theResult->trackParticleLinks()[i];
	    mylink.setStorableObject(*trackContainer.ptr(), true);
	    tpLinkVector.push_back( mylink );
	  }
	  theResult->clearTracks();
	  theResult->setTrackParticleLinks( tpLinkVector );

	  xAOD::Vertex* psi1Vertex_ = new xAOD::Vertex(*psi1Vertex);
	  xAOD::BPhysHelper psi1_helper(psi1Vertex_);
	  psi1_helper.setRefTrks();
	  TrackParticleLinkVector tpLinkVector_psi1;
	  for(size_t i=0; i<psi1Vertex_->trackParticleLinks().size(); i++) {
	    TrackParticleLink mylink = psi1Vertex_->trackParticleLinks()[i];
	    mylink.setStorableObject(*trackContainer.ptr(), true);
	    tpLinkVector_psi1.push_back( mylink );
	  }
	  psi1Vertex_->clearTracks();
	  psi1Vertex_->setTrackParticleLinks( tpLinkVector_psi1 );

	  xAOD::Vertex* psi2Vertex_ = new xAOD::Vertex(*psi2Vertex);
	  xAOD::BPhysHelper psi2_helper(psi2Vertex_);
	  psi2_helper.setRefTrks();
	  TrackParticleLinkVector tpLinkVector_psi2;
	  for(size_t i=0; i<psi2Vertex_->trackParticleLinks().size(); i++) {
	    TrackParticleLink mylink = psi2Vertex_->trackParticleLinks()[i];
	    mylink.setStorableObject(*trackContainer.ptr(), true);
	    tpLinkVector_psi2.push_back( mylink );
	  }
	  psi2Vertex_->clearTracks();
	  psi2Vertex_->setTrackParticleLinks( tpLinkVector_psi2 );

	  VtxWriteHandles[0].ptr()->push_back(psi1Vertex_);
	  VtxWriteHandles[1].ptr()->push_back(psi2Vertex_);

	  // Set links to preceding vertices
	  VertexLinkVector precedingVertexLinks;
	  VertexLink vertexLink1;
	  vertexLink1.setElement(psi1Vertex_);
	  vertexLink1.setStorableObject(*VtxWriteHandles[0].ptr());
	  if( vertexLink1.isValid() ) precedingVertexLinks.push_back( vertexLink1 );
	  VertexLink vertexLink2;
	  vertexLink2.setElement(psi2Vertex_);
	  vertexLink2.setStorableObject(*VtxWriteHandles[1].ptr());
	  if( vertexLink2.isValid() ) precedingVertexLinks.push_back( vertexLink2 );

	  SG::AuxElement::Decorator<VertexLinkVector> PrecedingLinksDecor("PrecedingVertexLinks");
	  PrecedingLinksDecor(*theResult.get()) = precedingVertexLinks;

	  xAOD::BPhysHypoHelper vtx(m_hypoName, theResult.get());
	  vtx.setRefTrks();
	  vtx.setPass(true);

	  VtxWriteHandles[2].ptr()->push_back(theResult.release());  //ptr is released preventing deletion
	}
      }
    } //Iterate over candidatePairs vertices

    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key);
    ATH_CHECK( evt.isValid() );
    BPhysPVTools helper(&(*m_V0Tools), evt.cptr());
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    if(m_refitPV) {
      SG::WriteHandle<xAOD::VertexContainer> refPvContainer(m_refPVContainerName);
      ATH_CHECK( refPvContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
      
      if(VtxWriteHandles[2]->size()>0) {
	for(int i=0; i<topoN; i++) {
	  StatusCode SC = helper.FillCandwithRefittedVertices(VtxWriteHandles[i].ptr(), pvContainer.cptr(), refPvContainer.ptr(), &(*m_pvRefitter), m_PV_max, m_DoVertexType);
	  if(SC.isFailure()){
	    ATH_MSG_FATAL("FillCandwithRefittedVertices failed - check the vertices you passed");
	    return SC;
	  }
	}
      }
    }
    else {
      if(VtxWriteHandles[2]->size()>0) {
	for(int i=0; i<topoN; i++) {
	  StatusCode SC = helper.FillCandExistingVertices(VtxWriteHandles[i].ptr(), pvContainer.cptr(), m_DoVertexType);
	  if(SC.isFailure()){
	    ATH_MSG_FATAL("FillCandExistingVertices failed - check the vertices you passed");
	    return SC;
	  }
	}
      }
    }

    return StatusCode::SUCCESS;
  }
}
