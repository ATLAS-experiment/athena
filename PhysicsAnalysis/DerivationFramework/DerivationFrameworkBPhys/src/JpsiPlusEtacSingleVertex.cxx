/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#include "DerivationFrameworkBPhys/JpsiPlusEtacSingleVertex.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetBeamSpotService/IBeamCondSvc.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "HepPDT/ParticleDataTable.hh"
#include <algorithm>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;
  typedef ElementLink<xAOD::TrackParticleContainer> TrackParticleLink;
  typedef std::vector<TrackParticleLink> TrackParticleLinkVector;

  struct RhoCandidate {
    const xAOD::TrackParticle* trackParticle1 = nullptr;
    const xAOD::TrackParticle* trackParticle2 = nullptr;
    double chi2NDF;
    Amg::Vector3D vtxPos;
  };

  struct EtacCandidate {
    const xAOD::TrackParticle* trackParticle1 = nullptr;
    const xAOD::TrackParticle* trackParticle2 = nullptr;
    const xAOD::TrackParticle* trackParticle3 = nullptr;
    const xAOD::TrackParticle* trackParticle4 = nullptr;
    double ptTot;
    double chi2NDFSum;
    Amg::Vector3D vtxPos1;
    Amg::Vector3D vtxPos2;
  };

  JpsiPlusEtacSingleVertex::JpsiPlusEtacSingleVertex(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_outputKey("OutputVtxContainer"),
    m_vertexContainerKey("InputJpsiContainer"),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(3500.0),
    m_rho1MassLower(0.0),
    m_rho1MassUpper(2500.0),
    m_rho2MassLower(0.0),
    m_rho2MassUpper(2500.0),
    m_etacMassLower(0.0),
    m_etacMassUpper(3500.0),
    m_MassLower(0.0),
    m_MassUpper(31000.0),
    m_constrJpsi(false),
    m_constrRho1(false),
    m_constrRho2(false),
    m_constrEtac(false),
    m_chi2cut_jpsi(-1.0),
    m_chi2cut_rho(-1.0),
    m_chi2cut(-1.0),
    m_beamSpotSvc("BeamCondSvc",name),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_trkSelector("InDet::TrackSelectorTool"),
    m_vertexEstimator("InDet::VertexPointEstimator"),
    m_refPVContainerName("RefittedPrimaryVertices")
  {
    declareProperty("JpsiVertices",               m_vertexContainerKey);
    declareProperty("JpsiVtxHypoNames",           m_vertexJpsiHypoNames);
    declareProperty("JpsiMassLowerCut",           m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",           m_jpsiMassUpper);
    declareProperty("Rho1MassLowerCut",           m_rho1MassLower);
    declareProperty("Rho1MassUpperCut",           m_rho1MassUpper);
    declareProperty("Rho2MassLowerCut",           m_rho2MassLower);
    declareProperty("Rho2MassUpperCut",           m_rho2MassUpper);
    declareProperty("EtacMassLowerCut",           m_etacMassLower);
    declareProperty("EtacMassUpperCut",           m_etacMassUpper);
    declareProperty("MassLowerCut",               m_MassLower);
    declareProperty("MassUpperCut",               m_MassUpper);
    declareProperty("ApplyJpsiMassConstraint",    m_constrJpsi);
    declareProperty("ApplyRho1MassConstraint",    m_constrRho1);
    declareProperty("ApplyRho2MassConstraint",    m_constrRho2);
    declareProperty("ApplyEtacMassConstraint",    m_constrEtac);
    declareProperty("Chi2CutJpsi",                m_chi2cut_jpsi);
    declareProperty("Chi2CutRho",                 m_chi2cut_rho);
    declareProperty("Chi2Cut",                    m_chi2cut);
    declareProperty("RefPVContainerName",         m_refPVContainerName);
    declareProperty("TrkVertexFitterTool",        m_iVertexFitter);
    declareProperty("PVRefitter",                 m_pvRefitter);
    declareProperty("V0Tools",                    m_V0Tools);
    declareProperty("TrackSelectorTool",          m_trkSelector);
    declareProperty("VertexPointEstimator",       m_vertexEstimator);
    declareProperty("OutputVertexCollection",     m_outputKey);
  }

  StatusCode JpsiPlusEtacSingleVertex::initialize() {
    // retrieving vertex Fitter
    ATH_CHECK( m_iVertexFitter.retrieve() );

    // retrieve PV refitter
    ATH_CHECK( m_pvRefitter.retrieve() );

    // retrieving the V0 tools
    ATH_CHECK( m_V0Tools.retrieve() );

    // retrieving the track selector tool
    ATH_CHECK( m_trkSelector.retrieve() );

    // retrieving the vertex point estimator tool
    ATH_CHECK( m_vertexEstimator.retrieve() );

    // Get the beam spot service
    ATH_CHECK( m_beamSpotSvc.retrieve() );

    IPartPropSvc* partPropSvc = nullptr;
    ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
    auto pdt = partPropSvc->PDT();

    // retrieve particle masses
    if(m_mass_jpsi < 0.) m_mass_jpsi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_mass_rho1 < 0.) m_mass_rho1 = BPhysPVCascadeTools::getParticleMass(pdt, 113); // rho
    if(m_mass_rho2 < 0.) m_mass_rho2 = BPhysPVCascadeTools::getParticleMass(pdt, 113); // rho
    if(m_mass_etac < 0.) m_mass_etac = BPhysPVCascadeTools::getParticleMass(pdt, 441); // eta_c

    if(m_vtx0Daug1MassHypo < 0.) m_vtx0Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx0Daug2MassHypo < 0.) m_vtx0Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx1Daug1MassHypo < 0.) m_vtx1Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx1Daug2MassHypo < 0.) m_vtx1Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx2Daug1MassHypo < 0.) m_vtx2Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_vtx2Daug2MassHypo < 0.) m_vtx2Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiPlusEtacSingleVertex::addBranches() const {
    xAOD::VertexContainer* VtxWriteHandle = new xAOD::VertexContainer;
    xAOD::VertexAuxContainer* VtxWriteHandleAux = new xAOD::VertexAuxContainer;
    VtxWriteHandle->setStore(VtxWriteHandleAux);
    ATH_CHECK(evtStore()->record(VtxWriteHandle   , m_outputKey));
    ATH_CHECK(evtStore()->record(VtxWriteHandleAux, m_outputKey + "Aux."));

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    const xAOD::VertexContainer *pvContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
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
      ATH_CHECK(evtStore()->record(refPvAuxContainer, m_refPVContainerName + "Aux."));
    }

    // Get TrackParticle container
    const xAOD::TrackParticleContainer  *trackContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(trackContainer, m_trackContainerName));

    // Get Jpsi container
    const xAOD::VertexContainer  *jpsiContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jpsiContainer, m_vertexContainerKey));

    std::vector<const xAOD::TrackParticle*> tracksJpsi;
    std::vector<const xAOD::TrackParticle*> tracksRho1;
    std::vector<const xAOD::TrackParticle*> inputTracks;
    std::vector<double> massesJpsi{m_vtx0Daug1MassHypo, m_vtx0Daug2MassHypo};
    std::vector<double> massesRho1{m_vtx1Daug1MassHypo, m_vtx1Daug2MassHypo};
    std::vector<double> massesRho2{m_vtx2Daug1MassHypo, m_vtx2Daug2MassHypo};
    std::vector<double> massesInputTracks{m_vtx0Daug1MassHypo, m_vtx0Daug2MassHypo, 
                                          m_vtx1Daug1MassHypo, m_vtx1Daug2MassHypo, 
                                          m_vtx2Daug1MassHypo, m_vtx2Daug2MassHypo};

    TLorentzVector p4_mu1, p4_mu2, p4_track1, p4_track2, p4_track3, p4_track4;

    // Select the J/psi candidates before calling fitter
    std::vector<const xAOD::Vertex*> selectedJpsiCandidates;
    for(auto vxcItr=jpsiContainer->cbegin(); vxcItr!=jpsiContainer->cend(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexJpsiHypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexJpsiHypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexJpsiHypoNames.size() && !passed) continue;

      // Check Jpsi candidate invariant mass and skip if need be
      double mass_jpsi = m_V0Tools->invariantMass(*vxcItr, massesJpsi);
      if (mass_jpsi < m_jpsiMassLower || mass_jpsi > m_jpsiMassUpper) continue;

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_jpsi>0 && chi2DOF>m_chi2cut_jpsi) continue;

      selectedJpsiCandidates.push_back(*vxcItr);
    }
    if(selectedJpsiCandidates.size()==0) return StatusCode::SUCCESS;

    std::vector<const xAOD::TrackParticle*> tracksPlus;
    std::vector<const xAOD::TrackParticle*> tracksMinus;
    for(auto tpIter=trackContainer->cbegin(); tpIter!=trackContainer->cend(); ++tpIter) {
      const xAOD::TrackParticle* track = (*tpIter);
      if ( track->pt()<m_trkMinPt4 ) continue;
      if ( !m_trkSelector->decision(*track, NULL) ) continue;

      bool passDR = false;
      for(auto jpsiItr=selectedJpsiCandidates.cbegin(); jpsiItr!=selectedJpsiCandidates.cend(); ++jpsiItr) {
	tracksJpsi.clear();
	for(size_t i=0; i<(*jpsiItr)->nTrackParticles(); i++) {
	  tracksJpsi.push_back((*jpsiItr)->trackParticle(i));
	}
	if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), track) != tracksJpsi.cend())
	  continue;
	p4_mu1.SetPtEtaPhiM( (*jpsiItr)->trackParticle(0)->pt(), (*jpsiItr)->trackParticle(0)->eta(), (*jpsiItr)->trackParticle(0)->phi(), m_vtx0Daug1MassHypo);
	p4_mu2.SetPtEtaPhiM( (*jpsiItr)->trackParticle(1)->pt(), (*jpsiItr)->trackParticle(1)->eta(), (*jpsiItr)->trackParticle(1)->phi(), m_vtx0Daug2MassHypo);
	if(DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),track->eta(),track->phi())<m_maxDR) {
	  passDR = true; break;
	}
      }
      if(passDR) {
	if(track->charge()>0) tracksPlus.push_back(track);
	else tracksMinus.push_back(track);
      }
    }

    std::vector<RhoCandidate> rhos;
    for(auto tpIter1=tracksPlus.cbegin(); tpIter1!=tracksPlus.cend(); ++tpIter1) {
      for(auto tpIter2=tracksMinus.cbegin(); tpIter2!=tracksMinus.cend(); ++tpIter2) {
	p4_track1.SetPtEtaPhiM((*tpIter1)->pt(), (*tpIter1)->eta(), (*tpIter1)->phi(), m_vtx1Daug1MassHypo);
	p4_track2.SetPtEtaPhiM((*tpIter2)->pt(), (*tpIter2)->eta(), (*tpIter2)->phi(), m_vtx1Daug2MassHypo);
	double mass1 = (p4_track1+p4_track2).M();
	p4_track1.SetPtEtaPhiM((*tpIter1)->pt(), (*tpIter1)->eta(), (*tpIter1)->phi(), m_vtx2Daug1MassHypo);
	p4_track2.SetPtEtaPhiM((*tpIter2)->pt(), (*tpIter2)->eta(), (*tpIter2)->phi(), m_vtx2Daug2MassHypo);
	double mass2 = (p4_track1+p4_track2).M();
	if((mass1>m_rho1MassLower && mass1<m_rho1MassUpper) || (mass2>m_rho2MassLower && mass2<m_rho2MassUpper)) {
	  std::unique_ptr<xAOD::Vertex> vtx = fitTwoTracks(*tpIter1, *tpIter2);
	  if(vtx) {
	    double chi2NDF = vtx->chiSquared()/vtx->numberDoF();
	    if(m_chi2cut_rho > 0.0 && chi2NDF > m_chi2cut_rho) continue;
	    RhoCandidate rho;
	    rho.trackParticle1 = *tpIter1; rho.trackParticle2 = *tpIter2;
	    rho.chi2NDF = chi2NDF; rho.vtxPos = vtx->position();
	    rhos.push_back(rho);
	  }
	}
      }
    }

    std::vector<EtacCandidate> candidates;
    for(auto tpIter1=rhos.cbegin(); tpIter1!=rhos.cend(); ++tpIter1) {
      tracksRho1.clear();
      tracksRho1.push_back(tpIter1->trackParticle1);
      tracksRho1.push_back(tpIter1->trackParticle2);
      for(auto tpIter2=tpIter1+1; tpIter2!=rhos.cend(); ++tpIter2) {
	if(std::find(tracksRho1.cbegin(), tracksRho1.cend(), tpIter2->trackParticle1) != tracksRho1.cend())
	  continue;
	if(std::find(tracksRho1.cbegin(), tracksRho1.cend(), tpIter2->trackParticle2) != tracksRho1.cend())
	  continue;
	std::vector<double> trackPt{tpIter1->trackParticle1->pt(), tpIter1->trackParticle2->pt(), tpIter2->trackParticle1->pt(), tpIter2->trackParticle2->pt()};
	std::sort( trackPt.begin(), trackPt.end(), [](double a, double b) { return a>b; } );
	if(trackPt[0]<m_trkMinPt1 || trackPt[1]<m_trkMinPt2 || trackPt[2]<m_trkMinPt3 || trackPt[3]<m_trkMinPt4) continue;
	p4_track1.SetPtEtaPhiM(tpIter1->trackParticle1->pt(), tpIter1->trackParticle1->eta(), tpIter1->trackParticle1->phi(), m_vtx1Daug1MassHypo);
	p4_track2.SetPtEtaPhiM(tpIter1->trackParticle2->pt(), tpIter1->trackParticle2->eta(), tpIter1->trackParticle2->phi(), m_vtx1Daug2MassHypo);
	p4_track3.SetPtEtaPhiM(tpIter2->trackParticle1->pt(), tpIter2->trackParticle1->eta(), tpIter2->trackParticle1->phi(), m_vtx2Daug1MassHypo);
	p4_track4.SetPtEtaPhiM(tpIter2->trackParticle2->pt(), tpIter2->trackParticle2->eta(), tpIter2->trackParticle2->phi(), m_vtx2Daug2MassHypo);
	if((p4_track1+p4_track2).M()>m_rho1MassLower && (p4_track1+p4_track2).M()<m_rho1MassUpper &&
	   (p4_track3+p4_track4).M()>m_rho2MassLower && (p4_track3+p4_track4).M()<m_rho2MassUpper &&
	   (p4_track1+p4_track2+p4_track3+p4_track4).M()>m_etacMassLower && (p4_track1+p4_track2+p4_track3+p4_track4).M()<m_etacMassUpper) {
	  EtacCandidate etac;
	  etac.trackParticle1 = tpIter1->trackParticle1; etac.trackParticle2 = tpIter1->trackParticle2;
	  etac.trackParticle3 = tpIter2->trackParticle1; etac.trackParticle4 = tpIter2->trackParticle2;
	  etac.ptTot = (p4_track1+p4_track2+p4_track3+p4_track4).Pt();
	  etac.chi2NDFSum = tpIter1->chi2NDF + tpIter2->chi2NDF;
	  etac.vtxPos1(0) = tpIter1->vtxPos(0); etac.vtxPos2(0) = tpIter2->vtxPos(0);
	  etac.vtxPos1(1) = tpIter1->vtxPos(1); etac.vtxPos2(1) = tpIter2->vtxPos(1);
	  etac.vtxPos1(2) = tpIter1->vtxPos(2); etac.vtxPos2(2) = tpIter2->vtxPos(2);

	  bool passDR = false;
	  for(auto jpsiItr=selectedJpsiCandidates.cbegin(); jpsiItr!=selectedJpsiCandidates.cend(); ++jpsiItr) {
	    tracksJpsi.clear();
	    for(size_t i=0; i<(*jpsiItr)->nTrackParticles(); i++) {
	      tracksJpsi.push_back((*jpsiItr)->trackParticle(i));
	    }
	    if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle1) != tracksJpsi.cend())
	      continue;
	    if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle2) != tracksJpsi.cend())
	      continue;
	    if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle3) != tracksJpsi.cend())
	      continue;
	    if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle4) != tracksJpsi.cend())
	      continue;
	    p4_mu1.SetPtEtaPhiM( (*jpsiItr)->trackParticle(0)->pt(), (*jpsiItr)->trackParticle(0)->eta(), (*jpsiItr)->trackParticle(0)->phi(), m_vtx0Daug1MassHypo);
	    p4_mu2.SetPtEtaPhiM( (*jpsiItr)->trackParticle(1)->pt(), (*jpsiItr)->trackParticle(1)->eta(), (*jpsiItr)->trackParticle(1)->phi(), m_vtx0Daug2MassHypo);
	    if(DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle1->eta(),etac.trackParticle1->phi())<m_maxDR &&
	       DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle2->eta(),etac.trackParticle2->phi())<m_maxDR &&
	       DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle3->eta(),etac.trackParticle3->phi())<m_maxDR &&
	       DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle4->eta(),etac.trackParticle4->phi())<m_maxDR) {
	      passDR = true; break;
	    }
	  }
	  if(passDR) candidates.push_back(etac);
	}
	if(m_vtx1Daug1MassHypo != m_vtx2Daug1MassHypo || m_vtx1Daug2MassHypo != m_vtx2Daug2MassHypo || m_rho1MassLower != m_rho2MassLower || m_rho1MassUpper != m_rho2MassUpper) {
	  p4_track1.SetPtEtaPhiM(tpIter2->trackParticle1->pt(), tpIter2->trackParticle1->eta(), tpIter2->trackParticle1->phi(), m_vtx1Daug1MassHypo);
	  p4_track2.SetPtEtaPhiM(tpIter2->trackParticle2->pt(), tpIter2->trackParticle2->eta(), tpIter2->trackParticle2->phi(), m_vtx1Daug2MassHypo);
	  p4_track3.SetPtEtaPhiM(tpIter1->trackParticle1->pt(), tpIter1->trackParticle1->eta(), tpIter1->trackParticle1->phi(), m_vtx2Daug1MassHypo);
	  p4_track4.SetPtEtaPhiM(tpIter1->trackParticle2->pt(), tpIter1->trackParticle2->eta(), tpIter1->trackParticle2->phi(), m_vtx2Daug2MassHypo);
	  if((p4_track1+p4_track2).M()>m_rho1MassLower && (p4_track1+p4_track2).M()<m_rho1MassUpper &&
	     (p4_track3+p4_track4).M()>m_rho2MassLower && (p4_track3+p4_track4).M()<m_rho2MassUpper &&
	     (p4_track1+p4_track2+p4_track3+p4_track4).M()>m_etacMassLower && (p4_track1+p4_track2+p4_track3+p4_track4).M()<m_etacMassUpper) {
	    EtacCandidate etac;
	    etac.trackParticle1 = tpIter2->trackParticle1; etac.trackParticle2 = tpIter2->trackParticle2;
	    etac.trackParticle3 = tpIter1->trackParticle1; etac.trackParticle4 = tpIter1->trackParticle2;
	    etac.ptTot = (p4_track1+p4_track2+p4_track3+p4_track4).Pt();
	    etac.chi2NDFSum = tpIter1->chi2NDF + tpIter2->chi2NDF;
	    etac.vtxPos1(0) = tpIter1->vtxPos(0); etac.vtxPos2(0) = tpIter2->vtxPos(0);
	    etac.vtxPos1(1) = tpIter1->vtxPos(1); etac.vtxPos2(1) = tpIter2->vtxPos(1);
	    etac.vtxPos1(2) = tpIter1->vtxPos(2); etac.vtxPos2(2) = tpIter2->vtxPos(2);

	    bool passDR = false;
	    for(auto jpsiItr=selectedJpsiCandidates.cbegin(); jpsiItr!=selectedJpsiCandidates.cend(); ++jpsiItr) {
	      tracksJpsi.clear();
	      for(size_t i=0; i<(*jpsiItr)->nTrackParticles(); i++) {
		tracksJpsi.push_back((*jpsiItr)->trackParticle(i));
	      }
	      if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle1) != tracksJpsi.cend())
		continue;
	      if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle2) != tracksJpsi.cend())
		continue;
	      if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle3) != tracksJpsi.cend())
		continue;
	      if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle4) != tracksJpsi.cend())
		continue;
	      p4_mu1.SetPtEtaPhiM( (*jpsiItr)->trackParticle(0)->pt(), (*jpsiItr)->trackParticle(0)->eta(), (*jpsiItr)->trackParticle(0)->phi(), m_vtx0Daug1MassHypo);
	      p4_mu2.SetPtEtaPhiM( (*jpsiItr)->trackParticle(1)->pt(), (*jpsiItr)->trackParticle(1)->eta(), (*jpsiItr)->trackParticle(1)->phi(), m_vtx0Daug2MassHypo);
	      if(DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle1->eta(),etac.trackParticle1->phi())<m_maxDR &&
		 DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle2->eta(),etac.trackParticle2->phi())<m_maxDR &&
		 DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle3->eta(),etac.trackParticle3->phi())<m_maxDR &&
		 DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),etac.trackParticle4->eta(),etac.trackParticle4->phi())<m_maxDR) {
		passDR = true; break;
	      }
	    }
	    if(passDR) candidates.push_back(etac);
	  }
	}
      }
    }

    if(m_ptOrdering) { // order by pt
      std::sort( candidates.begin(), candidates.end(), [](const EtacCandidate &a, const EtacCandidate &b) { return a.ptTot > b.ptTot; } );
    }
    else { // order by chi2/NDF sum
      std::sort( candidates.begin(), candidates.end(), [](const EtacCandidate &a, const EtacCandidate &b) { return a.chi2NDFSum < b.chi2NDFSum; } );
    }
    if(m_maxCandidates>0 && candidates.size()>m_maxCandidates) {
      candidates.erase(candidates.begin()+m_maxCandidates, candidates.end());
    }

    // loop over Jpsi
    for(auto jpsiItr=selectedJpsiCandidates.cbegin(); jpsiItr!=selectedJpsiCandidates.cend(); ++jpsiItr) {
      tracksJpsi.clear();
      for(size_t i=0; i<(*jpsiItr)->nTrackParticles(); i++) {
	tracksJpsi.push_back((*jpsiItr)->trackParticle(i));
      }
      for(auto&& etac : candidates) {
	if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle1) != tracksJpsi.cend())
	  continue;
	if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle2) != tracksJpsi.cend())
	  continue;
	if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle3) != tracksJpsi.cend())
	  continue;
	if(std::find(tracksJpsi.cbegin(), tracksJpsi.cend(), etac.trackParticle4) != tracksJpsi.cend())
	  continue;
	inputTracks.clear();
	inputTracks.push_back(tracksJpsi[0]);
	inputTracks.push_back(tracksJpsi[1]);
	inputTracks.push_back(etac.trackParticle1);
	inputTracks.push_back(etac.trackParticle2);
	inputTracks.push_back(etac.trackParticle3);
	inputTracks.push_back(etac.trackParticle4);

	p4_mu1.SetPtEtaPhiM(inputTracks[0]->pt(), inputTracks[0]->eta(), inputTracks[0]->phi(), m_vtx0Daug1MassHypo);
	p4_mu2.SetPtEtaPhiM(inputTracks[1]->pt(), inputTracks[1]->eta(), inputTracks[1]->phi(), m_vtx0Daug2MassHypo);
	p4_track1.SetPtEtaPhiM(inputTracks[2]->pt(), inputTracks[2]->eta(), inputTracks[2]->phi(), m_vtx1Daug1MassHypo);
	p4_track2.SetPtEtaPhiM(inputTracks[3]->pt(), inputTracks[3]->eta(), inputTracks[3]->phi(), m_vtx1Daug2MassHypo);
	p4_track3.SetPtEtaPhiM(inputTracks[4]->pt(), inputTracks[4]->eta(), inputTracks[4]->phi(), m_vtx2Daug1MassHypo);
	p4_track4.SetPtEtaPhiM(inputTracks[5]->pt(), inputTracks[5]->eta(), inputTracks[5]->phi(), m_vtx2Daug2MassHypo);

	if((p4_mu1+p4_mu2+p4_track1+p4_track2+p4_track3+p4_track4).M()<m_MassLower || (p4_mu1+p4_mu2+p4_track1+p4_track2+p4_track3+p4_track4).M()>m_MassUpper) continue;
	if(DR((p4_mu1+p4_mu2).Eta(),(p4_mu1+p4_mu2).Phi(),(p4_track1+p4_track2+p4_track3+p4_track4).Eta(),(p4_track1+p4_track2+p4_track3+p4_track4).Phi())>m_maxDR) continue;

	// start the fit
	m_iVertexFitter->setDefault();

	m_iVertexFitter->setMassInputParticles(massesInputTracks);
	if (m_constrJpsi) {
	  m_iVertexFitter->setMassForConstraint(m_mass_jpsi, std::vector<int>{1,2});
	}
	if (m_constrRho1) {
	  m_iVertexFitter->setMassForConstraint(m_mass_rho1, std::vector<int>{3,4});
	}
	if (m_constrRho2) {
	  m_iVertexFitter->setMassForConstraint(m_mass_rho2, std::vector<int>{5,6});
	}
	if (m_constrEtac) {
	  m_iVertexFitter->setMassForConstraint(m_mass_etac, std::vector<int>{3,4,5,6});
	}

	Amg::Vector3D startPoint;
	startPoint(0) = ((*jpsiItr)->x() + etac.vtxPos1(0) + etac.vtxPos2(0))/3;
	startPoint(1) = ((*jpsiItr)->y() + etac.vtxPos1(1) + etac.vtxPos2(1))/3;
	startPoint(2) = ((*jpsiItr)->z() + etac.vtxPos1(2) + etac.vtxPos2(2))/3;

	// do the fit
	std::unique_ptr<xAOD::Vertex> fittedVertex( m_iVertexFitter->fit(inputTracks, startPoint) );

	if(fittedVertex) {
	  // Chi2/DOF cut
	  double chi2DOF = fittedVertex->chiSquared()/fittedVertex->numberDoF();
	  bool chi2CutPassed = (m_chi2cut <= 0.0 || chi2DOF < m_chi2cut);
	  if(chi2CutPassed) {
	    // make TP links
	    TrackParticleLinkVector tpLinkVector;
	    for(size_t i=0; i<fittedVertex->trackParticleLinks().size(); i++) {
	      TrackParticleLink mylink = fittedVertex->trackParticleLinks()[i];
	      mylink.setStorableObject(*trackContainer, true);
	      tpLinkVector.push_back( mylink );
	    }
	    fittedVertex->clearTracks();
	    fittedVertex->setTrackParticleLinks( tpLinkVector );

	    xAOD::BPhysHypoHelper vtx(m_hypoName, fittedVertex.get());
	    vtx.setRefTrks();
	    vtx.setPass(true);

	    VtxWriteHandle->push_back(std::move(fittedVertex));  //ptr ownership is transferred
	  }
	}
      } //Iterate over etac candidates
    } //Iterate over jpsi candidates

    BPhysPVTools helper(&(*m_V0Tools), &m_beamSpotSvc);
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    if(m_refitPV) {
      if(VtxWriteHandle->size()>0) {
	ATH_CHECK( helper.FillCandwithRefittedVertices(VtxWriteHandle, pvContainer, refPvContainer, &(*m_pvRefitter), m_PV_max, m_DoVertexType) );
      }
    }
    else {
      if(VtxWriteHandle->size()>0) {
	ATH_CHECK( helper.FillCandExistingVertices(VtxWriteHandle, pvContainer, m_DoVertexType) );
      }
    }

    return StatusCode::SUCCESS;
  }

  std::unique_ptr<xAOD::Vertex> JpsiPlusEtacSingleVertex::fitTwoTracks(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2) const {
    // Starting point
    const Trk::Perigee& aPerigee1 = track1->perigeeParameters();
    const Trk::Perigee& aPerigee2 = track2->perigeeParameters();
    int sflag(0), errorcode(0);
    Amg::Vector3D startingPoint = m_vertexEstimator->getCirclesIntersectionPoint(&aPerigee1,&aPerigee2,sflag,errorcode);
    if(errorcode) startingPoint(0) = startingPoint(1) = startingPoint(2) = 0.0;
    m_iVertexFitter->setDefault();
    // do the fit
    std::unique_ptr<xAOD::Vertex> fittedVertex( m_iVertexFitter->fit(std::vector<const xAOD::TrackParticle*>{track1,track2}, startingPoint) );
    return fittedVertex;
  }

  double JpsiPlusEtacSingleVertex::DR(double eta1, double phi1, double eta2, double phi2) const {
    double deta = std::abs(eta1-eta2);
    double dphi = std::abs(phi1-phi2);
    if(dphi>M_PI) dphi = 2*M_PI-dphi;
    return std::sqrt(std::pow(deta,2)+std::pow(dphi,2));
  }
}
