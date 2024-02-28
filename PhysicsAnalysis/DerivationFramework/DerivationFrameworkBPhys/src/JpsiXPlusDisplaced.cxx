/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// JpsiXPlusDisplaced.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/JpsiXPlusDisplaced.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkV0Fitter/TrkV0VertexFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetBeamSpotService/IBeamCondSvc.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "DerivationFrameworkBPhys/LocalVector.h"
#include "HepPDT/ParticleDataTable.hh"
#include "VxVertex/RecVertex.h"
#include "xAODEventInfo/EventInfo.h"
#include <algorithm>
#include <functional>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;

  JpsiXPlusDisplaced::JpsiXPlusDisplaced(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexJXContainerKey("InputJXVertices"),
    m_vertexV0ContainerKey("InputV0Vertices"),
    m_vertexDisVContainerKey(""),
    m_cascadeOutputsKeys{ "JpsiXPlusDisplacedVtx1_sub", "JpsiXPlusDisplacedVtx1", "JpsiXPlusDisplacedVtx2", "JpsiXPlusDisplacedVtx3" },
    m_refitV0(false),
    m_v0VtxOutputsKey(""),
    m_disVtxOutputsKey(""),
    m_TrkParticleCollection("InDetTrackParticles"),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_jxMassLower(0.0),
    m_jxMassUpper(30000.0),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(20000.0),
    m_diTrackMassLower(-1.0),
    m_diTrackMassUpper(-1.0),
    m_V0Hypothesis("Lambda"),
    m_V0MassLower(0.0),
    m_V0MassUpper(20000.0),
    m_lxyV0_cut(-999.0),
    m_doV0Enum(false),
    m_decorV0P(false),
    m_minMass_gamma(-1.0),
    m_chi2cut_gamma(-1.0),
    m_DisplacedMassLower(0.0),
    m_DisplacedMassUpper(30000.0),
    m_lxyDisV_cut(-999.0),
    m_MassLower(0.0),
    m_MassUpper(41000.0),
    m_jxDaug_num(4),
    m_jxDaug1MassHypo(-1),
    m_jxDaug2MassHypo(-1),
    m_jxDaug3MassHypo(-1),
    m_jxDaug4MassHypo(-1),
    m_disVDaug_num(3),
    m_disVDaug3MassHypo(-1),
    m_extraTrkMassHypo(-1),
    m_extraTrkMinPt(500),
    m_massJX(-1),
    m_massJpsi(-1),
    m_massX(-1),
    m_massDisV(-1),
    m_massV0(-1),
    m_massMainV(-1),
    m_constrJX(false),
    m_constrJpsi(false),
    m_constrX(false),
    m_constrDisV(false),
    m_constrV0(false),
    m_constrMainV(false),
    m_chi2cut_JX(-1.0),
    m_chi2cut_V0(-1.0),
    m_chi2cut_DisV(-1.0),
    m_chi2cut(-1.0),
    m_useTRT(false),
    m_ptTRT(400),
    m_d0_cut(2),
    m_maxJXCandidates(0),
    m_maxV0Candidates(0),
    m_maxDisVCandidates(0),
    m_maxMainVCandidates(0),
    m_beamCondSvc("BeamCondSvc",name),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_iV0Fitter("Trk::V0VertexFitter"),
    m_iGammaFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_trackToVertexTool("Reco::TrackToVertex"),
    m_trkSelector("InDet::TrackSelectorTool"),
    m_v0TrkSelector("InDet::TrackSelectorTool"),
    m_CascadeTools("DerivationFramework::CascadeTools")
  {
    declareProperty("JXVertices",               m_vertexJXContainerKey);
    declareProperty("V0Vertices",               m_vertexV0ContainerKey);
    declareProperty("DisplacedVertices",        m_vertexDisVContainerKey);
    declareProperty("JXVtxHypoNames",           m_vertexJXHypoNames);
    declareProperty("V0VtxHypoNames",           m_vertexV0HypoNames);
    declareProperty("CascadeVertexCollections", m_cascadeOutputsKeys); // size is 3 or 4 only
    declareProperty("RefitV0",                  m_refitV0);
    declareProperty("OutoutV0VtxCollection",    m_v0VtxOutputsKey);
    declareProperty("OutoutDisVtxCollection",   m_disVtxOutputsKey);
    declareProperty("TrackParticleCollection",  m_TrkParticleCollection);
    declareProperty("VxPrimaryCandidateName",   m_VxPrimaryCandidateName);
    declareProperty("RefPVContainerName",       m_refPVContainerName = "RefittedPrimaryVertices");
    declareProperty("JXMassLowerCut",           m_jxMassLower); // only effective when m_jxDaug_num>2
    declareProperty("JXMassUpperCut",           m_jxMassUpper); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMassLowerCut",         m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",         m_jpsiMassUpper);
    declareProperty("DiTrackMassLower",         m_diTrackMassLower); // only effective when m_jxDaug_num=4
    declareProperty("DiTrackMassUpper",         m_diTrackMassUpper); // only effective when m_jxDaug_num=4
    declareProperty("V0Hypothesis",             m_V0Hypothesis); // "Ks" or "Lambda"
    declareProperty("V0MassLowerCut",           m_V0MassLower);
    declareProperty("V0MassUpperCut",           m_V0MassUpper);
    declareProperty("LxyV0Cut",                 m_lxyV0_cut);
    declareProperty("DoV0Enumeration",          m_doV0Enum);
    declareProperty("DecorateV0Momentum",       m_decorV0P); // only effective when m_refitV0=true and m_constrV0=true
    declareProperty("MassCutGamma",             m_minMass_gamma);
    declareProperty("Chi2CutGamma",             m_chi2cut_gamma);
    declareProperty("DisplacedMassLowerCut",    m_DisplacedMassLower); // only effective when m_disVDaug_num=3
    declareProperty("DisplacedMassUpperCut",    m_DisplacedMassUpper); // only effective when m_disVDaug_num=3
    declareProperty("LxyDisVtxCut",             m_lxyDisV_cut); // only effective when m_disVDaug_num=3
    declareProperty("MassLowerCut",             m_MassLower);
    declareProperty("MassUpperCut",             m_MassUpper);
    declareProperty("HypothesisName",           m_hypoName = "TQ");
    declareProperty("NumberOfJXDaughters",      m_jxDaug_num); // 2, or 3, or 4 only
    declareProperty("JXDaug1MassHypo",          m_jxDaug1MassHypo);
    declareProperty("JXDaug2MassHypo",          m_jxDaug2MassHypo);
    declareProperty("JXDaug3MassHypo",          m_jxDaug3MassHypo);
    declareProperty("JXDaug4MassHypo",          m_jxDaug4MassHypo);
    declareProperty("NumberOfDisVDaughters",    m_disVDaug_num); // 2 or 3 only
    declareProperty("DisVDaug3MassHypo",        m_disVDaug3MassHypo); // only effective when m_disVDaug_num=3
    declareProperty("ExtraTrackMassHypo",       m_extraTrkMassHypo); // for decays like B- -> J/psi Lambda pbar, the extra track is pbar (for m_disVDaug_num=2 only now)
    declareProperty("ExtraTrackMinPt",          m_extraTrkMinPt); // only effective if m_extraTrkMassHypo>0
    declareProperty("JXMass",                   m_massJX); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMass",                 m_massJpsi);
    declareProperty("XMass",                    m_massX); // only effective when m_jxDaug_num=4
    declareProperty("DisVtxMass",               m_massDisV); // only effective when m_disVDaug_num=3
    declareProperty("V0Mass",                   m_massV0);
    declareProperty("MainVtxMass",              m_massMainV);
    declareProperty("ApplyJXMassConstraint",    m_constrJX);
    declareProperty("ApplyJpsiMassConstraint",  m_constrJpsi); // only effective when m_jxDaug_num>2
    declareProperty("ApplyXMassConstraint",     m_constrX); // only effective when m_jxDaug_num=4
    declareProperty("ApplyDisVMassConstraint",  m_constrDisV); // only effective when m_disVDaug_num=3
    declareProperty("ApplyV0MassConstraint",    m_constrV0);
    declareProperty("ApplyMainVMassConstraint", m_constrMainV);
    declareProperty("Chi2CutJX",                m_chi2cut_JX);
    declareProperty("Chi2CutV0",                m_chi2cut_V0);
    declareProperty("Chi2CutDisV",              m_chi2cut_DisV); // only effective when m_disVDaug_num=3
    declareProperty("Chi2Cut",                  m_chi2cut);
    declareProperty("UseTRT",                   m_useTRT); // only effective when m_disVDaug_num=3
    declareProperty("PtTRT",                    m_ptTRT); // only effective when m_disVDaug_num=3
    declareProperty("Trackd0Cut",               m_d0_cut); // only effective when m_disVDaug_num=3
    declareProperty("MaxJXCandidates",          m_maxJXCandidates);
    declareProperty("MaxV0Candidates",          m_maxV0Candidates);
    declareProperty("MaxDisVCandidates",        m_maxDisVCandidates); // only effective when m_disVDaug_num=3
    declareProperty("MaxMainVCandidates",       m_maxMainVCandidates);
    declareProperty("RefitPV",                  m_refitPV         = true);
    declareProperty("MaxnPV",                   m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",           m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",             m_DoVertexType    = 7);
    declareProperty("BeamConditionsSvc",        m_beamCondSvc);
    declareProperty("TrkVertexFitterTool",      m_iVertexFitter);
    declareProperty("V0VertexFitterTool",       m_iV0Fitter);
    declareProperty("GammaFitterTool",          m_iGammaFitter);
    declareProperty("PVRefitter",               m_pvRefitter);
    declareProperty("V0Tools",                  m_V0Tools);
    declareProperty("TrackToVertexTool",        m_trackToVertexTool);
    declareProperty("TrackSelectorTool",        m_trkSelector);
    declareProperty("V0TrackSelectorTool",      m_v0TrkSelector);
    declareProperty("CascadeTools",             m_CascadeTools);
  }

  StatusCode JpsiXPlusDisplaced::initialize() {
    if(m_V0Hypothesis != "Ks" && m_V0Hypothesis != "Lambda") {
      ATH_MSG_FATAL("Incorrect V0 container hypothesis - not recognized");
      return StatusCode::FAILURE;
    }

    if(m_jxDaug_num<2 || m_jxDaug_num>4 || m_disVDaug_num<2 || m_disVDaug_num>3) {
      ATH_MSG_FATAL("Incorrect number of JX or DisVtx daughters");
      return StatusCode::FAILURE;
    }
    
    // Get the beam conditon service for beamspot
    ATH_CHECK( m_beamCondSvc.retrieve() );

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

    // retrieving the track selector tool
    ATH_CHECK( m_trkSelector.retrieve() );

    // retrieving the V0 track selector tool
    ATH_CHECK( m_v0TrkSelector.retrieve() );

    // retrieving the Cascade tools
    ATH_CHECK( m_CascadeTools.retrieve() );

    IPartPropSvc* partPropSvc = nullptr;
    ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
    auto pdt = partPropSvc->PDT();

    // https://pkg.go.dev/go-hep.org/x/hep/heppdt#section-readme
    mass_e = BPhysPVCascadeTools::getParticleMass(pdt, PDG::e_minus);
    mass_mu = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
    mass_pion = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
    mass_proton = BPhysPVCascadeTools::getParticleMass(pdt, PDG::p_plus);
    mass_Lambda = BPhysPVCascadeTools::getParticleMass(pdt, PDG::Lambda0);
    mass_Ks = BPhysPVCascadeTools::getParticleMass(pdt, PDG::K_S0);
    mass_Xi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::Xi_minus);
    mass_Bpm = BPhysPVCascadeTools::getParticleMass(pdt, PDG::B_minus);

    // retrieve particle masses
    if(m_constrJpsi && m_massJpsi<0) m_massJpsi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_constrJX && m_massJX<0) m_massJX = BPhysPVCascadeTools::getParticleMass(pdt, PDG::psi_2S);
    if(m_constrV0 && m_massV0<0) m_massV0 = m_V0Hypothesis=="Ks" ? mass_Ks : mass_Lambda;
    if(m_disVDaug_num==3 && m_constrDisV && m_massDisV<0) m_massDisV = mass_Xi;
    if(m_constrMainV && m_massMainV<0) m_massMainV = mass_Bpm;

    if(m_jxDaug1MassHypo < 0.) m_jxDaug1MassHypo = mass_mu;
    if(m_jxDaug2MassHypo < 0.) m_jxDaug2MassHypo = mass_mu;
    if(m_jxDaug_num>=3 && m_jxDaug3MassHypo < 0.) m_jxDaug3MassHypo = mass_pion;
    if(m_jxDaug_num==4 && m_jxDaug4MassHypo < 0.) m_jxDaug4MassHypo = mass_pion;
    if(m_disVDaug_num==3 && m_disVDaug3MassHypo < 0.) m_disVDaug3MassHypo = mass_pion;

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlusDisplaced::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, xAOD::VertexContainer* V0OutputContainer, xAOD::VertexContainer* disVtxOutputContainer) const {
    ATH_MSG_DEBUG( "JpsiXPlusDisplaced::performSearch" );
    assert(cascadeinfoContainer!=nullptr);

    // Get TrackParticle container
    const xAOD::TrackParticleContainer* trackContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(trackContainer, m_TrkParticleCollection));

    // Get the PrimaryVertices container
    const xAOD::Vertex* primaryVertex(nullptr);
    const xAOD::VertexContainer *pvContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
    if (pvContainer->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
      return StatusCode::RECOVERABLE;
    }
    else primaryVertex = (*pvContainer)[0];

    std::vector<double> massesJX;
    massesJX.push_back(m_jxDaug1MassHypo);
    massesJX.push_back(m_jxDaug2MassHypo);
    if(m_jxDaug_num>=3) massesJX.push_back(m_jxDaug3MassHypo);
    if(m_jxDaug_num==4) massesJX.push_back(m_jxDaug4MassHypo);
    std::vector<double> massesJXExtra;
    for(size_t i=0; i<massesJX.size(); i++) massesJXExtra.push_back(massesJX[i]);
    if(m_extraTrkMassHypo>0) massesJXExtra.push_back(m_extraTrkMassHypo);
    std::vector<double> massesV0_ppi;
    massesV0_ppi.push_back(mass_proton);
    massesV0_ppi.push_back(mass_pion);
    std::vector<double> massesV0_pip;
    massesV0_pip.push_back(mass_pion);
    massesV0_pip.push_back(mass_proton);
    std::vector<double> massesV0_pipi;
    massesV0_pipi.push_back(mass_pion);
    massesV0_pipi.push_back(mass_pion);
    std::vector<const xAOD::TrackParticle*> tracksJX;
    std::vector<const xAOD::TrackParticle*> tracksJpsi;
    std::vector<const xAOD::TrackParticle*> tracksX;
    std::vector<const xAOD::TrackParticle*> tracksV0;
    std::vector<const xAOD::TrackParticle*> tracksDis3;
    std::vector<const xAOD::TrackParticle*> tracksJXExtra;

    // Get Jpsi+X container
    const xAOD::VertexContainer *jxContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jxContainer, m_vertexJXContainerKey));

    // Get V0 container
    const xAOD::VertexContainer *V0Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(V0Container, m_vertexV0ContainerKey));

    // Get disV container
    const xAOD::VertexContainer *disVtxInputContainer(nullptr);
    if(m_disVDaug_num==3 && m_vertexDisVContainerKey != "") {
      ATH_CHECK(evtStore()->retrieve(disVtxInputContainer, m_vertexDisVContainerKey));
    }

    // select the 3rd track displaced from the beam spot center
    std::vector<const xAOD::TrackParticle*> tracksDisplaced;
    if(m_disVDaug_num==3 && !disVtxInputContainer) {
      for(xAOD::TrackParticleContainer::const_iterator tpIt=trackContainer->begin(); tpIt!=trackContainer->end(); ++tpIt) {
	const xAOD::TrackParticle* TP = (*tpIt);
	// V0 track selection (https://gitlab.cern.ch/atlas/athena/-/blob/21.2/InnerDetector/InDetRecTools/InDetTrackSelectorTool/src/InDetConversionTrackSelectorTool.cxx#L379)
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

    // Accessors of V0 with photon conversion info and no-mass-constraint track momenta
    static SG::AuxElement::Accessor<std::string> mAcc_type("Type_V0Vtx");
    static SG::AuxElement::Accessor<int>    mAcc_gfit("gamma_fit");
    static SG::AuxElement::Accessor<float>  mAcc_gmass("gamma_mass");
    static SG::AuxElement::Accessor<float>  mAcc_gmasserr("gamma_massError");
    static SG::AuxElement::Accessor<float>  mAcc_gchisq("gamma_chisq");
    static SG::AuxElement::Accessor<int>    mAcc_gndof("gamma_ndof");
    static SG::AuxElement::Accessor<float>  mAcc_gprob("gamma_probability");
    static SG::AuxElement::Accessor< std::vector<float> > trk_pxAcc("TrackPx_V0nc");
    static SG::AuxElement::Accessor< std::vector<float> > trk_pyAcc("TrackPy_V0nc");
    static SG::AuxElement::Accessor< std::vector<float> > trk_pzAcc("TrackPz_V0nc");
    // Decorators of V0 vertices
    static SG::AuxElement::Decorator<std::string> mDec_type("Type_V0Vtx");    
    static SG::AuxElement::Decorator<int>   mDec_gfit("gamma_fit");
    static SG::AuxElement::Decorator<float> mDec_gmass("gamma_mass");
    static SG::AuxElement::Decorator<float> mDec_gmasserr("gamma_massError");
    static SG::AuxElement::Decorator<float> mDec_gchisq("gamma_chisq");
    static SG::AuxElement::Decorator<int>   mDec_gndof("gamma_ndof");
    static SG::AuxElement::Decorator<float> mDec_gprob("gamma_probability");
    static SG::AuxElement::Decorator< std::vector<float> > trk_pxDeco("TrackPx_V0nc");
    static SG::AuxElement::Decorator< std::vector<float> > trk_pyDeco("TrackPy_V0nc");
    static SG::AuxElement::Decorator< std::vector<float> > trk_pzDeco("TrackPz_V0nc");

    std::vector<float> trk_px;
    std::vector<float> trk_py;
    std::vector<float> trk_pz;

    // Select the V0 candidates before calling cascade fit
    std::vector<std::pair<xAOD::Vertex*,V0Enum> > selectedV0Candidates;
    for(auto vxcItr=V0Container->cbegin(); vxcItr!=V0Container->cend(); ++vxcItr) {
      xAOD::Vertex* vtx = *vxcItr;
      // Check the passed flags first
      bool passed = false;
      for(auto name : m_vertexV0HypoNames) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed = true;
	}
      }
      if(m_vertexV0HypoNames.size() && !passed) continue;

      V0Enum opt(UNKNOWN); double massV0(0);
      if(m_doV0Enum) {
	// determine V0 candidate track masses
	double massSig_V0_Lambda1 = std::abs(m_V0Tools->invariantMass(vtx, massesV0_ppi)-mass_Lambda)/m_V0Tools->invariantMassError(vtx, massesV0_ppi);
	double massSig_V0_Lambda2 = std::abs(m_V0Tools->invariantMass(vtx, massesV0_pip)-mass_Lambda)/m_V0Tools->invariantMassError(vtx, massesV0_pip);
	double massSig_V0_Ks = std::abs(m_V0Tools->invariantMass(vtx, massesV0_pipi)-mass_Ks)/m_V0Tools->invariantMassError(vtx, massesV0_pipi);
	if(massSig_V0_Lambda1<=massSig_V0_Lambda2 && massSig_V0_Lambda1<=massSig_V0_Ks) {
	  opt = LAMBDA_EXISTING;
	  massV0 = m_V0Tools->invariantMass(vtx, massesV0_ppi);
	}
	else if(massSig_V0_Lambda2<=massSig_V0_Lambda1 && massSig_V0_Lambda2<=massSig_V0_Ks) {
	  opt = LAMBDABAR_EXISTING;
	  massV0 = m_V0Tools->invariantMass(vtx, massesV0_pip);
	}
	else if(massSig_V0_Ks<=massSig_V0_Lambda1 && massSig_V0_Ks<=massSig_V0_Lambda2) {
	  opt = KS_EXISTING;
	  massV0 = m_V0Tools->invariantMass(vtx, massesV0_pipi);
	}

	if(massV0<m_V0MassLower || massV0>m_V0MassUpper) continue;
      }
      else {
	std::string type_V0Vtx;
	if(mAcc_type.isAvailable(*vtx)) type_V0Vtx = mAcc_type(*vtx);
	if(type_V0Vtx == "Lambda")         opt = LAMBDA_EXISTING;
	else if(type_V0Vtx == "Lambdabar") opt = LAMBDABAR_EXISTING;
	else if(type_V0Vtx == "Ks")        opt = KS_EXISTING;
	else                               opt = UNKNOWN;
      }

      if(opt==UNKNOWN) continue;
      if((opt==LAMBDA_EXISTING || opt==LAMBDABAR_EXISTING) && m_V0Hypothesis != "Lambda")  continue;
      if(opt==KS_EXISTING && m_V0Hypothesis != "Ks") continue;

      tracksV0.clear();
      for(size_t i=0; i<vtx->nTrackParticles(); i++) tracksV0.push_back(vtx->trackParticle(i));
      Amg::Vector3D vtxPos = m_V0Tools->vtx(vtx);

      int gamma_fit = 0; int gamma_ndof = 0;
      double gamma_chisq = 999999., gamma_prob = -1., gamma_mass = -1., gamma_massErr = -1.;
      if(mAcc_gfit.isAvailable(*vtx)) {
	gamma_fit     = mAcc_gfit.isAvailable(*vtx) ? mAcc_gfit(*vtx) : 0;
	gamma_mass    = mAcc_gmass.isAvailable(*vtx) ? mAcc_gmass(*vtx) : -1;
	gamma_massErr = mAcc_gmasserr.isAvailable(*vtx) ? mAcc_gmasserr(*vtx) : -1;
	gamma_chisq   = mAcc_gchisq.isAvailable(*vtx) ? mAcc_gchisq(*vtx) : 999999;
	gamma_ndof    = mAcc_gndof.isAvailable(*vtx) ? mAcc_gndof(*vtx) : 0;
	gamma_prob    = mAcc_gprob.isAvailable(*vtx) ? mAcc_gprob(*vtx) : -1;
      }
      else {
	std::unique_ptr<xAOD::Vertex> gammaVtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iGammaFitter->fit(tracksV0, vtxPos) ));
	if (gammaVtx) {
	  gamma_fit     = 1;
	  gamma_mass    = m_V0Tools->invariantMass(gammaVtx.get(),mass_e,mass_e);
	  gamma_massErr = m_V0Tools->invariantMassError(gammaVtx.get(),mass_e,mass_e);
	  gamma_chisq   = m_V0Tools->chisq(gammaVtx.get());
	  gamma_ndof    = m_V0Tools->ndof(gammaVtx.get());
	  gamma_prob    = m_V0Tools->vertexProbability(gammaVtx.get());
	}
      }
      if(gamma_fit==1 && gamma_mass<m_minMass_gamma && gamma_chisq/gamma_ndof<m_chi2cut_gamma) continue;

      // store track momenta at vertex before refit
      trk_px.clear(); trk_py.clear(); trk_pz.clear();
      for(size_t i=0; i<vtx->vxTrackAtVertex().size(); ++i) {
	const Trk::TrackParameters* aPerigee = vtx->vxTrackAtVertex()[i].perigeeAtVertex();
	if(aPerigee) {
	  trk_px.push_back( aPerigee->momentum()[Trk::px] );
	  trk_py.push_back( aPerigee->momentum()[Trk::py] );
	  trk_pz.push_back( aPerigee->momentum()[Trk::pz] );
	}
      }

      if(m_refitV0) {
	if(opt == LAMBDA_EXISTING)         opt = LAMBDA_CREATED;
	else if(opt == LAMBDABAR_EXISTING) opt = LAMBDABAR_CREATED;
	else if(opt == KS_EXISTING)        opt = KS_CREATED;
	std::unique_ptr<xAOD::Vertex> V0vtx;
	if(m_constrV0) {
	  std::vector<double> massesV0;
	  if(opt == LAMBDA_CREATED)         massesV0 = massesV0_ppi;
	  else if(opt == LAMBDABAR_CREATED) massesV0 = massesV0_pip;
	  else if(opt == KS_CREATED)        massesV0 = massesV0_pipi;
	  // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkV0Fitter/TrkV0Fitter/TrkV0VertexFitter.h
	  V0vtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, massesV0, m_massV0, 0, vtxPos) ));
	}
	else {
	  V0vtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, vtxPos) ));
	}
	if(V0vtx && V0vtx->chiSquared()>=0) {
	  double chi2DOF = V0vtx->chiSquared()/V0vtx->numberDoF();
	  if(m_chi2cut_V0>0 && chi2DOF>m_chi2cut_V0) continue;

	  xAOD::BPhysHelper V0_helper(V0vtx.get());
	  V0_helper.setRefTrks(); // AOD only method

	  V0vtx->clearTracks();
	  ElementLink<xAOD::TrackParticleContainer> newLink1;
	  newLink1.setElement(tracksV0[0]);
	  newLink1.setStorableObject(*trackContainer);
	  ElementLink<xAOD::TrackParticleContainer> newLink2;
	  newLink2.setElement(tracksV0[1]);
	  newLink2.setStorableObject(*trackContainer);
	  V0vtx->addTrackAtVertex(newLink1);
	  V0vtx->addTrackAtVertex(newLink2);

	  mDec_gfit(*V0vtx.get())     = gamma_fit;
	  mDec_gmass(*V0vtx.get())    = gamma_mass;
	  mDec_gmasserr(*V0vtx.get()) = gamma_massErr;
	  mDec_gchisq(*V0vtx.get())   = gamma_chisq;
	  mDec_gndof(*V0vtx.get())    = gamma_ndof;
	  mDec_gprob(*V0vtx.get())    = gamma_prob;
	  if(opt==LAMBDA_CREATED)         mDec_type(*V0vtx.get()) = "Lambda";
	  else if(opt==LAMBDABAR_CREATED) mDec_type(*V0vtx.get()) = "Lambdabar";
	  else if(opt==KS_CREATED)        mDec_type(*V0vtx.get()) = "Ks";
	  if(m_constrV0 && m_decorV0P) {
	    trk_pxDeco(*V0vtx.get()) = trk_px;
	    trk_pyDeco(*V0vtx.get()) = trk_py;
	    trk_pzDeco(*V0vtx.get()) = trk_pz;
	  }
	  selectedV0Candidates.push_back(std::pair<xAOD::Vertex*,V0Enum>{V0vtx.release(),opt});
	}
      } // refitV0
      else { // no refit of V0
	double chi2DOF = vtx->chiSquared()/vtx->numberDoF();
	if(m_chi2cut_V0>0 && chi2DOF>m_chi2cut_V0) continue;
	mDec_gfit(*vtx)     = gamma_fit;
	mDec_gmass(*vtx)    = gamma_mass;
	mDec_gmasserr(*vtx) = gamma_massErr;
	mDec_gchisq(*vtx)   = gamma_chisq;
	mDec_gndof(*vtx)    = gamma_ndof;
	mDec_gprob(*vtx)    = gamma_prob;
	if(opt==LAMBDA_EXISTING)         mDec_type(*vtx) = "Lambda";
	else if(opt==LAMBDABAR_EXISTING) mDec_type(*vtx) = "Lambdabar";
	else if(opt==KS_EXISTING)        mDec_type(*vtx) = "Ks";
	selectedV0Candidates.push_back(std::pair<xAOD::Vertex*,V0Enum>{vtx,opt});
      }
    }
    if(selectedV0Candidates.size()==0) return StatusCode::SUCCESS;

    std::sort( selectedV0Candidates.begin(), selectedV0Candidates.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
    if(m_maxV0Candidates>0 && selectedV0Candidates.size()>m_maxV0Candidates) {
      for(auto it=selectedV0Candidates.begin()+m_maxV0Candidates; it!=selectedV0Candidates.end(); it++) if(it->second>=LAMBDA_CREATED) delete it->first;
      selectedV0Candidates.erase(selectedV0Candidates.begin()+m_maxV0Candidates, selectedV0Candidates.end());
    }

    if(V0OutputContainer) {
      for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) V0OutputContainer->push_back(v0VItr->first);
    }

    // Make the displaced candidates if needed
    static SG::AuxElement::Accessor<float> ChiSquared("ChiSquared_Cascade");
    static SG::AuxElement::Accessor<int>   nDoF("nDoF_Cascade");
    std::vector<std::pair<xAOD::Vertex*,size_t> > disVtxContainer;
    if(m_disVDaug_num==3) {
      if(!disVtxInputContainer) {
	for(size_t it=0; it<selectedV0Candidates.size(); ++it) {
	  std::pair<xAOD::Vertex*,V0Enum> elem = selectedV0Candidates[it];
	  std::vector<double> massesV0;
	  if(elem.second==LAMBDA_EXISTING || elem.second==LAMBDA_CREATED)            massesV0 = massesV0_ppi;
	  else if(elem.second==LAMBDABAR_EXISTING || elem.second==LAMBDABAR_CREATED) massesV0 = massesV0_pip;
	  else if(elem.second==KS_EXISTING || elem.second==KS_CREATED)               massesV0 = massesV0_pipi;

	  xAOD::BPhysHelper V0_helper(elem.first);
	  TLorentzVector p4_v0, tmp;
	  tracksV0.clear();
	  for(size_t i=0; i<elem.first->nTrackParticles(); i++) {
	    p4_v0 += V0_helper.refTrk(i,massesV0[i]);
	    tracksV0.push_back(elem.first->trackParticle(i));
	  }

	  for(auto trkItr=tracksDisplaced.cbegin(); trkItr!=tracksDisplaced.cend(); ++trkItr) {
	    // Check identical tracks in input
	    if(std::find(tracksV0.cbegin(), tracksV0.cend(), *trkItr) != tracksV0.cend()) continue;

	    tmp.SetPtEtaPhiM((*trkItr)->pt(),(*trkItr)->eta(),(*trkItr)->phi(),m_disVDaug3MassHypo);
	    // A rough mass window cut, as V0 and the track are not from a common vertex
	    if ((p4_v0+tmp).M() < m_DisplacedMassLower || (p4_v0+tmp).M() > m_DisplacedMassUpper) continue;

	    m_iVertexFitter->setDefault();
	    int robustness = 0;
	    m_iVertexFitter->setRobustness(robustness);
	    std::vector<Trk::VertexID> vrtList;
	    // V0 vertex
	    Trk::VertexID vID;
	    if(m_constrV0) vID = m_iVertexFitter->startVertex(tracksV0,massesV0,m_massV0);
	    else           vID = m_iVertexFitter->startVertex(tracksV0,massesV0);
	    vrtList.push_back(vID);
	    // Mother vertex
	    tracksDis3.clear(); tracksDis3.push_back(*trkItr);
	    std::vector<double> massesDis3{m_disVDaug3MassHypo};
	    if (m_constrDisV) m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList,m_massDisV);
	    else              m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList);
	    // Do the work
	    std::unique_ptr<Trk::VxCascadeInfo> cascade_info = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));
	    if(cascade_info != nullptr) {
	      BPhysPVCascadeTools::PrepareVertexLinks(cascade_info.get(), trackContainer);
	      cascade_info->getSVOwnership(false);
	      const std::vector<xAOD::Vertex*>& disVertices = cascade_info->vertices();
	      double chi2NDF = cascade_info->fitChi2()/cascade_info->nDoF();
	      if(m_chi2cut_DisV<=0 || chi2NDF < m_chi2cut_DisV) {
		xAOD::BPhysHelper disV_helper(disVertices[1]);
		// Get refitted track momenta from all vertices, charged tracks only
		BPhysPVCascadeTools::SetVectorInfo(disV_helper, cascade_info.get());

		static SG::AuxElement::Decorator<float> chi2_decor("ChiSquared_Cascade");
		static SG::AuxElement::Decorator<int> ndof_decor("nDoF_Cascade");
		chi2_decor(*disVertices[1]) = cascade_info->fitChi2();
		ndof_decor(*disVertices[1]) = cascade_info->nDoF();

		disVtxContainer.push_back(std::pair<xAOD::Vertex*,size_t>{disVertices[1],it});
	      }
	      else delete disVertices[1];
	      delete disVertices[0]; // always delete the newly fitted V0 vertices
	    }
	  } // tracksDisplaced
	} // selectedV0Candidates
      } // disVtxInputContainer==0
      else { // disVtxInputContainer!=0
	for(auto vxcItr=disVtxInputContainer->cbegin(); vxcItr!=disVtxInputContainer->cend(); ++vxcItr) {
	  xAOD::Vertex* vtx = *vxcItr;
	  static SG::AuxElement::Accessor<VertexLink> cascadeVertexLinkAcc("CascadeVertexLink");
	  if(cascadeVertexLinkAcc.isAvailable(*vtx)) {
	    const VertexLink& cascadeVertexLink = cascadeVertexLinkAcc(*vtx);
	    if(cascadeVertexLink.isValid()) {
	      xAOD::Vertex* V0Vtx = const_cast<xAOD::Vertex*>(*cascadeVertexLink);
	      for(size_t it=0; it<selectedV0Candidates.size(); ++it) {
		std::pair<xAOD::Vertex*,V0Enum> elem = selectedV0Candidates[it];
		if(BPhysPVCascadeTools::VerticesMatchTracks<2>(elem.first,V0Vtx)) {
		  disVtxContainer.push_back(std::pair<xAOD::Vertex*,size_t>{vtx,it});
		  break;
		}
	      }
	    }
	  }
	}
	if(disVtxInputContainer->size() != disVtxContainer.size()) ATH_MSG_ERROR("Input and selected displaced vertex containers have different sizes");
      }
      if(disVtxContainer.size()==0) {
	if(!V0OutputContainer) {
	  for(auto it=selectedV0Candidates.begin(); it!=selectedV0Candidates.end(); it++) if(it->second>=LAMBDA_CREATED) delete it->first;
	}
	return StatusCode::SUCCESS;
      }

      std::sort( disVtxContainer.begin(), disVtxContainer.end(), [](std::pair<xAOD::Vertex*,size_t> a, std::pair<xAOD::Vertex*,size_t> b) {
	float chi2_a = ChiSquared.isAvailable(*a.first) ? ChiSquared(*a.first) : 999999;
	int ndof_a = nDoF.isAvailable(*a.first) ? nDoF(*a.first) : 1;
	float chi2_b = ChiSquared.isAvailable(*b.first) ? ChiSquared(*b.first) : 999999;
	int ndof_b = nDoF.isAvailable(*b.first) ? nDoF(*b.first) : 1;
	return chi2_a/ndof_a < chi2_b/ndof_b; } );
      if(m_maxDisVCandidates>0 && disVtxContainer.size()>m_maxDisVCandidates) {
	if(!disVtxInputContainer) {
	  for(auto it=disVtxContainer.begin()+m_maxDisVCandidates; it!=disVtxContainer.end(); it++) delete it->first;
	}
	disVtxContainer.erase(disVtxContainer.begin()+m_maxDisVCandidates, disVtxContainer.end());
      }

      if(disVtxOutputContainer) {
	for(auto disVItr=disVtxContainer.cbegin(); disVItr!=disVtxContainer.cend(); ++disVItr) {
	  disVtxOutputContainer->push_back(disVItr->first);
	  static SG::AuxElement::Decorator<VertexLink> cascadeVertexLinkDecor("CascadeVertexLink");
	  VertexLink vertexLink;
          if(V0OutputContainer) {
	    vertexLink.setElement(V0OutputContainer->at(disVItr->second));
	    vertexLink.setStorableObject(*V0OutputContainer);
	  }
	  else {
	    xAOD::Vertex* V0Vtx = FindVertex<2>(V0Container, selectedV0Candidates[disVItr->second].first);
	    vertexLink.setElement(V0Vtx);
	    vertexLink.setStorableObject(*V0Container);
	  }
	  cascadeVertexLinkDecor(*disVItr->first) = vertexLink;
	}
      }
    }

    // Select the JX candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedJXCandidates;
    for(auto vxcItr=jxContainer->cbegin(); vxcItr!=jxContainer->cend(); ++vxcItr) {
      // Check the passed flag first
      xAOD::Vertex* vtx = *vxcItr;
      bool passed = false;
      for(auto name : m_vertexJXHypoNames) {
	SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
	if(flagAcc.isAvailable(*vtx) && flagAcc(*vtx)) {
	  passed = true;
	}
      }
      if(m_vertexJXHypoNames.size() && !passed) continue;
      
      // Check Psi candidate invariant mass and skip if need be
      if(m_jxDaug_num>2) {
	double mass_jx = m_V0Tools->invariantMass(*vxcItr,massesJX);
	if(mass_jx < m_jxMassLower || mass_jx > m_jxMassUpper) continue;
      }

      // Add loose cut on Jpsi mass from e.g. JX -> Jpsi pi+ pi-
      TLorentzVector p4_mu1, p4_mu2;
      p4_mu1.SetPtEtaPhiM( vtx->trackParticle(0)->pt(),
			   vtx->trackParticle(0)->eta(),
			   vtx->trackParticle(0)->phi(), m_jxDaug1MassHypo);
      p4_mu2.SetPtEtaPhiM( vtx->trackParticle(1)->pt(),
			   vtx->trackParticle(1)->eta(),
			   vtx->trackParticle(1)->phi(), m_jxDaug2MassHypo);
      double mass_jpsi = (p4_mu1 + p4_mu2).M();
      if (mass_jpsi < m_jpsiMassLower || mass_jpsi > m_jpsiMassUpper) continue;

      if(m_jxDaug_num==4 && m_diTrackMassLower>=0 && m_diTrackMassUpper>m_diTrackMassLower) {
	TLorentzVector p4_trk1, p4_trk2;
	p4_trk1.SetPtEtaPhiM( vtx->trackParticle(2)->pt(),
			      vtx->trackParticle(2)->eta(),
			      vtx->trackParticle(2)->phi(), m_jxDaug3MassHypo);
	p4_trk2.SetPtEtaPhiM( vtx->trackParticle(3)->pt(),
			      vtx->trackParticle(3)->eta(),
			      vtx->trackParticle(3)->phi(), m_jxDaug4MassHypo);
	double mass_diTrk = (p4_trk1 + p4_trk2).M();
	if (mass_diTrk < m_diTrackMassLower || mass_diTrk > m_diTrackMassUpper) continue;
      }

      double chi2DOF = vtx->chiSquared()/vtx->numberDoF();
      if(m_chi2cut_JX>0 && chi2DOF>m_chi2cut_JX) continue;

      selectedJXCandidates.push_back(vtx);
    }
    if(selectedJXCandidates.size()==0) {
      if(!V0OutputContainer) {
	for(auto it=selectedV0Candidates.begin(); it!=selectedV0Candidates.end(); it++) if(it->second>=LAMBDA_CREATED) delete it->first;
      }
      if(!disVtxOutputContainer && !disVtxInputContainer) {
	for(auto it=disVtxContainer.begin(); it!=disVtxContainer.end(); it++) delete it->first;
      }
      return StatusCode::SUCCESS;
    }

    std::sort( selectedJXCandidates.begin(), selectedJXCandidates.end(), [](const xAOD::Vertex* a, const xAOD::Vertex* b) { return a->chiSquared()/a->numberDoF() < b->chiSquared()/b->numberDoF(); } );
    if(m_maxJXCandidates>0 && selectedJXCandidates.size()>m_maxJXCandidates) {
      selectedJXCandidates.erase(selectedJXCandidates.begin()+m_maxJXCandidates, selectedJXCandidates.end());
    }

    // Select JX+DisV candidates
    // Iterate over JX vertices
    for(auto jxItr=selectedJXCandidates.cbegin(); jxItr!=selectedJXCandidates.cend(); ++jxItr) {
      tracksJX.clear();
      for(size_t i=0; i<(*jxItr)->nTrackParticles(); i++) tracksJX.push_back((*jxItr)->trackParticle(i));
      if (tracksJX.size() != massesJX.size()) {
	ATH_MSG_ERROR("Problems with JX input: number of tracks or track mass inputs is not correct!");
      }
      tracksJpsi.clear();
      tracksJpsi.push_back((*jxItr)->trackParticle(0));
      tracksJpsi.push_back((*jxItr)->trackParticle(1));
      tracksX.clear();
      if(m_jxDaug_num>=3) tracksX.push_back((*jxItr)->trackParticle(2));
      if(m_jxDaug_num==4) tracksX.push_back((*jxItr)->trackParticle(3));

      static SG::AuxElement::Decorator<float>       chi2_V1_decor("ChiSquared_V1");
      static SG::AuxElement::Decorator<int>         ndof_V1_decor("nDoF_V1");
      static SG::AuxElement::Decorator<std::string> type_V1_decor("Type_V1");

      // Iterate over displaced vertices
      if(m_disVDaug_num==2) {
	for(auto V0Itr=selectedV0Candidates.cbegin(); V0Itr!=selectedV0Candidates.cend(); ++V0Itr) {
	  // Check identical tracks in input
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr->first->trackParticle(0)) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr->first->trackParticle(1)) != tracksJX.cend()) continue;
	  tracksV0.clear();
	  for(size_t j=0; j<V0Itr->first->nTrackParticles(); j++) tracksV0.push_back(V0Itr->first->trackParticle(j));
	  std::vector<double> massesV0;
	  if(V0Itr->second==LAMBDA_EXISTING || V0Itr->second==LAMBDA_CREATED)            massesV0 = massesV0_ppi;
	  else if(V0Itr->second==LAMBDABAR_EXISTING || V0Itr->second==LAMBDABAR_CREATED) massesV0 = massesV0_pip;
	  else if(V0Itr->second==KS_EXISTING || V0Itr->second==KS_CREATED)               massesV0 = massesV0_pipi;

	  TLorentzVector p4_moth;
	  xAOD::BPhysHelper JX_helper(*jxItr);
	  for(size_t it=0; it<(*jxItr)->nTrackParticles(); it++) {
	    p4_moth += JX_helper.refTrk(it,massesJX[it]);
	  }
	  xAOD::BPhysHelper V0_helper(V0Itr->first);
	  for(size_t it=0; it<V0Itr->first->nTrackParticles(); it++) {
	    p4_moth += V0_helper.refTrk(it,massesV0[it]);
	  }

	  if(m_extraTrkMassHypo<=0) {
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
	    // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	    // V0 vertex
	    Trk::VertexID vID1;
	    if (m_constrV0) {
	      vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,m_massV0);
	    } else {
	      vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0);
	    }
	    vrtList.push_back(vID1);
	    // JX vertex
	    Trk::VertexID vID2;
	    if (m_constrJX && m_jxDaug_num>2) {
	      vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,m_massJX);
	    } else {
	      vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX);
	    }
	    vrtList.push_back(vID2);
	    // Mother vertex including JX and V0
	    std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	    std::vector<double> tp_masses; tp_masses.clear();
	    if(m_constrMainV) {
	      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,m_massMainV);
	    } else {
	      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
	    }
	    if (m_constrJpsi) {
	      std::vector<Trk::VertexID> cnstV; cnstV.clear();
	      if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJpsi,cnstV,m_massJpsi).isSuccess() ) {
		ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	      }
	    }
	    if (m_constrX && m_jxDaug_num==4 && m_massX>0) {
	      std::vector<Trk::VertexID> cnstV; cnstV.clear();
	      if ( !m_iVertexFitter->addMassConstraint(vID2,tracksX,cnstV,m_massX).isSuccess() ) {
		ATH_MSG_WARNING("addMassConstraint for X failed");
	      }
	    }
	    // Do the work
	    std::unique_ptr<Trk::VxCascadeInfo> result = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));

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

	      const std::vector<std::vector<TLorentzVector> > &moms = result->getParticleMoms();
	      const std::vector<xAOD::Vertex*> &cascadeVertices = result->vertices();
	      double lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[2]);
	      if(chi2CutPassed && lxy_SV1>m_lxyV0_cut) {
		chi2_V1_decor(*cascadeVertices[0]) = V0Itr->first->chiSquared();
		ndof_V1_decor(*cascadeVertices[0]) = V0Itr->first->numberDoF();
		if(V0Itr->second==LAMBDA_EXISTING || V0Itr->second==LAMBDA_CREATED)            type_V1_decor(*cascadeVertices[0]) = "Lambda";
		else if(V0Itr->second==LAMBDABAR_EXISTING || V0Itr->second==LAMBDABAR_CREATED) type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
		else if(V0Itr->second==KS_EXISTING || V0Itr->second==KS_CREATED)               type_V1_decor(*cascadeVertices[0]) = "Ks";
		mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0Itr->first) ? mAcc_gfit(*V0Itr->first) : 0;
		mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0Itr->first) ? mAcc_gmass(*V0Itr->first) : -1;
		mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0Itr->first) ? mAcc_gmasserr(*V0Itr->first) : -1;
		mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0Itr->first) ? mAcc_gchisq(*V0Itr->first) : 999999;
		mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0Itr->first) ? mAcc_gndof(*V0Itr->first) : 0;
		mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0Itr->first) ? mAcc_gprob(*V0Itr->first) : -1;
		trk_px.clear(); trk_py.clear(); trk_pz.clear();
		if(trk_pxAcc.isAvailable(*V0Itr->first)) {
		  trk_px = trk_pxAcc(*V0Itr->first);
		  trk_py = trk_pyAcc(*V0Itr->first);
		  trk_pz = trk_pzAcc(*V0Itr->first);
		}
		trk_pxDeco(*cascadeVertices[0]) = trk_px;
		trk_pyDeco(*cascadeVertices[0]) = trk_py;
		trk_pzDeco(*cascadeVertices[0]) = trk_pz;
		  
		cascadeinfoContainer->push_back(result.release());
	      }
	    }
	  } // m_extraTrkMassHypo<=0
	  else { // m_extraTrkMassHypo>0
	    for(xAOD::TrackParticleContainer::const_iterator tpIter=trackContainer->begin(); tpIter!=trackContainer->end(); ++tpIter) {
	      const xAOD::TrackParticle* tpExtra = (*tpIter);
	      if ( tpExtra->pt()<m_extraTrkMinPt ) continue;
	      if ( !m_trkSelector->decision(*tpExtra, NULL) ) continue;
	      // Check identical tracks in input
	      if(std::find(tracksJX.cbegin(),tracksJX.cend(),tpExtra) != tracksJX.cend()) continue;
	      if(std::find(tracksV0.cbegin(),tracksV0.cend(),tpExtra) != tracksV0.cend()) continue;

	      TLorentzVector tmp;
	      tmp.SetPtEtaPhiM(tpExtra->pt(),tpExtra->eta(),tpExtra->phi(),m_extraTrkMassHypo);
	      if ((p4_moth+tmp).M() < m_MassLower || (p4_moth+tmp).M() > m_MassUpper) continue;

	      tracksJXExtra.clear();
	      for(size_t it=0; it<tracksJX.size(); it++) tracksJXExtra.push_back(tracksJX[it]);
	      tracksJXExtra.push_back(tpExtra);

	      // Apply the user's settings to the fitter
	      // Reset
	      m_iVertexFitter->setDefault();
	      // Robustness: http://cdsweb.cern.ch/record/685551
	      int robustness = 0;
	      m_iVertexFitter->setRobustness(robustness);
	      // Build up the topology
	      // Vertex list
	      std::vector<Trk::VertexID> vrtList;
	      // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	      // V0 vertex
	      Trk::VertexID vID1;
	      if (m_constrV0) {
		vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,m_massV0);
	      } else {
		vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0);
	      }
	      vrtList.push_back(vID1);
	      // JXExtra vertex
	      Trk::VertexID vID2 = m_iVertexFitter->nextVertex(tracksJXExtra,massesJXExtra);     
	      vrtList.push_back(vID2);
	      // Mother vertex including JX, V0 and the extra track
	      std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	      std::vector<double> tp_masses; tp_masses.clear();
	      if(m_constrMainV) {
		m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,m_massMainV);
	      } else {
		m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
	      }
	      if (m_constrJX && m_jxDaug_num>2) {
		std::vector<Trk::VertexID> cnstV; cnstV.clear();
		if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJX,cnstV,m_massJX).isSuccess() ) {
		  ATH_MSG_WARNING("addMassConstraint for JX failed");
		}
	      }
	      if (m_constrJpsi) {
		std::vector<Trk::VertexID> cnstV; cnstV.clear();
		if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJpsi,cnstV,m_massJpsi).isSuccess() ) {
		  ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
		}
	      }
	      if (m_constrX && m_jxDaug_num==4 && m_massX>0) {
		std::vector<Trk::VertexID> cnstV; cnstV.clear();
		if ( !m_iVertexFitter->addMassConstraint(vID2,tracksX,cnstV,m_massX).isSuccess() ) {
		  ATH_MSG_WARNING("addMassConstraint for X failed");
		}
	      }
	      // Do the work
	      std::unique_ptr<Trk::VxCascadeInfo> result = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));

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
		const std::vector<std::vector<TLorentzVector> > &moms = result->getParticleMoms();
		const std::vector<xAOD::Vertex*> &cascadeVertices = result->vertices();
		double lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[2]);
		if(chi2CutPassed && lxy_SV1>m_lxyV0_cut) {
		  chi2_V1_decor(*cascadeVertices[0]) = V0Itr->first->chiSquared();
		  ndof_V1_decor(*cascadeVertices[0]) = V0Itr->first->numberDoF();
		  if(V0Itr->second==LAMBDA_EXISTING || V0Itr->second==LAMBDA_CREATED)            type_V1_decor(*cascadeVertices[0]) = "Lambda";
		  else if(V0Itr->second==LAMBDABAR_EXISTING || V0Itr->second==LAMBDABAR_CREATED) type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
		  else if(V0Itr->second==KS_EXISTING || V0Itr->second==KS_CREATED)               type_V1_decor(*cascadeVertices[0]) = "Ks";
		  mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0Itr->first) ? mAcc_gfit(*V0Itr->first) : 0;
		  mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0Itr->first) ? mAcc_gmass(*V0Itr->first) : -1;
		  mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0Itr->first) ? mAcc_gmasserr(*V0Itr->first) : -1;
		  mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0Itr->first) ? mAcc_gchisq(*V0Itr->first) : 999999;
		  mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0Itr->first) ? mAcc_gndof(*V0Itr->first) : 0;
		  mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0Itr->first) ? mAcc_gprob(*V0Itr->first) : -1;
		  trk_px.clear(); trk_py.clear(); trk_pz.clear();
		  if(trk_pxAcc.isAvailable(*V0Itr->first)) {
		    trk_px = trk_pxAcc(*V0Itr->first);
		    trk_py = trk_pyAcc(*V0Itr->first);
		    trk_pz = trk_pzAcc(*V0Itr->first);
		  }
		  trk_pxDeco(*cascadeVertices[0]) = trk_px;
		  trk_pyDeco(*cascadeVertices[0]) = trk_py;
		  trk_pzDeco(*cascadeVertices[0]) = trk_pz;

		  cascadeinfoContainer->push_back(result.release());
		}
	      }
	    } // loop over trackContainer
	  } // m_extraTrkMassHypo>0
	} // Iterate over V0Itr
      } // m_disVDaug_num==2
      else if(m_disVDaug_num==3) {
	for(auto disVItr=disVtxContainer.cbegin(); disVItr!=disVtxContainer.cend(); ++disVItr) {
	  xAOD::Vertex* disVtx = disVItr->first;
	  xAOD::Vertex* v0Vtx = selectedV0Candidates[disVItr->second].first;
	  V0Enum opt = selectedV0Candidates[disVItr->second].second;

	  tracksV0.clear();
	  for(size_t i=0; i<v0Vtx->nTrackParticles(); i++) tracksV0.push_back(v0Vtx->trackParticle(i));
	  if(tracksV0.size()!=2) ATH_MSG_ERROR("Number of retrieved V0 tracks is not 2");

	  tracksDis3.clear();
	  for(size_t i=0; i<disVtx->nTrackParticles(); i++) tracksDis3.push_back(disVtx->trackParticle(i));
	  if(tracksDis3.size()!=1) ATH_MSG_ERROR("Number of tracksDis3 tracks is not 1");

	  // Check identical tracks in input
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), tracksV0[0]) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), tracksV0[1]) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), tracksDis3[0]) != tracksJX.cend()) continue;

	  std::vector<double> massesV0;
	  if(opt==LAMBDA_EXISTING || opt==LAMBDA_CREATED)            massesV0 = massesV0_ppi;
	  else if(opt==LAMBDABAR_EXISTING || opt==LAMBDABAR_CREATED) massesV0 = massesV0_pip;
	  else if(opt==KS_EXISTING || opt==KS_CREATED)               massesV0 = massesV0_pipi;

	  std::vector<double> massesDis3 = {m_disVDaug3MassHypo};

	  std::vector<double> massesDisV = massesV0;
	  massesDisV.push_back(massesDis3[0]);

	  TLorentzVector p4_moth;
	  xAOD::BPhysHelper JX_helper(*jxItr);
	  for(size_t it=0; it<(*jxItr)->nTrackParticles(); it++) {
	    p4_moth += JX_helper.refTrk(it,massesJX[it]);
	  }
	  xAOD::BPhysHelper disV_helper(disVtx);
	  for(size_t it=0; it<disVtx->nTrackParticles(); it++) {
	    p4_moth += disV_helper.refTrk(it,massesDisV[it]);
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
	  std::vector<Trk::VertexID> vrtList2;
	  // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	  // V0 vertex
	  Trk::VertexID vID1;
	  if (m_constrV0) {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,m_massV0);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0);
	  }
	  vrtList.push_back(vID1);
	  // Displaced vertex
	  Trk::VertexID vID2;
	  if (m_constrDisV) {
	    vID2 = m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList,m_massDisV);
	  } else {
	    vID2 = m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList);
	  }
	  vrtList2.push_back(vID2);
	  // JX vertex
	  Trk::VertexID vID3;
	  if (m_constrJX && m_jxDaug_num>2) {
	    vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,m_massJX);
	  } else {
	    vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX);
	  }
	  vrtList2.push_back(vID3);
	  // Mother vertex including JX and DisV
	  std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	  std::vector<double> tp_masses; tp_masses.clear();
	  if(m_constrMainV) {
	    m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2,m_massMainV);
	  } else {
	    m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2);
	  }
	  if (m_constrJpsi) {
	    std::vector<Trk::VertexID> cnstV; cnstV.clear();
	    if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJpsi,cnstV,m_massJpsi).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	    }
	  }
	  if (m_constrX && m_jxDaug_num==4 && m_massX>0) {
	    std::vector<Trk::VertexID> cnstV; cnstV.clear();
	    if ( !m_iVertexFitter->addMassConstraint(vID3,tracksX,cnstV,m_massX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for X failed");
	    }
	  }
	  // Do the work
	  std::unique_ptr<Trk::VxCascadeInfo> result = std::move(std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade() ));

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

	    const std::vector<std::vector<TLorentzVector> > &moms = result->getParticleMoms();
	    const std::vector<xAOD::Vertex*> &cascadeVertices = result->vertices();
	    double lxy_SV1_sub = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	    double lxy_SV1 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[3]);

	    if(chi2CutPassed && lxy_SV1>m_lxyDisV_cut && lxy_SV1_sub>m_lxyV0_cut) {
              chi2_V1_decor(*cascadeVertices[1]) = ChiSquared.isAvailable(*disVtx) ? ChiSquared(*disVtx) : 999999;
              ndof_V1_decor(*cascadeVertices[1]) = nDoF.isAvailable(*disVtx) ? nDoF(*disVtx) : 1;

              static SG::AuxElement::Decorator<float>       chi2_V0_decor("ChiSquared_V0");
              static SG::AuxElement::Decorator<int>         ndof_V0_decor("nDoF_V0");
	      static SG::AuxElement::Decorator<std::string> type_V0_decor("Type_V0");
              chi2_V0_decor(*cascadeVertices[0]) = v0Vtx->chiSquared();
	      ndof_V0_decor(*cascadeVertices[0]) = v0Vtx->numberDoF();
	      if(opt==LAMBDA_EXISTING || opt==LAMBDA_CREATED)            type_V0_decor(*cascadeVertices[0]) = "Lambda";
	      else if(opt==LAMBDABAR_EXISTING || opt==LAMBDABAR_CREATED) type_V0_decor(*cascadeVertices[0]) = "Lambdabar";
	      else if(opt==KS_EXISTING || opt==KS_CREATED)               type_V0_decor(*cascadeVertices[0]) = "Ks";
	      mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*v0Vtx) ? mAcc_gfit(*v0Vtx) : 0;
	      mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*v0Vtx) ? mAcc_gmass(*v0Vtx) : -1;
	      mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*v0Vtx) ? mAcc_gmasserr(*v0Vtx) : -1;
	      mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*v0Vtx) ? mAcc_gchisq(*v0Vtx) : 999999;
	      mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*v0Vtx) ? mAcc_gndof(*v0Vtx) : 0;
	      mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*v0Vtx) ? mAcc_gprob(*v0Vtx) : -1;
	      trk_px.clear(); trk_py.clear(); trk_pz.clear();
	      if(trk_pxAcc.isAvailable(*v0Vtx)) {
		trk_px = trk_pxAcc(*v0Vtx);
		trk_py = trk_pyAcc(*v0Vtx);
		trk_pz = trk_pzAcc(*v0Vtx);
	      }
	      trk_pxDeco(*cascadeVertices[0]) = trk_px;
	      trk_pyDeco(*cascadeVertices[0]) = trk_py;
	      trk_pzDeco(*cascadeVertices[0]) = trk_pz;

	      cascadeinfoContainer->push_back(result.release());
	    }
	  }
	} // Iterate over disVItr
      } // m_disVDaug_num==3
    } // Iterate over JX vertices

    // clean up transient objects
    if(!disVtxOutputContainer && !disVtxInputContainer) {
      for(auto disVItr=disVtxContainer.cbegin(); disVItr!=disVtxContainer.cend(); ++disVItr) delete disVItr->first;
    }
    if(!V0OutputContainer) {
      for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlusDisplaced::addBranches() const {
    std::array<std::unique_ptr<xAOD::VertexContainer>, 4> VtxWriteHandles;
    std::array<std::unique_ptr<xAOD::VertexAuxContainer>, 4> VtxWriteHandlesAux;
    size_t topoN = (m_disVDaug_num==2 ? 3 : 4);
    if(m_cascadeOutputsKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of output cascade vertices");
      return StatusCode::FAILURE;
    }

    for(size_t i=0; i<topoN; i++){
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

    // output V0 vertices
    std::unique_ptr<xAOD::VertexContainer> V0OutputContainer;
    std::unique_ptr<xAOD::VertexAuxContainer> V0OutputAuxContainer;
    if(m_v0VtxOutputsKey != "") {
      V0OutputContainer = std::make_unique<xAOD::VertexContainer>();
      V0OutputAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
      V0OutputContainer->setStore(V0OutputAuxContainer.get());
    }

    // output displaced vertices
    std::unique_ptr<xAOD::VertexContainer> disVtxOutputContainer;
    std::unique_ptr<xAOD::VertexAuxContainer> disVtxOutputAuxContainer;
    if(m_disVDaug_num==3 && m_disVtxOutputsKey != "") {
      disVtxOutputContainer = std::make_unique<xAOD::VertexContainer>();
      disVtxOutputAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
      disVtxOutputContainer->setStore(disVtxOutputAuxContainer.get());
    }

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
    ATH_CHECK(performSearch(&cascadeinfoContainer,V0OutputContainer.get(),disVtxOutputContainer.get()));

    std::sort( cascadeinfoContainer.begin(), cascadeinfoContainer.end(), [](Trk::VxCascadeInfo* a, Trk::VxCascadeInfo* b) { return a->fitChi2()/a->nDoF() < b->fitChi2()/b->nDoF(); } );
    if(m_maxMainVCandidates>0 && cascadeinfoContainer.size()>m_maxMainVCandidates) {
      for(auto it=cascadeinfoContainer.begin()+m_maxMainVCandidates; it!=cascadeinfoContainer.end(); it++) delete *it;
      cascadeinfoContainer.erase(cascadeinfoContainer.begin()+m_maxMainVCandidates, cascadeinfoContainer.end());
    }

    BPhysPVCascadeTools helper(&(*m_CascadeTools), &m_beamCondSvc);
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    // Decorators for the main vertex: chi2, ndf, pt and pt error, plus the V0 vertex variables
    static SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
    static SG::AuxElement::Decorator<VertexLinkVector> JXLinksDecor("JXVertexLinks");
    static SG::AuxElement::Decorator<VertexLinkVector> V0LinksDecor("V0VertexLinks");
    static SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
    static SG::AuxElement::Decorator<int> ndof_decor("nDoF");
    static SG::AuxElement::Decorator<float> Pt_decor("Pt");
    static SG::AuxElement::Decorator<float> PtErr_decor("PtErr");

    static SG::AuxElement::Decorator<float> lxy_SV0_decor("lxy_SV0");
    static SG::AuxElement::Decorator<float> lxyErr_SV0_decor("lxyErr_SV0");
    static SG::AuxElement::Decorator<float> a0xy_SV0_decor("a0xy_SV0");
    static SG::AuxElement::Decorator<float> a0xyErr_SV0_decor("a0xyErr_SV0");
    static SG::AuxElement::Decorator<float> a0z_SV0_decor("a0z_SV0");
    static SG::AuxElement::Decorator<float> a0zErr_SV0_decor("a0zErr_SV0");

    static SG::AuxElement::Decorator<float> lxy_SV1_decor("lxy_SV1");
    static SG::AuxElement::Decorator<float> lxyErr_SV1_decor("lxyErr_SV1");
    static SG::AuxElement::Decorator<float> a0xy_SV1_decor("a0xy_SV1");
    static SG::AuxElement::Decorator<float> a0xyErr_SV1_decor("a0xyErr_SV1");
    static SG::AuxElement::Decorator<float> a0z_SV1_decor("a0z_SV1");
    static SG::AuxElement::Decorator<float> a0zErr_SV1_decor("a0zErr_SV1");

    static SG::AuxElement::Decorator<float> lxy_SV2_decor("lxy_SV2");
    static SG::AuxElement::Decorator<float> lxyErr_SV2_decor("lxyErr_SV2");
    static SG::AuxElement::Decorator<float> a0xy_SV2_decor("a0xy_SV2");
    static SG::AuxElement::Decorator<float> a0xyErr_SV2_decor("a0xyErr_SV2");
    static SG::AuxElement::Decorator<float> a0z_SV2_decor("a0z_SV2");
    static SG::AuxElement::Decorator<float> a0zErr_SV2_decor("a0zErr_SV2");

    static SG::AuxElement::Decorator<float> chi2_V2_decor("ChiSquared_V2");
    static SG::AuxElement::Decorator<int> ndof_V2_decor("nDoF_V2");

    // Get the input containers
    const xAOD::VertexContainer *jxContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jxContainer, m_vertexJXContainerKey));
    const xAOD::VertexContainer *V0Container(nullptr);
    ATH_CHECK(evtStore()->retrieve(V0Container, m_vertexV0ContainerKey));

    for(auto cascade_info : cascadeinfoContainer) {
      if(cascade_info==nullptr) ATH_MSG_ERROR("CascadeInfo is null");

      const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info->vertices();
      if(cascadeVertices.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices");
      for(size_t i=0; i<topoN; i++) {
	if(cascadeVertices[i]==nullptr) ATH_MSG_ERROR("Error null vertex");
      }

      cascade_info->getSVOwnership(false); // Prevent Container from deleting vertices
      const auto mainVertex = cascadeVertices[topoN-1]; // this is the mother vertex
      const std::vector< std::vector<TLorentzVector> > &moms = cascade_info->getParticleMoms();

      // Identify the input V0
      xAOD::Vertex* v0Vtx = FindVertex<2>(V0Container, cascadeVertices[0]);
      // Identify the input JX
      int ijx = topoN==3 ? 1 : 2;
      xAOD::Vertex* jxVtx(nullptr);
      if(m_jxDaug_num==4) jxVtx = FindVertex<4>(jxContainer, cascadeVertices[ijx]);
      else if(m_jxDaug_num==3) jxVtx = FindVertex<3>(jxContainer, cascadeVertices[ijx]);
      else jxVtx = FindVertex<2>(jxContainer, cascadeVertices[ijx]);

      // transfer hypotheses to output vertices
      for(auto name : m_vertexV0HypoNames) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
        if(flagAcc.isAvailable(*v0Vtx) && flagAcc(*v0Vtx)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
	  flagDec(*cascadeVertices[0]) = true;
        }
      }

      for(auto name : m_vertexJXHypoNames) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
        if(flagAcc.isAvailable(*jxVtx) && flagAcc(*jxVtx)) {
          SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
	  flagDec(*cascadeVertices[ijx]) = true;
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
      BPHYS_CHECK( vtx.setMass(m_CascadeTools->invariantMass(moms[topoN-1])) );
      BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[topoN-1],cascade_info->getCovariance()[topoN-1])) );
      // pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
      Pt_decor(*mainVertex)       = m_CascadeTools->pT(moms[topoN-1]);
      PtErr_decor(*mainVertex)    = m_CascadeTools->pTError(moms[topoN-1],cascade_info->getCovariance()[topoN-1]);
      // chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
      chi2_decor(*mainVertex)     = cascade_info->fitChi2();
      ndof_decor(*mainVertex)     = cascade_info->nDoF();

      if(m_disVDaug_num==2) {
	// decorate the newly fitted V0 vertex
	lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
	lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
	a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
	a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
	a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
	a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      }
      else {
	// decorate the newly fitted V0 vertex
	lxy_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	lxyErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	a0z_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],cascadeVertices[1]);
	a0zErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	a0xy_SV0_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	a0xyErr_SV0_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);

	// decorate the newly fitted disV vertex
	lxy_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	lxyErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
	a0xy_SV1_decor(*cascadeVertices[1])    = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	a0xyErr_SV1_decor(*cascadeVertices[1]) = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
	a0z_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	a0zErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      }

      // decorate the newly fitted JX vertex
      lxy_SV2_decor(*cascadeVertices[ijx])     = m_CascadeTools->lxy(moms[ijx],cascadeVertices[ijx],mainVertex);
      lxyErr_SV2_decor(*cascadeVertices[ijx])  = m_CascadeTools->lxyError(moms[ijx],cascade_info->getCovariance()[ijx],cascadeVertices[ijx],mainVertex);
      a0z_SV2_decor(*cascadeVertices[ijx])     = m_CascadeTools->a0z(moms[ijx],cascadeVertices[ijx],mainVertex);
      a0zErr_SV2_decor(*cascadeVertices[ijx])  = m_CascadeTools->a0zError(moms[ijx],cascade_info->getCovariance()[ijx],cascadeVertices[ijx],mainVertex);
      a0xy_SV2_decor(*cascadeVertices[ijx])    = m_CascadeTools->a0xy(moms[ijx],cascadeVertices[ijx],mainVertex);
      a0xyErr_SV2_decor(*cascadeVertices[ijx]) = m_CascadeTools->a0xyError(moms[ijx],cascade_info->getCovariance()[ijx],cascadeVertices[ijx],mainVertex);

      chi2_V2_decor(*cascadeVertices[ijx])     = m_V0Tools->chisq(jxVtx);
      ndof_V2_decor(*cascadeVertices[ijx])     = m_V0Tools->ndof(jxVtx);
      
      double Mass_Moth = m_CascadeTools->invariantMass(moms[topoN-1]);
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer, refPvContainer, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info, topoN-1, Mass_Moth, vtx));

      for(size_t i=0; i<topoN; i++) {
        VtxWriteHandles[i]->push_back(cascadeVertices[i]);
      }

      // Set links to cascade vertices
      VertexLinkVector precedingVertexLinks;
      VertexLink vertexLink1;
      vertexLink1.setElement(cascadeVertices[0]);
      vertexLink1.setStorableObject(*VtxWriteHandles[0].get());
      if( vertexLink1.isValid() ) precedingVertexLinks.push_back( vertexLink1 );
      VertexLink vertexLink2;
      vertexLink2.setElement(cascadeVertices[1]);
      vertexLink2.setStorableObject(*VtxWriteHandles[1].get());
      if( vertexLink2.isValid() ) precedingVertexLinks.push_back( vertexLink2 );
      if(topoN==4) {
	VertexLink vertexLink3;
	vertexLink3.setElement(cascadeVertices[2]);
	vertexLink3.setStorableObject(*VtxWriteHandles[2].get());
	if( vertexLink3.isValid() ) precedingVertexLinks.push_back( vertexLink3 );
      }
      CascadeLinksDecor(*mainVertex) = precedingVertexLinks;
    } // loop over cascadeinfoContainer

    for(size_t i=0; i<topoN; i++){
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandles[i])   , m_cascadeOutputsKeys[i]));
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandlesAux[i]), m_cascadeOutputsKeys[i] + "Aux."));
    }
    if(m_v0VtxOutputsKey != "") {
      ATH_CHECK(evtStore()->record(std::move(V0OutputContainer)   , m_v0VtxOutputsKey));
      ATH_CHECK(evtStore()->record(std::move(V0OutputAuxContainer), m_v0VtxOutputsKey + "Aux."));
    }
    if(m_disVDaug_num==3 && m_disVtxOutputsKey != "") {
      ATH_CHECK(evtStore()->record(std::move(disVtxOutputContainer)   , m_disVtxOutputsKey));
      ATH_CHECK(evtStore()->record(std::move(disVtxOutputAuxContainer), m_disVtxOutputsKey + "Aux."));
    }

    // Deleting cascadeinfo since this won't be stored.
    // Vertices have been kept in m_cascadeOutputs and should be owned by their container
    for (auto cascade_info : cascadeinfoContainer) delete cascade_info;

    return StatusCode::SUCCESS;
  }

  bool JpsiXPlusDisplaced::d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const {
    bool pass = false;
    if(PV) {
      const Trk::Perigee* per = m_trackToVertexTool->perigeeAtVertex(*track, PV->position());
      if(per == 0) return pass;
      double d0 = per->parameters()[Trk::d0];
      double sig_d0 = sqrt((*per->covariance())(0,0));
      if(std::abs(d0/sig_d0) > m_d0_cut) pass = true; 
      delete per;
    }
    else {
      Amg::Vector3D beamspot = Amg::Vector3D(m_beamCondSvc->beamVtx().position());
      const Trk::Perigee* per = m_trackToVertexTool->perigeeAtVertex(*track, beamspot);
      if(per == 0) return pass;
      double d0 = per->parameters()[Trk::d0];
      double sig_d0 = sqrt((*per->covariance())(0,0));
      if(std::abs(d0/sig_d0) > m_d0_cut) pass = true; 
      delete per;
    }
    return pass;
  }

  template<size_t NTracks>
  xAOD::Vertex* JpsiXPlusDisplaced::FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const {
    for (xAOD::Vertex* v1 : *cont) {
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
