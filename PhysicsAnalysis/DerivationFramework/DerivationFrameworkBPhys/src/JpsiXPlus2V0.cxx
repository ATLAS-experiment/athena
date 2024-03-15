/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#include "DerivationFrameworkBPhys/JpsiXPlus2V0.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtFitter/VxCascadeInfo.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "HepPDT/ParticleDataTable.hh"
#include "VxVertex/RecVertex.h"
#include "TruthUtils/HepMCHelpers.h"
#include <algorithm>
#include <functional>

namespace DerivationFramework {
  typedef ElementLink<xAOD::VertexContainer> VertexLink;
  typedef std::vector<VertexLink> VertexLinkVector;

  JpsiXPlus2V0::JpsiXPlus2V0(const std::string& type, const std::string& name, const IInterface* parent) : AthAlgTool(type,name,parent),
    m_vertexJXContainerKey("InputJXVertices"),
    m_vertexV0ContainerKeys{"InputV0Containers"},
    m_cascadeOutputKeys({"JpsiXPlus2V0_SubVtx1", "JpsiXPlus2V0_SubVtx2", "JpsiXPlus2V0_SubVtx3", "JpsiXPlus2V0_MainVtx"}),
    m_refitV0(false),
    m_constrV0(true),
    m_v0VtxOutputKeys(this,"OutoutV0VtxCollections",{}),
    m_TrkParticleCollection("InDetTrackParticles"),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_eventInfo_key("EventInfo"),
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
    m_JXSubVtx(true),
    m_JXV02SubVtx(false),
    m_chi2cut_JX(-1.0),
    m_chi2cut_V0(-1.0),
    m_chi2cut(-1.0),
    m_maxJXCandidates(0),
    m_maxV0Candidates(0),
    m_maxMainVCandidates(0),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_iV0Fitter("Trk::V0VertexFitter"),
    m_iGammaFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter", this),
    m_V0Tools("Trk::V0Tools"),
    m_CascadeTools("DerivationFramework::CascadeTools")
  {
    declareProperty("JXVertices",               m_vertexJXContainerKey);
    declareProperty("V0Containers",             m_vertexV0ContainerKeys);
    declareProperty("JXVtxHypoNames",           m_vertexJXHypoNames);
    declareProperty("V0VtxHypoNames",           m_vertexV0HypoNames);
    declareProperty("CascadeVertexCollections", m_cascadeOutputKeys); // size is 3 or 4 only
    declareProperty("RefitV0",                  m_refitV0);
    declareProperty("ApplyV0MassConstraint",    m_constrV0); // only effective when m_refitV0 = true
    declareProperty("OutoutV0VtxCollections",   m_v0VtxOutputKeys);
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
    declareProperty("JXV02VtxMass",             m_massJXV02); // mass of JX + 2nd V0
    declareProperty("MainVtxMass",              m_massMainV);
    declareProperty("ApplyJXMassConstraint",    m_constrJX);
    declareProperty("ApplyJpsiMassConstraint",  m_constrJpsi); // only effective when m_jxDaug_num>2
    declareProperty("ApplyXMassConstraint",     m_constrX); // only effective when m_jxDaug_num=4
    declareProperty("ApplyV01MassConstraint",   m_constrV01); // first V0
    declareProperty("ApplyV02MassConstraint",   m_constrV02); // second V0
    declareProperty("ApplyJXV02MassConstraint", m_constrJXV02); // constrain JX + 2nd V0
    declareProperty("ApplyMainVMassConstraint", m_constrMainV);
    declareProperty("HasJXSubVertex",           m_JXSubVtx);
    declareProperty("HasJXV02SubVertex",        m_JXV02SubVtx); // only effective when m_JXSubVtx=true
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

    if( (m_V01Hypothesis == m_V02Hypothesis && (m_v0VtxOutputKeys.size() != 0 && m_v0VtxOutputKeys.size() != 1)) ||
        (m_V01Hypothesis != m_V02Hypothesis && (m_v0VtxOutputKeys.size() != 0 && m_v0VtxOutputKeys.size() != 2)) ) {
      ATH_MSG_FATAL("V0OutputKeys size is not correct!");
    }

    if(m_jxDaug_num<2 || m_jxDaug_num>4) {
      ATH_MSG_FATAL("Incorrect number of JX daughters");
      return StatusCode::FAILURE;
    }

    if(m_vertexV0ContainerKeys.size()>10) {
      ATH_MSG_FATAL("Number of input V0 containers is too large");
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

    // retrieving the Cascade tools
    ATH_CHECK( m_CascadeTools.retrieve() );

    ATH_CHECK( m_vertexJXContainerKey.initialize() );
    ATH_CHECK( m_vertexV0ContainerKeys.initialize() );
    ATH_CHECK( m_VxPrimaryCandidateName.initialize() );
    ATH_CHECK( m_TrkParticleCollection.initialize() );
    ATH_CHECK( m_refPVContainerName.initialize() );
    ATH_CHECK( m_cascadeOutputKeys.initialize() );
    ATH_CHECK( m_eventInfo_key.initialize() );
    ATH_CHECK( m_v0VtxOutputKeys.initialize() );

    IPartPropSvc* partPropSvc = nullptr;
    ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
    auto pdt = partPropSvc->PDT();

    // https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/TruthUtils/TruthUtils/AtlasPID.h
    m_mass_e = BPhysPVCascadeTools::getParticleMass(pdt, MC::ELECTRON);
    m_mass_mu = BPhysPVCascadeTools::getParticleMass(pdt, MC::MUON);
    m_mass_pion = BPhysPVCascadeTools::getParticleMass(pdt, MC::PIPLUS);
    m_mass_proton = BPhysPVCascadeTools::getParticleMass(pdt, MC::PROTON);
    m_mass_Lambda = BPhysPVCascadeTools::getParticleMass(pdt, MC::LAMBDA0);
    m_mass_Lambda_b = BPhysPVCascadeTools::getParticleMass(pdt, MC::LAMBDAB0);
    m_mass_Ks = BPhysPVCascadeTools::getParticleMass(pdt, MC::K0S);
    m_mass_Bpm = BPhysPVCascadeTools::getParticleMass(pdt, 521);

    // retrieve particle masses
    if(m_constrJpsi && m_massJpsi<0) m_massJpsi = BPhysPVCascadeTools::getParticleMass(pdt, MC::JPSI);
    if(m_constrJX && m_massJX<0) m_massJX = BPhysPVCascadeTools::getParticleMass(pdt, MC::PSI2S);
    if(m_constrJXV02 && m_massJXV02<0) m_massJXV02 = m_mass_Lambda_b;
    if(m_constrMainV && m_massMainV<0) m_massMainV = m_mass_Bpm;

    if(m_jxDaug1MassHypo < 0.) m_jxDaug1MassHypo = m_mass_mu;
    if(m_jxDaug2MassHypo < 0.) m_jxDaug2MassHypo = m_mass_mu;
    if(m_jxDaug_num>=3 && m_jxDaug3MassHypo < 0.) m_jxDaug3MassHypo = m_mass_pion;
    if(m_jxDaug_num==4 && m_jxDaug4MassHypo < 0.) m_jxDaug4MassHypo = m_mass_pion;

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlus2V0::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<xAOD::VertexContainer*> V0OutputContainers) const {
    ATH_MSG_DEBUG( "JpsiXPlus2V0::performSearch" );
    assert(cascadeinfoContainer!=nullptr);

    // Get TrackParticle container
    SG::ReadHandle<xAOD::TrackParticleContainer> trackContainer(m_TrkParticleCollection);
    ATH_CHECK( trackContainer.isValid() );

    // Get the PrimaryVertices container
    SG::ReadHandle<xAOD::VertexContainer> pvContainer(m_VxPrimaryCandidateName);
    ATH_CHECK( pvContainer.isValid() );
    if (pvContainer.cptr()->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer.cptr()->size());
      return StatusCode::RECOVERABLE;
    }

    std::vector<double> massesJX;
    massesJX.push_back(m_jxDaug1MassHypo);
    massesJX.push_back(m_jxDaug2MassHypo);
    if(m_jxDaug_num>=3) massesJX.push_back(m_jxDaug3MassHypo);
    if(m_jxDaug_num==4) massesJX.push_back(m_jxDaug4MassHypo);
    std::vector<double> massesV0_ppi;
    massesV0_ppi.push_back(m_mass_proton);
    massesV0_ppi.push_back(m_mass_pion);
    std::vector<double> massesV0_pip;
    massesV0_pip.push_back(m_mass_pion);
    massesV0_pip.push_back(m_mass_proton);
    std::vector<double> massesV0_pipi;
    massesV0_pipi.push_back(m_mass_pion);
    massesV0_pipi.push_back(m_mass_pion);

    // Get Jpsi+X container
    SG::ReadHandle<xAOD::VertexContainer> jxContainer(m_vertexJXContainerKey);
    ATH_CHECK( jxContainer.isValid() );

    // Get V0 containers
    std::vector<const xAOD::VertexContainer*> V0Containers;
    std::array<SG::ReadHandle<xAOD::VertexContainer>, 10> V0Handles; int ikey(0);
    for(const SG::ReadHandleKey<xAOD::VertexContainer>& key : m_vertexV0ContainerKeys) {
      V0Handles[ikey] = SG::ReadHandle<xAOD::VertexContainer>(key);
      ATH_CHECK( V0Handles[ikey].isValid() );
      V0Containers.push_back(V0Handles[ikey].cptr());
      ikey++;
    }

    // Accessors of V0 with photon conversion info and no-mass-constraint track momenta
    SG::AuxElement::Accessor<std::string> mAcc_type("Type_V0Vtx");
    SG::AuxElement::Accessor<int>    mAcc_gfit("gamma_fit");
    SG::AuxElement::Accessor<float>  mAcc_gmass("gamma_mass");
    SG::AuxElement::Accessor<float>  mAcc_gmasserr("gamma_massError");
    SG::AuxElement::Accessor<float>  mAcc_gchisq("gamma_chisq");
    SG::AuxElement::Accessor<int>    mAcc_gndof("gamma_ndof");
    SG::AuxElement::Accessor<float>  mAcc_gprob("gamma_probability");
    SG::AuxElement::Accessor< std::vector<float> > trk_pxAcc("TrackPx_V0nc");
    SG::AuxElement::Accessor< std::vector<float> > trk_pyAcc("TrackPy_V0nc");
    SG::AuxElement::Accessor< std::vector<float> > trk_pzAcc("TrackPz_V0nc");
    // Decorators of V0 vertices
    SG::AuxElement::Decorator<std::string> mDec_type("Type_V0Vtx");
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
    std::vector<const xAOD::TrackParticle*> tracksV0;

    // Select the V0 candidates before calling cascade fit
    std::vector<std::pair<const xAOD::Vertex*,V0Enum> > selectedV0Candidates_EXISTING;
    std::vector<std::pair<xAOD::Vertex*,V0Enum> > selectedV0Candidates_CREATED;
    for(size_t ic=0; ic<V0Containers.size(); ic++) {
      const xAOD::VertexContainer* V0Container = V0Containers[ic];
      for(auto vxcItr=V0Container->begin(); vxcItr!=V0Container->end(); ++vxcItr) {
	const xAOD::Vertex* vtx = *vxcItr;
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
	  double massSig_V0_Lambda1 = std::abs(m_V0Tools->invariantMass(vtx, massesV0_ppi)-m_mass_Lambda)/m_V0Tools->invariantMassError(vtx, massesV0_ppi);
	  double massSig_V0_Lambda2 = std::abs(m_V0Tools->invariantMass(vtx, massesV0_pip)-m_mass_Lambda)/m_V0Tools->invariantMassError(vtx, massesV0_pip);
	  double massSig_V0_Ks = std::abs(m_V0Tools->invariantMass(vtx, massesV0_pipi)-m_mass_Ks)/m_V0Tools->invariantMassError(vtx, massesV0_pipi);
	  if(massSig_V0_Lambda1<=massSig_V0_Lambda2 && massSig_V0_Lambda1<=massSig_V0_Ks) {
	    opt = LAMBDA;
	    massV0 = m_V0Tools->invariantMass(vtx, massesV0_ppi);
	  }
	  else if(massSig_V0_Lambda2<=massSig_V0_Lambda1 && massSig_V0_Lambda2<=massSig_V0_Ks) {
	    opt = LAMBDABAR;
	    massV0 = m_V0Tools->invariantMass(vtx, massesV0_pip);
	  }
	  else if(massSig_V0_Ks<=massSig_V0_Lambda1 && massSig_V0_Ks<=massSig_V0_Lambda2) {
	    opt = KS;
	    massV0 = m_V0Tools->invariantMass(vtx, massesV0_pipi);
	  }

	  if(opt==LAMBDA || opt==LAMBDABAR) {
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
	  else if(opt==KS) {
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
	  if(type_V0Vtx == "Lambda")         opt = LAMBDA;
	  else if(type_V0Vtx == "Lambdabar") opt = LAMBDABAR;
	  else if(type_V0Vtx == "Ks")        opt = KS;
	  else                               opt = UNKNOWN;
	}

	if(opt==UNKNOWN) continue;
	if(m_V01Hypothesis == m_V02Hypothesis) {
	  if((opt==LAMBDA || opt==LAMBDABAR) && m_V01Hypothesis != "Lambda")  continue;
	  if(opt==KS && m_V01Hypothesis != "Ks") continue;
	}

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
	  std::unique_ptr<xAOD::Vertex> gammaVtx = std::unique_ptr<xAOD::Vertex>( m_iGammaFitter->fit(tracksV0, vtxPos) );
	  if (gammaVtx) {
	    gamma_fit     = 1;
	    gamma_mass    = m_V0Tools->invariantMass(gammaVtx.get(),m_mass_e,m_mass_e);
	    gamma_massErr = m_V0Tools->invariantMassError(gammaVtx.get(),m_mass_e,m_mass_e);
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
	  std::unique_ptr<xAOD::Vertex> V0vtx;
	  if(m_constrV0) {
	    std::vector<double> massesV0;
	    if(opt == LAMBDA)         massesV0 = massesV0_ppi;
	    else if(opt == LAMBDABAR) massesV0 = massesV0_pip;
	    else if(opt == KS)        massesV0 = massesV0_pipi;
	    // https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkV0Fitter/TrkV0Fitter/TrkV0VertexFitter.h
	    V0vtx = std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, massesV0, opt==KS ? m_mass_Ks : m_mass_Lambda, 0, vtxPos) );
	  }
	  else {
	    V0vtx = std::unique_ptr<xAOD::Vertex>( m_iV0Fitter->fit(tracksV0, vtxPos) );
	  }
	  if(V0vtx && V0vtx->chiSquared()>=0) {
	    double chi2DOF = V0vtx->chiSquared()/V0vtx->numberDoF();
	    if(m_chi2cut_V0>0 && chi2DOF>m_chi2cut_V0) continue;

	    xAOD::BPhysHelper V0_helper(V0vtx.get());
	    V0_helper.setRefTrks(); // AOD only method

	    V0vtx->clearTracks();
	    ElementLink<xAOD::TrackParticleContainer> newLink1;
	    newLink1.setElement(tracksV0[0]);
	    newLink1.setStorableObject(*trackContainer.cptr());
	    ElementLink<xAOD::TrackParticleContainer> newLink2;
	    newLink2.setElement(tracksV0[1]);
	    newLink2.setStorableObject(*trackContainer.cptr());
	    V0vtx->addTrackAtVertex(newLink1);
	    V0vtx->addTrackAtVertex(newLink2);

	    mDec_gfit(*V0vtx.get())     = gamma_fit;
	    mDec_gmass(*V0vtx.get())    = gamma_mass;
	    mDec_gmasserr(*V0vtx.get()) = gamma_massErr;
	    mDec_gchisq(*V0vtx.get())   = gamma_chisq;
	    mDec_gndof(*V0vtx.get())    = gamma_ndof;
	    mDec_gprob(*V0vtx.get())    = gamma_prob;
	    if(opt==LAMBDA)         mDec_type(*V0vtx.get()) = "Lambda";
	    else if(opt==LAMBDABAR) mDec_type(*V0vtx.get()) = "Lambdabar";
	    else if(opt==KS)        mDec_type(*V0vtx.get()) = "Ks";
	    if(m_constrV0 && m_decorV0P) {
	      trk_pxDeco(*V0vtx.get()) = trk_px;
	      trk_pyDeco(*V0vtx.get()) = trk_py;
	      trk_pzDeco(*V0vtx.get()) = trk_pz;
	    }
	    selectedV0Candidates_CREATED.push_back(std::pair<xAOD::Vertex*,V0Enum>{V0vtx.release(),opt});
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
	  if(opt==LAMBDA)         mDec_type(*vtx) = "Lambda";
	  else if(opt==LAMBDABAR) mDec_type(*vtx) = "Lambdabar";
	  else if(opt==KS)        mDec_type(*vtx) = "Ks";
	  selectedV0Candidates_EXISTING.push_back(std::pair<const xAOD::Vertex*,V0Enum>{vtx,opt});
	} // no V0 refit
      } // V0 candidate
    } // V0Container
    if((m_refitV0 && selectedV0Candidates_CREATED.size()==0) || (!m_refitV0 && selectedV0Candidates_EXISTING.size()==0)) return StatusCode::SUCCESS;

    std::vector<std::pair<const xAOD::Vertex*,V0Enum> > selectedV01Candidates_EXISTING;
    std::vector<std::pair<const xAOD::Vertex*,V0Enum> > selectedV02Candidates_EXISTING;
    std::vector<std::pair<xAOD::Vertex*,V0Enum> > selectedV01Candidates_CREATED;
    std::vector<std::pair<xAOD::Vertex*,V0Enum> > selectedV02Candidates_CREATED;
    if(m_V01Hypothesis != m_V02Hypothesis) {
      if(m_refitV0) {
	for(size_t j=0; j<selectedV0Candidates_CREATED.size(); j++) {
	  std::pair<xAOD::Vertex*,V0Enum> candidate = selectedV0Candidates_CREATED[j];
	  if(candidate.second==LAMBDA || candidate.second==LAMBDABAR) {
	    if(m_V01Hypothesis == "Lambda") selectedV01Candidates_CREATED.push_back(candidate);
	    else if(m_V02Hypothesis == "Lambda") selectedV02Candidates_CREATED.push_back(candidate);
	  }
	  else if(candidate.second==KS) {
	    if(m_V01Hypothesis == "Ks") selectedV01Candidates_CREATED.push_back(candidate);
	    else if(m_V02Hypothesis == "Ks") selectedV02Candidates_CREATED.push_back(candidate);
	  }
	}

	std::sort( selectedV01Candidates_CREATED.begin(), selectedV01Candidates_CREATED.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
	if(m_maxV0Candidates>0 && selectedV01Candidates_CREATED.size()>m_maxV0Candidates) {
	  for(auto it=selectedV01Candidates_CREATED.begin()+m_maxV0Candidates; it!=selectedV01Candidates_CREATED.end(); it++) delete it->first;
	  selectedV01Candidates_CREATED.erase(selectedV01Candidates_CREATED.begin()+m_maxV0Candidates, selectedV01Candidates_CREATED.end());
	}
	std::sort( selectedV02Candidates_CREATED.begin(), selectedV02Candidates_CREATED.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
	if(m_maxV0Candidates>0 && selectedV02Candidates_CREATED.size()>m_maxV0Candidates) {
	  for(auto it=selectedV02Candidates_CREATED.begin()+m_maxV0Candidates; it!=selectedV02Candidates_CREATED.end(); it++) delete it->first;
	  selectedV02Candidates_CREATED.erase(selectedV02Candidates_CREATED.begin()+m_maxV0Candidates, selectedV02Candidates_CREATED.end());
	}

	if(V0OutputContainers.size()==2) {
	  for(auto v0VItr=selectedV01Candidates_CREATED.begin(); v0VItr!=selectedV01Candidates_CREATED.end(); ++v0VItr) V0OutputContainers[0]->push_back(v0VItr->first);
	  for(auto v0VItr=selectedV02Candidates_CREATED.begin(); v0VItr!=selectedV02Candidates_CREATED.end(); ++v0VItr) V0OutputContainers[1]->push_back(v0VItr->first);
	}
      }
      else { // not refit V0
	for(size_t j=0; j<selectedV0Candidates_EXISTING.size(); j++) {
	  std::pair<const xAOD::Vertex*,V0Enum> candidate = selectedV0Candidates_EXISTING[j];
	  if(candidate.second==LAMBDA || candidate.second==LAMBDABAR) {
	    if(m_V01Hypothesis == "Lambda") selectedV01Candidates_EXISTING.push_back(candidate);
	    else if(m_V02Hypothesis == "Lambda") selectedV02Candidates_EXISTING.push_back(candidate);
	  }
	  else if(candidate.second==KS) {
	    if(m_V01Hypothesis == "Ks") selectedV01Candidates_EXISTING.push_back(candidate);
	    else if(m_V02Hypothesis == "Ks") selectedV02Candidates_EXISTING.push_back(candidate);
	  }
	}

	std::sort( selectedV01Candidates_EXISTING.begin(), selectedV01Candidates_EXISTING.end(), [](std::pair<const xAOD::Vertex*,V0Enum> a, std::pair<const xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
	if(m_maxV0Candidates>0 && selectedV01Candidates_EXISTING.size()>m_maxV0Candidates) {
	  selectedV01Candidates_EXISTING.erase(selectedV01Candidates_EXISTING.begin()+m_maxV0Candidates, selectedV01Candidates_EXISTING.end());
	}
	std::sort( selectedV02Candidates_EXISTING.begin(), selectedV02Candidates_EXISTING.end(), [](std::pair<const xAOD::Vertex*,V0Enum> a, std::pair<const xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
	if(m_maxV0Candidates>0 && selectedV02Candidates_EXISTING.size()>m_maxV0Candidates) {
	  selectedV02Candidates_EXISTING.erase(selectedV02Candidates_EXISTING.begin()+m_maxV0Candidates, selectedV02Candidates_EXISTING.end());
	}
      }
    }
    else { // m_V01Hypothesis == m_V02Hypothesis
      if(m_refitV0) {
	std::sort( selectedV0Candidates_CREATED.begin(), selectedV0Candidates_CREATED.end(), [](std::pair<xAOD::Vertex*,V0Enum> a, std::pair<xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
	if(m_maxV0Candidates>0 && selectedV0Candidates_CREATED.size()>m_maxV0Candidates) {
	  for(auto it=selectedV0Candidates_CREATED.begin()+m_maxV0Candidates; it!=selectedV0Candidates_CREATED.end(); it++) delete it->first;
	  selectedV0Candidates_CREATED.erase(selectedV0Candidates_CREATED.begin()+m_maxV0Candidates, selectedV0Candidates_CREATED.end());
	}

	if(V0OutputContainers.size()==1) {
	  for(auto v0VItr=selectedV0Candidates_CREATED.begin(); v0VItr!=selectedV0Candidates_CREATED.end(); ++v0VItr) V0OutputContainers[0]->push_back(v0VItr->first);
	}
      }
      else { // not refit V0
	std::sort( selectedV0Candidates_EXISTING.begin(), selectedV0Candidates_EXISTING.end(), [](std::pair<const xAOD::Vertex*,V0Enum> a, std::pair<const xAOD::Vertex*,V0Enum> b) { return a.first->chiSquared()/a.first->numberDoF() < b.first->chiSquared()/b.first->numberDoF(); } );
	if(m_maxV0Candidates>0 && selectedV0Candidates_EXISTING.size()>m_maxV0Candidates) {
	  selectedV0Candidates_EXISTING.erase(selectedV0Candidates_EXISTING.begin()+m_maxV0Candidates, selectedV0Candidates_EXISTING.end());
	}
      }
    }

    // Select the JX candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedJXCandidates;
    for(auto vxcItr=jxContainer.ptr()->begin(); vxcItr!=jxContainer.ptr()->end(); ++vxcItr) {
      // Check the passed flag first
      const xAOD::Vertex* vtx = *vxcItr;
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
	  for(auto v0VItr=selectedV01Candidates_CREATED.begin(); v0VItr!=selectedV01Candidates_CREATED.end(); ++v0VItr) delete v0VItr->first;
	  for(auto v0VItr=selectedV02Candidates_CREATED.begin(); v0VItr!=selectedV02Candidates_CREATED.end(); ++v0VItr) delete v0VItr->first;
	} else {
	  for(auto v0VItr=selectedV0Candidates_CREATED.begin(); v0VItr!=selectedV0Candidates_CREATED.end(); ++v0VItr) delete v0VItr->first;
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
      // Iterate over V0 vertices
      if(m_V01Hypothesis == m_V02Hypothesis) {
	if(m_refitV0) {
	  for(auto V0Itr1=selectedV0Candidates_CREATED.begin(); V0Itr1!=selectedV0Candidates_CREATED.end(); ++V0Itr1) {
	    for(auto V0Itr2=V0Itr1+1; V0Itr2!=selectedV0Candidates_CREATED.end(); ++V0Itr2) {
	      Trk::VxCascadeInfo* result = fitMainVtx(*jxItr, massesJX, V0Itr1->first, V0Itr1->second, V0Itr2->first, V0Itr2->second, trackContainer.ptr());
	      if(result) cascadeinfoContainer->push_back(result);
	    }
	  }
	}
	else { // not refit V0
	  for(auto V0Itr1=selectedV0Candidates_EXISTING.begin(); V0Itr1!=selectedV0Candidates_EXISTING.end(); ++V0Itr1) {
	    for(auto V0Itr2=V0Itr1+1; V0Itr2!=selectedV0Candidates_EXISTING.end(); ++V0Itr2) {
	      Trk::VxCascadeInfo* result = fitMainVtx(*jxItr, massesJX, V0Itr1->first, V0Itr1->second, V0Itr2->first, V0Itr2->second, trackContainer.ptr());
	      if(result) cascadeinfoContainer->push_back(result);
	    }
	  }
	}
      }
      else { // m_V01Hypothesis != m_V02Hypothesis
	if(m_refitV0) {
	  for(auto V0Itr1=selectedV01Candidates_CREATED.begin(); V0Itr1!=selectedV01Candidates_CREATED.end(); ++V0Itr1) {
	    for(auto V0Itr2=selectedV02Candidates_CREATED.begin(); V0Itr2!=selectedV02Candidates_CREATED.end(); ++V0Itr2) {
	      Trk::VxCascadeInfo* result = fitMainVtx(*jxItr, massesJX, V0Itr1->first, V0Itr1->second, V0Itr2->first, V0Itr2->second, trackContainer.ptr());
	      if(result) cascadeinfoContainer->push_back(result);
	    }
	  }
	}
	else { // not refit V0
	  for(auto V0Itr1=selectedV01Candidates_EXISTING.begin(); V0Itr1!=selectedV01Candidates_EXISTING.end(); ++V0Itr1) {
	    for(auto V0Itr2=selectedV02Candidates_EXISTING.begin(); V0Itr2!=selectedV02Candidates_EXISTING.end(); ++V0Itr2) {
	      Trk::VxCascadeInfo* result = fitMainVtx(*jxItr, massesJX, V0Itr1->first, V0Itr1->second, V0Itr2->first, V0Itr2->second, trackContainer.ptr());
	      if(result) cascadeinfoContainer->push_back(result);
	    }
	  }
	}
      }
    } // jxItr

    // clean up transient objects
    if(V0OutputContainers.size()==0) {
      if(m_V01Hypothesis != m_V02Hypothesis) {
	for(auto v0VItr=selectedV01Candidates_CREATED.begin(); v0VItr!=selectedV01Candidates_CREATED.end(); ++v0VItr) delete v0VItr->first;
	for(auto v0VItr=selectedV02Candidates_CREATED.begin(); v0VItr!=selectedV02Candidates_CREATED.end(); ++v0VItr) delete v0VItr->first;
      } else {
	for(auto v0VItr=selectedV0Candidates_CREATED.begin(); v0VItr!=selectedV0Candidates_CREATED.end(); ++v0VItr) delete v0VItr->first;
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlus2V0::addBranches() const {
    size_t topoN = 4;
    if(!m_JXSubVtx) topoN--;

    if(m_cascadeOutputKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of output cascade vertices");
      return StatusCode::FAILURE;
    }

    std::array<SG::WriteHandle<xAOD::VertexContainer>, 4> VtxWriteHandles; int ikey(0);
    for(const SG::WriteHandleKey<xAOD::VertexContainer>& key : m_cascadeOutputKeys) {
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

    //----------------------------------------------------
    // Record refitted primary vertices
    //----------------------------------------------------
    SG::WriteHandle<xAOD::VertexContainer> refPvContainer;
    if(m_refitPV) {
      refPvContainer = SG::WriteHandle<xAOD::VertexContainer>(m_refPVContainerName);
      ATH_CHECK( refPvContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
    }

    // output V0 vertices
    std::vector<xAOD::VertexContainer*> V0OutputContainers;
    std::array<SG::WriteHandle<xAOD::VertexContainer>, 2> V0OutputHandles; ikey = 0;
    for(const SG::WriteHandleKey<xAOD::VertexContainer>& key : m_v0VtxOutputKeys) {
      V0OutputHandles[ikey] = SG::WriteHandle<xAOD::VertexContainer>(key);
      ATH_CHECK( V0OutputHandles[ikey].record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
      V0OutputContainers.push_back(V0OutputHandles[ikey].ptr());
      ikey++;
    }

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
    ATH_CHECK(performSearch(&cascadeinfoContainer,V0OutputContainers));

    std::sort( cascadeinfoContainer.begin(), cascadeinfoContainer.end(), [](Trk::VxCascadeInfo* a, Trk::VxCascadeInfo* b) { return a->fitChi2()/a->nDoF() < b->fitChi2()/b->nDoF(); } );
    if(m_maxMainVCandidates>0 && cascadeinfoContainer.size()>m_maxMainVCandidates) {
      for(auto it=cascadeinfoContainer.begin()+m_maxMainVCandidates; it!=cascadeinfoContainer.end(); it++) delete *it;
      cascadeinfoContainer.erase(cascadeinfoContainer.begin()+m_maxMainVCandidates, cascadeinfoContainer.end());
    }

    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key);
    ATH_CHECK( evt.isValid() );
    BPhysPVCascadeTools helper(&(*m_CascadeTools), evt.cptr());
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    // Decorators for the main vertex: chi2, ndf, pt and pt error, plus the V0 vertex variables
    SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> JXLinksDecor("JXVertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> V0LinksDecor("V0VertexLinks");
    SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
    SG::AuxElement::Decorator<int> ndof_decor("nDoF");
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
    SG::AuxElement::Decorator<int> ndof_V3_decor("nDoF_V3");

    // Get the input containers
    SG::ReadHandle<xAOD::VertexContainer> jxContainer(m_vertexJXContainerKey);
    ATH_CHECK( jxContainer.isValid() );

    for(auto cascade_info : cascadeinfoContainer) {
      if(cascade_info==nullptr) ATH_MSG_ERROR("CascadeInfo is null");

      const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info->vertices();
      if(cascadeVertices.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices");
      for(size_t i=0; i<topoN; i++) {
	if(cascadeVertices[i]==nullptr) ATH_MSG_ERROR("Error null vertex");
      }

      cascade_info->setSVOwnership(false); // Prevent Container from deleting vertices
      const auto mainVertex = cascadeVertices[topoN-1]; // this is the mother vertex
      const std::vector< std::vector<TLorentzVector> > &moms = cascade_info->getParticleMoms();

      // Identify the input JX
      int ijx = !m_JXSubVtx ? topoN-1 : topoN-2;
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
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer.cptr(), m_refitPV ? refPvContainer.ptr() : 0, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info, topoN-1, Mass_Moth, vtx));

      for(size_t i=0; i<topoN; i++) {
        VtxWriteHandles[i].ptr()->push_back(cascadeVertices[i]);
      }

      // Set links to cascade vertices
      VertexLinkVector precedingVertexLinks;
      VertexLink vertexLink1;
      vertexLink1.setElement(cascadeVertices[0]);
      vertexLink1.setStorableObject(*VtxWriteHandles[0].ptr());
      if( vertexLink1.isValid() ) precedingVertexLinks.push_back( vertexLink1 );
      VertexLink vertexLink2;
      vertexLink2.setElement(cascadeVertices[1]);
      vertexLink2.setStorableObject(*VtxWriteHandles[1].ptr());
      if( vertexLink2.isValid() ) precedingVertexLinks.push_back( vertexLink2 );
      if(topoN==4) {
	VertexLink vertexLink3;
	vertexLink3.setElement(cascadeVertices[2]);
	vertexLink3.setStorableObject(*VtxWriteHandles[2].ptr());
	if( vertexLink3.isValid() ) precedingVertexLinks.push_back( vertexLink3 );
      }
      CascadeLinksDecor(*mainVertex) = precedingVertexLinks;
    } // loop over cascadeinfoContainer

    // Deleting cascadeinfo since this won't be stored.
    // Vertices have been kept in m_cascadeOutputs and should be owned by their container
    for (auto cascade_info : cascadeinfoContainer) delete cascade_info;

    return StatusCode::SUCCESS;
  }

  Trk::VxCascadeInfo* JpsiXPlus2V0::fitMainVtx(const xAOD::Vertex* JXvtx, std::vector<double>& massesJX, const xAOD::Vertex* V01vtx, const V0Enum V01, const xAOD::Vertex* V02vtx, const V0Enum V02, const xAOD::TrackParticleContainer* trackContainer) const {
    Trk::VxCascadeInfo* result(nullptr);

    std::vector<const xAOD::TrackParticle*> tracksJX;
    for(size_t i=0; i<JXvtx->nTrackParticles(); i++) tracksJX.push_back(JXvtx->trackParticle(i));
    if (tracksJX.size() != massesJX.size()) {
      ATH_MSG_ERROR("Problems with JX input: number of tracks or track mass inputs is not correct!");
      return result;
    }

    // Check identical tracks in input
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V01vtx->trackParticle(0)) != tracksJX.cend()) return result;
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V01vtx->trackParticle(1)) != tracksJX.cend()) return result;
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V02vtx->trackParticle(0)) != tracksJX.cend()) return result;
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V02vtx->trackParticle(1)) != tracksJX.cend()) return result;

    std::vector<const xAOD::TrackParticle*> tracksV01;
    for(size_t j=0; j<V01vtx->nTrackParticles(); j++) tracksV01.push_back(V01vtx->trackParticle(j));
    if(std::find(tracksV01.cbegin(), tracksV01.cend(), V02vtx->trackParticle(0)) != tracksV01.cend()) return result;
    if(std::find(tracksV01.cbegin(), tracksV01.cend(), V02vtx->trackParticle(1)) != tracksV01.cend()) return result;
    std::vector<const xAOD::TrackParticle*> tracksV02;
    for(size_t j=0; j<V02vtx->nTrackParticles(); j++) tracksV02.push_back(V02vtx->trackParticle(j));

    std::vector<double> massesV01;
    if(V01==LAMBDA)         massesV01 = std::vector<double>{m_mass_proton,m_mass_pion};
    else if(V01==LAMBDABAR) massesV01 = std::vector<double>{m_mass_pion,m_mass_proton};
    else if(V01==KS)        massesV01 = std::vector<double>{m_mass_pion,m_mass_pion};
    std::vector<double> massesV02;
    if(V02==LAMBDA)         massesV02 = std::vector<double>{m_mass_proton,m_mass_pion};
    else if(V02==LAMBDABAR) massesV02 = std::vector<double>{m_mass_pion,m_mass_proton};
    else if(V02==KS)        massesV02 = std::vector<double>{m_mass_pion,m_mass_pion};

    TLorentzVector p4_moth, tmp;
    for(size_t it=0; it<JXvtx->nTrackParticles(); it++) {
      tmp.SetPtEtaPhiM(JXvtx->trackParticle(it)->pt(), JXvtx->trackParticle(it)->eta(), JXvtx->trackParticle(it)->phi(), massesJX[it]);
      p4_moth += tmp;
    }
    xAOD::BPhysHelper V01_helper(V01vtx);
    for(int it=0; it<V01_helper.nRefTrks(); it++) {
      p4_moth += V01_helper.refTrk(it,massesV01[it]);
    }
    xAOD::BPhysHelper V02_helper(V02vtx);
    for(int it=0; it<V02_helper.nRefTrks(); it++) {
      p4_moth += V02_helper.refTrk(it,massesV02[it]);
    }
    if (p4_moth.M() < m_MassLower || p4_moth.M() > m_MassUpper) return result;

    std::vector<const xAOD::TrackParticle*> tracksJpsi;
    tracksJpsi.push_back(tracksJX[0]);
    tracksJpsi.push_back(tracksJX[1]);
    std::vector<const xAOD::TrackParticle*> tracksX;
    if(m_jxDaug_num>=3) tracksX.push_back(tracksJX[2]);
    if(m_jxDaug_num==4) tracksX.push_back(tracksJX[3]);

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
      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01,*state,m_V01Hypothesis=="Ks" ? m_mass_Ks : m_mass_Lambda);
    } else {
      vID1 = m_iVertexFitter->startVertex(tracksV01,massesV01,*state);
    }
    vrtList.push_back(vID1);
    // V02 vertex
    Trk::VertexID vID2;
    if (m_constrV02) {
      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02,*state,m_V02Hypothesis=="Ks" ? m_mass_Ks : m_mass_Lambda);
    } else {
      vID2 = m_iVertexFitter->nextVertex(tracksV02,massesV02,*state);
    }
    vrtList.push_back(vID2);
    Trk::VertexID vID3;
    if(m_JXSubVtx) {
      if(m_JXV02SubVtx) {
	// JX+V02 vertex
	std::vector<Trk::VertexID> vrtList1{vID1};
	std::vector<Trk::VertexID> vrtList2{vID2};
	if (m_constrJXV02) {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,*state,m_massJXV02);
	} else {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,*state);
	}
	vrtList1.push_back(vID3);
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
    std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

    if (fit_result != nullptr) {
      for(auto v : fit_result->vertices()) {
	if(v->nTrackParticles()==0) {
	  std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
	  v->setTrackParticleLinks(nullLinkVector);
	}
      }
      // reset links to original tracks
      BPhysPVCascadeTools::PrepareVertexLinks(fit_result.get(), trackContainer);

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
	SG::AuxElement::Accessor< std::vector<float> > trk_pxAcc("TrackPx_V0nc");
	SG::AuxElement::Accessor< std::vector<float> > trk_pyAcc("TrackPy_V0nc");
	SG::AuxElement::Accessor< std::vector<float> > trk_pzAcc("TrackPz_V0nc");

	SG::AuxElement::Decorator<int>   mDec_gfit("gamma_fit");
	SG::AuxElement::Decorator<float> mDec_gmass("gamma_mass");
	SG::AuxElement::Decorator<float> mDec_gmasserr("gamma_massError");
	SG::AuxElement::Decorator<float> mDec_gchisq("gamma_chisq");
	SG::AuxElement::Decorator<int>   mDec_gndof("gamma_ndof");
	SG::AuxElement::Decorator<float> mDec_gprob("gamma_probability");
	SG::AuxElement::Decorator< std::vector<float> > trk_pxDeco("TrackPx_V0nc");
	SG::AuxElement::Decorator< std::vector<float> > trk_pyDeco("TrackPy_V0nc");
	SG::AuxElement::Decorator< std::vector<float> > trk_pzDeco("TrackPz_V0nc");

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
	if(trk_pxAcc.isAvailable(*V01vtx)) trk_px = trk_pxAcc(*V01vtx);
	if(trk_pyAcc.isAvailable(*V01vtx)) trk_py = trk_pyAcc(*V01vtx);
	if(trk_pzAcc.isAvailable(*V01vtx)) trk_pz = trk_pzAcc(*V01vtx);
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
	if(trk_pxAcc.isAvailable(*V02vtx)) trk_px = trk_pxAcc(*V02vtx);
	if(trk_pyAcc.isAvailable(*V02vtx)) trk_py = trk_pyAcc(*V02vtx);
	if(trk_pzAcc.isAvailable(*V02vtx)) trk_pz = trk_pzAcc(*V02vtx);
	trk_pxDeco(*cascadeVertices[1]) = trk_px;
	trk_pyDeco(*cascadeVertices[1]) = trk_py;
	trk_pzDeco(*cascadeVertices[1]) = trk_pz;

	result = fit_result.release();
      }
    }

    return result;
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

  template<size_t NTracks>
  const xAOD::Vertex* JpsiXPlus2V0::FindVertex(std::vector<const xAOD::VertexContainer*> containers, const xAOD::Vertex* v) const {
    for (const xAOD::VertexContainer* cont : containers) {
      const xAOD::Vertex* vtx = FindVertex<NTracks>(cont, v);
      if(vtx) return vtx;
    }
    return nullptr;
  }
}
