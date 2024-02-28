/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// JpsiXPlus2V0.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/JpsiXPlus2V0.h"
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

  JpsiXPlus2V0::JpsiXPlus2V0(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexJXContainerKey("InputJXVertices"),
    m_vertexV0ContainerKeys{"InputV0Vertices"},
    m_cascadeOutputsKeys{ "JpsiXPlus2V0_SubVtx1", "JpsiXPlus2V0_SubVtx2", "JpsiXPlus2V0_SubVtx3", "JpsiXPlus2V0_MainVtx" },
    m_refitV0(false),
    m_constrV0(true),
    m_v0VtxOutputsKeys{},
    m_TrkParticleCollection("InDetTrackParticles"),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_jxMassLower(0.0),
    m_jxMassUpper(30000.0),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(20000.0),
    m_diTrackMassLower(-1.0),
    m_diTrackMassUpper(-1.0),
    m_V01Hypothesis("Ks"),
    m_V01MassLower(0.0),
    m_V01MassUpper(20000.0),
    m_lxyV01_cut(-999.0),
    m_V02Hypothesis("Lambda"),
    m_V02MassLower(0.0),
    m_V02MassUpper(20000.0),
    m_lxyV02_cut(-999.0),
    m_doV0Enum(false),
    m_decorV0P(false),
    m_minMass_gamma(-1.0),
    m_chi2cut_gamma(-1.0),
    m_MassLower(0.0),
    m_MassUpper(41000.0),
    m_jxDaug_num(4),
    m_jxDaug1MassHypo(-1),
    m_jxDaug2MassHypo(-1),
    m_jxDaug3MassHypo(-1),
    m_jxDaug4MassHypo(-1),
    m_massJX(-1),
    m_massJpsi(-1),
    m_massX(-1),
    m_massJXV02(-1),
    m_massMainV(-1),
    m_constrJX(false),
    m_constrJpsi(false),
    m_constrX(false),
    m_constrV01(false),
    m_constrV02(false),
    m_constrJXV02(false),
    m_constrMainV(false),
    m_chi2cut_JX(-1.0),
    m_chi2cut_V0(-1.0),
    m_chi2cut(-1.0),
    m_maxJXCandidates(0),
    m_maxV0Candidates(0),
    m_maxMainVCandidates(0),
    m_beamCondSvc("BeamCondSvc",name),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_iV0Fitter("Trk::V0VertexFitter"),
    m_iGammaFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_CascadeTools("DerivationFramework::CascadeTools")
  {
    declareProperty("JXVertices",               m_vertexJXContainerKey);
    declareProperty("V0Containers",             m_vertexV0ContainerKeys);
    declareProperty("JXVtxHypoNames",           m_vertexJXHypoNames);
    declareProperty("V0VtxHypoNames",           m_vertexV0HypoNames);
    declareProperty("CascadeVertexCollections", m_cascadeOutputsKeys); // size is 3 or 4 only
    declareProperty("RefitV0",                  m_refitV0);
    declareProperty("ApplyV0MassConstraint",    m_constrV0); // only effective when m_refitV0 = true
    declareProperty("OutoutV0VtxCollections",   m_v0VtxOutputsKeys);
    declareProperty("TrackParticleCollection",  m_TrkParticleCollection);
    declareProperty("VxPrimaryCandidateName",   m_VxPrimaryCandidateName);
    declareProperty("RefPVContainerName",       m_refPVContainerName = "RefittedPrimaryVertices");
    declareProperty("JXMassLowerCut",           m_jxMassLower); // only effective when m_jxDaug_num>2
    declareProperty("JXMassUpperCut",           m_jxMassUpper); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMassLowerCut",         m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",         m_jpsiMassUpper);
    declareProperty("DiTrackMassLower",         m_diTrackMassLower); // only effective when m_jxDaug_num=4
    declareProperty("DiTrackMassUpper",         m_diTrackMassUpper); // only effective when m_jxDaug_num=4
    declareProperty("V01Hypothesis",            m_V01Hypothesis); // "Ks" or "Lambda"
    declareProperty("V01MassLowerCut",          m_V01MassLower);
    declareProperty("V01MassUpperCut",          m_V01MassUpper);
    declareProperty("LxyV01Cut",                m_lxyV01_cut);
    declareProperty("V02Hypothesis",            m_V02Hypothesis); // "Ks" or "Lambda"
    declareProperty("V02MassLowerCut",          m_V02MassLower);
    declareProperty("V02MassUpperCut",          m_V02MassUpper);
    declareProperty("LxyV02Cut",                m_lxyV02_cut);
    declareProperty("DoV0Enumeration",          m_doV0Enum);
    declareProperty("DecorateV0Momentum",       m_decorV0P); // only effective when m_refitV0=true and m_constrV0=true
    declareProperty("MassCutGamma",             m_minMass_gamma);
    declareProperty("Chi2CutGamma",             m_chi2cut_gamma);
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
    declareProperty("JXV02VtxMass",             m_massJXV02);
    declareProperty("MainVtxMass",              m_massMainV);
    declareProperty("ApplyJXMassConstraint",    m_constrJX);
    declareProperty("ApplyJpsiMassConstraint",  m_constrJpsi); // only effective when m_jxDaug_num>2
    declareProperty("ApplyXMassConstraint",     m_constrX); // only effective when m_jxDaug_num=4
    declareProperty("ApplyV01MassConstraint",   m_constrV01);
    declareProperty("ApplyV02MassConstraint",   m_constrV02);
    declareProperty("ApplyJXV02MassConstraint", m_constrJXV02);
    declareProperty("ApplyMainVMassConstraint", m_constrMainV);
    declareProperty("Chi2CutJX",                m_chi2cut_JX);
    declareProperty("Chi2CutV0",                m_chi2cut_V0);
    declareProperty("Chi2Cut",                  m_chi2cut);
    declareProperty("MaxJXCandidates",          m_maxJXCandidates);
    declareProperty("MaxV0Candidates",          m_maxV0Candidates);
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
    declareProperty("CascadeTools",             m_CascadeTools);
  }

  StatusCode JpsiXPlus2V0::initialize() {
    if((m_V01Hypothesis != "Ks" && m_V01Hypothesis != "Lambda") || (m_V02Hypothesis != "Ks" && m_V02Hypothesis != "Lambda")) {
      ATH_MSG_FATAL("Incorrect V0 container hypothesis - not recognized");
      return StatusCode::FAILURE;
    }

    if(m_jxDaug_num<2 || m_jxDaug_num>4) {
      ATH_MSG_FATAL("Incorrect number of JX daughters");
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
    mass_Lambda_b = BPhysPVCascadeTools::getParticleMass(pdt, PDG::Lambda_b0);
    mass_Ks = BPhysPVCascadeTools::getParticleMass(pdt, PDG::K_S0);
    mass_Bpm = BPhysPVCascadeTools::getParticleMass(pdt, PDG::B_minus);

    // retrieve particle masses
    if(m_constrJpsi && m_massJpsi<0) m_massJpsi = BPhysPVCascadeTools::getParticleMass(pdt, PDG::J_psi);
    if(m_constrJX && m_massJX<0) m_massJX = BPhysPVCascadeTools::getParticleMass(pdt, PDG::psi_2S);
    if(m_constrJXV02 && m_massJXV02<0) m_massJXV02 = mass_Lambda_b;
    if(m_constrMainV && m_massMainV<0) m_massMainV = mass_Bpm;

    if(m_jxDaug1MassHypo < 0.) m_jxDaug1MassHypo = mass_mu;
    if(m_jxDaug2MassHypo < 0.) m_jxDaug2MassHypo = mass_mu;
    if(m_jxDaug_num>=3 && m_jxDaug3MassHypo < 0.) m_jxDaug3MassHypo = mass_pion;
    if(m_jxDaug_num==4 && m_jxDaug4MassHypo < 0.) m_jxDaug4MassHypo = mass_pion;

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlus2V0::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<xAOD::VertexContainer*> V0OutputContainers) const {
    ATH_MSG_DEBUG( "JpsiXPlus2V0::performSearch" );
    assert(cascadeinfoContainer!=nullptr);

    if( (m_V01Hypothesis == m_V02Hypothesis && (V0OutputContainers.size() != 0 && V0OutputContainers.size() != 1)) ||
	(m_V01Hypothesis != m_V02Hypothesis && (V0OutputContainers.size() != 0 && V0OutputContainers.size() != 2)) ) {
      ATH_MSG_ERROR("V0OutputContainers size is not correct!");
    }

    // Get TrackParticle container
    const xAOD::TrackParticleContainer* trackContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(trackContainer, m_TrkParticleCollection));

    // Get the PrimaryVertices container
    const xAOD::VertexContainer *pvContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
    if (pvContainer->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
      return StatusCode::RECOVERABLE;
    }

    std::vector<double> massesJX;
    massesJX.push_back(m_jxDaug1MassHypo);
    massesJX.push_back(m_jxDaug2MassHypo);
    if(m_jxDaug_num>=3) massesJX.push_back(m_jxDaug3MassHypo);
    if(m_jxDaug_num==4) massesJX.push_back(m_jxDaug4MassHypo);
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
    std::vector<const xAOD::TrackParticle*> tracksV01;
    std::vector<const xAOD::TrackParticle*> tracksV02;

    // Get Jpsi+X container
    const xAOD::VertexContainer *jxContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jxContainer, m_vertexJXContainerKey));

    // Get V0 container
    std::vector<const xAOD::VertexContainer*> V0Containers;
    for(size_t i=0; i<m_vertexV0ContainerKeys.size(); i++) {
      const xAOD::VertexContainer *V0Container(nullptr);
      ATH_CHECK(evtStore()->retrieve(V0Container, m_vertexV0ContainerKeys[i]));
      V0Containers.push_back(V0Container);
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
    for(size_t ic=0; ic<V0Containers.size(); ic++) {
      const xAOD::VertexContainer* V0Container = V0Containers[ic];
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

	  if(opt==LAMBDA_EXISTING || opt==LAMBDABAR_EXISTING) {
	    if(m_V01Hypothesis == "Lambda" && m_V02Hypothesis == "Lambda") {
	      if((massV0<m_V01MassLower || massV0>m_V01MassUpper) && (massV0<m_V02MassLower || massV0>m_V02MassUpper)) continue;
	    }
	    else if(m_V01Hypothesis == "Lambda") {
	      if(massV0<m_V01MassLower || massV0>m_V01MassUpper) continue;
	    }
	    else if(m_V02Hypothesis == "Lambda") {
	      if(massV0<m_V02MassLower || massV0>m_V02MassUpper) continue;
	    }
	    else continue;
	  }
	  else if(opt==KS_EXISTING) {
	    if(m_V01Hypothesis == "Ks" && m_V02Hypothesis == "Ks") {
	      if((massV0<m_V01MassLower || massV0>m_V01MassUpper) && (massV0<m_V02MassLower || massV0>m_V02MassUpper)) continue;
	    }
	    else if(m_V01Hypothesis == "Ks") {
	      if(massV0<m_V01MassLower || massV0>m_V01MassUpper) continue;
	    }
	    else if(m_V02Hypothesis == "Ks") {
	      if(massV0<m_V02MassLower || massV0>m_V02MassUpper) continue;
	    }
	    else continue;
	  }
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
	    V0vtx = std::move(std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, massesV0, opt==KS_EXISTING || opt==KS_CREATED ? mass_Ks : mass_Lambda, 0, vtxPos) ));
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
	else { // no V0 refit
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
	} // no V0 refit
      } // V0 candidate
    } // V0Container
    if(selectedV0Candidates.size()==0) return StatusCode::SUCCESS;

    std::vector<std::pair<xAOD::Vertex*,V0Enum> > selectedV01Candidates;
    std::vector<std::pair<xAOD::Vertex*,V0Enum> > selectedV02Candidates;
    if(m_V01Hypothesis != m_V02Hypothesis) {
      for(size_t j=0; j<selectedV0Candidates.size(); j++) {
	std::pair<xAOD::Vertex*,V0Enum> candidate = selectedV0Candidates[j];
	if(candidate.second==LAMBDA_EXISTING || candidate.second==LAMBDABAR_EXISTING || candidate.second==LAMBDA_CREATED || candidate.second==LAMBDABAR_CREATED) {
	  if(m_V01Hypothesis == "Lambda") selectedV01Candidates.push_back(candidate);
	  else if(m_V02Hypothesis == "Lambda") selectedV02Candidates.push_back(candidate);
	}
	else if(candidate.second==KS_EXISTING || candidate.second==KS_CREATED) {
	  if(m_V01Hypothesis == "Ks") selectedV01Candidates.push_back(candidate);
	  else if(m_V02Hypothesis == "Ks") selectedV02Candidates.push_back(candidate);
	}
      }
      std::sort( selectedV01Candidates.begin(), selectedV01Candidates.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
      if(m_maxV0Candidates>0 && selectedV01Candidates.size()>m_maxV0Candidates) {
	for(auto it=selectedV01Candidates.begin()+m_maxV0Candidates; it!=selectedV01Candidates.end(); it++) if(it->second>=LAMBDA_CREATED) delete it->first;
	selectedV01Candidates.erase(selectedV01Candidates.begin()+m_maxV0Candidates, selectedV01Candidates.end());
      }
      std::sort( selectedV02Candidates.begin(), selectedV02Candidates.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
      if(m_maxV0Candidates>0 && selectedV02Candidates.size()>m_maxV0Candidates) {
	for(auto it=selectedV02Candidates.begin()+m_maxV0Candidates; it!=selectedV02Candidates.end(); it++) if(it->second>=LAMBDA_CREATED) delete it->first;
	selectedV02Candidates.erase(selectedV02Candidates.begin()+m_maxV0Candidates, selectedV02Candidates.end());
      }

      if(V0OutputContainers.size()==2) {
	for(auto v0VItr=selectedV01Candidates.cbegin(); v0VItr!=selectedV01Candidates.cend(); ++v0VItr) V0OutputContainers[0]->push_back(v0VItr->first);
	for(auto v0VItr=selectedV02Candidates.cbegin(); v0VItr!=selectedV02Candidates.cend(); ++v0VItr) V0OutputContainers[1]->push_back(v0VItr->first);
      }
    }
    else { // m_V01Hypothesis == m_V02Hypothesis
      std::sort( selectedV0Candidates.begin(), selectedV0Candidates.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
      if(m_maxV0Candidates>0 && selectedV0Candidates.size()>m_maxV0Candidates) {
	for(auto it=selectedV0Candidates.begin()+m_maxV0Candidates; it!=selectedV0Candidates.end(); it++) if(it->second>=LAMBDA_CREATED) delete it->first;
	selectedV0Candidates.erase(selectedV0Candidates.begin()+m_maxV0Candidates, selectedV0Candidates.end());
      }

      if(V0OutputContainers.size()==1) {
	for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) V0OutputContainers[0]->push_back(v0VItr->first);
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
      if(V0OutputContainers.size()==0) {
	if(m_V01Hypothesis != m_V02Hypothesis) {
	  for(auto v0VItr=selectedV01Candidates.cbegin(); v0VItr!=selectedV01Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
	  for(auto v0VItr=selectedV02Candidates.cbegin(); v0VItr!=selectedV02Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
	} else {
	  for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
	}
      }
      return StatusCode::SUCCESS;
    }

    std::sort( selectedJXCandidates.begin(), selectedJXCandidates.end(), [](const xAOD::Vertex* a, const xAOD::Vertex* b) { return a->chiSquared()/a->numberDoF() < b->chiSquared()/b->numberDoF(); } );
    if(m_maxJXCandidates>0 && selectedJXCandidates.size()>m_maxJXCandidates) {
      selectedJXCandidates.erase(selectedJXCandidates.begin()+m_maxJXCandidates, selectedJXCandidates.end());
    }

    // Select JX+V0+V0 candidates
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
      static SG::AuxElement::Decorator<float>       chi2_V2_decor("ChiSquared_V2");
      static SG::AuxElement::Decorator<int>         ndof_V2_decor("nDoF_V2");
      static SG::AuxElement::Decorator<std::string> type_V2_decor("Type_V2");

      // Iterate over V0 vertices
      if(m_V01Hypothesis == m_V02Hypothesis) {
	for(auto V0Itr1=selectedV0Candidates.cbegin(); V0Itr1!=selectedV0Candidates.cend(); ++V0Itr1) {
	  if(m_V01Hypothesis == "Lambda") {
	    if(V0Itr1->second != LAMBDA_EXISTING && V0Itr1->second != LAMBDABAR_EXISTING && V0Itr1->second != LAMBDA_CREATED && V0Itr1->second != LAMBDABAR_CREATED) continue;
	  }
	  else if(m_V01Hypothesis == "Ks") {
	    if(V0Itr1->second != KS_EXISTING && V0Itr1->second != KS_CREATED) continue;
	  }
	  // Check identical tracks in input
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr1->first->trackParticle(0)) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr1->first->trackParticle(1)) != tracksJX.cend()) continue;
	  tracksV01.clear();
	  for(size_t j=0; j<V0Itr1->first->nTrackParticles(); j++) tracksV01.push_back(V0Itr1->first->trackParticle(j));
	  std::vector<double> massesV01;
	  if(V0Itr1->second==LAMBDA_EXISTING || V0Itr1->second==LAMBDA_CREATED)            massesV01 = massesV0_ppi;
	  else if(V0Itr1->second==LAMBDABAR_EXISTING || V0Itr1->second==LAMBDABAR_CREATED) massesV01 = massesV0_pip;
	  else if(V0Itr1->second==KS_EXISTING || V0Itr1->second==KS_CREATED)               massesV01 = massesV0_pipi;
	  TLorentzVector p4_jxv01;
	  xAOD::BPhysHelper JX_helper(*jxItr);
	  for(size_t it=0; it<(*jxItr)->nTrackParticles(); it++) {
	    p4_jxv01 += JX_helper.refTrk(it,massesJX[it]);
	  }
	  xAOD::BPhysHelper V01_helper(V0Itr1->first);
	  for(size_t it=0; it<V0Itr1->first->nTrackParticles(); it++) {
	    p4_jxv01 += V01_helper.refTrk(it,massesV01[it]);
	  }

	  for(auto V0Itr2=V0Itr1+1; V0Itr2!=selectedV0Candidates.cend(); ++V0Itr2) {
	    if(m_V02Hypothesis == "Lambda") {
	      if(V0Itr2->second != LAMBDA_EXISTING && V0Itr2->second != LAMBDABAR_EXISTING && V0Itr2->second != LAMBDA_CREATED && V0Itr2->second != LAMBDABAR_CREATED) continue;
	    }
	    else if(m_V02Hypothesis == "Ks") {
	      if(V0Itr2->second != KS_EXISTING && V0Itr2->second != KS_CREATED) continue;
	    }
	    // Check identical tracks in input
	    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr2->first->trackParticle(0)) != tracksJX.cend()) continue;
	    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr2->first->trackParticle(1)) != tracksJX.cend()) continue;
	    if(std::find(tracksV01.cbegin(), tracksV01.cend(), V0Itr2->first->trackParticle(0)) != tracksV01.cend()) continue;
	    if(std::find(tracksV01.cbegin(), tracksV01.cend(), V0Itr2->first->trackParticle(1)) != tracksV01.cend()) continue;
	    tracksV02.clear();
	    for(size_t k=0; k<V0Itr2->first->nTrackParticles(); k++) tracksV02.push_back(V0Itr2->first->trackParticle(k));
	    std::vector<double> massesV02;
	    if(V0Itr2->second==LAMBDA_EXISTING || V0Itr2->second==LAMBDA_CREATED)            massesV02 = massesV0_ppi;
	    else if(V0Itr2->second==LAMBDABAR_EXISTING || V0Itr2->second==LAMBDABAR_CREATED) massesV02 = massesV0_pip;
	    else if(V0Itr2->second==KS_EXISTING || V0Itr2->second==KS_CREATED)               massesV02 = massesV0_pipi;
	    TLorentzVector p4_moth = p4_jxv01;
	    xAOD::BPhysHelper V02_helper(V0Itr2->first);
	    for(size_t it=0; it<V0Itr2->first->nTrackParticles(); it++) {
	      p4_moth += V02_helper.refTrk(it,massesV02[it]);
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
	    // https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	    // V01 vertex
	    Trk::VertexID vID1;
	    if (m_constrV01) {
	      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01,m_V01Hypothesis=="Ks" ? mass_Ks : mass_Lambda);
	    } else {
	      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01);
	    }
	    vrtList.push_back(vID1);
	    // V02 vertex
	    Trk::VertexID vID2;
	    if (m_constrV02) {
	      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02,m_V02Hypothesis=="Ks" ? mass_Ks : mass_Lambda);
	    } else {
	      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02);
	    }
	    vrtList.push_back(vID2);
	    // JX vertex
	    Trk::VertexID vID3;
	    if (m_constrJX && m_jxDaug_num>2) {
	      vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,m_massJX);
	    } else {
	      vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX);
	    }
	    vrtList.push_back(vID3);
	    // Mother vertex including JX and two V0's
	    std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	    std::vector<double> tp_masses; tp_masses.clear();
	    if(m_constrMainV) {
	      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,m_massMainV);
	    } else {
	      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
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
	      double lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[3]);
	      double lxy_SV2 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[3]);
	      if(chi2CutPassed && lxy_SV1>m_lxyV01_cut && lxy_SV2>m_lxyV02_cut) {
		chi2_V1_decor(*cascadeVertices[0]) = V0Itr1->first->chiSquared();
		ndof_V1_decor(*cascadeVertices[0]) = V0Itr1->first->numberDoF();
		if(V0Itr1->second==LAMBDA_EXISTING || V0Itr1->second==LAMBDA_CREATED)            type_V1_decor(*cascadeVertices[0]) = "Lambda";
		else if(V0Itr1->second==LAMBDABAR_EXISTING || V0Itr1->second==LAMBDABAR_CREATED) type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
		else if(V0Itr1->second==KS_EXISTING || V0Itr1->second==KS_CREATED)               type_V1_decor(*cascadeVertices[0]) = "Ks";
		mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0Itr1->first) ? mAcc_gfit(*V0Itr1->first) : 0;
		mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0Itr1->first) ? mAcc_gmass(*V0Itr1->first) : -1;
		mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0Itr1->first) ? mAcc_gmasserr(*V0Itr1->first) : -1;
		mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0Itr1->first) ? mAcc_gchisq(*V0Itr1->first) : 999999;
		mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0Itr1->first) ? mAcc_gndof(*V0Itr1->first) : 0;
		mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0Itr1->first) ? mAcc_gprob(*V0Itr1->first) : -1;
		trk_px.clear(); trk_py.clear(); trk_pz.clear();
		if(trk_pxAcc.isAvailable(*V0Itr1->first)) {
		  trk_px = trk_pxAcc(*V0Itr1->first);
		  trk_py = trk_pyAcc(*V0Itr1->first);
		  trk_pz = trk_pzAcc(*V0Itr1->first);
		}
		trk_pxDeco(*cascadeVertices[0]) = trk_px;
		trk_pyDeco(*cascadeVertices[0]) = trk_py;
		trk_pzDeco(*cascadeVertices[0]) = trk_pz;

		chi2_V2_decor(*cascadeVertices[1]) = V0Itr2->first->chiSquared();
		ndof_V2_decor(*cascadeVertices[1]) = V0Itr2->first->numberDoF();
		if(V0Itr2->second==LAMBDA_EXISTING || V0Itr2->second==LAMBDA_CREATED)            type_V2_decor(*cascadeVertices[1]) = "Lambda";
		else if(V0Itr2->second==LAMBDABAR_EXISTING || V0Itr2->second==LAMBDABAR_CREATED) type_V2_decor(*cascadeVertices[1]) = "Lambdabar";
		else if(V0Itr2->second==KS_EXISTING || V0Itr2->second==KS_CREATED)               type_V2_decor(*cascadeVertices[1]) = "Ks";
		mDec_gfit(*cascadeVertices[1])     = mAcc_gfit.isAvailable(*V0Itr2->first) ? mAcc_gfit(*V0Itr2->first) : 0;
		mDec_gmass(*cascadeVertices[1])    = mAcc_gmass.isAvailable(*V0Itr2->first) ? mAcc_gmass(*V0Itr2->first) : -1;
		mDec_gmasserr(*cascadeVertices[1]) = mAcc_gmasserr.isAvailable(*V0Itr2->first) ? mAcc_gmasserr(*V0Itr2->first) : -1;
		mDec_gchisq(*cascadeVertices[1])   = mAcc_gchisq.isAvailable(*V0Itr2->first) ? mAcc_gchisq(*V0Itr2->first) : 999999;
		mDec_gndof(*cascadeVertices[1])    = mAcc_gndof.isAvailable(*V0Itr2->first) ? mAcc_gndof(*V0Itr2->first) : 0;
		mDec_gprob(*cascadeVertices[1])    = mAcc_gprob.isAvailable(*V0Itr2->first) ? mAcc_gprob(*V0Itr2->first) : -1;
		trk_px.clear(); trk_py.clear(); trk_pz.clear();
		if(trk_pxAcc.isAvailable(*V0Itr2->first)) {
		  trk_px = trk_pxAcc(*V0Itr2->first);
		  trk_py = trk_pyAcc(*V0Itr2->first);
		  trk_pz = trk_pzAcc(*V0Itr2->first);
		}
		trk_pxDeco(*cascadeVertices[1]) = trk_px;
		trk_pyDeco(*cascadeVertices[1]) = trk_py;
		trk_pzDeco(*cascadeVertices[1]) = trk_pz;

		cascadeinfoContainer->push_back(result.release());
	      }
	    }
	  } // V0Itr2
	} // V0Itr1
      } // m_V01Hypothesis == m_V02Hypothesis
      else { // m_V01Hypothesis != m_V02Hypothesis
	for(auto V0Itr1=selectedV01Candidates.cbegin(); V0Itr1!=selectedV01Candidates.cend(); ++V0Itr1) {
	  // Check identical tracks in input
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr1->first->trackParticle(0)) != tracksJX.cend()) continue;
	  if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr1->first->trackParticle(1)) != tracksJX.cend()) continue;
	  tracksV01.clear();
	  for(size_t j=0; j<V0Itr1->first->nTrackParticles(); j++) tracksV01.push_back(V0Itr1->first->trackParticle(j));
	  std::vector<double> massesV01;
	  if(V0Itr1->second==LAMBDA_EXISTING || V0Itr1->second==LAMBDA_CREATED)            massesV01 = massesV0_ppi;
	  else if(V0Itr1->second==LAMBDABAR_EXISTING || V0Itr1->second==LAMBDABAR_CREATED) massesV01 = massesV0_pip;
	  else if(V0Itr1->second==KS_EXISTING || V0Itr1->second==KS_CREATED)               massesV01 = massesV0_pipi;
	  TLorentzVector p4_jxv01;
	  xAOD::BPhysHelper JX_helper(*jxItr);
	  for(size_t it=0; it<(*jxItr)->nTrackParticles(); it++) {
	    p4_jxv01 += JX_helper.refTrk(it,massesJX[it]);
	  }
	  xAOD::BPhysHelper V01_helper(V0Itr1->first);
	  for(size_t it=0; it<V0Itr1->first->nTrackParticles(); it++) {
	    p4_jxv01 += V01_helper.refTrk(it,massesV01[it]);
	  }

	  for(auto V0Itr2=selectedV02Candidates.cbegin(); V0Itr2!=selectedV02Candidates.cend(); ++V0Itr2) {
	    // Check identical tracks in input
	    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr2->first->trackParticle(0)) != tracksJX.cend()) continue;
	    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0Itr2->first->trackParticle(1)) != tracksJX.cend()) continue;
	    if(std::find(tracksV01.cbegin(), tracksV01.cend(), V0Itr2->first->trackParticle(0)) != tracksV01.cend()) continue;
	    if(std::find(tracksV01.cbegin(), tracksV01.cend(), V0Itr2->first->trackParticle(1)) != tracksV01.cend()) continue;
	    tracksV02.clear();
	    for(size_t k=0; k<V0Itr2->first->nTrackParticles(); k++) tracksV02.push_back(V0Itr2->first->trackParticle(k));
	    std::vector<double> massesV02;
	    if(V0Itr2->second==LAMBDA_EXISTING || V0Itr2->second==LAMBDA_CREATED)            massesV02 = massesV0_ppi;
	    else if(V0Itr2->second==LAMBDABAR_EXISTING || V0Itr2->second==LAMBDABAR_CREATED) massesV02 = massesV0_pip;
	    else if(V0Itr2->second==KS_EXISTING || V0Itr2->second==KS_CREATED)               massesV02 = massesV0_pipi;
	    TLorentzVector p4_moth = p4_jxv01;
	    xAOD::BPhysHelper V02_helper(V0Itr2->first);
	    for(size_t it=0; it<V0Itr2->first->nTrackParticles(); it++) {
	      p4_moth += V02_helper.refTrk(it,massesV02[it]);
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
	    // V01 vertex
	    Trk::VertexID vID1;
	    if (m_constrV01) {
	      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01,m_V01Hypothesis=="Ks" ? mass_Ks : mass_Lambda);
	    } else {
	      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01);
	    }
	    vrtList.push_back(vID1);
	    // V02 vertex
	    Trk::VertexID vID2;
	    if (m_constrV02) {
	      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02,m_V02Hypothesis=="Ks" ? mass_Ks : mass_Lambda);
	    } else {
	      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02);
	    }
	    vrtList2.push_back(vID2);
	    // JXV02 vertex
	    Trk::VertexID vID3;
	    if (m_constrJXV02) {
	      vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,m_massJXV02);
	    } else {
	      vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2);
	    }
	    vrtList.push_back(vID3);
	    // Mother vertex including V01, V02 and JX
	    std::vector<const xAOD::TrackParticle*> tp; tp.clear();
	    std::vector<double> tp_masses; tp_masses.clear();
	    if(m_constrMainV) {
	      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,m_massMainV);
	    } else {
	      m_iVertexFitter->nextVertex(tp,tp_masses,vrtList);
	    }
	    if (m_constrJX && m_jxDaug_num>2) {
	      std::vector<Trk::VertexID> cnstV; cnstV.clear();
	      if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,m_massJX).isSuccess() ) {
		ATH_MSG_WARNING("addMassConstraint for JX failed");
	      }
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
	      double lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[3]);
	      double lxy_SV2 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	      if(chi2CutPassed && lxy_SV1>m_lxyV01_cut && lxy_SV2>m_lxyV02_cut) {
		chi2_V1_decor(*cascadeVertices[0]) = V0Itr1->first->chiSquared();
		ndof_V1_decor(*cascadeVertices[0]) = V0Itr1->first->numberDoF();
		if(V0Itr1->second==LAMBDA_EXISTING || V0Itr1->second==LAMBDA_CREATED)            type_V1_decor(*cascadeVertices[0]) = "Lambda";
		else if(V0Itr1->second==LAMBDABAR_EXISTING || V0Itr1->second==LAMBDABAR_CREATED) type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
		else if(V0Itr1->second==KS_EXISTING || V0Itr1->second==KS_CREATED)               type_V1_decor(*cascadeVertices[0]) = "Ks";
		mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0Itr1->first) ? mAcc_gfit(*V0Itr1->first) : 0;
		mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0Itr1->first) ? mAcc_gmass(*V0Itr1->first) : -1;
		mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0Itr1->first) ? mAcc_gmasserr(*V0Itr1->first) : -1;
		mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0Itr1->first) ? mAcc_gchisq(*V0Itr1->first) : 999999;
		mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0Itr1->first) ? mAcc_gndof(*V0Itr1->first) : 0;
		mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0Itr1->first) ? mAcc_gprob(*V0Itr1->first) : -1;
		trk_px.clear(); trk_py.clear(); trk_pz.clear();
		if(trk_pxAcc.isAvailable(*V0Itr1->first)) {
		  trk_px = trk_pxAcc(*V0Itr1->first);
		  trk_py = trk_pyAcc(*V0Itr1->first);
		  trk_pz = trk_pzAcc(*V0Itr1->first);
		}
		trk_pxDeco(*cascadeVertices[0]) = trk_px;
		trk_pyDeco(*cascadeVertices[0]) = trk_py;
		trk_pzDeco(*cascadeVertices[0]) = trk_pz;

		chi2_V2_decor(*cascadeVertices[1]) = V0Itr2->first->chiSquared();
		ndof_V2_decor(*cascadeVertices[1]) = V0Itr2->first->numberDoF();
		if(V0Itr2->second==LAMBDA_EXISTING || V0Itr2->second==LAMBDA_CREATED)            type_V2_decor(*cascadeVertices[1]) = "Lambda";
		else if(V0Itr2->second==LAMBDABAR_EXISTING || V0Itr2->second==LAMBDABAR_CREATED) type_V2_decor(*cascadeVertices[1]) = "Lambdabar";
		else if(V0Itr2->second==KS_EXISTING || V0Itr2->second==KS_CREATED)               type_V2_decor(*cascadeVertices[1]) = "Ks";
		mDec_gfit(*cascadeVertices[1])     = mAcc_gfit.isAvailable(*V0Itr2->first) ? mAcc_gfit(*V0Itr2->first) : 0;
		mDec_gmass(*cascadeVertices[1])    = mAcc_gmass.isAvailable(*V0Itr2->first) ? mAcc_gmass(*V0Itr2->first) : -1;
		mDec_gmasserr(*cascadeVertices[1]) = mAcc_gmasserr.isAvailable(*V0Itr2->first) ? mAcc_gmasserr(*V0Itr2->first) : -1;
		mDec_gchisq(*cascadeVertices[1])   = mAcc_gchisq.isAvailable(*V0Itr2->first) ? mAcc_gchisq(*V0Itr2->first) : 999999;
		mDec_gndof(*cascadeVertices[1])    = mAcc_gndof.isAvailable(*V0Itr2->first) ? mAcc_gndof(*V0Itr2->first) : 0;
		mDec_gprob(*cascadeVertices[1])    = mAcc_gprob.isAvailable(*V0Itr2->first) ? mAcc_gprob(*V0Itr2->first) : -1;
		trk_px.clear(); trk_py.clear(); trk_pz.clear();
		if(trk_pxAcc.isAvailable(*V0Itr2->first)) {
		  trk_px = trk_pxAcc(*V0Itr2->first);
		  trk_py = trk_pyAcc(*V0Itr2->first);
		  trk_pz = trk_pzAcc(*V0Itr2->first);
		}
		trk_pxDeco(*cascadeVertices[1]) = trk_px;
		trk_pyDeco(*cascadeVertices[1]) = trk_py;
		trk_pzDeco(*cascadeVertices[1]) = trk_pz;

		cascadeinfoContainer->push_back(result.release());
	      }
	    }
	  } // V0Itr2
	} // V0Itr1
      } // m_V01Hypothesis != m_V02Hypothesis
    } // jxItr

    // clean up transient objects
    if(V0OutputContainers.size()==0) {
      if(m_V01Hypothesis != m_V02Hypothesis) {
	for(auto v0VItr=selectedV01Candidates.cbegin(); v0VItr!=selectedV01Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
	for(auto v0VItr=selectedV02Candidates.cbegin(); v0VItr!=selectedV02Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
      } else {
	for(auto v0VItr=selectedV0Candidates.cbegin(); v0VItr!=selectedV0Candidates.cend(); ++v0VItr) if(v0VItr->second>=LAMBDA_CREATED) delete v0VItr->first;
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlus2V0::addBranches() const {
    const size_t topoN = 4;
    std::array<std::unique_ptr<xAOD::VertexContainer>, topoN> VtxWriteHandles;
    std::array<std::unique_ptr<xAOD::VertexAuxContainer>, topoN> VtxWriteHandlesAux;
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
    std::vector<xAOD::VertexContainer*> V0OutputContainers;
    std::vector<xAOD::VertexAuxContainer*> V0OutputAuxContainers;
    for(size_t i=0; i<m_v0VtxOutputsKeys.size(); i++) {
      xAOD::VertexContainer* V0OutputContainer = new xAOD::VertexContainer;
      xAOD::VertexAuxContainer* V0OutputAuxContainer = new xAOD::VertexAuxContainer;
      V0OutputContainer->setStore(V0OutputAuxContainer);
      ATH_CHECK(evtStore()->record(V0OutputContainer   , m_v0VtxOutputsKeys[i]));
      ATH_CHECK(evtStore()->record(V0OutputAuxContainer, m_v0VtxOutputsKeys[i] + "Aux."));
      V0OutputContainers.push_back(V0OutputContainer);
      V0OutputAuxContainers.push_back(V0OutputAuxContainer);
    }

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
    ATH_CHECK(performSearch(&cascadeinfoContainer,V0OutputContainers));

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

    static SG::AuxElement::Decorator<float> lxy_SV3_decor("lxy_SV3");
    static SG::AuxElement::Decorator<float> lxyErr_SV3_decor("lxyErr_SV3");
    static SG::AuxElement::Decorator<float> a0xy_SV3_decor("a0xy_SV3");
    static SG::AuxElement::Decorator<float> a0xyErr_SV3_decor("a0xyErr_SV3");
    static SG::AuxElement::Decorator<float> a0z_SV3_decor("a0z_SV3");
    static SG::AuxElement::Decorator<float> a0zErr_SV3_decor("a0zErr_SV3");

    static SG::AuxElement::Decorator<float> chi2_V3_decor("ChiSquared_V3");
    static SG::AuxElement::Decorator<int> ndof_V3_decor("nDoF_V3");

    // Get the input containers
    const xAOD::VertexContainer *jxContainer(nullptr);
    ATH_CHECK(evtStore()->retrieve(jxContainer, m_vertexJXContainerKey));
    std::vector<const xAOD::VertexContainer*> V0Containers;
    for(size_t i=0; i<m_vertexV0ContainerKeys.size(); i++) {
      const xAOD::VertexContainer *V0Container(nullptr);
      ATH_CHECK(evtStore()->retrieve(V0Container, m_vertexV0ContainerKeys[i]));
      V0Containers.push_back(V0Container);
    }

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

      // Identify the input V01
      xAOD::Vertex* v01Vtx = FindVertex<2>(V0Containers, cascadeVertices[0]);
      // Identify the input V02
      xAOD::Vertex* v02Vtx = FindVertex<2>(V0Containers, cascadeVertices[1]);
      // Identify the input JX
      xAOD::Vertex* jxVtx(nullptr);
      if(m_jxDaug_num==4) jxVtx = FindVertex<4>(jxContainer, cascadeVertices[2]);
      else if(m_jxDaug_num==3) jxVtx = FindVertex<3>(jxContainer, cascadeVertices[2]);
      else jxVtx = FindVertex<2>(jxContainer, cascadeVertices[2]);

      // transfer hypotheses to output vertices
      for(auto name : m_vertexV0HypoNames) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
	if(flagAcc.isAvailable(*v01Vtx) && flagAcc(*v01Vtx)) {
	  SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
 	  flagDec(*cascadeVertices[0]) = true;
        }
        if(flagAcc.isAvailable(*v02Vtx) && flagAcc(*v02Vtx)) {
	  SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
	  flagDec(*cascadeVertices[1]) = true;
        }
      }

      for(auto name : m_vertexJXHypoNames) {
        SG::AuxElement::Accessor<Char_t> flagAcc("passed_"+name);
        if(flagAcc.isAvailable(*jxVtx) && flagAcc(*jxVtx)) {
	  SG::AuxElement::Decorator<Char_t> flagDec("passed_"+name);
	  flagDec(*cascadeVertices[2]) = true;
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

      // decorate the cascade vertices
      lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
      lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
      a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);
      a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
      a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info->getCovariance()[0],cascadeVertices[0],mainVertex);

      if(m_V01Hypothesis == m_V02Hypothesis) {
	lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
	a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
	a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],mainVertex);
      }
      else {
	lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
	a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],cascadeVertices[2]);
	a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
	a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],cascadeVertices[2]);
	a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info->getCovariance()[1],cascadeVertices[1],cascadeVertices[2]);
      }

      lxy_SV3_decor(*cascadeVertices[2])     = m_CascadeTools->lxy(moms[2],cascadeVertices[2],mainVertex);
      lxyErr_SV3_decor(*cascadeVertices[2])  = m_CascadeTools->lxyError(moms[2],cascade_info->getCovariance()[2],cascadeVertices[2],mainVertex);
      a0z_SV3_decor(*cascadeVertices[2])     = m_CascadeTools->a0z(moms[2],cascadeVertices[2],mainVertex);
      a0zErr_SV3_decor(*cascadeVertices[2])  = m_CascadeTools->a0zError(moms[2],cascade_info->getCovariance()[2],cascadeVertices[2],mainVertex);
      a0xy_SV3_decor(*cascadeVertices[2])    = m_CascadeTools->a0xy(moms[2],cascadeVertices[2],mainVertex);
      a0xyErr_SV3_decor(*cascadeVertices[2]) = m_CascadeTools->a0xyError(moms[2],cascade_info->getCovariance()[2],cascadeVertices[2],mainVertex);

      chi2_V3_decor(*cascadeVertices[2])     = m_V0Tools->chisq(jxVtx);
      ndof_V3_decor(*cascadeVertices[2])     = m_V0Tools->ndof(jxVtx);

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
      VertexLink vertexLink3;
      vertexLink3.setElement(cascadeVertices[2]);
      vertexLink3.setStorableObject(*VtxWriteHandles[2].get());
      if( vertexLink3.isValid() ) precedingVertexLinks.push_back( vertexLink3 );

      CascadeLinksDecor(*mainVertex) = precedingVertexLinks;
    } // loop over cascadeinfoContainer

    for(size_t i=0; i<topoN; i++){
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandles[i])   , m_cascadeOutputsKeys[i]));
      ATH_CHECK(evtStore()->record(std::move(VtxWriteHandlesAux[i]), m_cascadeOutputsKeys[i] + "Aux."));
    }

    // Deleting cascadeinfo since this won't be stored.
    // Vertices have been kept in m_cascadeOutputs and should be owned by their container
    for (auto cascade_info : cascadeinfoContainer) delete cascade_info;

    return StatusCode::SUCCESS;
  }

  template<size_t NTracks>
  xAOD::Vertex* JpsiXPlus2V0::FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const {
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

  template<size_t NTracks>
  xAOD::Vertex* JpsiXPlus2V0::FindVertex(std::vector<const xAOD::VertexContainer*> containers, const xAOD::Vertex* v) const {
    for (const xAOD::VertexContainer* cont : containers) {
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
    }
    return nullptr;
  }
}
