/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#include "DerivationFrameworkBPhys/DiJpsiPlusTracksSingleVertex.h"
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
    double mass_hypo1;
    double mass_hypo2;
    double mass_hypo3;
    double chi2NDF;
    Amg::Vector3D vtxPos;
  };

  struct EtacCandidate {
    const xAOD::TrackParticle* trackParticle1 = nullptr;
    const xAOD::TrackParticle* trackParticle2 = nullptr;
    const xAOD::TrackParticle* trackParticle3 = nullptr;
    const xAOD::TrackParticle* trackParticle4 = nullptr;
    const xAOD::TrackParticle* trackParticle5 = nullptr;
    const xAOD::TrackParticle* trackParticle6 = nullptr;
    int nTracks = 0;
    double ptTot;
    double chi2NDFSum;
    Amg::Vector3D vtxPos1;
    Amg::Vector3D vtxPos2;
    Amg::Vector3D vtxPos3;
  };

  DiJpsiPlusTracksSingleVertex::DiJpsiPlusTracksSingleVertex(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_outputKey("OutputVtxContainer"),
    m_vertex1ContainerKey("InputJpsi1Container"),
    m_vertex2ContainerKey("InputJpsi2Container"),
    m_jpsi1MassLower(0.0),
    m_jpsi1MassUpper(3500.0),
    m_jpsi2MassLower(0.0),
    m_jpsi2MassUpper(3500.0),
    m_tracks_num(4),
    m_rho1MassLower(0.0),
    m_rho1MassUpper(2500.0),
    m_rho2MassLower(0.0),
    m_rho2MassUpper(2500.0),
    m_rho3MassLower(0.0),
    m_rho3MassUpper(2500.0),
    m_etacMassLower(0.0),
    m_etacMassUpper(3500.0),
    m_MassLower(0.0),
    m_MassUpper(31000.0),
    m_constrJpsi1(false),
    m_constrJpsi2(false),
    m_constrRho1(false),
    m_constrRho2(false),
    m_constrRho3(false),
    m_constrEtac(false),
    m_chi2cut_jpsi1(-1.0),
    m_chi2cut_jpsi2(-1.0),
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
    declareProperty("Jpsi1Vertices",              m_vertex1ContainerKey);
    declareProperty("Jpsi2Vertices",              m_vertex2ContainerKey);
    declareProperty("Jpsi1VtxHypoNames",          m_vertexJpsi1HypoNames);
    declareProperty("Jpsi2VtxHypoNames",          m_vertexJpsi2HypoNames);
    declareProperty("Jpsi1MassLowerCut",          m_jpsi1MassLower);
    declareProperty("Jpsi1MassUpperCut",          m_jpsi1MassUpper);
    declareProperty("Jpsi2MassLowerCut",          m_jpsi2MassLower);
    declareProperty("Jpsi2MassUpperCut",          m_jpsi2MassUpper);
    declareProperty("NumberOfTracks",             m_tracks_num); // 1, 2, 4 or 6
    declareProperty("Rho1MassLowerCut",           m_rho1MassLower);
    declareProperty("Rho1MassUpperCut",           m_rho1MassUpper);
    declareProperty("Rho2MassLowerCut",           m_rho2MassLower);
    declareProperty("Rho2MassUpperCut",           m_rho2MassUpper);
    declareProperty("Rho3MassLowerCut",           m_rho3MassLower);
    declareProperty("Rho3MassUpperCut",           m_rho3MassUpper);
    declareProperty("EtacMassLowerCut",           m_etacMassLower);
    declareProperty("EtacMassUpperCut",           m_etacMassUpper);
    declareProperty("MassLowerCut",               m_MassLower);
    declareProperty("MassUpperCut",               m_MassUpper);
    declareProperty("ApplyJpsi1MassConstraint",   m_constrJpsi1);
    declareProperty("ApplyJpsi2MassConstraint",   m_constrJpsi2);
    declareProperty("ApplyRho1MassConstraint",    m_constrRho1);
    declareProperty("ApplyRho2MassConstraint",    m_constrRho2);
    declareProperty("ApplyRho3MassConstraint",    m_constrRho3);
    declareProperty("ApplyEtacMassConstraint",    m_constrEtac);
    declareProperty("Chi2CutJpsi1",               m_chi2cut_jpsi1);
    declareProperty("Chi2CutJpsi2",               m_chi2cut_jpsi2);
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

  StatusCode DiJpsiPlusTracksSingleVertex::initialize() {
    if(m_tracks_num!=1 && m_tracks_num!=2 && m_tracks_num!=4 && m_tracks_num!=6) {
      ATH_MSG_FATAL("Incorrect number of tracks");
      return StatusCode::FAILURE;
    }

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
    if(m_mass_jpsi1 < 0.) m_mass_jpsi1 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_mass_jpsi2 < 0.) m_mass_jpsi2 = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_tracks_num>=2 && m_mass_rho1 < 0.) m_mass_rho1 = BPhysPVCascadeTools::getParticleMass(pdt, 113); // rho
    if(m_tracks_num>=4 && m_mass_rho2 < 0.) m_mass_rho2 = BPhysPVCascadeTools::getParticleMass(pdt, 113); // rho
    if(m_tracks_num==6 && m_mass_rho3 < 0.) m_mass_rho3 = BPhysPVCascadeTools::getParticleMass(pdt, 113); // rho
    if(m_tracks_num>=2 && m_mass_etac < 0.) m_mass_etac = BPhysPVCascadeTools::getParticleMass(pdt, 441); // eta_c

    if(m_vtx0Daug1MassHypo < 0.) m_vtx0Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx0Daug2MassHypo < 0.) m_vtx0Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx0Daug3MassHypo < 0.) m_vtx0Daug3MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx0Daug4MassHypo < 0.) m_vtx0Daug4MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    if(m_vtx1Daug1MassHypo < 0.) m_vtx1Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_tracks_num>=2 && m_vtx1Daug2MassHypo < 0.) m_vtx1Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_tracks_num>=4 && m_vtx2Daug1MassHypo < 0.) m_vtx2Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_tracks_num>=4 && m_vtx2Daug2MassHypo < 0.) m_vtx2Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    if(m_tracks_num==6) {
      if(m_vtx3Daug1MassHypo < 0.) m_vtx3Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
      if(m_vtx3Daug2MassHypo < 0.) m_vtx3Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    }

    m_same_mass12 = std::vector<double>{m_vtx1Daug1MassHypo,m_vtx1Daug2MassHypo} == std::vector<double>{m_vtx2Daug1MassHypo,m_vtx2Daug2MassHypo};
    m_same_mass13 = std::vector<double>{m_vtx1Daug1MassHypo,m_vtx1Daug2MassHypo} == std::vector<double>{m_vtx3Daug1MassHypo,m_vtx3Daug2MassHypo};
    m_same_mass23 = std::vector<double>{m_vtx2Daug1MassHypo,m_vtx2Daug2MassHypo} == std::vector<double>{m_vtx3Daug1MassHypo,m_vtx3Daug2MassHypo};
    m_same_mass123 = m_same_mass12 && m_same_mass23;

    return StatusCode::SUCCESS;
  }

  StatusCode DiJpsiPlusTracksSingleVertex::addBranches() const {
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

    // Get Jpsi1 container
    const xAOD::VertexContainer  *jpsi1Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(jpsi1Container, m_vertex1ContainerKey));

    // Get Jpsi2 container
    const xAOD::VertexContainer  *jpsi2Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(jpsi2Container, m_vertex2ContainerKey));

    std::vector<const xAOD::TrackParticle*> tracksJpsis;
    std::vector<const xAOD::TrackParticle*> tracksRho1;
    std::vector<const xAOD::TrackParticle*> tracksRho12;
    std::vector<const xAOD::TrackParticle*> inputTracks;
    std::vector<double> massesJpsi1{m_vtx0Daug1MassHypo, m_vtx0Daug2MassHypo};
    std::vector<double> massesJpsi2{m_vtx0Daug3MassHypo, m_vtx0Daug4MassHypo};
    std::vector<double> massesInputTracks{m_vtx0Daug1MassHypo, m_vtx0Daug2MassHypo, 
                                          m_vtx0Daug3MassHypo, m_vtx0Daug4MassHypo, 
                                          m_vtx1Daug1MassHypo};
    if(m_tracks_num>=2) {
      massesInputTracks.push_back(m_vtx1Daug2MassHypo);
    }
    if(m_tracks_num>=4) {
      massesInputTracks.push_back(m_vtx2Daug1MassHypo);
      massesInputTracks.push_back(m_vtx2Daug2MassHypo);
    }
    if(m_tracks_num==6) {
      massesInputTracks.push_back(m_vtx3Daug1MassHypo);
      massesInputTracks.push_back(m_vtx3Daug2MassHypo);
    }

    TLorentzVector p4_mu1, p4_mu2, p4_mu3, p4_mu4, p4_track1, p4_track2, p4_track3, p4_track4, p4_track5, p4_track6;

    // Select the Jpsi1 candidates before calling fitter
    std::vector<const xAOD::Vertex*> selectedJpsi1Candidates;
    for(auto vxcItr=jpsi1Container->cbegin(); vxcItr!=jpsi1Container->cend(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexJpsi1HypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexJpsi1HypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexJpsi1HypoNames.size() && !passed) continue;

      // Check Jpsi1 candidate invariant mass and skip if need be
      double mass_jpsi1 = m_V0Tools->invariantMass(*vxcItr, massesJpsi1);
      if (mass_jpsi1 < m_jpsi1MassLower || mass_jpsi1 > m_jpsi1MassUpper) continue;

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_jpsi1>0 && chi2DOF>m_chi2cut_jpsi1) continue;

      selectedJpsi1Candidates.push_back(*vxcItr);
    }
    if(selectedJpsi1Candidates.size()==0) return StatusCode::SUCCESS;

    // Select the Jpsi2 candidates before calling fitter
    std::vector<const xAOD::Vertex*> selectedJpsi2Candidates;
    for(auto vxcItr=jpsi2Container->cbegin(); vxcItr!=jpsi2Container->cend(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(size_t i=0; i<m_vertexJpsi2HypoNames.size(); i++) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+m_vertexJpsi2HypoNames[i]);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed |= 1;
	}
      }
      if(m_vertexJpsi2HypoNames.size() && !passed) continue;

      // Check Jpsi2 candidate invariant mass and skip if need be
      double mass_jpsi2 = m_V0Tools->invariantMass(*vxcItr, massesJpsi2);
      if (mass_jpsi2 < m_jpsi2MassLower || mass_jpsi2 > m_jpsi2MassUpper) continue;

      double chi2DOF = (*vxcItr)->chiSquared()/(*vxcItr)->numberDoF();
      if(m_chi2cut_jpsi2>0 && chi2DOF>m_chi2cut_jpsi2) continue;

      selectedJpsi2Candidates.push_back(*vxcItr);
    }
    if(selectedJpsi2Candidates.size()==0) return StatusCode::SUCCESS;

    // Select the di-Jpsi candidates before calling fitter
    std::vector<std::pair<const xAOD::Vertex*, const xAOD::Vertex*> > selectedDiJpsiCandidates;
    for(auto jpsi1Itr=selectedJpsi1Candidates.cbegin(); jpsi1Itr!=selectedJpsi1Candidates.cend(); ++jpsi1Itr) {
      const xAOD::Vertex* jpsi1 = *jpsi1Itr;
      std::set<const xAOD::TrackParticle*> set1{jpsi1->trackParticle(0), jpsi1->trackParticle(1)};
      for(auto jpsi2Itr=selectedJpsi2Candidates.cbegin(); jpsi2Itr!=selectedJpsi2Candidates.cend(); ++jpsi2Itr) {
	const xAOD::Vertex* jpsi2 = *jpsi2Itr;
	if(set1.find(jpsi2->trackParticle(0)) != set1.end()) continue;
	if(set1.find(jpsi2->trackParticle(1)) != set1.end()) continue;
	std::set<const xAOD::TrackParticle*> set2{jpsi2->trackParticle(0), jpsi2->trackParticle(1)};
	bool found_pair = false;
	for(auto iter=selectedDiJpsiCandidates.cbegin(); iter!=selectedDiJpsiCandidates.cend(); ++iter) {
	  std::set<const xAOD::TrackParticle*> setA{iter->first->trackParticle(0), iter->first->trackParticle(1)};
	  std::set<const xAOD::TrackParticle*> setB{iter->second->trackParticle(0), iter->second->trackParticle(1)};
	  if((set1==setA && set2==setB) || (set1==setB && set2==setA)) {
	    found_pair = true; break;
	  }
	}
	if(!found_pair) selectedDiJpsiCandidates.push_back( std::make_pair(jpsi1, jpsi2) );
      }
    }
    if(selectedDiJpsiCandidates.size()==0) return StatusCode::SUCCESS;

    std::vector<const xAOD::TrackParticle*> tracksPlus;
    std::vector<const xAOD::TrackParticle*> tracksMinus;
    for(auto iter=trackContainer->cbegin(); iter!=trackContainer->cend(); ++iter) {
      const xAOD::TrackParticle* track = *iter;
      if ( m_tracks_num==1 && track->pt()<m_trkMinPt1 ) continue;
      else if ( m_tracks_num==2 && track->pt()<m_trkMinPt2 ) continue;
      else if ( m_tracks_num==4 && track->pt()<m_trkMinPt4 ) continue;
      else if ( m_tracks_num==6 && track->pt()<m_trkMinPt6 ) continue;
      if ( !m_trkSelector->decision(*track, NULL) ) continue;

      bool passDR = false;
      for(auto iter=selectedDiJpsiCandidates.cbegin(); iter!=selectedDiJpsiCandidates.cend(); ++iter) {
	tracksJpsis.clear();
	tracksJpsis.push_back(iter->first->trackParticle(0));
	tracksJpsis.push_back(iter->first->trackParticle(1));
	tracksJpsis.push_back(iter->second->trackParticle(0));
	tracksJpsis.push_back(iter->second->trackParticle(1));
	if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), track) != tracksJpsis.cend())
	  continue;
	p4_mu1.SetPtEtaPhiM(tracksJpsis[0]->pt(), tracksJpsis[0]->eta(), tracksJpsis[0]->phi(), m_vtx0Daug1MassHypo);
	p4_mu2.SetPtEtaPhiM(tracksJpsis[1]->pt(), tracksJpsis[1]->eta(), tracksJpsis[1]->phi(), m_vtx0Daug2MassHypo);
	p4_mu3.SetPtEtaPhiM(tracksJpsis[2]->pt(), tracksJpsis[2]->eta(), tracksJpsis[2]->phi(), m_vtx0Daug3MassHypo);
	p4_mu4.SetPtEtaPhiM(tracksJpsis[3]->pt(), tracksJpsis[3]->eta(), tracksJpsis[3]->phi(), m_vtx0Daug4MassHypo);
	if(DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),track->eta(),track->phi())<m_maxDR) {
	  passDR = true; break;
	}
      }
      if(passDR) {
	if(track->charge()>0) tracksPlus.push_back(track);
	else tracksMinus.push_back(track);
      }
    }

    std::vector<RhoCandidate> rhos;
    if(m_tracks_num>=2) {
      for(auto iter1=tracksPlus.cbegin(); iter1!=tracksPlus.cend(); ++iter1) {
	for(auto iter2=tracksMinus.cbegin(); iter2!=tracksMinus.cend(); ++iter2) {
	  p4_track1.SetPtEtaPhiM((*iter1)->pt(), (*iter1)->eta(), (*iter1)->phi(), m_vtx1Daug1MassHypo);
	  p4_track2.SetPtEtaPhiM((*iter2)->pt(), (*iter2)->eta(), (*iter2)->phi(), m_vtx1Daug2MassHypo);
	  double mass1 = (p4_track1+p4_track2).M();
	  double mass2 = -999;
	  if(m_tracks_num>=4) {
	    p4_track1.SetPtEtaPhiM((*iter1)->pt(), (*iter1)->eta(), (*iter1)->phi(), m_vtx2Daug1MassHypo);
	    p4_track2.SetPtEtaPhiM((*iter2)->pt(), (*iter2)->eta(), (*iter2)->phi(), m_vtx2Daug2MassHypo);
	    mass2 = (p4_track1+p4_track2).M();
	  }
	  double mass3 = -999;
	  if(m_tracks_num==6) {
	    p4_track1.SetPtEtaPhiM((*iter1)->pt(), (*iter1)->eta(), (*iter1)->phi(), m_vtx3Daug1MassHypo);
	    p4_track2.SetPtEtaPhiM((*iter2)->pt(), (*iter2)->eta(), (*iter2)->phi(), m_vtx3Daug2MassHypo);
	    mass3 = (p4_track1+p4_track2).M();
	  }
	  if((mass1>m_rho1MassLower && mass1<m_rho1MassUpper) || (m_tracks_num>=4 && mass2>m_rho2MassLower && mass2<m_rho2MassUpper) || (m_tracks_num==6 && mass3>m_rho3MassLower && mass3<m_rho3MassUpper)) {
	    std::unique_ptr<xAOD::Vertex> vtx = fitTwoTracks(*iter1, *iter2);
	    if(vtx) {
	      double chi2NDF = vtx->chiSquared()/vtx->numberDoF();
	      if(m_chi2cut_rho > 0.0 && chi2NDF > m_chi2cut_rho)
		continue;
	      RhoCandidate rho;
	      rho.trackParticle1 = *iter1; rho.trackParticle2 = *iter2;
	      rho.mass_hypo1 = mass1;
	      rho.mass_hypo2 = mass2;
	      rho.mass_hypo3 = mass3;
	      rho.chi2NDF = chi2NDF; rho.vtxPos = vtx->position();
	      rhos.push_back(rho);
	    }
	  }
	}
      }
    }

    std::vector<EtacCandidate> candidates;
    if(m_tracks_num==1) {
      std::vector<const xAOD::TrackParticle*> tracksSelected;
      tracksSelected.insert(tracksSelected.begin(), tracksPlus.begin(), tracksPlus.end());
      tracksSelected.insert(tracksSelected.end(), tracksMinus.begin(), tracksMinus.end());
      for(auto iter=selectedDiJpsiCandidates.cbegin(); iter!=selectedDiJpsiCandidates.cend(); ++iter) {
	tracksJpsis.clear();
	tracksJpsis.push_back(iter->first->trackParticle(0));
	tracksJpsis.push_back(iter->first->trackParticle(1));
	tracksJpsis.push_back(iter->second->trackParticle(0));
	tracksJpsis.push_back(iter->second->trackParticle(1));
	p4_mu1.SetPtEtaPhiM(tracksJpsis[0]->pt(), tracksJpsis[0]->eta(), tracksJpsis[0]->phi(), m_vtx0Daug1MassHypo);
      	p4_mu2.SetPtEtaPhiM(tracksJpsis[1]->pt(), tracksJpsis[1]->eta(), tracksJpsis[1]->phi(), m_vtx0Daug2MassHypo);
	p4_mu3.SetPtEtaPhiM(tracksJpsis[2]->pt(), tracksJpsis[2]->eta(), tracksJpsis[2]->phi(), m_vtx0Daug3MassHypo);
	p4_mu4.SetPtEtaPhiM(tracksJpsis[3]->pt(), tracksJpsis[3]->eta(), tracksJpsis[3]->phi(), m_vtx0Daug4MassHypo);
	for(auto iter=tracksSelected.cbegin(); iter!=tracksSelected.cend(); ++iter) {
	  const xAOD::TrackParticle* track = *iter;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), track) != tracksJpsis.cend())
	    continue;
	  if ( track->pt()<m_trkMinPt1 )
	    continue;
	  p4_track1.SetPtEtaPhiM(track->pt(), track->eta(), track->phi(), m_vtx1Daug1MassHypo);
	  if((p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1).M()<m_MassLower || (p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1).M()>m_MassUpper)
	    continue;
	  if(DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),track->eta(),track->phi())<m_maxDR) {
	    EtacCandidate etac;
	    etac.nTracks = 1;
	    etac.trackParticle1 = track;
	    etac.ptTot = track->pt();
	    candidates.push_back(etac);
	  }
	} // tracksSelected
      } // selectedDiJpsiCandidates
    } // m_tracks_num==1
    else if(m_tracks_num==2) {
      for(auto iter=selectedDiJpsiCandidates.cbegin(); iter!=selectedDiJpsiCandidates.cend(); ++iter) {
	tracksJpsis.clear();
	tracksJpsis.push_back(iter->first->trackParticle(0));
	tracksJpsis.push_back(iter->first->trackParticle(1));
	tracksJpsis.push_back(iter->second->trackParticle(0));
	tracksJpsis.push_back(iter->second->trackParticle(1));
	p4_mu1.SetPtEtaPhiM(tracksJpsis[0]->pt(), tracksJpsis[0]->eta(), tracksJpsis[0]->phi(), m_vtx0Daug1MassHypo);
      	p4_mu2.SetPtEtaPhiM(tracksJpsis[1]->pt(), tracksJpsis[1]->eta(), tracksJpsis[1]->phi(), m_vtx0Daug2MassHypo);
	p4_mu3.SetPtEtaPhiM(tracksJpsis[2]->pt(), tracksJpsis[2]->eta(), tracksJpsis[2]->phi(), m_vtx0Daug3MassHypo);
	p4_mu4.SetPtEtaPhiM(tracksJpsis[3]->pt(), tracksJpsis[3]->eta(), tracksJpsis[3]->phi(), m_vtx0Daug4MassHypo);
	for(auto iter1=rhos.cbegin(); iter1!=rhos.cend(); ++iter1) {
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), iter1->trackParticle1) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), iter1->trackParticle2) != tracksJpsis.cend())
	    continue;
	  std::vector<double> trackPt{iter1->trackParticle1->pt(), iter1->trackParticle2->pt()};
	  std::sort( trackPt.begin(), trackPt.end(), [](double a, double b) { return a>b; } );
	  if(trackPt[0]<m_trkMinPt1 || trackPt[1]<m_trkMinPt2) continue;
	  p4_track1.SetPtEtaPhiM(iter1->trackParticle1->pt(), iter1->trackParticle1->eta(), iter1->trackParticle1->phi(), m_vtx1Daug1MassHypo);
	  p4_track2.SetPtEtaPhiM(iter1->trackParticle2->pt(), iter1->trackParticle2->eta(), iter1->trackParticle2->phi(), m_vtx1Daug2MassHypo);
	  if((p4_track1+p4_track2).M()<m_rho1MassLower || (p4_track1+p4_track2).M()>m_rho1MassUpper)
	    continue;
	  if((p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2).M()<m_MassLower || (p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2).M()>m_MassUpper)
	    continue;
	  if(DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),iter1->trackParticle1->eta(),iter1->trackParticle1->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),iter1->trackParticle2->eta(),iter1->trackParticle2->phi())<m_maxDR) {
	    EtacCandidate etac;
	    etac.nTracks = 2;
	    etac.trackParticle1 = iter1->trackParticle1;
	    etac.trackParticle2 = iter1->trackParticle2;
	    etac.ptTot = (p4_track1+p4_track2).Pt();
	    etac.chi2NDFSum = iter1->chi2NDF;
	    etac.vtxPos1(0) = iter1->vtxPos(0);
	    etac.vtxPos1(1) = iter1->vtxPos(1);
	    etac.vtxPos1(2) = iter1->vtxPos(2);
	    candidates.push_back(etac);
	  }
	} // rhos
      } // selectedDiJpsiCandidates
    } // m_tracks_num==2
    else if(m_tracks_num==4) {
      for(auto iter1=rhos.cbegin(); iter1!=rhos.cend(); ++iter1) {
	tracksRho1.clear();
	tracksRho1.push_back(iter1->trackParticle1);
	tracksRho1.push_back(iter1->trackParticle2);
	for(auto iter2=iter1+1; iter2!=rhos.cend(); ++iter2) {
	  if(std::find(tracksRho1.cbegin(), tracksRho1.cend(), iter2->trackParticle1) != tracksRho1.cend())
	    continue;
	  if(std::find(tracksRho1.cbegin(), tracksRho1.cend(), iter2->trackParticle2) != tracksRho1.cend())
	    continue;
	  std::vector<double> trackPt{iter1->trackParticle1->pt(), iter1->trackParticle2->pt(), iter2->trackParticle1->pt(), iter2->trackParticle2->pt()};
	  std::sort( trackPt.begin(), trackPt.end(), [](double a, double b) { return a>b; } );
	  if(trackPt[0]<m_trkMinPt1 || trackPt[1]<m_trkMinPt2 || trackPt[2]<m_trkMinPt3 || trackPt[3]<m_trkMinPt4)
	    continue;
	  EtacCandidate etac1 = getEtacCandidate(*iter1, *iter2, selectedDiJpsiCandidates);
	  EtacCandidate etac2 = getEtacCandidate(*iter2, *iter1, selectedDiJpsiCandidates);
	  bool etac1_pass = (etac1.nTracks != 0); bool etac2_pass = (etac2.nTracks != 0);
	  if(etac1_pass) {
	    if(isFound(etac1,candidates))      etac1_pass = false;
	    else candidates.push_back(etac1);
	  }
	  if(etac2_pass) {
	    if(etac1_pass && m_same_mass12)    etac2_pass = false;
	    else if(isFound(etac2,candidates)) etac2_pass = false;
	    else candidates.push_back(etac2);
	  }
	} // iter2 of rhos
      } // iter1 of rhos
    } // m_tracks_num==4
    else { // m_tracks_num==6
      for(auto iter1=rhos.cbegin(); iter1!=rhos.cend(); ++iter1) {
	tracksRho1.clear();
	tracksRho1.push_back(iter1->trackParticle1);
	tracksRho1.push_back(iter1->trackParticle2);
	for(auto iter2=iter1+1; iter2!=rhos.cend(); ++iter2) {
	  if(std::find(tracksRho1.cbegin(), tracksRho1.cend(), iter2->trackParticle1) != tracksRho1.cend())
	    continue;
	  if(std::find(tracksRho1.cbegin(), tracksRho1.cend(), iter2->trackParticle2) != tracksRho1.cend())
	    continue;
	  tracksRho12.clear();
	  tracksRho12.push_back(iter1->trackParticle1);
	  tracksRho12.push_back(iter1->trackParticle2);
	  tracksRho12.push_back(iter2->trackParticle1);
	  tracksRho12.push_back(iter2->trackParticle2);
	  for(auto iter3=iter2+1; iter3!=rhos.cend(); ++iter3) {
	    if(std::find(tracksRho12.cbegin(), tracksRho12.cend(), iter3->trackParticle1) != tracksRho12.cend())
	      continue;
	    if(std::find(tracksRho12.cbegin(), tracksRho12.cend(), iter3->trackParticle2) != tracksRho12.cend())
	      continue;	
	    std::vector<double> trackPt{iter1->trackParticle1->pt(), iter1->trackParticle2->pt(), iter2->trackParticle1->pt(), iter2->trackParticle2->pt(), iter3->trackParticle1->pt(), iter3->trackParticle2->pt()};
	    std::sort( trackPt.begin(), trackPt.end(), [](double a, double b) { return a>b; } );
	    if(trackPt[0]<m_trkMinPt1 || trackPt[1]<m_trkMinPt2 || trackPt[2]<m_trkMinPt3 || trackPt[3]<m_trkMinPt4 || trackPt[4]<m_trkMinPt5 || trackPt[5]<m_trkMinPt6)
	      continue;
	    EtacCandidate etac1 = getEtacCandidate(*iter1, *iter2, *iter3, selectedDiJpsiCandidates);
	    EtacCandidate etac2 = getEtacCandidate(*iter1, *iter3, *iter2, selectedDiJpsiCandidates);
	    EtacCandidate etac3 = getEtacCandidate(*iter2, *iter1, *iter3, selectedDiJpsiCandidates);
	    EtacCandidate etac4 = getEtacCandidate(*iter2, *iter3, *iter1, selectedDiJpsiCandidates);
	    EtacCandidate etac5 = getEtacCandidate(*iter3, *iter1, *iter2, selectedDiJpsiCandidates);
	    EtacCandidate etac6 = getEtacCandidate(*iter3, *iter2, *iter1, selectedDiJpsiCandidates);
	    bool etac1_pass = (etac1.nTracks != 0); bool etac2_pass = (etac2.nTracks != 0);
	    bool etac3_pass = (etac3.nTracks != 0); bool etac4_pass = (etac4.nTracks != 0);
	    bool etac5_pass = (etac5.nTracks != 0); bool etac6_pass = (etac6.nTracks != 0);

	    if(etac1_pass) {
	      if(isFound(etac1,candidates))         etac1_pass = false;
	      else candidates.push_back(etac1);
	    }
	    if(etac2_pass) {
	      if(etac1_pass && m_same_mass23)       etac2_pass = false;
	      else if(isFound(etac2,candidates))    etac2_pass = false;
	      else candidates.push_back(etac2);
	    }
	    if(etac3_pass) {
	      if(etac1_pass && m_same_mass12)       etac3_pass = false;
	      else if(etac2_pass && m_same_mass123) etac3_pass = false;
	      else if(isFound(etac3,candidates))    etac3_pass = false;
	      else candidates.push_back(etac3);
	    }
	    if(etac4_pass) {
	      if(etac1_pass && m_same_mass123)      etac4_pass = false;
	      else if(etac2_pass && m_same_mass12)  etac4_pass = false;
	      else if(etac3_pass && m_same_mass13)  etac4_pass = false;
	      else if(isFound(etac4,candidates))    etac4_pass = false;
	      else candidates.push_back(etac4);
	    }
	    if(etac5_pass) {
	      if(etac1_pass && m_same_mass123)      etac5_pass = false;
	      else if(etac2_pass && m_same_mass13)  etac5_pass = false;
	      else if(etac3_pass && m_same_mass23)  etac5_pass = false;
	      else if(etac4_pass && m_same_mass123) etac5_pass = false;
	      else if(isFound(etac5,candidates))    etac5_pass = false;
	      else candidates.push_back(etac5);
	    }
	    if(etac6_pass) {
	      if(etac1_pass && m_same_mass13)       etac6_pass = false;
	      else if(etac2_pass && m_same_mass123) etac6_pass = false;
	      else if(etac3_pass && m_same_mass123) etac6_pass = false;
	      else if(etac4_pass && m_same_mass23)  etac6_pass = false;
	      else if(etac5_pass && m_same_mass12)  etac6_pass = false;
	      else if(isFound(etac6,candidates))    etac6_pass = false;
	      else candidates.push_back(etac6);
	    }
	  } // iter3 of rhos
	} // iter2 of rhos
      } // iter1 of rhos
    } // m_tracks_num==6

    if(m_ptOrdering) { // order by pt
      std::sort( candidates.begin(), candidates.end(), [](const EtacCandidate& a, const EtacCandidate& b) { return a.ptTot > b.ptTot; } );
    }
    else { // order by chi2/NDF sum
      std::sort( candidates.begin(), candidates.end(), [](const EtacCandidate& a, const EtacCandidate& b) { return a.chi2NDFSum < b.chi2NDFSum; } );
    }
    if(m_maxCandidates>0 && candidates.size()>m_maxCandidates) {
      candidates.erase(candidates.begin()+m_maxCandidates, candidates.end());
    }

    // loop over di-Jpsi
    for(auto iter=selectedDiJpsiCandidates.cbegin(); iter!=selectedDiJpsiCandidates.cend(); ++iter) {
      tracksJpsis.clear();
      tracksJpsis.push_back(iter->first->trackParticle(0));
      tracksJpsis.push_back(iter->first->trackParticle(1));
      tracksJpsis.push_back(iter->second->trackParticle(0));
      tracksJpsis.push_back(iter->second->trackParticle(1));
      for(auto&& etac : candidates) {
	if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), etac.trackParticle1) != tracksJpsis.cend())
	  continue;
	if(m_tracks_num>=2) {
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), etac.trackParticle2) != tracksJpsis.cend())
	    continue;
	}
	if(m_tracks_num>=4) {
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), etac.trackParticle3) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), etac.trackParticle4) != tracksJpsis.cend())
	    continue;
	}
	if(m_tracks_num==6) {
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), etac.trackParticle5) != tracksJpsis.cend())
	  continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), etac.trackParticle6) != tracksJpsis.cend())
	  continue;
	}
	inputTracks.clear();
	inputTracks.push_back(tracksJpsis[0]);
	inputTracks.push_back(tracksJpsis[1]);
	inputTracks.push_back(tracksJpsis[2]);
	inputTracks.push_back(tracksJpsis[3]);
	inputTracks.push_back(etac.trackParticle1);
	if(m_tracks_num>=2) {
	  inputTracks.push_back(etac.trackParticle2);
	}
	if(m_tracks_num>=4) {
	  inputTracks.push_back(etac.trackParticle3);
	  inputTracks.push_back(etac.trackParticle4);
	}
	if(m_tracks_num==6) {
	  inputTracks.push_back(etac.trackParticle5);
	  inputTracks.push_back(etac.trackParticle6);
	}

	p4_mu1.SetPtEtaPhiM(inputTracks[0]->pt(), inputTracks[0]->eta(), inputTracks[0]->phi(), m_vtx0Daug1MassHypo);
	p4_mu2.SetPtEtaPhiM(inputTracks[1]->pt(), inputTracks[1]->eta(), inputTracks[1]->phi(), m_vtx0Daug2MassHypo);
	p4_mu3.SetPtEtaPhiM(inputTracks[2]->pt(), inputTracks[2]->eta(), inputTracks[2]->phi(), m_vtx0Daug3MassHypo);
	p4_mu4.SetPtEtaPhiM(inputTracks[3]->pt(), inputTracks[3]->eta(), inputTracks[3]->phi(), m_vtx0Daug4MassHypo);
	p4_track1.SetPtEtaPhiM(inputTracks[4]->pt(), inputTracks[4]->eta(), inputTracks[4]->phi(), m_vtx1Daug1MassHypo);
	if(m_tracks_num>=2) {
	  p4_track2.SetPtEtaPhiM(inputTracks[5]->pt(), inputTracks[5]->eta(), inputTracks[5]->phi(), m_vtx1Daug2MassHypo);
	}
	if(m_tracks_num>=4) {
	  p4_track3.SetPtEtaPhiM(inputTracks[6]->pt(), inputTracks[6]->eta(), inputTracks[6]->phi(), m_vtx2Daug1MassHypo);
	  p4_track4.SetPtEtaPhiM(inputTracks[7]->pt(), inputTracks[7]->eta(), inputTracks[7]->phi(), m_vtx2Daug2MassHypo);
	}
	if(m_tracks_num==6) {
	  p4_track5.SetPtEtaPhiM(inputTracks[8]->pt(), inputTracks[8]->eta(), inputTracks[8]->phi(), m_vtx3Daug1MassHypo);
	  p4_track6.SetPtEtaPhiM(inputTracks[9]->pt(), inputTracks[9]->eta(), inputTracks[9]->phi(), m_vtx3Daug2MassHypo);
	}

	if(m_tracks_num==1) {
	  if((p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1).M()<m_MassLower || (p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1).M()>m_MassUpper) continue;
	}
	else if(m_tracks_num==2) {
	  if((p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2).M()<m_MassLower || (p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2).M()>m_MassUpper) continue;
	}
	else if(m_tracks_num==4) {
	  if((p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2+p4_track3+p4_track4).M()<m_MassLower || (p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2+p4_track3+p4_track4).M()>m_MassUpper) continue;
	}
	else {
	  if((p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2+p4_track3+p4_track4+p4_track5+p4_track6).M()<m_MassLower || (p4_mu1+p4_mu2+p4_mu3+p4_mu4+p4_track1+p4_track2+p4_track3+p4_track4+p4_track5+p4_track6).M()>m_MassUpper) continue;
	}
	bool passDR = true;
	int j = 4;
	if(m_tracks_num==2)      j = 5;
	else if(m_tracks_num==4) j = 7;
	else if(m_tracks_num==6) j = 9;
	for(int i=4; i<=j; i++) {
	  if(DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),inputTracks[i]->eta(),inputTracks[i]->phi())>m_maxDR) { passDR = false; break; }
	}
	if(!passDR) continue;

	// start the fit
	m_iVertexFitter->setDefault();

	m_iVertexFitter->setMassInputParticles(massesInputTracks);
	if (m_constrJpsi1) {
	  m_iVertexFitter->setMassForConstraint(m_mass_jpsi1, std::vector<int>{1,2});
	}
	if (m_constrJpsi2) {
	  m_iVertexFitter->setMassForConstraint(m_mass_jpsi2, std::vector<int>{3,4});
	}
	if (m_tracks_num>=2 && m_constrRho1) {
	  m_iVertexFitter->setMassForConstraint(m_mass_rho1, std::vector<int>{5,6});
	}
	if (m_tracks_num>=4 && m_constrRho2) {
	  m_iVertexFitter->setMassForConstraint(m_mass_rho2, std::vector<int>{7,8});
	}
	if (m_tracks_num==6 && m_constrRho3) {
	  m_iVertexFitter->setMassForConstraint(m_mass_rho3, std::vector<int>{9,10});
	}
	if (m_constrEtac) {
	  if(m_tracks_num==4) {
	    m_iVertexFitter->setMassForConstraint(m_mass_etac, std::vector<int>{5,6,7,8});
	  }
	  else if(m_tracks_num==6) {
	    m_iVertexFitter->setMassForConstraint(m_mass_etac, std::vector<int>{5,6,7,8,9,10});
	  }
	}

	Amg::Vector3D startPoint;
	if(m_tracks_num==1) {
	  startPoint(0) = (iter->first->x() + iter->second->x())/2;
	  startPoint(1) = (iter->first->y() + iter->second->y())/2;
	  startPoint(2) = (iter->first->z() + iter->second->z())/2;
	}
	else if(m_tracks_num==2) {
	  startPoint(0) = (iter->first->x() + iter->second->x() + etac.vtxPos1(0))/3;
	  startPoint(1) = (iter->first->y() + iter->second->y() + etac.vtxPos1(1))/3;
	  startPoint(2) = (iter->first->z() + iter->second->z() + etac.vtxPos1(2))/3;
	}
	else if(m_tracks_num==4) {
	  startPoint(0) = (iter->first->x() + iter->second->x() + etac.vtxPos1(0) + etac.vtxPos2(0))/4;
	  startPoint(1) = (iter->first->y() + iter->second->y() + etac.vtxPos1(1) + etac.vtxPos2(1))/4;
	  startPoint(2) = (iter->first->z() + iter->second->z() + etac.vtxPos1(2) + etac.vtxPos2(2))/4;
	}
	else {
	  startPoint(0) = (iter->first->x() + iter->second->x() + etac.vtxPos1(0) + etac.vtxPos2(0) + etac.vtxPos3(0))/5;
	  startPoint(1) = (iter->first->y() + iter->second->y() + etac.vtxPos1(1) + etac.vtxPos2(1) + etac.vtxPos3(1))/5;
	  startPoint(2) = (iter->first->z() + iter->second->z() + etac.vtxPos1(2) + etac.vtxPos2(2) + etac.vtxPos3(2))/5;
	}

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

  std::unique_ptr<xAOD::Vertex> DiJpsiPlusTracksSingleVertex::fitTwoTracks(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2) const {
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

  double DiJpsiPlusTracksSingleVertex::DR(double eta1, double phi1, double eta2, double phi2) const {
    double deta = std::abs(eta1-eta2);
    double dphi = std::abs(phi1-phi2);
    if(dphi>M_PI) dphi = 2*M_PI-dphi;
    return std::sqrt(std::pow(deta,2)+std::pow(dphi,2));
  }

  EtacCandidate DiJpsiPlusTracksSingleVertex::getEtacCandidate(const RhoCandidate& rho1, const RhoCandidate& rho2, const std::vector<std::pair<const xAOD::Vertex*, const xAOD::Vertex*> >& diJpsiCandidates) const {
    EtacCandidate etac;
    if(rho1.mass_hypo1>m_rho1MassLower && rho1.mass_hypo1<m_rho1MassUpper &&
       rho2.mass_hypo2>m_rho2MassLower && rho2.mass_hypo2<m_rho2MassUpper) {
      TLorentzVector p4_trk1, p4_trk2, p4_trk3, p4_trk4;
      p4_trk1.SetPtEtaPhiM(rho1.trackParticle1->pt(), rho1.trackParticle1->eta(), rho1.trackParticle1->phi(), m_vtx1Daug1MassHypo);
      p4_trk2.SetPtEtaPhiM(rho1.trackParticle2->pt(), rho1.trackParticle2->eta(), rho1.trackParticle2->phi(), m_vtx1Daug2MassHypo);
      p4_trk3.SetPtEtaPhiM(rho2.trackParticle1->pt(), rho2.trackParticle1->eta(), rho2.trackParticle1->phi(), m_vtx2Daug1MassHypo);
      p4_trk4.SetPtEtaPhiM(rho2.trackParticle2->pt(), rho2.trackParticle2->eta(), rho2.trackParticle2->phi(), m_vtx2Daug2MassHypo);
      if((p4_trk1+p4_trk2+p4_trk3+p4_trk4).M()>m_etacMassLower && (p4_trk1+p4_trk2+p4_trk3+p4_trk4).M()<m_etacMassUpper) {
	bool passDR = false;
	std::vector<const xAOD::TrackParticle*> tracksJpsis;
	TLorentzVector p4_mu1, p4_mu2, p4_mu3, p4_mu4;
	for(auto iter=diJpsiCandidates.cbegin(); iter!=diJpsiCandidates.cend(); ++iter) {
	  tracksJpsis.clear();
	  tracksJpsis.push_back(iter->first->trackParticle(0));
	  tracksJpsis.push_back(iter->first->trackParticle(1));
	  tracksJpsis.push_back(iter->second->trackParticle(0));
	  tracksJpsis.push_back(iter->second->trackParticle(1));
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho1.trackParticle1) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho1.trackParticle2) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho2.trackParticle1) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho2.trackParticle2) != tracksJpsis.cend())
	    continue;
	  p4_mu1.SetPtEtaPhiM(tracksJpsis[0]->pt(), tracksJpsis[0]->eta(), tracksJpsis[0]->phi(), m_vtx0Daug1MassHypo);
	  p4_mu2.SetPtEtaPhiM(tracksJpsis[1]->pt(), tracksJpsis[1]->eta(), tracksJpsis[1]->phi(), m_vtx0Daug2MassHypo);
	  p4_mu3.SetPtEtaPhiM(tracksJpsis[2]->pt(), tracksJpsis[2]->eta(), tracksJpsis[2]->phi(), m_vtx0Daug3MassHypo);
	  p4_mu4.SetPtEtaPhiM(tracksJpsis[3]->pt(), tracksJpsis[3]->eta(), tracksJpsis[3]->phi(), m_vtx0Daug4MassHypo);
	  if(DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho1.trackParticle1->eta(),rho1.trackParticle1->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho1.trackParticle2->eta(),rho1.trackParticle2->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho2.trackParticle1->eta(),rho2.trackParticle1->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho2.trackParticle2->eta(),rho2.trackParticle2->phi())<m_maxDR) {
	    passDR = true; break;
	  }
	}
	if(passDR) {
	  etac.nTracks = 4;
	  etac.trackParticle1 = rho1.trackParticle1; etac.trackParticle2 = rho1.trackParticle2;
	  etac.trackParticle3 = rho2.trackParticle1; etac.trackParticle4 = rho2.trackParticle2;
	  etac.ptTot = (p4_trk1+p4_trk2+p4_trk3+p4_trk4).Pt();
	  etac.chi2NDFSum = rho1.chi2NDF + rho2.chi2NDF;
	  etac.vtxPos1(0) = rho1.vtxPos(0); etac.vtxPos2(0) = rho2.vtxPos(0);
	  etac.vtxPos1(1) = rho1.vtxPos(1); etac.vtxPos2(1) = rho2.vtxPos(1);
	  etac.vtxPos1(2) = rho1.vtxPos(2); etac.vtxPos2(2) = rho2.vtxPos(2);
	}
      }
    }
    return etac;
  }

  EtacCandidate DiJpsiPlusTracksSingleVertex::getEtacCandidate(const RhoCandidate& rho1, const RhoCandidate& rho2, const RhoCandidate& rho3, const std::vector<std::pair<const xAOD::Vertex*, const xAOD::Vertex*> >& diJpsiCandidates) const {
    EtacCandidate etac;
    if(rho1.mass_hypo1>m_rho1MassLower && rho1.mass_hypo1<m_rho1MassUpper &&
       rho2.mass_hypo2>m_rho2MassLower && rho2.mass_hypo2<m_rho2MassUpper &&
       rho3.mass_hypo3>m_rho3MassLower && rho3.mass_hypo3<m_rho3MassUpper) {
      TLorentzVector p4_trk1, p4_trk2, p4_trk3, p4_trk4, p4_trk5, p4_trk6;
      p4_trk1.SetPtEtaPhiM(rho1.trackParticle1->pt(), rho1.trackParticle1->eta(), rho1.trackParticle1->phi(), m_vtx1Daug1MassHypo);
      p4_trk2.SetPtEtaPhiM(rho1.trackParticle2->pt(), rho1.trackParticle2->eta(), rho1.trackParticle2->phi(), m_vtx1Daug2MassHypo);
      p4_trk3.SetPtEtaPhiM(rho2.trackParticle1->pt(), rho2.trackParticle1->eta(), rho2.trackParticle1->phi(), m_vtx2Daug1MassHypo);
      p4_trk4.SetPtEtaPhiM(rho2.trackParticle2->pt(), rho2.trackParticle2->eta(), rho2.trackParticle2->phi(), m_vtx2Daug2MassHypo);
      p4_trk5.SetPtEtaPhiM(rho3.trackParticle1->pt(), rho3.trackParticle1->eta(), rho3.trackParticle1->phi(), m_vtx3Daug1MassHypo);
      p4_trk6.SetPtEtaPhiM(rho3.trackParticle2->pt(), rho3.trackParticle2->eta(), rho3.trackParticle2->phi(), m_vtx3Daug2MassHypo);
      if((p4_trk1+p4_trk2+p4_trk3+p4_trk4+p4_trk5+p4_trk6).M()>m_etacMassLower && (p4_trk1+p4_trk2+p4_trk3+p4_trk4+p4_trk5+p4_trk6).M()<m_etacMassUpper) {
	bool passDR = false;
	std::vector<const xAOD::TrackParticle*> tracksJpsis;
	TLorentzVector p4_mu1, p4_mu2, p4_mu3, p4_mu4;
	for(auto iter=diJpsiCandidates.cbegin(); iter!=diJpsiCandidates.cend(); ++iter) {
	  tracksJpsis.clear();
	  tracksJpsis.push_back(iter->first->trackParticle(0));
	  tracksJpsis.push_back(iter->first->trackParticle(1));
	  tracksJpsis.push_back(iter->second->trackParticle(0));
	  tracksJpsis.push_back(iter->second->trackParticle(1));
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho1.trackParticle1) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho1.trackParticle2) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho2.trackParticle1) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho2.trackParticle2) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho3.trackParticle1) != tracksJpsis.cend())
	    continue;
	  if(std::find(tracksJpsis.cbegin(), tracksJpsis.cend(), rho3.trackParticle2) != tracksJpsis.cend())
	    continue;
	  p4_mu1.SetPtEtaPhiM(tracksJpsis[0]->pt(), tracksJpsis[0]->eta(), tracksJpsis[0]->phi(), m_vtx0Daug1MassHypo);
	  p4_mu2.SetPtEtaPhiM(tracksJpsis[1]->pt(), tracksJpsis[1]->eta(), tracksJpsis[1]->phi(), m_vtx0Daug2MassHypo);
	  p4_mu3.SetPtEtaPhiM(tracksJpsis[2]->pt(), tracksJpsis[2]->eta(), tracksJpsis[2]->phi(), m_vtx0Daug3MassHypo);
	  p4_mu4.SetPtEtaPhiM(tracksJpsis[3]->pt(), tracksJpsis[3]->eta(), tracksJpsis[3]->phi(), m_vtx0Daug4MassHypo);
	  if(DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho1.trackParticle1->eta(),rho1.trackParticle1->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho1.trackParticle2->eta(),rho1.trackParticle2->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho2.trackParticle1->eta(),rho2.trackParticle1->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho2.trackParticle2->eta(),rho2.trackParticle2->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho3.trackParticle1->eta(),rho3.trackParticle1->phi())<m_maxDR &&
	     DR((p4_mu1+p4_mu2+p4_mu3+p4_mu4).Eta(),(p4_mu1+p4_mu2+p4_mu3+p4_mu4).Phi(),rho3.trackParticle2->eta(),rho3.trackParticle2->phi())<m_maxDR) {
	    passDR = true; break;
	  }
	}
	if(passDR) {
	  etac.nTracks = 6;
	  etac.trackParticle1 = rho1.trackParticle1; etac.trackParticle2 = rho1.trackParticle2;
	  etac.trackParticle3 = rho2.trackParticle1; etac.trackParticle4 = rho2.trackParticle2;
	  etac.trackParticle5 = rho3.trackParticle1; etac.trackParticle6 = rho3.trackParticle2;
	  etac.ptTot = (p4_trk1+p4_trk2+p4_trk3+p4_trk4+p4_trk5+p4_trk6).Pt();
	  etac.chi2NDFSum = rho1.chi2NDF + rho2.chi2NDF + rho3.chi2NDF;
	  etac.vtxPos1(0) = rho1.vtxPos(0); etac.vtxPos2(0) = rho2.vtxPos(0); etac.vtxPos3(0) = rho3.vtxPos(0);
	  etac.vtxPos1(1) = rho1.vtxPos(1); etac.vtxPos2(1) = rho2.vtxPos(1); etac.vtxPos3(1) = rho3.vtxPos(1);
	  etac.vtxPos1(2) = rho1.vtxPos(2); etac.vtxPos2(2) = rho2.vtxPos(2); etac.vtxPos3(2) = rho3.vtxPos(2);
	}
      }
    }
    return etac;
  }

  bool DiJpsiPlusTracksSingleVertex::isFound(const EtacCandidate& etac, const std::vector<EtacCandidate>& candidates) const {
    std::set<const xAOD::TrackParticle*> tracks_set1_etac{etac.trackParticle1, etac.trackParticle2};
    std::set<const xAOD::TrackParticle*> tracks_set2_etac{etac.trackParticle3, etac.trackParticle4};
    std::set<const xAOD::TrackParticle*> tracks_set3_etac{etac.trackParticle5, etac.trackParticle6};
    std::set<const xAOD::TrackParticle*> tracks_set12_etac{etac.trackParticle1, etac.trackParticle2, etac.trackParticle3, etac.trackParticle4};
    std::set<const xAOD::TrackParticle*> tracks_set13_etac{etac.trackParticle1, etac.trackParticle2, etac.trackParticle5, etac.trackParticle6};
    std::set<const xAOD::TrackParticle*> tracks_set23_etac{etac.trackParticle3, etac.trackParticle4, etac.trackParticle5, etac.trackParticle6};
    std::set<const xAOD::TrackParticle*> tracks_all_etac{etac.trackParticle1, etac.trackParticle2, etac.trackParticle3, etac.trackParticle4, etac.trackParticle5, etac.trackParticle6};
    for(auto iter=candidates.cbegin(); iter!=candidates.cend(); ++iter) {
      if(etac.nTracks != iter->nTracks) continue;
      if(etac.nTracks==4) {
	std::set<const xAOD::TrackParticle*> tracks_cand{iter->trackParticle1, iter->trackParticle2, iter->trackParticle3, iter->trackParticle4};
	if(tracks_set12_etac == tracks_cand && m_same_mass12) return true;
      }
      else { // etac.nTracks==6
	std::set<const xAOD::TrackParticle*> tracks_set1_cand{iter->trackParticle1, iter->trackParticle2};
	std::set<const xAOD::TrackParticle*> tracks_set2_cand{iter->trackParticle3, iter->trackParticle4};
	std::set<const xAOD::TrackParticle*> tracks_set3_cand{iter->trackParticle5, iter->trackParticle6};
	std::set<const xAOD::TrackParticle*> tracks_set12_cand{iter->trackParticle1, iter->trackParticle2, iter->trackParticle3, iter->trackParticle4};
	std::set<const xAOD::TrackParticle*> tracks_set13_cand{iter->trackParticle1, iter->trackParticle2, iter->trackParticle5, iter->trackParticle6};
	std::set<const xAOD::TrackParticle*> tracks_set23_cand{iter->trackParticle3, iter->trackParticle4, iter->trackParticle5, iter->trackParticle6};
	std::set<const xAOD::TrackParticle*> tracks_all_cand{iter->trackParticle1, iter->trackParticle2, iter->trackParticle3, iter->trackParticle4, iter->trackParticle5, iter->trackParticle6};
	if(tracks_set12_etac == tracks_set12_cand && tracks_set3_etac == tracks_set3_cand && m_same_mass12) return true;
	if(tracks_set13_etac == tracks_set13_cand && tracks_set2_etac == tracks_set2_cand && m_same_mass13) return true;
	if(tracks_set23_etac == tracks_set23_cand && tracks_set1_etac == tracks_set1_cand && m_same_mass23) return true;
	if(tracks_all_etac == tracks_all_cand && m_same_mass123) return true;
      }
    }
    return false;
  }
}
