/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#include "JpsiXPlusDisplaced.h"
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

  JpsiXPlusDisplaced::MesonCandidateVector::MesonCandidateVector(size_t num, bool orderByPt = true) {
    m_num = num;
    m_orderByPt = orderByPt;
  }

  void JpsiXPlusDisplaced::MesonCandidateVector::push_back(const MesonCandidate& etac) {
    if(m_num>0 && m_vector.size()>=m_num) {
      if(m_orderByPt && etac.pt<=m_vector.back().pt) return;
      else if(!m_orderByPt && etac.chi2NDF>=m_vector.back().chi2NDF) return;
    }
    auto pos = m_vector.cend();
    for(auto iter=m_vector.cbegin(); iter!=m_vector.cend(); iter++) {
      if(m_orderByPt) {
        if(etac.pt>iter->pt) { pos = iter; break; }
      }
      else {
        if(etac.chi2NDF<iter->chi2NDF) { pos = iter; break; }
      }
    }
    m_vector.insert(pos, etac);
    if(m_num>0 && m_vector.size()>m_num) m_vector.pop_back();
  }

  const std::vector<JpsiXPlusDisplaced::MesonCandidate>& JpsiXPlusDisplaced::MesonCandidateVector::vector() const {
    return m_vector;
  }

  JpsiXPlusDisplaced::JpsiXPlusDisplaced(const std::string& type, const std::string& name, const IInterface* parent) : base_class(type,name,parent),
    m_vertexJXContainerKey("InputJXVertices"),
    m_vertexV0ContainerKey(""),
    m_cascadeOutputKeys({"JpsiXPlusDisVtx1_sub", "JpsiXPlusDisVtx1", "JpsiXPlusDisVtx2", "JpsiXPlusDisVtx3"}),
    m_cascadeOutputKeys_mvc({"JpsiXPlusDisVtx1_sub_mvc", "JpsiXPlusDisVtx1_mvc", "JpsiXPlusDisVtx2_mvc", "JpsiXPlusDisVtx3_mvc"}),
    m_v0VtxOutputKey(""),
    m_TrkParticleCollection("InDetTrackParticles"),
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_refPVContainerName("RefittedPrimaryVertices"),
    m_eventInfo_key("EventInfo"),
    m_RelinkContainers({"InDetTrackParticles","InDetLargeD0TrackParticles"}),
    m_useImprovedMass(false),
    m_jxMassLower(0.0),
    m_jxMassUpper(10000.0),
    m_jpsiMassLower(0.0),
    m_jpsiMassUpper(10000.0),
    m_diTrackMassLower(-1.0),
    m_diTrackMassUpper(-1.0),
    m_V0Hypothesis("Lambda"),
    m_LambdaMassLower(0.0),
    m_LambdaMassUpper(10000.0),
    m_KsMassLower(0.0),
    m_KsMassUpper(10000.0),
    m_lxyV0_cut(-999.0),
    m_minMass_gamma(-1.0),
    m_chi2cut_gamma(-1.0),
    m_DisplacedMassLower(0.0),
    m_DisplacedMassUpper(10000.0),
    m_lxyDisV_cut(-999.0),
    m_lxyDpm_cut(-999.0),
    m_lxyD0_cut(-999.0),
    m_MassLower(0.0),
    m_MassUpper(31000.0),
    m_PostMassLower(0.0),
    m_PostMassUpper(31000.0),
    m_jxDaug_num(4),
    m_jxDaug1MassHypo(-1),
    m_jxDaug2MassHypo(-1),
    m_jxDaug3MassHypo(-1),
    m_jxDaug4MassHypo(-1),
    m_jxPtOrdering(false),
    m_disVDaug_num(3),
    m_disVDaug3MassHypo(-1),
    m_disVDaug3MinPt(480),
    m_extraTrk1MassHypo(-1),
    m_extraTrk1MinPt(480),
    m_extraTrk2MassHypo(-1),
    m_extraTrk2MinPt(480),
    m_extraTrk3MassHypo(-1),
    m_extraTrk3MinPt(480),
    m_DpmMassLower(0.0),
    m_DpmMassUpper(10000.0),
    m_D0MassLower(0.0),
    m_D0MassUpper(10000.0),
    m_maxMesonCandidates(400),
    m_MesonPtOrdering(true),
    m_massJX(-1),
    m_massJpsi(-1),
    m_massX(-1),
    m_massDisV(-1),
    m_massLd(-1),
    m_massKs(-1),
    m_massDpm(-1),
    m_massD0(-1),
    m_massJXV0(-1),
    m_massMainV(-1),
    m_constrJX(false),
    m_constrJpsi(false),
    m_constrX(false),
    m_constrDisV(false),
    m_constrV0(false),
    m_constrDpm(false),
    m_constrD0(false),
    m_constrJXV0(false),
    m_constrMainV(false),
    m_doPostMainVContrFit(false),
    m_JXSubVtx(false),
    m_JXV0SubVtx(false),
    m_chi2cut_JX(-1.0),
    m_chi2cut_V0(-1.0),
    m_chi2cut_DisV(-1.0),
    m_chi2cut_Dpm(-1.0),
    m_chi2cut_D0(-1.0),
    m_chi2cut(-1.0),
    m_useTRT(false),
    m_ptTRT(450),
    m_d0_cut(2),
    m_maxJXCandidates(0),
    m_maxV0Candidates(0),
    m_maxDisVCandidates(0),
    m_maxMainVCandidates(0),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
    m_iV0Fitter("Trk::V0VertexFitter"),
    m_iGammaFitter("Trk::TrkVKalVrtFitter"),
    m_pvRefitter("Analysis::PrimaryVertexRefitter", this),
    m_V0Tools("Trk::V0Tools"),
    m_trackToVertexTool("Reco::TrackToVertex"),
    m_trkSelector("InDet::TrackSelectorTool"),
    m_v0TrkSelector("InDet::TrackSelectorTool"),
    m_CascadeTools("DerivationFramework::CascadeTools"),
    m_vertexEstimator("InDet::VertexPointEstimator"),
    m_extrapolator("Trk::Extrapolator/AtlasExtrapolator")
  {
    declareProperty("JXVertices",                 m_vertexJXContainerKey);
    declareProperty("V0Vertices",                 m_vertexV0ContainerKey);
    declareProperty("JXVtxHypoNames",             m_vertexJXHypoNames);
    declareProperty("CascadeVertexCollections",   m_cascadeOutputKeys); // size is 3 or 4 only
    declareProperty("CascadeVertexCollectionsMVC",m_cascadeOutputKeys_mvc); // size is 3 or 4 only
    declareProperty("OutputV0VtxCollection",      m_v0VtxOutputKey);
    declareProperty("TrackParticleCollection",    m_TrkParticleCollection);
    declareProperty("VxPrimaryCandidateName",     m_VxPrimaryCandidateName);
    declareProperty("RefPVContainerName",         m_refPVContainerName);
    declareProperty("EventInfoKey",               m_eventInfo_key);
    declareProperty("RelinkTracks",               m_RelinkContainers);
    declareProperty("UseImprovedMass",            m_useImprovedMass);
    declareProperty("JXMassLowerCut",             m_jxMassLower); // only effective when m_jxDaug_num>2
    declareProperty("JXMassUpperCut",             m_jxMassUpper); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMassLowerCut",           m_jpsiMassLower);
    declareProperty("JpsiMassUpperCut",           m_jpsiMassUpper);
    declareProperty("DiTrackMassLower",           m_diTrackMassLower); // only effective when m_jxDaug_num=4
    declareProperty("DiTrackMassUpper",           m_diTrackMassUpper); // only effective when m_jxDaug_num=4
    declareProperty("V0Hypothesis",               m_V0Hypothesis); // "Ks" or "Lambda"
    declareProperty("LambdaMassLowerCut",         m_LambdaMassLower);
    declareProperty("LambdaMassUpperCut",         m_LambdaMassUpper);
    declareProperty("KsMassLowerCut",             m_KsMassLower);
    declareProperty("KsMassUpperCut",             m_KsMassUpper);
    declareProperty("LxyV0Cut",                   m_lxyV0_cut);
    declareProperty("MassCutGamma",               m_minMass_gamma);
    declareProperty("Chi2CutGamma",               m_chi2cut_gamma);
    declareProperty("DisplacedMassLowerCut",      m_DisplacedMassLower); // only effective when m_disVDaug_num=3
    declareProperty("DisplacedMassUpperCut",      m_DisplacedMassUpper); // only effective when m_disVDaug_num=3
    declareProperty("LxyDisVtxCut",               m_lxyDisV_cut); // only effective when m_disVDaug_num=3
    declareProperty("LxyDpmCut",                  m_lxyDpm_cut); // only effective for D+/-
    declareProperty("LxyD0Cut",                   m_lxyD0_cut); // only effective for D0
    declareProperty("MassLowerCut",               m_MassLower);
    declareProperty("MassUpperCut",               m_MassUpper);
    declareProperty("PostMassLowerCut",           m_PostMassLower); // only effective when m_doPostMainVContrFit=true
    declareProperty("PostMassUpperCut",           m_PostMassUpper); // only effective when m_doPostMainVContrFit=true
    declareProperty("HypothesisName",             m_hypoName = "TQ");
    declareProperty("NumberOfJXDaughters",        m_jxDaug_num); // 2, or 3, or 4 only
    declareProperty("JXDaug1MassHypo",            m_jxDaug1MassHypo);
    declareProperty("JXDaug2MassHypo",            m_jxDaug2MassHypo);
    declareProperty("JXDaug3MassHypo",            m_jxDaug3MassHypo);
    declareProperty("JXDaug4MassHypo",            m_jxDaug4MassHypo);
    declareProperty("JXPtOrdering",               m_jxPtOrdering);
    declareProperty("NumberOfDisVDaughters",      m_disVDaug_num); // 2 or 3 only
    declareProperty("DisVDaug3MassHypo",          m_disVDaug3MassHypo); // only effective when m_disVDaug_num=3
    declareProperty("DisVDaug3MinPt",             m_disVDaug3MinPt); // only effective when m_disVDaug_num=3
    declareProperty("ExtraTrack1MassHypo",        m_extraTrk1MassHypo); // for decays like B- -> J/psi Lambda pbar, the extra track is pbar (for m_disVDaug_num=2 only now)
    declareProperty("ExtraTrack1MinPt",           m_extraTrk1MinPt); // only effective if m_extraTrk1MassHypo>0
    declareProperty("ExtraTrack2MassHypo",        m_extraTrk2MassHypo); // for decays like Xi_bc^0 -> Jpsi Lambda D0(->Kpi) (for m_disVDaug_num=2 only now)
    declareProperty("ExtraTrack2MinPt",           m_extraTrk2MinPt); // only effective if m_extraTrk2MassHypo>0
    declareProperty("ExtraTrack3MassHypo",        m_extraTrk3MassHypo); // for decays like Bc+ -> Jpsi D+ Ks (for m_disVDaug_num=2 only now)
    declareProperty("ExtraTrack3MinPt",           m_extraTrk3MinPt); // only effective if m_extraTrk3MassHypo>0
    declareProperty("DpmMassLowerCut",            m_DpmMassLower); // only for D+/-
    declareProperty("DpmMassUpperCut",            m_DpmMassUpper); // only for D+/-
    declareProperty("D0MassLowerCut",             m_D0MassLower); // only for D0
    declareProperty("D0MassUpperCut",             m_D0MassUpper); // only for D0
    declareProperty("MaxMesonCandidates",         m_maxMesonCandidates); // only for 2/3 extra tracks
    declareProperty("MesonPtOrdering",            m_MesonPtOrdering); // only for 2/3 extra tracks
    declareProperty("JXMass",                     m_massJX); // only effective when m_jxDaug_num>2
    declareProperty("JpsiMass",                   m_massJpsi);
    declareProperty("XMass",                      m_massX); // only effective when m_jxDaug_num=4
    declareProperty("DisVtxMass",                 m_massDisV); // only effective when m_disVDaug_num=3
    declareProperty("LambdaMass",                 m_massLd);
    declareProperty("KsMass",                     m_massKs);
    declareProperty("DpmMass",                    m_massDpm);
    declareProperty("D0Mass",                     m_massD0);
    declareProperty("JXV0Mass",                   m_massJXV0);
    declareProperty("MainVtxMass",                m_massMainV);
    declareProperty("ApplyJXMassConstraint",      m_constrJX); // only effective when m_jxDaug_num>2
    declareProperty("ApplyJpsiMassConstraint",    m_constrJpsi);
    declareProperty("ApplyXMassConstraint",       m_constrX); // only effective when m_jxDaug_num=4
    declareProperty("ApplyDisVMassConstraint",    m_constrDisV); // only effective when m_disVDaug_num=3
    declareProperty("ApplyV0MassConstraint",      m_constrV0);
    declareProperty("ApplyDpmMassConstraint",     m_constrDpm); // only for D+/-
    declareProperty("ApplyD0MassConstraint",      m_constrD0); // only for D0
    declareProperty("ApplyJXV0MassConstraint",    m_constrJXV0);
    declareProperty("ApplyMainVMassConstraint",   m_constrMainV);
    declareProperty("DoPostMainVContrFit",        m_doPostMainVContrFit); // only effective when m_constrMainV=false
    declareProperty("HasJXSubVertex",             m_JXSubVtx);
    declareProperty("HasJXV0SubVertex",           m_JXV0SubVtx);
    declareProperty("Chi2CutJX",                  m_chi2cut_JX);
    declareProperty("Chi2CutV0",                  m_chi2cut_V0);
    declareProperty("Chi2CutDisV",                m_chi2cut_DisV); // only effective when m_disVDaug_num=3
    declareProperty("Chi2CutDpm",                 m_chi2cut_Dpm); // only for D+/-
    declareProperty("Chi2CutD0",                  m_chi2cut_D0); // only for D0
    declareProperty("Chi2Cut",                    m_chi2cut);
    declareProperty("UseTRT",                     m_useTRT);
    declareProperty("PtTRT",                      m_ptTRT);
    declareProperty("Trackd0Cut",                 m_d0_cut);
    declareProperty("MaxJXCandidates",            m_maxJXCandidates);
    declareProperty("MaxV0Candidates",            m_maxV0Candidates);
    declareProperty("MaxDisVCandidates",          m_maxDisVCandidates); // only effective when m_disVDaug_num=3
    declareProperty("MaxMainVCandidates",         m_maxMainVCandidates);
    declareProperty("RefitPV",                    m_refitPV         = true);
    declareProperty("MaxnPV",                     m_PV_max          = 1000);
    declareProperty("MinNTracksInPV",             m_PV_minNTracks   = 0);
    declareProperty("DoVertexType",               m_DoVertexType    = 7);
    declareProperty("TrkVertexFitterTool",        m_iVertexFitter);
    declareProperty("V0VertexFitterTool",         m_iV0Fitter);
    declareProperty("GammaFitterTool",            m_iGammaFitter);
    declareProperty("PVRefitter",                 m_pvRefitter);
    declareProperty("V0Tools",                    m_V0Tools);
    declareProperty("TrackToVertexTool",          m_trackToVertexTool);
    declareProperty("TrackSelectorTool",          m_trkSelector);
    declareProperty("V0TrackSelectorTool",        m_v0TrkSelector);
    declareProperty("CascadeTools",               m_CascadeTools);
    declareProperty("VertexPointEstimator",       m_vertexEstimator);
    declareProperty("Extrapolator",               m_extrapolator);
  }

  StatusCode JpsiXPlusDisplaced::initialize() {
    if(m_V0Hypothesis != "Ks" && m_V0Hypothesis != "Lambda"
       && m_V0Hypothesis != "Lambda/Ks"&& m_V0Hypothesis != "Ks/Lambda") {
      ATH_MSG_FATAL("Incorrect V0 container hypothesis - not recognized");
      return StatusCode::FAILURE;
    }

    if(m_jxDaug_num<2 || m_jxDaug_num>4 || m_disVDaug_num<2 || m_disVDaug_num>3) {
      ATH_MSG_FATAL("Incorrect number of JX or DisVtx daughters");
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

    // retrieving the track selector tool
    ATH_CHECK( m_trkSelector.retrieve() );

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
    ATH_CHECK( m_refPVContainerName.initialize() );
    ATH_CHECK( m_cascadeOutputKeys.initialize() );
    ATH_CHECK( m_cascadeOutputKeys_mvc.initialize() );
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
    m_mass_Ks = BPhysPVCascadeTools::getParticleMass(pdt, MC::K0S);
    m_mass_Xi = BPhysPVCascadeTools::getParticleMass(pdt, 3312);
    m_mass_phi = BPhysPVCascadeTools::getParticleMass(pdt, 333);
    m_mass_B0 = BPhysPVCascadeTools::getParticleMass(pdt, MC::B0);
    m_mass_Dpm = BPhysPVCascadeTools::getParticleMass(pdt, MC::DPLUS);
    m_mass_D0 = BPhysPVCascadeTools::getParticleMass(pdt, MC::D0);
    m_mass_BCPLUS = BPhysPVCascadeTools::getParticleMass(pdt, MC::BCPLUS);
    m_mass_Lambdab = BPhysPVCascadeTools::getParticleMass(pdt, MC::LAMBDAB0);

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
    if(m_constrV0) {
      if(m_massLd<0) m_massLd = m_mass_Lambda;
      if(m_massKs<0) m_massKs = m_mass_Ks;
    }
    if(m_disVDaug_num==3 && m_constrDisV && m_massDisV<0) m_massDisV = m_mass_Xi;
    if(m_massMainV<0) m_massMainV = m_mass_BCPLUS;
    if(m_constrDpm && m_massDpm<0) m_massDpm = m_mass_Dpm;
    if(m_constrD0 && m_massD0<0) m_massD0 = m_mass_D0;
    if(m_constrJXV0 && m_massJXV0<0) m_massJXV0 = m_mass_Lambdab;

    if(m_jxDaug1MassHypo < 0.) m_jxDaug1MassHypo = m_mass_mu;
    if(m_jxDaug2MassHypo < 0.) m_jxDaug2MassHypo = m_mass_mu;
    if(m_jxDaug_num>=3 && m_jxDaug3MassHypo < 0.) m_jxDaug3MassHypo = m_mass_pion;
    if(m_jxDaug_num==4 && m_jxDaug4MassHypo < 0.) m_jxDaug4MassHypo = m_mass_pion;
    if(m_disVDaug_num==3 && m_disVDaug3MassHypo < 0.) m_disVDaug3MassHypo = m_mass_pion;

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlusDisplaced::performSearch(std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> >& cascadeinfoContainer, const std::vector<std::pair<const xAOD::Vertex*,V0Enum> >& selectedV0Candidates, const std::vector<const xAOD::TrackParticle*>& tracksDisplaced) const {
    ATH_MSG_DEBUG( "JpsiXPlusDisplaced::performSearch" );
    if(selectedV0Candidates.size()==0) return StatusCode::SUCCESS;
    
    // Get TrackParticle container (standard + LRT)
    SG::ReadHandle<xAOD::TrackParticleContainer> trackContainer(m_TrkParticleCollection);
    ATH_CHECK( trackContainer.isValid() );

    // Get all track containers when m_RelinkContainers is not empty
    std::vector<const xAOD::TrackParticleContainer*> trackCols;
    for(const SG::ReadHandleKey<xAOD::TrackParticleContainer>& key : m_RelinkContainers){
      SG::ReadHandle<xAOD::TrackParticleContainer> handle(key);
      ATH_CHECK( handle.isValid() );
      trackCols.push_back(handle.cptr());
    }

    // Get Jpsi+X container
    SG::ReadHandle<xAOD::VertexContainer> jxContainer(m_vertexJXContainerKey);
    ATH_CHECK( jxContainer.isValid() );

    std::vector<double> massesJX{m_jxDaug1MassHypo, m_jxDaug2MassHypo};
    if(m_jxDaug_num>=3) massesJX.push_back(m_jxDaug3MassHypo);
    if(m_jxDaug_num==4) massesJX.push_back(m_jxDaug4MassHypo);

    // Make the displaced candidates if needed
    std::vector<XiCandidate> disVtxContainer;
    if(m_disVDaug_num==3) {
      for(size_t it=0; it<selectedV0Candidates.size(); ++it) {
	std::pair<const xAOD::Vertex*,V0Enum> elem = selectedV0Candidates[it];
	std::vector<const xAOD::TrackParticle*> tracksV0;
	tracksV0.reserve(elem.first->nTrackParticles());
	for(size_t i=0; i<elem.first->nTrackParticles(); i++) tracksV0.push_back(elem.first->trackParticle(i));
	std::vector<double> massesV0;
	if(elem.second==LAMBDA)         massesV0 = m_massesV0_ppi;
	else if(elem.second==LAMBDABAR) massesV0 = m_massesV0_pip;
	else if(elem.second==KS)        massesV0 = m_massesV0_pipi;
	xAOD::BPhysHelper V0_helper(elem.first); TLorentzVector p4_v0;
	for(int i=0; i<V0_helper.nRefTrks(); i++) p4_v0 += V0_helper.refTrk(i,massesV0[i]);
	for(const xAOD::TrackParticle* TP : tracksDisplaced) {
	  if(TP->pt() < m_disVDaug3MinPt) continue;
	  // Check overlap
	  if(std::find(tracksV0.cbegin(), tracksV0.cend(), TP) != tracksV0.cend()) continue;
	  TLorentzVector tmp;
	  tmp.SetPtEtaPhiM(TP->pt(),TP->eta(),TP->phi(),m_disVDaug3MassHypo);
	  double disV_mass = (p4_v0+tmp).M();
	  if(m_useImprovedMass) {
	    if((elem.second==LAMBDA || elem.second==LAMBDABAR) && m_massLd>0) disV_mass += - p4_v0.M() + m_massLd;
	    else if(elem.second==KS && m_massKs>0) disV_mass += - p4_v0.M() + m_massKs;
	  }
	  // A rough mass window cut, as V0 and track3 are not from a common vertex
	  if(disV_mass > m_DisplacedMassLower-400. && disV_mass < m_DisplacedMassUpper+400.) {
	    auto disVtx = getXiCandidate(elem.first,elem.second,TP);
	    if(disVtx.V0vtx && disVtx.track) disVtxContainer.push_back(disVtx);
	  }
	}
      }

      std::sort( disVtxContainer.begin(), disVtxContainer.end(), [](const XiCandidate& a, const XiCandidate& b) { return a.chi2NDF < b.chi2NDF; } );
      if(m_maxDisVCandidates>0 && disVtxContainer.size()>m_maxDisVCandidates) {
	disVtxContainer.erase(disVtxContainer.begin()+m_maxDisVCandidates, disVtxContainer.end());
      }
      if(disVtxContainer.size()==0) return StatusCode::SUCCESS;
    } // m_disVDaug_num==3

    // Select the JX candidates before calling cascade fit
    std::vector<const xAOD::Vertex*> selectedJXCandidates;
    for(const xAOD::Vertex* vtx : *jxContainer.cptr()) {
      // Check the passed flag first
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

    if(m_jxPtOrdering) {
      std::sort( selectedJXCandidates.begin(), selectedJXCandidates.end(), [massesJX](const xAOD::Vertex* a, const xAOD::Vertex* b) {
	TLorentzVector p4_a, p4_b, tmp;
	for(size_t it=0; it<a->nTrackParticles(); it++) {
	  tmp.SetPtEtaPhiM(a->trackParticle(it)->pt(), a->trackParticle(it)->eta(), a->trackParticle(it)->phi(), massesJX[it]);
	  p4_a += tmp;
	}
	for(size_t it=0; it<b->nTrackParticles(); it++) {
	  tmp.SetPtEtaPhiM(b->trackParticle(it)->pt(), b->trackParticle(it)->eta(), b->trackParticle(it)->phi(), massesJX[it]);
	  p4_b += tmp;
	}
	return p4_a.Pt() > p4_b.Pt();
      } );
    }
    else {
      std::sort( selectedJXCandidates.begin(), selectedJXCandidates.end(), [](const xAOD::Vertex* a, const xAOD::Vertex* b) { return a->chiSquared()/a->numberDoF() < b->chiSquared()/b->numberDoF(); } );
    }
    if(m_maxJXCandidates>0 && selectedJXCandidates.size()>m_maxJXCandidates) {
      selectedJXCandidates.erase(selectedJXCandidates.begin()+m_maxJXCandidates, selectedJXCandidates.end());
    }

    // Select JX+DisV candidates
    // Iterate over JX vertices
    for(const xAOD::Vertex* jxVtx : selectedJXCandidates) {
      // Iterate over displaced vertices
      if(m_disVDaug_num==2) {
	for(auto&& V0Candidate : selectedV0Candidates) {
	  std::vector<std::pair<Trk::VxCascadeInfo*, Trk::VxCascadeInfo*> > result = fitMainVtx(jxVtx, massesJX, V0Candidate.first, V0Candidate.second, trackContainer.cptr(), trackCols);
	  for(auto cascade_info_pair : result) {
	    if(cascade_info_pair.first) cascadeinfoContainer.push_back(cascade_info_pair);
	  }
	}
      } // m_disVDaug_num==2
      else if(m_disVDaug_num==3) {
	for(auto&& disVtx : disVtxContainer) {
	  std::vector<std::pair<Trk::VxCascadeInfo*, Trk::VxCascadeInfo*> > result = fitMainVtx(jxVtx, massesJX, disVtx, trackContainer.cptr(), trackCols);
	  for(auto cascade_info_pair : result) {
	    if(cascade_info_pair.first) cascadeinfoContainer.push_back(cascade_info_pair);
	  }
	}
      } // m_disVDaug_num==3
    } // Iterate over JX vertices

    return StatusCode::SUCCESS;
  }

  StatusCode JpsiXPlusDisplaced::addBranches() const {
    size_t topoN = (m_disVDaug_num==2 ? 3 : 4);
    if(!m_JXSubVtx) topoN--;
    if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0) { // special cases
      if(m_JXV0SubVtx) topoN = 4;
      else topoN = 3;
    }
    else if(m_extraTrk1MassHypo>0 && m_disVDaug_num==2) { // special cases
      if(m_JXV0SubVtx || m_JXSubVtx) topoN = 3;
      else topoN = 2;
    }

    if(m_cascadeOutputKeys.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of output cascade vertices");
      return StatusCode::FAILURE;
    }
    if(!m_constrMainV && m_doPostMainVContrFit && m_cascadeOutputKeys_mvc.size() != topoN) {
      ATH_MSG_FATAL("Incorrect number of output (mvc) cascade vertices");
      return StatusCode::FAILURE;
    }

    std::array<SG::WriteHandle<xAOD::VertexContainer>, 4> VtxWriteHandles; int ikey(0);
    for(const SG::WriteHandleKey<xAOD::VertexContainer>& key : m_cascadeOutputKeys) {
      VtxWriteHandles[ikey] = SG::WriteHandle<xAOD::VertexContainer>(key);
      ATH_CHECK( VtxWriteHandles[ikey].record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
      ikey++;
    }
    std::array<SG::WriteHandle<xAOD::VertexContainer>, 4> VtxWriteHandles_mvc; int ikey_mvc(0);
    if(!m_constrMainV && m_doPostMainVContrFit) {
      for(const SG::WriteHandleKey<xAOD::VertexContainer>& key_mvc : m_cascadeOutputKeys_mvc) {
	VtxWriteHandles_mvc[ikey_mvc] = SG::WriteHandle<xAOD::VertexContainer>(key_mvc);
	ATH_CHECK( VtxWriteHandles_mvc[ikey_mvc].record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
	ikey_mvc++;
      }
    }

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    const xAOD::Vertex* primaryVertex(nullptr);
    SG::ReadHandle<xAOD::VertexContainer> pvContainer(m_VxPrimaryCandidateName);
    ATH_CHECK( pvContainer.isValid() );
    if (pvContainer.cptr()->size()==0) {
      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer.cptr()->size());
      return StatusCode::RECOVERABLE;
    }
    else primaryVertex = (*pvContainer.cptr())[0];

    //----------------------------------------------------
    // Record refitted primary vertices
    //----------------------------------------------------
    SG::WriteHandle<xAOD::VertexContainer> refPvContainer;
    if(m_refitPV) {
      refPvContainer = SG::WriteHandle<xAOD::VertexContainer>(m_refPVContainerName);
      ATH_CHECK( refPvContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
    }

    // Get TrackParticle container (standard + LRT)
    SG::ReadHandle<xAOD::TrackParticleContainer> trackContainer(m_TrkParticleCollection);
    ATH_CHECK( trackContainer.isValid() );

    // Get all track containers when m_RelinkContainers is not empty
    std::vector<const xAOD::TrackParticleContainer*> trackCols;
    for(const SG::ReadHandleKey<xAOD::TrackParticleContainer>& key : m_RelinkContainers){
      SG::ReadHandle<xAOD::TrackParticleContainer> handle(key);
      ATH_CHECK( handle.isValid() );
      trackCols.push_back(handle.cptr());
    }

    // output V0 vertices
    SG::WriteHandle<xAOD::VertexContainer> V0OutputContainer;
    if(m_vertexV0ContainerKey.key()=="" && m_v0VtxOutputKey.key()!="") {
      V0OutputContainer = SG::WriteHandle<xAOD::VertexContainer>(m_v0VtxOutputKey);
      ATH_CHECK( V0OutputContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()) );
    }

    // Get Jpsi+X container
    // Note: If the event does not contain a JX candidate, it is skipped and the V0 container will not be constructed if not previously available in StoreGate
    SG::ReadHandle<xAOD::VertexContainer> jxContainer(m_vertexJXContainerKey);
    ATH_CHECK( jxContainer.isValid() );
    if(jxContainer->size()==0) return StatusCode::SUCCESS;

    // Select the displaced tracks
    std::vector<const xAOD::TrackParticle*> tracksDisplaced;
    if(m_v0VtxOutputKey.key()!="" || m_disVDaug_num==3) {
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
      V0Container = SG::ReadHandle<xAOD::VertexContainer>(m_vertexV0ContainerKey);
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
	if((opt==LAMBDA || opt==LAMBDABAR) && m_V0Hypothesis == "Ks")  continue;
	if(opt==KS && m_V0Hypothesis == "Lambda") continue;

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
	if((opt==LAMBDA || opt==LAMBDABAR) && m_V0Hypothesis == "Ks")  continue;
	if(opt==KS && m_V0Hypothesis == "Lambda") continue;

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

    std::vector<std::pair<Trk::VxCascadeInfo*, Trk::VxCascadeInfo*> > cascadeinfoContainer;
    ATH_CHECK( performSearch(cascadeinfoContainer, selectedV0Candidates, tracksDisplaced) );

    // sort and chop the main candidates
    std::sort( cascadeinfoContainer.begin(), cascadeinfoContainer.end(), [](std::pair<Trk::VxCascadeInfo*, Trk::VxCascadeInfo*> a, std::pair<Trk::VxCascadeInfo*, Trk::VxCascadeInfo*> b) { return a.first->fitChi2()/a.first->nDoF() < b.first->fitChi2()/b.first->nDoF(); } );
    if(m_maxMainVCandidates>0 && cascadeinfoContainer.size()>m_maxMainVCandidates) {
      for(auto it=cascadeinfoContainer.begin()+m_maxMainVCandidates; it!=cascadeinfoContainer.end(); it++) {
	if(it->first) delete it->first;
	if(it->second) delete it->second;
      }
      cascadeinfoContainer.erase(cascadeinfoContainer.begin()+m_maxMainVCandidates, cascadeinfoContainer.end());
    }
    if(cascadeinfoContainer.size()==0) return StatusCode::SUCCESS;

    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key);
    ATH_CHECK( evt.isValid() );
    BPhysPVCascadeTools helper(&(*m_CascadeTools), evt.cptr());
    helper.SetMinNTracksInPV(m_PV_minNTracks);

    // Decorators for the cascade vertices
    SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
    SG::AuxElement::Decorator<VertexLinkVector> PrecedingLinksDecor("PrecedingVertexLinks");
    SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
    SG::AuxElement::Decorator<int>   ndof_decor("nDoF");
    SG::AuxElement::Decorator<float> Pt_decor("Pt");
    SG::AuxElement::Decorator<float> PtErr_decor("PtErr");

    SG::AuxElement::Decorator<float> lxy_SV0_decor("lxy_SV0");
    SG::AuxElement::Decorator<float> lxyErr_SV0_decor("lxyErr_SV0");
    SG::AuxElement::Decorator<float> a0xy_SV0_decor("a0xy_SV0");
    SG::AuxElement::Decorator<float> a0xyErr_SV0_decor("a0xyErr_SV0");
    SG::AuxElement::Decorator<float> a0z_SV0_decor("a0z_SV0");
    SG::AuxElement::Decorator<float> a0zErr_SV0_decor("a0zErr_SV0");

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

    SG::AuxElement::Decorator<float> chi2_V2_decor("ChiSquared_V2");
    SG::AuxElement::Decorator<int>   ndof_V2_decor("nDoF_V2");

    for(auto cascade_info_pair : cascadeinfoContainer) {
      if(cascade_info_pair.first==nullptr) {
	ATH_MSG_ERROR("CascadeInfo is null");
	continue;
      }

      const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info_pair.first->vertices();
      if(cascadeVertices.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices");
      for(size_t i=0; i<topoN; i++) {
	if(cascadeVertices[i]==nullptr) ATH_MSG_ERROR("Error null vertex");
      }

      cascade_info_pair.first->setSVOwnership(false); // Prevent Container from deleting vertices
      const auto mainVertex = cascadeVertices[topoN-1]; // this is the mother vertex
      const std::vector< std::vector<TLorentzVector> > &moms = cascade_info_pair.first->getParticleMoms();

      // Identify the input JX
      int ijx = m_JXSubVtx ? topoN-2 : topoN-1;
      if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0) {
	if(m_JXV0SubVtx) ijx = 1;
	else ijx = topoN-1;
      }
      else if(m_extraTrk1MassHypo>0 && m_disVDaug_num==2) {
	if(m_JXV0SubVtx || m_JXSubVtx) ijx = 1;
	else ijx = topoN-1;
      }
      const xAOD::Vertex* jxVtx(nullptr);
      if(m_jxDaug_num==4) jxVtx = FindVertex<4>(jxContainer.ptr(), cascadeVertices[ijx]);
      else if(m_jxDaug_num==3) jxVtx = FindVertex<3>(jxContainer.ptr(), cascadeVertices[ijx]);
      else jxVtx = FindVertex<2>(jxContainer.ptr(), cascadeVertices[ijx]);

      xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);

      // Get refitted track momenta from all vertices, charged tracks only
      BPhysPVCascadeTools::SetVectorInfo(vtx, cascade_info_pair.first);
      vtx.setPass(true);

      //
      // Decorate main vertex
      //
      // mass, mass error
      // https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/VxCascadeInfo.h
      BPHYS_CHECK( vtx.setMass(m_CascadeTools->invariantMass(moms[topoN-1])) );
      BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[topoN-1],cascade_info_pair.first->getCovariance()[topoN-1])) );
      // pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
      Pt_decor(*mainVertex)       = m_CascadeTools->pT(moms[topoN-1]);
      PtErr_decor(*mainVertex)    = m_CascadeTools->pTError(moms[topoN-1],cascade_info_pair.first->getCovariance()[topoN-1]);
      // chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
      chi2_decor(*mainVertex)     = cascade_info_pair.first->fitChi2();
      ndof_decor(*mainVertex)     = cascade_info_pair.first->nDoF();

      if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0) { // special cases
	if(m_JXV0SubVtx) {
	  lxy_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  lxyErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  a0z_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  a0zErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  a0xy_SV0_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  a0xyErr_SV0_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  lxy_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	  lxyErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0z_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	  a0zErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0xy_SV1_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	  a0xyErr_SV1_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  lxy_SV2_decor(*cascadeVertices[2])     = m_CascadeTools->lxy(moms[2],cascadeVertices[2],mainVertex);
	  lxyErr_SV2_decor(*cascadeVertices[2])  = m_CascadeTools->lxyError(moms[2],cascade_info_pair.first->getCovariance()[2],cascadeVertices[2],mainVertex);
	  a0z_SV2_decor(*cascadeVertices[2])     = m_CascadeTools->a0z(moms[2],cascadeVertices[2],mainVertex);
	  a0zErr_SV2_decor(*cascadeVertices[2])  = m_CascadeTools->a0zError(moms[2],cascade_info_pair.first->getCovariance()[2],cascadeVertices[2],mainVertex);
	  a0xy_SV2_decor(*cascadeVertices[2])    = m_CascadeTools->a0xy(moms[2],cascadeVertices[2],mainVertex);
	  a0xyErr_SV2_decor(*cascadeVertices[2]) = m_CascadeTools->a0xyError(moms[2],cascade_info_pair.first->getCovariance()[2],cascadeVertices[2],mainVertex);
	}
	else {
	  lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
	  lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
	  a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
	  a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	  lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	  a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	  a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	}
      }
      else if(m_extraTrk1MassHypo>0 && m_disVDaug_num==2) { // special cases
	if(m_JXV0SubVtx) {
	  lxy_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  lxyErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  a0z_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  a0zErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  a0xy_SV0_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  a0xyErr_SV0_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  lxy_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	  lxyErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0z_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	  a0zErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0xy_SV1_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	  a0xyErr_SV1_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	}
	else {
	  lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
	  lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
	  a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
	  a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  if(m_JXSubVtx) {
	    lxy_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	    lxyErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	    a0z_SV2_decor(*cascadeVertices[1])     = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	    a0zErr_SV2_decor(*cascadeVertices[1])  = m_CascadeTools->a0zError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	    a0xy_SV2_decor(*cascadeVertices[1])    = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	    a0xyErr_SV2_decor(*cascadeVertices[1]) = m_CascadeTools->a0xyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  }
	}
      }
      else { // other normal cases
	if(m_disVDaug_num==2) {
	  lxy_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],mainVertex);
	  lxyErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  a0z_SV1_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],mainVertex);
	  a0zErr_SV1_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	  a0xy_SV1_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],mainVertex);
	  a0xyErr_SV1_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],mainVertex);
	}
	else {
	  lxy_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  lxyErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->lxyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  a0z_SV0_decor(*cascadeVertices[0])     = m_CascadeTools->a0z(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  a0zErr_SV0_decor(*cascadeVertices[0])  = m_CascadeTools->a0zError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  a0xy_SV0_decor(*cascadeVertices[0])    = m_CascadeTools->a0xy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  a0xyErr_SV0_decor(*cascadeVertices[0]) = m_CascadeTools->a0xyError(moms[0],cascade_info_pair.first->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
	  lxy_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->lxy(moms[1],cascadeVertices[1],mainVertex);
	  lxyErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->lxyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0xy_SV1_decor(*cascadeVertices[1])    = m_CascadeTools->a0z(moms[1],cascadeVertices[1],mainVertex);
	  a0xyErr_SV1_decor(*cascadeVertices[1]) = m_CascadeTools->a0zError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	  a0z_SV1_decor(*cascadeVertices[1])     = m_CascadeTools->a0xy(moms[1],cascadeVertices[1],mainVertex);
	  a0zErr_SV1_decor(*cascadeVertices[1])  = m_CascadeTools->a0xyError(moms[1],cascade_info_pair.first->getCovariance()[1],cascadeVertices[1],mainVertex);
	}

	if(m_JXSubVtx) {
	  lxy_SV2_decor(*cascadeVertices[ijx])     = m_CascadeTools->lxy(moms[ijx],cascadeVertices[ijx],mainVertex);
	  lxyErr_SV2_decor(*cascadeVertices[ijx])  = m_CascadeTools->lxyError(moms[ijx],cascade_info_pair.first->getCovariance()[ijx],cascadeVertices[ijx],mainVertex);
	  a0z_SV2_decor(*cascadeVertices[ijx])     = m_CascadeTools->a0z(moms[ijx],cascadeVertices[ijx],mainVertex);
	  a0zErr_SV2_decor(*cascadeVertices[ijx])  = m_CascadeTools->a0zError(moms[ijx],cascade_info_pair.first->getCovariance()[ijx],cascadeVertices[ijx],mainVertex);
	  a0xy_SV2_decor(*cascadeVertices[ijx])    = m_CascadeTools->a0xy(moms[ijx],cascadeVertices[ijx],mainVertex);
	  a0xyErr_SV2_decor(*cascadeVertices[ijx]) = m_CascadeTools->a0xyError(moms[ijx],cascade_info_pair.first->getCovariance()[ijx],cascadeVertices[ijx],mainVertex);
	}
      }

      chi2_V2_decor(*cascadeVertices[ijx])     = m_V0Tools->chisq(jxVtx);
      ndof_V2_decor(*cascadeVertices[ijx])     = m_V0Tools->ndof(jxVtx);
      
      double Mass_Moth = m_CascadeTools->invariantMass(moms[topoN-1]);
      ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer.cptr(), m_refitPV ? refPvContainer.ptr() : 0, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info_pair.first, topoN-1, Mass_Moth, vtx));

      for(size_t i=0; i<topoN; i++) {
        VtxWriteHandles[i].ptr()->push_back(cascadeVertices[i]);
      }

      // Set links to cascade vertices
      VertexLinkVector cascadeVertexLinks;
      VertexLink vertexLink1;
      vertexLink1.setElement(cascadeVertices[0]);
      vertexLink1.setStorableObject(*VtxWriteHandles[0].ptr());
      if( vertexLink1.isValid() ) cascadeVertexLinks.push_back( vertexLink1 );
      if(topoN>=3) {
	VertexLink vertexLink2;
	vertexLink2.setElement(cascadeVertices[1]);
	vertexLink2.setStorableObject(*VtxWriteHandles[1].ptr());
	if( vertexLink2.isValid() ) cascadeVertexLinks.push_back( vertexLink2 );
      }
      if(topoN==4) {
	VertexLink vertexLink3;
	vertexLink3.setElement(cascadeVertices[2]);
	vertexLink3.setStorableObject(*VtxWriteHandles[2].ptr());
	if( vertexLink3.isValid() ) cascadeVertexLinks.push_back( vertexLink3 );
      }
      CascadeLinksDecor(*mainVertex) = cascadeVertexLinks;

      // process the posterior cascade with mass constraint for the main vertex
      if(cascade_info_pair.second) {
	const std::vector<xAOD::Vertex*> &cascadeVertices_mvc = cascade_info_pair.second->vertices();
	if(cascadeVertices_mvc.size() != topoN) ATH_MSG_ERROR("Incorrect number of vertices (mvc)");
	for(size_t i=0; i<topoN; i++) {
	  if(cascadeVertices_mvc[i]==nullptr) ATH_MSG_ERROR("Error null vertex (mvc)");
	}
	cascade_info_pair.second->setSVOwnership(false);
	const auto mainVertex_mvc = cascadeVertices_mvc[topoN-1];
	const std::vector< std::vector<TLorentzVector> > &moms_mvc = cascade_info_pair.second->getParticleMoms();
	// Identify the input JX
	int ijx_mvc = m_JXSubVtx ? topoN-2 : topoN-1;
	if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0) {
	  if(m_JXV0SubVtx) ijx_mvc = 1;
	  else ijx_mvc = topoN-1;
	}
	else if(m_extraTrk1MassHypo>0 && m_disVDaug_num==2) {
	  if(m_JXV0SubVtx || m_JXSubVtx) ijx_mvc = 1;
	  else ijx_mvc = topoN-1;
	}
	const xAOD::Vertex* jxVtx_mvc(nullptr);
	if(m_jxDaug_num==4) jxVtx_mvc = FindVertex<4>(jxContainer.ptr(), cascadeVertices_mvc[ijx_mvc]);
	else if(m_jxDaug_num==3) jxVtx_mvc = FindVertex<3>(jxContainer.ptr(), cascadeVertices_mvc[ijx_mvc]);
	else jxVtx_mvc = FindVertex<2>(jxContainer.ptr(), cascadeVertices_mvc[ijx_mvc]);

	xAOD::BPhysHypoHelper vtx_mvc(m_hypoName, mainVertex_mvc);
	BPhysPVCascadeTools::SetVectorInfo(vtx_mvc, cascade_info_pair.second);
	vtx_mvc.setPass(true);

	BPHYS_CHECK( vtx_mvc.setMass(m_CascadeTools->invariantMass(moms_mvc[topoN-1])) );
	BPHYS_CHECK( vtx_mvc.setMassErr(m_CascadeTools->invariantMassError(moms_mvc[topoN-1],cascade_info_pair.second->getCovariance()[topoN-1])) );
	Pt_decor(*mainVertex_mvc)       = m_CascadeTools->pT(moms_mvc[topoN-1]);
	PtErr_decor(*mainVertex_mvc)    = m_CascadeTools->pTError(moms_mvc[topoN-1],cascade_info_pair.second->getCovariance()[topoN-1]);
	chi2_decor(*mainVertex_mvc)     = cascade_info_pair.second->fitChi2();
	ndof_decor(*mainVertex_mvc)     = cascade_info_pair.second->nDoF();

	if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0) { // special cases
	  if(m_JXV0SubVtx) {
	    lxy_SV0_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->lxy(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    lxyErr_SV0_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->lxyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0z_SV0_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->a0z(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0zErr_SV0_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->a0zError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0xy_SV0_decor(*cascadeVertices_mvc[0])    = m_CascadeTools->a0xy(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0xyErr_SV0_decor(*cascadeVertices_mvc[0]) = m_CascadeTools->a0xyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    lxy_SV1_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->lxy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    lxyErr_SV1_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->lxyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0z_SV1_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->a0z(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0zErr_SV1_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->a0zError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xy_SV1_decor(*cascadeVertices_mvc[1])    = m_CascadeTools->a0xy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xyErr_SV1_decor(*cascadeVertices_mvc[1]) = m_CascadeTools->a0xyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    lxy_SV2_decor(*cascadeVertices_mvc[2])     = m_CascadeTools->lxy(moms_mvc[2],cascadeVertices_mvc[2],mainVertex_mvc);
	    lxyErr_SV2_decor(*cascadeVertices_mvc[2])  = m_CascadeTools->lxyError(moms_mvc[2],cascade_info_pair.second->getCovariance()[2],cascadeVertices_mvc[2],mainVertex_mvc);
	    a0z_SV2_decor(*cascadeVertices_mvc[2])     = m_CascadeTools->a0z(moms_mvc[2],cascadeVertices_mvc[2],mainVertex_mvc);
	    a0zErr_SV2_decor(*cascadeVertices_mvc[2])  = m_CascadeTools->a0zError(moms_mvc[2],cascade_info_pair.second->getCovariance()[2],cascadeVertices_mvc[2],mainVertex_mvc);
	    a0xy_SV2_decor(*cascadeVertices_mvc[2])    = m_CascadeTools->a0xy(moms_mvc[2],cascadeVertices_mvc[2],mainVertex_mvc);
	    a0xyErr_SV2_decor(*cascadeVertices_mvc[2]) = m_CascadeTools->a0xyError(moms_mvc[2],cascade_info_pair.second->getCovariance()[2],cascadeVertices_mvc[2],mainVertex_mvc);
	  }
	  else {
	    lxy_SV1_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->lxy(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    lxyErr_SV1_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->lxyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0z_SV1_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->a0z(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0zErr_SV1_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->a0zError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0xy_SV1_decor(*cascadeVertices_mvc[0])    = m_CascadeTools->a0xy(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0xyErr_SV1_decor(*cascadeVertices_mvc[0]) = m_CascadeTools->a0xyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    lxy_SV2_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->lxy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    lxyErr_SV2_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->lxyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0z_SV2_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->a0z(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0zErr_SV2_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->a0zError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xy_SV2_decor(*cascadeVertices_mvc[1])    = m_CascadeTools->a0xy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xyErr_SV2_decor(*cascadeVertices_mvc[1]) = m_CascadeTools->a0xyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	  }
	}
	else if(m_extraTrk1MassHypo>0 && m_disVDaug_num==2) { // special cases
	  if(m_JXV0SubVtx) {
	    lxy_SV0_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->lxy(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    lxyErr_SV0_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->lxyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0z_SV0_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->a0z(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0zErr_SV0_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->a0zError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0xy_SV0_decor(*cascadeVertices_mvc[0])    = m_CascadeTools->a0xy(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0xyErr_SV0_decor(*cascadeVertices_mvc[0]) = m_CascadeTools->a0xyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    lxy_SV1_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->lxy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    lxyErr_SV1_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->lxyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0z_SV1_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->a0z(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0zErr_SV1_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->a0zError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xy_SV1_decor(*cascadeVertices_mvc[1])    = m_CascadeTools->a0xy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xyErr_SV1_decor(*cascadeVertices_mvc[1]) = m_CascadeTools->a0xyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	  }
	  else {
	    lxy_SV1_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->lxy(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    lxyErr_SV1_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->lxyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0z_SV1_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->a0z(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0zErr_SV1_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->a0zError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0xy_SV1_decor(*cascadeVertices_mvc[0])    = m_CascadeTools->a0xy(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0xyErr_SV1_decor(*cascadeVertices_mvc[0]) = m_CascadeTools->a0xyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    if(m_JXSubVtx) {
	      lxy_SV2_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->lxy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	      lxyErr_SV2_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->lxyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	      a0z_SV2_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->a0z(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	      a0zErr_SV2_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->a0zError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	      a0xy_SV2_decor(*cascadeVertices_mvc[1])    = m_CascadeTools->a0xy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	      a0xyErr_SV2_decor(*cascadeVertices_mvc[1]) = m_CascadeTools->a0xyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    }
	  }
	}
	else { // other normal cases
	  if(m_disVDaug_num==2) {
	    lxy_SV1_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->lxy(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    lxyErr_SV1_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->lxyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0z_SV1_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->a0z(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0zErr_SV1_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->a0zError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0xy_SV1_decor(*cascadeVertices_mvc[0])    = m_CascadeTools->a0xy(moms_mvc[0],cascadeVertices_mvc[0],mainVertex_mvc);
	    a0xyErr_SV1_decor(*cascadeVertices_mvc[0]) = m_CascadeTools->a0xyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],mainVertex_mvc);
	  }
	  else {
	    lxy_SV0_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->lxy(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    lxyErr_SV0_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->lxyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0z_SV0_decor(*cascadeVertices_mvc[0])     = m_CascadeTools->a0z(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0zErr_SV0_decor(*cascadeVertices_mvc[0])  = m_CascadeTools->a0zError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0xy_SV0_decor(*cascadeVertices_mvc[0])    = m_CascadeTools->a0xy(moms_mvc[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    a0xyErr_SV0_decor(*cascadeVertices_mvc[0]) = m_CascadeTools->a0xyError(moms_mvc[0],cascade_info_pair.second->getCovariance()[0],cascadeVertices_mvc[0],cascadeVertices_mvc[1]);
	    lxy_SV1_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->lxy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    lxyErr_SV1_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->lxyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xy_SV1_decor(*cascadeVertices_mvc[1])    = m_CascadeTools->a0z(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0xyErr_SV1_decor(*cascadeVertices_mvc[1]) = m_CascadeTools->a0zError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0z_SV1_decor(*cascadeVertices_mvc[1])     = m_CascadeTools->a0xy(moms_mvc[1],cascadeVertices_mvc[1],mainVertex_mvc);
	    a0zErr_SV1_decor(*cascadeVertices_mvc[1])  = m_CascadeTools->a0xyError(moms_mvc[1],cascade_info_pair.second->getCovariance()[1],cascadeVertices_mvc[1],mainVertex_mvc);
	  }
	  if(m_JXSubVtx) {
	    lxy_SV2_decor(*cascadeVertices_mvc[ijx_mvc])     = m_CascadeTools->lxy(moms_mvc[ijx_mvc],cascadeVertices_mvc[ijx_mvc],mainVertex_mvc);
	    lxyErr_SV2_decor(*cascadeVertices_mvc[ijx_mvc])  = m_CascadeTools->lxyError(moms_mvc[ijx_mvc],cascade_info_pair.second->getCovariance()[ijx_mvc],cascadeVertices_mvc[ijx_mvc],mainVertex_mvc);
	    a0z_SV2_decor(*cascadeVertices_mvc[ijx_mvc])     = m_CascadeTools->a0z(moms_mvc[ijx_mvc],cascadeVertices_mvc[ijx_mvc],mainVertex_mvc);
	    a0zErr_SV2_decor(*cascadeVertices_mvc[ijx_mvc])  = m_CascadeTools->a0zError(moms_mvc[ijx_mvc],cascade_info_pair.second->getCovariance()[ijx_mvc],cascadeVertices_mvc[ijx_mvc],mainVertex_mvc);
	    a0xy_SV2_decor(*cascadeVertices_mvc[ijx_mvc])    = m_CascadeTools->a0xy(moms_mvc[ijx_mvc],cascadeVertices_mvc[ijx_mvc],mainVertex_mvc);
	    a0xyErr_SV2_decor(*cascadeVertices_mvc[ijx_mvc]) = m_CascadeTools->a0xyError(moms_mvc[ijx_mvc],cascade_info_pair.second->getCovariance()[ijx_mvc],cascadeVertices_mvc[ijx_mvc],mainVertex_mvc);
	  }
	}
	chi2_V2_decor(*cascadeVertices_mvc[ijx_mvc])     = m_V0Tools->chisq(jxVtx_mvc);
	ndof_V2_decor(*cascadeVertices_mvc[ijx_mvc])     = m_V0Tools->ndof(jxVtx_mvc);

	double Mass_Moth_mvc = m_CascadeTools->invariantMass(moms_mvc[topoN-1]);
	ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer.cptr(), m_refitPV ? refPvContainer.ptr() : 0, &(*m_pvRefitter), m_PV_max, m_DoVertexType, cascade_info_pair.second, topoN-1, Mass_Moth_mvc, vtx_mvc));

	for(size_t i=0; i<topoN; i++) {
	  VtxWriteHandles_mvc[i].ptr()->push_back(cascadeVertices_mvc[i]);
	}

	VertexLinkVector cascadeVertexLinks_mvc;
	VertexLink vertexLink1_mvc;
	vertexLink1_mvc.setElement(cascadeVertices_mvc[0]);
	vertexLink1_mvc.setStorableObject(*VtxWriteHandles_mvc[0].ptr());
	if( vertexLink1_mvc.isValid() ) cascadeVertexLinks_mvc.push_back( vertexLink1_mvc );
	if(topoN>=3) {
	  VertexLink vertexLink2_mvc;
	  vertexLink2_mvc.setElement(cascadeVertices_mvc[1]);
	  vertexLink2_mvc.setStorableObject(*VtxWriteHandles_mvc[1].ptr());
	  if( vertexLink2_mvc.isValid() ) cascadeVertexLinks_mvc.push_back( vertexLink2_mvc );
	}
	if(topoN==4) {
	  VertexLink vertexLink3_mvc;
	  vertexLink3_mvc.setElement(cascadeVertices_mvc[2]);
	  vertexLink3_mvc.setStorableObject(*VtxWriteHandles_mvc[2].ptr());
	  if( vertexLink3_mvc.isValid() ) cascadeVertexLinks_mvc.push_back( vertexLink3_mvc );
	}
	CascadeLinksDecor(*mainVertex_mvc) = cascadeVertexLinks_mvc;

	VertexLinkVector precedingVertexLinks_mvc;
	VertexLink vertexLink_mvc;
	vertexLink_mvc.setElement(cascadeVertices[topoN-1]);
	vertexLink_mvc.setStorableObject(*VtxWriteHandles[topoN-1].ptr());
	if( vertexLink_mvc.isValid() ) precedingVertexLinks_mvc.push_back( vertexLink_mvc );
	PrecedingLinksDecor(*mainVertex_mvc) = precedingVertexLinks_mvc;
      } // cascade_info_pair.second!=0
    } // loop over cascadeinfoContainer

    // Deleting cascadeinfo since this won't be stored.
    for (auto cascade_info_pair : cascadeinfoContainer) {
      if(cascade_info_pair.first) delete cascade_info_pair.first;
      if(cascade_info_pair.second) delete cascade_info_pair.second;
    }

    return StatusCode::SUCCESS;
  }

  bool JpsiXPlusDisplaced::d0Pass(const xAOD::TrackParticle* track, const xAOD::Vertex* PV) const {
    bool pass = false;
    const EventContext& ctx = Gaudi::Hive::currentContext();
    std::unique_ptr<Trk::Perigee> per = m_trackToVertexTool->perigeeAtVertex(ctx, *track, PV->position());
    if(!per) return pass;
    double d0 = per->parameters()[Trk::d0];
    double sig_d0 = sqrt((*per->covariance())(0,0));
    if(std::abs(d0/sig_d0) > m_d0_cut) pass = true;
    return pass;
  }

  JpsiXPlusDisplaced::XiCandidate JpsiXPlusDisplaced::getXiCandidate(const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticle* track3) const {
    XiCandidate disVtx;

    std::vector<const xAOD::TrackParticle*> tracksV0;
    tracksV0.reserve(V0vtx->nTrackParticles());
    for(size_t i=0; i<V0vtx->nTrackParticles(); i++) tracksV0.push_back(V0vtx->trackParticle(i));
    std::vector<double> massesV0;
    if(V0==LAMBDA)         massesV0 = m_massesV0_ppi;
    else if(V0==LAMBDABAR) massesV0 = m_massesV0_pip;
    else if(V0==KS)        massesV0 = m_massesV0_pipi;

    std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
    int robustness = 0;
    m_iVertexFitter->setRobustness(robustness, *state);
    std::vector<Trk::VertexID> vrtList;
    // V0 vertex
    Trk::VertexID vID = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
    vrtList.push_back(vID);
    // Mother vertex
    std::vector<const xAOD::TrackParticle*> tracksDis3{track3};
    std::vector<double> massesDis3{m_disVDaug3MassHypo};
    m_iVertexFitter->nextVertex(tracksDis3,massesDis3,vrtList,*state);
    // Do the work
    std::unique_ptr<Trk::VxCascadeInfo> cascade_info = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );
    if(cascade_info) {
      cascade_info->setSVOwnership(true);
      double chi2NDF = cascade_info->fitChi2()/cascade_info->nDoF();
      if(m_chi2cut_DisV<=0 || chi2NDF < m_chi2cut_DisV) {
	const std::vector<std::vector<TLorentzVector> > &moms = cascade_info->getParticleMoms();
	const std::vector<xAOD::Vertex*> &cascadeVertices = cascade_info->vertices();
	double lxy_SV1_sub = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	if(lxy_SV1_sub > m_lxyV0_cut) {
	  TLorentzVector totalMom;
	  for(size_t it=0; it<moms[1].size(); it++) totalMom += moms[1][it];
	  double disV_mass = totalMom.M();
	  if(m_useImprovedMass) {
	    if((V0==LAMBDA || V0==LAMBDABAR) && m_massLd>0) disV_mass += - moms[1][1].M() + m_massLd;
	    else if(V0==KS && m_massKs>0) disV_mass += - moms[1][1].M() + m_massKs;
	  }
	  if(disV_mass>m_DisplacedMassLower && disV_mass<m_DisplacedMassUpper) {
	    disVtx.track = track3; disVtx.V0vtx = V0vtx; disVtx.V0type = V0;
	    disVtx.chi2NDF = chi2NDF; disVtx.p4_V0track1 = moms[0][0];
	    disVtx.p4_V0track2 = moms[0][1]; disVtx.p4_disVtrack = moms[1][0];
	  }
	}
      }
    }
    return disVtx;
  }

  std::unique_ptr<xAOD::Vertex> JpsiXPlusDisplaced::fitTracks(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2, const xAOD::TrackParticle* track3) const {
    // Starting point
    const Trk::Perigee& aPerigee1 = track1->perigeeParameters();
    const Trk::Perigee& aPerigee2 = track2->perigeeParameters();
    int sflag(0), errorcode(0);
    Amg::Vector3D startingPoint = m_vertexEstimator->getCirclesIntersectionPoint(&aPerigee1,&aPerigee2,sflag,errorcode);
    if(errorcode) startingPoint(0) = startingPoint(1) = startingPoint(2) = 0.0;
    std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
    // do the fit
    if(track3) {
      std::unique_ptr<xAOD::Vertex> fittedVertex( m_iVertexFitter->fit(std::vector<const xAOD::TrackParticle*>{track1,track2,track3}, startingPoint, *state) );
      return fittedVertex;
    }
    else {
      std::unique_ptr<xAOD::Vertex> fittedVertex( m_iVertexFitter->fit(std::vector<const xAOD::TrackParticle*>{track1,track2}, startingPoint, *state) );
      return fittedVertex;
    }
  }

  JpsiXPlusDisplaced::MesonCandidate JpsiXPlusDisplaced::getDpmCandidate(const xAOD::Vertex* JXvtx, const xAOD::TrackParticle* extraTrk1, const xAOD::TrackParticle* extraTrk2, const xAOD::TrackParticle* extraTrk3) const {
    MesonCandidate Dpm;
    // Check overlap
    std::vector<const xAOD::TrackParticle*> tracksJX;
    tracksJX.reserve(JXvtx->nTrackParticles());
    for(size_t i=0; i<JXvtx->nTrackParticles(); i++) tracksJX.push_back(JXvtx->trackParticle(i));

    TLorentzVector tmp1, tmp2, tmp3;
    tmp1.SetPtEtaPhiM(extraTrk1->pt(),extraTrk1->eta(),extraTrk1->phi(),m_extraTrk1MassHypo);
    tmp2.SetPtEtaPhiM(extraTrk2->pt(),extraTrk2->eta(),extraTrk2->phi(),m_extraTrk2MassHypo);
    tmp3.SetPtEtaPhiM(extraTrk3->pt(),extraTrk3->eta(),extraTrk3->phi(),m_extraTrk3MassHypo);
    if((tmp1+tmp2+tmp3).M() < m_DpmMassLower || (tmp1+tmp2+tmp3).M() > m_DpmMassUpper) return Dpm;

    std::unique_ptr<xAOD::Vertex> vtx = fitTracks(extraTrk1, extraTrk2, extraTrk3);
    if(vtx) {
      double chi2NDF = vtx->chiSquared()/vtx->numberDoF();
      if(m_chi2cut_Dpm<=0.0 || chi2NDF < m_chi2cut_Dpm) {
	double lxyDpm = m_V0Tools->lxy(vtx.get(),JXvtx);
	if(lxyDpm>m_lxyDpm_cut) {
	  Dpm.extraTrack1 = extraTrk1; Dpm.extraTrack2 = extraTrk2; Dpm.extraTrack3 = extraTrk3;
	  Dpm.chi2NDF = chi2NDF;
	  TVector3 tot_pt; TVector3 tmp;
	  for(size_t i=0; i<vtx->vxTrackAtVertex().size(); ++i) {
	    const Trk::TrackParameters* aPerigee = vtx->vxTrackAtVertex()[i].perigeeAtVertex();
	    if(aPerigee) {
	      tmp.SetXYZ(aPerigee->momentum()[Trk::px],aPerigee->momentum()[Trk::py],aPerigee->momentum()[Trk::pz]);
	      tot_pt += tmp;
	    }
	  }
	  Dpm.pt = tot_pt.Pt();
	}
      }
    }
    return Dpm;
  }

  JpsiXPlusDisplaced::MesonCandidate JpsiXPlusDisplaced::getD0Candidate(const xAOD::Vertex* JXvtx, const xAOD::TrackParticle* extraTrk1, const xAOD::TrackParticle* extraTrk2) const {
    MesonCandidate D0;

    TLorentzVector tmp1, tmp2;
    tmp1.SetPtEtaPhiM(extraTrk1->pt(),extraTrk1->eta(),extraTrk1->phi(),m_extraTrk1MassHypo);
    tmp2.SetPtEtaPhiM(extraTrk2->pt(),extraTrk2->eta(),extraTrk2->phi(),m_extraTrk2MassHypo);
    if((tmp1+tmp2).M() < m_D0MassLower || (tmp1+tmp2).M() > m_D0MassUpper) return D0;

    std::unique_ptr<xAOD::Vertex> vtx = fitTracks(extraTrk1, extraTrk2);
    if(vtx) {
      double chi2NDF = vtx->chiSquared()/vtx->numberDoF();
      if(m_chi2cut_D0<=0.0 || chi2NDF < m_chi2cut_D0) {
	double lxyD0 = m_V0Tools->lxy(vtx.get(),JXvtx);
	if(lxyD0>m_lxyD0_cut) {
	  D0.extraTrack1 = extraTrk1; D0.extraTrack2 = extraTrk2;
	  D0.chi2NDF = chi2NDF;
	  TVector3 tot_pt; TVector3 tmp;
	  for(size_t i=0; i<vtx->vxTrackAtVertex().size(); ++i) {
	    const Trk::TrackParameters* aPerigee = vtx->vxTrackAtVertex()[i].perigeeAtVertex();
	    if(aPerigee) {
	      tmp.SetXYZ(aPerigee->momentum()[Trk::px],aPerigee->momentum()[Trk::py],aPerigee->momentum()[Trk::pz]);
	      tot_pt += tmp;
	    }
	  }
	  D0.pt = tot_pt.Pt();
	}
      }
    }
    return D0;
  }

  std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> > JpsiXPlusDisplaced::fitMainVtx(const xAOD::Vertex* JXvtx, const std::vector<double>& massesJX, const xAOD::Vertex* V0vtx, const V0Enum V0, const xAOD::TrackParticleContainer* trackContainer, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const {
    std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> > result;

    std::vector<const xAOD::TrackParticle*> tracksJX;
    tracksJX.reserve(JXvtx->nTrackParticles());
    for(size_t i=0; i<JXvtx->nTrackParticles(); i++) tracksJX.push_back(JXvtx->trackParticle(i));
    if (tracksJX.size() != massesJX.size()) {
      ATH_MSG_ERROR("Problems with JX input: number of tracks or track mass inputs is not correct!");
      return result;
    }
    // Check identical tracks in input
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0vtx->trackParticle(0)) != tracksJX.cend()) return result;
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), V0vtx->trackParticle(1)) != tracksJX.cend()) return result;
    std::vector<const xAOD::TrackParticle*> tracksV0;
    tracksV0.reserve(V0vtx->nTrackParticles());
    for(size_t j=0; j<V0vtx->nTrackParticles(); j++) tracksV0.push_back(V0vtx->trackParticle(j));

    std::vector<const xAOD::TrackParticle*> tracksJpsi{tracksJX[0], tracksJX[1]};
    std::vector<const xAOD::TrackParticle*> tracksX;
    if(m_jxDaug_num>=3) tracksX.push_back(tracksJX[2]);
    if(m_jxDaug_num==4) tracksX.push_back(tracksJX[3]);

    std::vector<double> massesV0;
    if(V0==LAMBDA) {
      massesV0 = m_massesV0_ppi;
    }
    else if(V0==LAMBDABAR) {
      massesV0 = m_massesV0_pip;
    }
    else if(V0==KS) {
      massesV0 = m_massesV0_pipi;
    }

    TLorentzVector p4_moth, p4_v0, tmp;
    for(size_t it=0; it<JXvtx->nTrackParticles(); it++) {
      tmp.SetPtEtaPhiM(JXvtx->trackParticle(it)->pt(), JXvtx->trackParticle(it)->eta(), JXvtx->trackParticle(it)->phi(), massesJX[it]);
      p4_moth += tmp;
    }
    xAOD::BPhysHelper V0_helper(V0vtx);
    for(int it=0; it<V0_helper.nRefTrks(); it++) {
      p4_moth += V0_helper.refTrk(it,massesV0[it]);
      p4_v0 += V0_helper.refTrk(it,massesV0[it]);
    }

    SG::AuxElement::Decorator<float>       chi2_V1_decor("ChiSquared_V1");
    SG::AuxElement::Decorator<int>         ndof_V1_decor("nDoF_V1");
    SG::AuxElement::Decorator<std::string> type_V1_decor("Type_V1");

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

    if(m_extraTrk1MassHypo<=0) {
      double main_mass = p4_moth.M();
      if(m_useImprovedMass) {
	if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_v0).M() + m_massJpsi;
	else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_v0).M() + m_massJX;
	if((V0==LAMBDA || V0==LAMBDABAR) && m_massLd>0) main_mass += - p4_v0.M() + m_massLd;
	else if(V0==KS && m_massKs>0) main_mass += - p4_v0.M() + m_massKs;
      }
      if (main_mass < m_MassLower || main_mass > m_MassUpper) return result;

      // Apply the user's settings to the fitter
      std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
      // Robustness: http://cdsweb.cern.ch/record/685551
      int robustness = 0;
      m_iVertexFitter->setRobustness(robustness, *state);
      // Build up the topology
      // Vertex list
      std::vector<Trk::VertexID> vrtList;
      // https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
      // V0 vertex
      Trk::VertexID vID1;
      if (m_constrV0) {
	vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,V0==KS?m_massKs:m_massLd);
      } else {
	vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
      }
      vrtList.push_back(vID1);
      Trk::VertexID vID2;
      if(m_JXSubVtx) {
	// JX vertex
	if (m_constrJX && m_jxDaug_num>2) {
	  vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state,m_massJX);
	} else {
	  vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state);
	}
	vrtList.push_back(vID2);
	// Mother vertex including JX and V0
	std::vector<const xAOD::TrackParticle*> tp;
	std::vector<double> tp_masses;
	if(m_constrMainV) {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,*state,m_massMainV);
	} else {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList,*state);
	}
      }
      else { // m_JXSubVtx=false
	// Mother vertex including JX and V0
	if(m_constrMainV) {
	  vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state,m_massMainV);
	} else {
	  vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state);
	}
	if (m_constrJX && m_jxDaug_num>2) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for JX failed");
	  }
	}
      }
      if (m_constrJpsi) {
	std::vector<Trk::VertexID> cnstV;
	if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	}
      }
      if (m_constrX && m_jxDaug_num==4) {
	std::vector<Trk::VertexID> cnstV;
	if ( !m_iVertexFitter->addMassConstraint(vID2,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for X failed");
	}
      }
      // Do the work
      std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

      if (fit_result) {
	for(auto& v : fit_result->vertices()) {
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
	if(chi2CutPassed && lxy_SV1>m_lxyV0_cut) {
	  chi2_V1_decor(*cascadeVertices[0]) = V0vtx->chiSquared();
	  ndof_V1_decor(*cascadeVertices[0]) = V0vtx->numberDoF();
	  if(V0==LAMBDA) {
	    type_V1_decor(*cascadeVertices[0]) = "Lambda";
	  }
	  else if(V0==LAMBDABAR) {
	    type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
	  }
	  else if(V0==KS) {
	    type_V1_decor(*cascadeVertices[0]) = "Ks";
	  }
	  mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0vtx) ? mAcc_gfit(*V0vtx) : 0;
	  mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0vtx) ? mAcc_gmass(*V0vtx) : -1;
	  mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0vtx) ? mAcc_gmasserr(*V0vtx) : -1;
	  mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0vtx) ? mAcc_gchisq(*V0vtx) : 999999;
	  mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0vtx) ? mAcc_gndof(*V0vtx) : 0;
	  mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0vtx) ? mAcc_gprob(*V0vtx) : -1;
	  trk_px.clear(); trk_py.clear(); trk_pz.clear();
	  trk_px.reserve(V0_helper.nRefTrks());
	  trk_py.reserve(V0_helper.nRefTrks());
	  trk_pz.reserve(V0_helper.nRefTrks());
	  for(auto&& vec3 : V0_helper.refTrks()) {
	    trk_px.push_back( vec3.Px() );
	    trk_py.push_back( vec3.Py() );
	    trk_pz.push_back( vec3.Pz() );
	  }
	  trk_pxDeco(*cascadeVertices[0]) = trk_px;
	  trk_pyDeco(*cascadeVertices[0]) = trk_py;
	  trk_pzDeco(*cascadeVertices[0]) = trk_pz;

	  result.push_back( std::make_pair(fit_result.release(),nullptr) );
	}
      }
    } // for m_extraTrk1MassHypo<=0
    else if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo<=0) {
      std::vector<double> massesExtra{m_extraTrk1MassHypo};
      std::vector<double> massesJXExtra = massesJX; massesJXExtra.push_back(m_extraTrk1MassHypo);

      for(const xAOD::TrackParticle* tpExtra : *trackContainer) {
	if( tpExtra->pt()<m_extraTrk1MinPt ) continue;
	if( !m_trkSelector->decision(*tpExtra, nullptr) ) continue;
	// Check identical tracks in input
	if(std::find(tracksJX.cbegin(),tracksJX.cend(),tpExtra) != tracksJX.cend()) continue;
	if(std::find(tracksV0.cbegin(),tracksV0.cend(),tpExtra) != tracksV0.cend()) continue;

	TLorentzVector tmp;
	tmp.SetPtEtaPhiM(tpExtra->pt(),tpExtra->eta(),tpExtra->phi(),m_extraTrk1MassHypo);
	double main_mass = (p4_moth+tmp).M();
	if(m_useImprovedMass) {
	  if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_v0).M() + m_massJpsi;
	  else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_v0).M() + m_massJX;
	  if((V0==LAMBDA || V0==LAMBDABAR) && m_massLd>0) main_mass += - p4_v0.M() + m_massLd;
	  else if(V0==KS && m_massKs>0) main_mass += - p4_v0.M() + m_massKs;
	}
	if(main_mass < m_MassLower || main_mass > m_MassUpper) continue;

	std::vector<const xAOD::TrackParticle*> tracksExtra{tpExtra};
	std::vector<const xAOD::TrackParticle*> tracksJXExtra = tracksJX; tracksJXExtra.push_back(tpExtra);

	// Apply the user's settings to the fitter
	std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
	// Robustness: http://cdsweb.cern.ch/record/685551
	int robustness = 0;
	m_iVertexFitter->setRobustness(robustness, *state);
	// Build up the topology
	// Vertex list
	std::vector<Trk::VertexID> vrtList;
	std::vector<Trk::VertexID> vrtList2;
	Trk::VertexID vID2;
	if(m_JXV0SubVtx) {
	  // V0 vertex
	  Trk::VertexID vID1;
	  if (m_constrV0) {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,V0==KS?m_massKs:m_massLd);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
	  }
	  vrtList.push_back(vID1);
	  // JX+V0 vertex
	  if(m_constrJXV0) {
	    vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state,m_massJXV0);
	  } else {
	    vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state);
	  }
	  vrtList2.push_back(vID2);
	  // Main vertex
	  if(m_constrMainV) {
	    m_iVertexFitter->nextVertex(tracksExtra,massesExtra,vrtList2,*state,m_massMainV);
	  } else {
	    m_iVertexFitter->nextVertex(tracksExtra,massesExtra,vrtList2,*state);
	  }
	}
	else { // m_JXV0SubVtx==false
	  // V0 vertex
	  Trk::VertexID vID1;
	  if (m_constrV0) {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,V0==KS?m_massKs:m_massLd);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
	  }
	  vrtList.push_back(vID1);
	  if(m_JXSubVtx) {
	    // JX vertex
	    vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state);     
	    vrtList.push_back(vID2);
	    // Mother vertex
	    if(m_constrMainV) {
	      m_iVertexFitter->nextVertex(tracksExtra,massesExtra,vrtList,*state,m_massMainV);
	    } else {
	      m_iVertexFitter->nextVertex(tracksExtra,massesExtra,vrtList,*state);
	    }
	  }
	  else { // m_JXSubVtx=false
	    // Mother vertex includes one subvertex (V0) and JX tracks + extra track
	    if(m_constrMainV) {
	      vID2 = m_iVertexFitter->nextVertex(tracksJXExtra,massesJXExtra,vrtList,*state,m_massMainV);
	    } else {
	      vID2 = m_iVertexFitter->nextVertex(tracksJXExtra,massesJXExtra,vrtList,*state);
	    }
	  }
	}
	if (m_constrJX && m_jxDaug_num>2) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for JX failed");
	  }
	}
	if (m_constrJpsi) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	  }
	}
	if (m_constrX && m_jxDaug_num==4) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID2,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for X failed");
	  }
	}
	// Do the work
	std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

	if (fit_result) {
	  for(auto& v : fit_result->vertices()) {
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
	  double lxy_SV1(0);
	  if(m_JXV0SubVtx) {
	    lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  }
	  else {
	    lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[iMoth]);
	  }
	  if(chi2CutPassed && lxy_SV1>m_lxyV0_cut) {
	    chi2_V1_decor(*cascadeVertices[0]) = V0vtx->chiSquared();
	    ndof_V1_decor(*cascadeVertices[0]) = V0vtx->numberDoF();
	    if(V0==LAMBDA) {
	      type_V1_decor(*cascadeVertices[0]) = "Lambda";
	    }
	    else if(V0==LAMBDABAR) {
	      type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
	    }
	    else if(V0==KS) {
	      type_V1_decor(*cascadeVertices[0]) = "Ks";
	    }
	    mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0vtx) ? mAcc_gfit(*V0vtx) : 0;
	    mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0vtx) ? mAcc_gmass(*V0vtx) : -1;
	    mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0vtx) ? mAcc_gmasserr(*V0vtx) : -1;
	    mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0vtx) ? mAcc_gchisq(*V0vtx) : 999999;
	    mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0vtx) ? mAcc_gndof(*V0vtx) : 0;
	    mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0vtx) ? mAcc_gprob(*V0vtx) : -1;
	    trk_px.clear(); trk_py.clear(); trk_pz.clear();
	    trk_px.reserve(V0_helper.nRefTrks());
	    trk_py.reserve(V0_helper.nRefTrks());
	    trk_pz.reserve(V0_helper.nRefTrks());
	    for(auto&& vec3 : V0_helper.refTrks()) {
	      trk_px.push_back( vec3.Px() );
	      trk_py.push_back( vec3.Py() );
	      trk_pz.push_back( vec3.Pz() );
	    }
	    trk_pxDeco(*cascadeVertices[0]) = trk_px;
	    trk_pyDeco(*cascadeVertices[0]) = trk_py;
	    trk_pzDeco(*cascadeVertices[0]) = trk_pz;

	    result.push_back( std::make_pair(fit_result.release(),nullptr) );
	  }
	}
      } // loop over trackContainer
    } // for m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo<=0
    else if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0 && m_extraTrk3MassHypo<=0) {
      std::vector<const xAOD::TrackParticle*> tracksPlus;
      std::vector<const xAOD::TrackParticle*> tracksMinus;
      for(const xAOD::TrackParticle* tpExtra : *trackContainer) {
	if( tpExtra->pt() < std::fmin(m_extraTrk1MinPt,m_extraTrk2MinPt) ) continue;
	if( !m_trkSelector->decision(*tpExtra, nullptr) ) continue;
	// Check identical tracks in input
	if(std::find(tracksJX.cbegin(),tracksJX.cend(),tpExtra) != tracksJX.cend()) continue;
	if(std::find(tracksV0.cbegin(),tracksV0.cend(),tpExtra) != tracksV0.cend()) continue;
	if(tpExtra->charge()>0) {
	  tracksPlus.push_back(tpExtra);
	}
        else {
	  tracksMinus.push_back(tpExtra);
	}
      }

      MesonCandidateVector D0Candidates(m_maxMesonCandidates, m_MesonPtOrdering);
      TLorentzVector p4_ExtraTrk1, p4_ExtraTrk2;
      for(const xAOD::TrackParticle* tp1 : tracksPlus) {
	for(const xAOD::TrackParticle* tp2 : tracksMinus) {
	  if((tp1->pt()>m_extraTrk1MinPt && tp2->pt()>m_extraTrk2MinPt) ||
	     (tp1->pt()>m_extraTrk2MinPt && tp2->pt()>m_extraTrk1MinPt)) {
	    p4_ExtraTrk1.SetPtEtaPhiM(tp1->pt(), tp1->eta(), tp1->phi(), m_extraTrk1MassHypo);
	    p4_ExtraTrk2.SetPtEtaPhiM(tp2->pt(), tp2->eta(), tp2->phi(), m_extraTrk2MassHypo);
	    double main_mass = (p4_moth+p4_ExtraTrk1+p4_ExtraTrk2).M();
	    if(m_useImprovedMass) {
	      if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_v0).M() + m_massJpsi;
	      else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_v0).M() + m_massJX;
	      if((V0==LAMBDA || V0==LAMBDABAR) && m_massLd>0) main_mass += - p4_v0.M() + m_massLd;
	      else if(V0==KS && m_massKs>0) main_mass += - p4_v0.M() + m_massKs;
	      if(m_massD0>0) main_mass += - (p4_ExtraTrk1+p4_ExtraTrk2).M() + m_massD0;
	    }
	    if(main_mass < m_MassLower || main_mass > m_MassUpper) continue;
	    auto D0 = getD0Candidate(JXvtx,tp1,tp2);
	    if(D0.extraTrack1) D0Candidates.push_back(D0);
	  }
	}
      }

      std::vector<double> massesExtra{m_extraTrk1MassHypo,m_extraTrk2MassHypo};
      
      for(auto&& D0 : D0Candidates.vector()) {
	std::vector<const xAOD::TrackParticle*> tracksExtra{D0.extraTrack1,D0.extraTrack2};

	// Apply the user's settings to the fitter
	std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
	// Robustness: http://cdsweb.cern.ch/record/685551
	int robustness = 0;
	m_iVertexFitter->setRobustness(robustness, *state);
	// Build up the topology
	// Vertex list
	std::vector<Trk::VertexID> vrtList;
	// https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	// V0 vertex
	Trk::VertexID vID1;
	if (m_constrV0) {
	  vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,V0==KS?m_massKs:m_massLd);
	} else {
	  vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
	}
	vrtList.push_back(vID1);
	// D0 vertex
	Trk::VertexID vID2;
	if (m_constrD0) {
          vID2 = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state,m_massD0);
        } else {
          vID2 = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state);
        }
        vrtList.push_back(vID2);	
	// Mother vertex includes one subvertex (V0) and JX tracks + extra track
	Trk::VertexID vID3;
        if(m_constrMainV) {
          vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state,m_massMainV);
        } else {
          vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state);
        }
	if (m_constrJX && m_jxDaug_num>2) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for JX failed");
	  }
	}
	if (m_constrJpsi) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	  }
	}
	if (m_constrX && m_jxDaug_num==4) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for X failed");
	  }
	}
	// Do the work
	std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

	if (fit_result) {
	  for(auto& v : fit_result->vertices()) {
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
	  double lxy_SV2 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[iMoth]);
	  if(chi2CutPassed && lxy_SV1>m_lxyV0_cut && lxy_SV2>m_lxyD0_cut) {
	    chi2_V1_decor(*cascadeVertices[0]) = V0vtx->chiSquared();
	    ndof_V1_decor(*cascadeVertices[0]) = V0vtx->numberDoF();
	    if(V0==LAMBDA) {
	      type_V1_decor(*cascadeVertices[0]) = "Lambda";
	    }
	    else if(V0==LAMBDABAR) {
	      type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
	    }
	    else if(V0==KS) {
	      type_V1_decor(*cascadeVertices[0]) = "Ks";
	    }
	    mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0vtx) ? mAcc_gfit(*V0vtx) : 0;
	    mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0vtx) ? mAcc_gmass(*V0vtx) : -1;
	    mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0vtx) ? mAcc_gmasserr(*V0vtx) : -1;
	    mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0vtx) ? mAcc_gchisq(*V0vtx) : 999999;
	    mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0vtx) ? mAcc_gndof(*V0vtx) : 0;
	    mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0vtx) ? mAcc_gprob(*V0vtx) : -1;
	    trk_px.clear(); trk_py.clear(); trk_pz.clear();
	    trk_px.reserve(V0_helper.nRefTrks());
	    trk_py.reserve(V0_helper.nRefTrks());
	    trk_pz.reserve(V0_helper.nRefTrks());
	    for(auto&& vec3 : V0_helper.refTrks()) {
	      trk_px.push_back( vec3.Px() );
	      trk_py.push_back( vec3.Py() );
	      trk_pz.push_back( vec3.Pz() );
	    }
	    trk_pxDeco(*cascadeVertices[0]) = trk_px;
	    trk_pyDeco(*cascadeVertices[0]) = trk_py;
	    trk_pzDeco(*cascadeVertices[0]) = trk_pz;

	    result.push_back( std::make_pair(fit_result.release(),nullptr) );
	  }
	}
      } // loop over D0Candidates
    } // for m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0 && m_extraTrk3MassHypo<=0
    else if(m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0 && m_extraTrk3MassHypo>0) {
      std::vector<const xAOD::TrackParticle*> tracksPlus;
      std::vector<const xAOD::TrackParticle*> tracksMinus;
      double minTrkPt = std::fmin(std::fmin(m_extraTrk1MinPt,m_extraTrk2MinPt),m_extraTrk3MinPt);
      for(const xAOD::TrackParticle* tpExtra : *trackContainer) {
	if( tpExtra->pt() < minTrkPt ) continue;
	if( !m_trkSelector->decision(*tpExtra, nullptr) ) continue;
	// Check identical tracks in input
	if(std::find(tracksJX.cbegin(),tracksJX.cend(),tpExtra) != tracksJX.cend()) continue;
	if(std::find(tracksV0.cbegin(),tracksV0.cend(),tpExtra) != tracksV0.cend()) continue;
	if(tpExtra->charge()>0) {
	  tracksPlus.push_back(tpExtra);
	}
        else {
	  tracksMinus.push_back(tpExtra);
	}
      }

      MesonCandidateVector DpmCandidates(m_maxMesonCandidates, m_MesonPtOrdering);
      TLorentzVector p4_ExtraTrk1, p4_ExtraTrk2, p4_ExtraTrk3;
      // +,-,- combination (D- -> K+ pi- pi-)
      for(const xAOD::TrackParticle* tp1 : tracksPlus) {
	if( tp1->pt() < m_extraTrk1MinPt ) continue;
	for(auto tp2Itr=tracksMinus.cbegin(); tp2Itr!=tracksMinus.cend(); ++tp2Itr) {
	  const xAOD::TrackParticle* tp2 = *tp2Itr;
	  for(auto tp3Itr=tp2Itr+1; tp3Itr!=tracksMinus.cend(); ++tp3Itr) {
	    const xAOD::TrackParticle* tp3 = *tp3Itr;
	    if((tp2->pt()>m_extraTrk2MinPt && tp3->pt()>m_extraTrk3MinPt) ||
	       (tp2->pt()>m_extraTrk3MinPt && tp3->pt()>m_extraTrk2MinPt)) {
	      p4_ExtraTrk1.SetPtEtaPhiM(tp1->pt(), tp1->eta(), tp1->phi(), m_extraTrk1MassHypo);
	      p4_ExtraTrk2.SetPtEtaPhiM(tp2->pt(), tp2->eta(), tp2->phi(), m_extraTrk2MassHypo);
	      p4_ExtraTrk3.SetPtEtaPhiM(tp3->pt(), tp3->eta(), tp3->phi(), m_extraTrk3MassHypo);
	      double main_mass = (p4_moth+p4_ExtraTrk1+p4_ExtraTrk2+p4_ExtraTrk3).M();
	      if(m_useImprovedMass) {
		if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_v0).M() + m_massJpsi;
		else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_v0).M() + m_massJX;
		if((V0==LAMBDA || V0==LAMBDABAR) && m_massLd>0) main_mass += - p4_v0.M() + m_massLd;
		else if(V0==KS && m_massKs>0) main_mass += - p4_v0.M() + m_massKs;
		if(m_massDpm>0) main_mass += - (p4_ExtraTrk1+p4_ExtraTrk2+p4_ExtraTrk3).M() + m_massDpm;
	      }
	      if(main_mass < m_MassLower || main_mass > m_MassUpper) continue;
	      auto Dpm = getDpmCandidate(JXvtx,tp1,tp2,tp3);
	      if(Dpm.extraTrack1) DpmCandidates.push_back(Dpm);
	    }
	  }
	}
      }
      // -,+,+ combination (D+ -> K- pi+ pi+)
      for(const xAOD::TrackParticle* tp1 : tracksMinus) {
	if( tp1->pt() < m_extraTrk1MinPt ) continue;
	for(auto tp2Itr=tracksPlus.cbegin(); tp2Itr!=tracksPlus.cend(); ++tp2Itr) {
	  const xAOD::TrackParticle* tp2 = *tp2Itr;
	  for(auto tp3Itr=tp2Itr+1; tp3Itr!=tracksPlus.cend(); ++tp3Itr) {
	    const xAOD::TrackParticle* tp3 = *tp3Itr;
	    if((tp2->pt()>m_extraTrk2MinPt && tp3->pt()>m_extraTrk3MinPt) ||
	       (tp2->pt()>m_extraTrk3MinPt && tp3->pt()>m_extraTrk2MinPt)) {
	      p4_ExtraTrk1.SetPtEtaPhiM(tp1->pt(), tp1->eta(), tp1->phi(), m_extraTrk1MassHypo);
	      p4_ExtraTrk2.SetPtEtaPhiM(tp2->pt(), tp2->eta(), tp2->phi(), m_extraTrk2MassHypo);
	      p4_ExtraTrk3.SetPtEtaPhiM(tp3->pt(), tp3->eta(), tp3->phi(), m_extraTrk3MassHypo);
	      double main_mass = (p4_moth+p4_ExtraTrk1+p4_ExtraTrk2+p4_ExtraTrk3).M();
	      if(m_useImprovedMass) {
		if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_v0).M() + m_massJpsi;
		else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_v0).M() + m_massJX;
		if((V0==LAMBDA || V0==LAMBDABAR) && m_massLd>0) main_mass += - p4_v0.M() + m_massLd;
		else if(V0==KS && m_massKs>0) main_mass += - p4_v0.M() + m_massKs;
		if(m_massDpm>0) main_mass += - (p4_ExtraTrk1+p4_ExtraTrk2+p4_ExtraTrk3).M() + m_massDpm;
	      }
	      if(main_mass < m_MassLower || main_mass > m_MassUpper) continue;
	      auto Dpm = getDpmCandidate(JXvtx,tp1,tp2,tp3);
	      if(Dpm.extraTrack1) DpmCandidates.push_back(Dpm);
	    }
	  }
	}
      }

      std::vector<double> massesExtra{m_extraTrk1MassHypo,m_extraTrk2MassHypo,m_extraTrk3MassHypo};

      for(auto&& Dpm : DpmCandidates.vector()) {
	std::vector<const xAOD::TrackParticle*> tracksExtra{Dpm.extraTrack1,Dpm.extraTrack2,Dpm.extraTrack3};

	// Apply the user's settings to the fitter
	std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
	// Robustness: http://cdsweb.cern.ch/record/685551
	int robustness = 0;
	m_iVertexFitter->setRobustness(robustness, *state);
	// Build up the topology
	// Vertex list
	std::vector<Trk::VertexID> vrtList;
	std::vector<Trk::VertexID> vrtList2;
	// https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	if(m_JXV0SubVtx) {
	  // V0 vertex
	  Trk::VertexID vID1;
	  if (m_constrV0) {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,V0==KS?m_massKs:m_massLd);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
	  }
	  vrtList.push_back(vID1);
	  // JX+V0 vertex
	  Trk::VertexID vID2;
	  if(m_constrJXV0) {
	    vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state,m_massJXV0);
	  } else {
	    vID2 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state);
	  }
	  vrtList2.push_back(vID2);
	  Trk::VertexID vID3;
	  if (m_constrDpm) {
	    vID3 = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state,m_massDpm);
	  } else {
	    vID3 = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state);
	  }
	  vrtList2.push_back(vID3);
	  // Mother vertex includes two subvertices: V0, Dpm and JX tracks
	  std::vector<const xAOD::TrackParticle*> tp;
	  std::vector<double> tp_masses;
	  if(m_constrMainV) {
	    m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2,*state,m_massMainV);
	  } else {
	    m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2,*state);
	  }
	  if (m_constrJX && m_jxDaug_num>2) {
	    std::vector<Trk::VertexID> cnstV;
	    if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for JX failed");
	    }
	  }
	  if (m_constrJpsi) {
	    std::vector<Trk::VertexID> cnstV;
	    if ( !m_iVertexFitter->addMassConstraint(vID2,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	    }
	  }
	  if (m_constrX && m_jxDaug_num==4) {
	    std::vector<Trk::VertexID> cnstV;
	    if ( !m_iVertexFitter->addMassConstraint(vID2,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for X failed");
	    }
	  }
	}
	else { // m_JXV0SubVtx==false
	  // V0 vertex
	  Trk::VertexID vID1;
	  if (m_constrV0) {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,V0==KS?m_massKs:m_massLd);
	  } else {
	    vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
	  }
	  vrtList.push_back(vID1);
	  // Dpm vertex
	  Trk::VertexID vID2;
	  if (m_constrDpm) {
	    vID2 = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state,m_massDpm);
	  } else {
	    vID2 = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state);
	  }
	  vrtList.push_back(vID2);
	  // Mother vertex includes two subvertices: V0, Dpm and JX tracks
	  Trk::VertexID vID3;
	  if(m_constrMainV) {
	    vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state,m_massMainV);
	  } else {
	    vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList,*state);
	  }
	  if (m_constrJX && m_jxDaug_num>2) {
	    std::vector<Trk::VertexID> cnstV;
	    if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for JX failed");
	    }
	  }
	  if (m_constrJpsi) {
	    std::vector<Trk::VertexID> cnstV;
	    if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	    }
	  }
	  if (m_constrX && m_jxDaug_num==4) {
	    std::vector<Trk::VertexID> cnstV;
	    if ( !m_iVertexFitter->addMassConstraint(vID3,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	      ATH_MSG_WARNING("addMassConstraint for X failed");
	    }
	  }
	}
	// Do the work
	std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

	if (fit_result) {
	  for(auto& v : fit_result->vertices()) {
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
	  double lxy_SV1(0), lxy_SV2(0);
	  if(m_JXV0SubVtx) {
	    lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	    lxy_SV2 = m_CascadeTools->lxy(moms[2],cascadeVertices[2],cascadeVertices[iMoth]);
	  }
	  else {
	    lxy_SV1 = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[iMoth]);
	    lxy_SV2 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[iMoth]);
	  }
	  if(chi2CutPassed && lxy_SV1>m_lxyV0_cut && lxy_SV2>m_lxyDpm_cut) {
	    chi2_V1_decor(*cascadeVertices[0]) = V0vtx->chiSquared();
	    ndof_V1_decor(*cascadeVertices[0]) = V0vtx->numberDoF();
	    if(V0==LAMBDA) {
	      type_V1_decor(*cascadeVertices[0]) = "Lambda";
	    }
	    else if(V0==LAMBDABAR) {
	      type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
	    }
	    else if(V0==KS) {
	      type_V1_decor(*cascadeVertices[0]) = "Ks";
	    }
	    mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0vtx) ? mAcc_gfit(*V0vtx) : 0;
	    mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0vtx) ? mAcc_gmass(*V0vtx) : -1;
	    mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0vtx) ? mAcc_gmasserr(*V0vtx) : -1;
	    mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0vtx) ? mAcc_gchisq(*V0vtx) : 999999;
	    mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0vtx) ? mAcc_gndof(*V0vtx) : 0;
	    mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0vtx) ? mAcc_gprob(*V0vtx) : -1;
	    trk_px.clear(); trk_py.clear(); trk_pz.clear();
	    trk_px.reserve(V0_helper.nRefTrks());
	    trk_py.reserve(V0_helper.nRefTrks());
	    trk_pz.reserve(V0_helper.nRefTrks());
	    for(auto&& vec3 : V0_helper.refTrks()) {
	      trk_px.push_back( vec3.Px() );
	      trk_py.push_back( vec3.Py() );
	      trk_pz.push_back( vec3.Pz() );
	    }
	    trk_pxDeco(*cascadeVertices[0]) = trk_px;
	    trk_pyDeco(*cascadeVertices[0]) = trk_py;
	    trk_pzDeco(*cascadeVertices[0]) = trk_pz;

	    // refit with main vertex mass constraint if required
	    if(!m_constrMainV && m_doPostMainVContrFit) {
	      TLorentzVector totalMom;
	      for(size_t it=0; it<moms[iMoth].size(); it++) totalMom += moms[iMoth][it];
	      double mainV_mass = totalMom.M();
	      if(mainV_mass>m_PostMassLower && mainV_mass<m_PostMassUpper) {
		std::unique_ptr<Trk::IVKalState> state_mvc = m_iVertexFitter->makeState();
		int robustness_mvc = 0;
		m_iVertexFitter->setRobustness(robustness_mvc, *state_mvc);
		std::vector<Trk::VertexID> vrtList_mvc;
		std::vector<Trk::VertexID> vrtList2_mvc;
		if(m_JXV0SubVtx) {
		  Trk::VertexID vID1_mvc; // V0 vertex
		  if (m_constrV0) {
		    vID1_mvc = m_iVertexFitter->startVertex(tracksV0,massesV0,*state_mvc,V0==KS?m_massKs:m_massLd);
		  } else {
		    vID1_mvc = m_iVertexFitter->startVertex(tracksV0,massesV0,*state_mvc);
		  }
		  vrtList_mvc.push_back(vID1_mvc);
		  Trk::VertexID vID2_mvc; // JX+V0 vertex
		  if(m_constrJXV0) {
		    vID2_mvc = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList_mvc,*state_mvc,m_massJXV0);
		  } else {
		    vID2_mvc = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList_mvc,*state_mvc);
		  }
		  vrtList2_mvc.push_back(vID2_mvc);
		  Trk::VertexID vID3_mvc; // Dpm vertex
		  if (m_constrDpm) {
		    vID3_mvc = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state_mvc,m_massDpm);
		  } else {
		    vID3_mvc = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state_mvc);
		  }
		  vrtList2_mvc.push_back(vID3_mvc);
		  std::vector<const xAOD::TrackParticle*> tp;
		  std::vector<double> tp_masses;
		  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2_mvc,*state_mvc,m_massMainV); // Mother vertex
		  if (m_constrJX && m_jxDaug_num>2) {
		    std::vector<Trk::VertexID> cnstV_mvc;
		    if ( !m_iVertexFitter->addMassConstraint(vID2_mvc,tracksJX,cnstV_mvc,*state_mvc,m_massJX).isSuccess() ) {
		      ATH_MSG_WARNING("addMassConstraint for JX failed");
		    }
		  }
		  if (m_constrJpsi) {
		    std::vector<Trk::VertexID> cnstV_mvc;
		    if ( !m_iVertexFitter->addMassConstraint(vID2_mvc,tracksJpsi,cnstV_mvc,*state_mvc,m_massJpsi).isSuccess() ) {
		      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
		    }
		  }
		  if (m_constrX && m_jxDaug_num==4) {
		    std::vector<Trk::VertexID> cnstV_mvc;
		    if ( !m_iVertexFitter->addMassConstraint(vID2_mvc,tracksX,cnstV_mvc,*state_mvc,m_massX).isSuccess() ) {
		      ATH_MSG_WARNING("addMassConstraint for X failed");
		    }
		  }
		}
		else {
		  Trk::VertexID vID1_mvc; // V0 vertex
		  if (m_constrV0) {
		    vID1_mvc = m_iVertexFitter->startVertex(tracksV0,massesV0,*state_mvc,V0==KS?m_massKs:m_massLd);
		  } else {
		    vID1_mvc = m_iVertexFitter->startVertex(tracksV0,massesV0,*state_mvc);
		  }
		  vrtList_mvc.push_back(vID1_mvc);
		  Trk::VertexID vID2_mvc; // Dpm vertex
		  if (m_constrDpm) {
		    vID2_mvc = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state_mvc,m_massDpm);
		  } else {
		    vID2_mvc = m_iVertexFitter->nextVertex(tracksExtra,massesExtra,*state_mvc);
		  }
		  vrtList_mvc.push_back(vID2_mvc);
		  Trk::VertexID vID3_mvc = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList_mvc,*state_mvc,m_massMainV); // Mother vertex
		  if (m_constrJX && m_jxDaug_num>2) {
		    std::vector<Trk::VertexID> cnstV_mvc;
		    if ( !m_iVertexFitter->addMassConstraint(vID3_mvc,tracksJX,cnstV_mvc,*state_mvc,m_massJX).isSuccess() ) {
		      ATH_MSG_WARNING("addMassConstraint for JX failed");
		    }
		  }
		  if (m_constrJpsi) {
		    std::vector<Trk::VertexID> cnstV_mvc;
		    if ( !m_iVertexFitter->addMassConstraint(vID3_mvc,tracksJpsi,cnstV_mvc,*state_mvc,m_massJpsi).isSuccess() ) {
		      ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
		    }
		  }
		  if (m_constrX && m_jxDaug_num==4) {
		    std::vector<Trk::VertexID> cnstV_mvc;
		    if ( !m_iVertexFitter->addMassConstraint(vID3_mvc,tracksX,cnstV_mvc,*state_mvc,m_massX).isSuccess() ) {
		      ATH_MSG_WARNING("addMassConstraint for X failed");
		    }
		  }
		}
		// Do the work
		std::unique_ptr<Trk::VxCascadeInfo> fit_result_mvc = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state_mvc) );

		if (fit_result_mvc) {
		  for(auto& v : fit_result_mvc->vertices()) {
		    if(v->nTrackParticles()==0) {
		      std::vector<ElementLink<xAOD::TrackParticleContainer> > nullLinkVector;
		      v->setTrackParticleLinks(nullLinkVector);
		    }
		  }
		  BPhysPVCascadeTools::PrepareVertexLinks(fit_result_mvc.get(), trackCols);
		  fit_result_mvc->setSVOwnership(true);
		  chi2_V1_decor(*cascadeVertices[0]) = V0vtx->chiSquared();
		  ndof_V1_decor(*cascadeVertices[0]) = V0vtx->numberDoF();
		  if(V0==LAMBDA) {
		    type_V1_decor(*cascadeVertices[0]) = "Lambda";
		  }
		  else if(V0==LAMBDABAR) {
		    type_V1_decor(*cascadeVertices[0]) = "Lambdabar";
		  }
		  else if(V0==KS) {
		    type_V1_decor(*cascadeVertices[0]) = "Ks";
		  }
		  mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*V0vtx) ? mAcc_gfit(*V0vtx) : 0;
		  mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*V0vtx) ? mAcc_gmass(*V0vtx) : -1;
		  mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*V0vtx) ? mAcc_gmasserr(*V0vtx) : -1;
		  mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*V0vtx) ? mAcc_gchisq(*V0vtx) : 999999;
		  mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*V0vtx) ? mAcc_gndof(*V0vtx) : 0;
		  mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*V0vtx) ? mAcc_gprob(*V0vtx) : -1;
		  trk_px.clear(); trk_py.clear(); trk_pz.clear();
		  trk_px.reserve(V0_helper.nRefTrks());
		  trk_py.reserve(V0_helper.nRefTrks());
		  trk_pz.reserve(V0_helper.nRefTrks());
		  for(auto&& vec3 : V0_helper.refTrks()) {
		    trk_px.push_back( vec3.Px() );
		    trk_py.push_back( vec3.Py() );
		    trk_pz.push_back( vec3.Pz() );
		  }
		  trk_pxDeco(*cascadeVertices[0]) = trk_px;
		  trk_pyDeco(*cascadeVertices[0]) = trk_py;
		  trk_pzDeco(*cascadeVertices[0]) = trk_pz;

		  result.push_back( std::make_pair(fit_result.release(),fit_result_mvc.release()) );
		}
		else result.push_back( std::make_pair(fit_result.release(),nullptr) );
	      }
	      else result.push_back( std::make_pair(fit_result.release(),nullptr) );
	    }
	    else result.push_back( std::make_pair(fit_result.release(),nullptr) );
	  }
	}
      } // loop over DpmCandidates
    } // for m_extraTrk1MassHypo>0 && m_extraTrk2MassHypo>0 && m_extraTrk3MassHypo>0

    return result;
  }

  std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> > JpsiXPlusDisplaced::fitMainVtx(const xAOD::Vertex* JXvtx, const std::vector<double>& massesJX, const XiCandidate& disVtx, const xAOD::TrackParticleContainer* trackContainer, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const {
    std::vector<std::pair<Trk::VxCascadeInfo*,Trk::VxCascadeInfo*> > result;

    std::vector<const xAOD::TrackParticle*> tracksJX;
    tracksJX.reserve(JXvtx->nTrackParticles());
    for(size_t i=0; i<JXvtx->nTrackParticles(); i++) tracksJX.push_back(JXvtx->trackParticle(i));
    if (tracksJX.size() != massesJX.size()) {
      ATH_MSG_ERROR("Problems with JX input: number of tracks or track mass inputs is not correct!");
      return result;
    }
    // Check identical tracks in input
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), disVtx.V0vtx->trackParticle(0)) != tracksJX.cend()) return result;
    if(std::find(tracksJX.cbegin(), tracksJX.cend(), disVtx.V0vtx->trackParticle(1)) != tracksJX.cend()) return result;
    std::vector<const xAOD::TrackParticle*> tracksV0;
    tracksV0.reserve(disVtx.V0vtx->nTrackParticles());
    for(size_t j=0; j<disVtx.V0vtx->nTrackParticles(); j++) tracksV0.push_back(disVtx.V0vtx->trackParticle(j));

    if(std::find(tracksJX.cbegin(), tracksJX.cend(), disVtx.track) != tracksJX.cend()) return result;
    std::vector<const xAOD::TrackParticle*> tracks3{disVtx.track};
    std::vector<double> massesDis3{m_disVDaug3MassHypo};

    std::vector<const xAOD::TrackParticle*> tracksJpsi{tracksJX[0], tracksJX[1]};
    std::vector<const xAOD::TrackParticle*> tracksX;
    if(m_jxDaug_num>=3) tracksX.push_back(tracksJX[2]);
    if(m_jxDaug_num==4) tracksX.push_back(tracksJX[3]);

    std::vector<double> massesV0;
    if(disVtx.V0type==LAMBDA) {
      massesV0 = m_massesV0_ppi;
    }
    else if(disVtx.V0type==LAMBDABAR) {
      massesV0 = m_massesV0_pip;
    }
    else if(disVtx.V0type==KS) {
      massesV0 = m_massesV0_pipi;
    }

    std::vector<double> massesDisV = massesV0; massesDisV.push_back(m_disVDaug3MassHypo);

    TLorentzVector p4_moth, p4_disV, tmp;
    for(size_t it=0; it<JXvtx->nTrackParticles(); it++) {
      tmp.SetPtEtaPhiM(JXvtx->trackParticle(it)->pt(), JXvtx->trackParticle(it)->eta(), JXvtx->trackParticle(it)->phi(), massesJX[it]);
      p4_moth += tmp;
    }
    p4_disV += disVtx.p4_V0track1; p4_disV += disVtx.p4_V0track2; p4_disV += disVtx.p4_disVtrack;
    p4_moth += p4_disV;

    SG::AuxElement::Decorator<float>       chi2_V0_decor("ChiSquared_V0");
    SG::AuxElement::Decorator<int>         ndof_V0_decor("nDoF_V0");
    SG::AuxElement::Decorator<std::string> type_V0_decor("Type_V0");

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
    SG::AuxElement::Decorator<float> trk_px_deco("TrackPx_DisVnc");
    SG::AuxElement::Decorator<float> trk_py_deco("TrackPy_DisVnc");
    SG::AuxElement::Decorator<float> trk_pz_deco("TrackPz_DisVnc");

    std::vector<float> trk_px;
    std::vector<float> trk_py;
    std::vector<float> trk_pz;

    if(m_extraTrk1MassHypo<=0) {
      double main_mass = p4_moth.M();
      if(m_useImprovedMass) {
        if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_disV).M() + m_massJpsi;
        else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_disV).M() + m_massJX;
        if(m_massDisV>0) main_mass += - p4_disV.M() + m_massDisV;
      }
      if (main_mass < m_MassLower || main_mass > m_MassUpper) return result;

      // Apply the user's settings to the fitter
      std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
      // Robustness: http://cdsweb.cern.ch/record/685551
      int robustness = 0;
      m_iVertexFitter->setRobustness(robustness, *state);
      // Build up the topology
      // Vertex list
      std::vector<Trk::VertexID> vrtList;
      std::vector<Trk::VertexID> vrtList2;
      // https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
      // V0 vertex
      Trk::VertexID vID1;
      if (m_constrV0) {
	vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,disVtx.V0type==KS?m_massKs:m_massLd);
      } else {
	vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
      }
      vrtList.push_back(vID1);
      // Displaced vertex
      Trk::VertexID vID2;
      if (m_constrDisV) {
	vID2 = m_iVertexFitter->nextVertex(tracks3,massesDis3,vrtList,*state,m_massDisV);
      } else {
	vID2 = m_iVertexFitter->nextVertex(tracks3,massesDis3,vrtList,*state);
      }
      vrtList2.push_back(vID2);
      Trk::VertexID vID3;
      if(m_JXSubVtx) {
	// JX vertex
	if (m_constrJX && m_jxDaug_num>2) {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state,m_massJX);
	} else {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state);
	}
	vrtList2.push_back(vID3);
	// Mother vertex includes two subvertices: DisV and JX
	std::vector<const xAOD::TrackParticle*> tp;
	std::vector<double> tp_masses;
	if(m_constrMainV) {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2,*state,m_massMainV);
	} else {
	  m_iVertexFitter->nextVertex(tp,tp_masses,vrtList2,*state);
	}
      }
      else { // m_JXSubVtx=false
	// Mother vertex includes just one subvertex (DisV) and JX tracks
	if(m_constrMainV) {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,*state,m_massMainV);
	} else {
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,vrtList2,*state);
	}
	if (m_constrJX && m_jxDaug_num>2) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for JX failed");
	  }
	}
      }
      if (m_constrJpsi) {
	std::vector<Trk::VertexID> cnstV;
	if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	}
      }
      if (m_constrX && m_jxDaug_num==4) {
	std::vector<Trk::VertexID> cnstV;
	if ( !m_iVertexFitter->addMassConstraint(vID3,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	  ATH_MSG_WARNING("addMassConstraint for X failed");
	}
      }
      // Do the work
      std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

      if (fit_result) {
	for(auto& v : fit_result->vertices()) {
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
	double lxy_SV1_sub = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	double lxy_SV1 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[iMoth]);

	if(chi2CutPassed && lxy_SV1>m_lxyDisV_cut && lxy_SV1_sub>m_lxyV0_cut) {
	  chi2_V0_decor(*cascadeVertices[0]) = disVtx.V0vtx->chiSquared();
	  ndof_V0_decor(*cascadeVertices[0]) = disVtx.V0vtx->numberDoF();
	  if(disVtx.V0type==LAMBDA) {
	    type_V0_decor(*cascadeVertices[0]) = "Lambda";
	  }
	  else if(disVtx.V0type==LAMBDABAR) {
	    type_V0_decor(*cascadeVertices[0]) = "Lambdabar";
	  }
	  else if(disVtx.V0type==KS) {
	    type_V0_decor(*cascadeVertices[0]) = "Ks";
	  }
	  mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*disVtx.V0vtx) ? mAcc_gfit(*disVtx.V0vtx) : 0;
	  mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*disVtx.V0vtx) ? mAcc_gmass(*disVtx.V0vtx) : -1;
	  mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*disVtx.V0vtx) ? mAcc_gmasserr(*disVtx.V0vtx) : -1;
	  mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*disVtx.V0vtx) ? mAcc_gchisq(*disVtx.V0vtx) : 999999;
	  mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*disVtx.V0vtx) ? mAcc_gndof(*disVtx.V0vtx) : 0;
	  mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*disVtx.V0vtx) ? mAcc_gprob(*disVtx.V0vtx) : -1;
	  trk_px.clear(); trk_py.clear(); trk_pz.clear();
	  trk_px.push_back( disVtx.p4_V0track1.Px() ); trk_px.push_back( disVtx.p4_V0track2.Px() );
	  trk_py.push_back( disVtx.p4_V0track1.Py() ); trk_py.push_back( disVtx.p4_V0track2.Py() );
	  trk_pz.push_back( disVtx.p4_V0track1.Pz() ); trk_pz.push_back( disVtx.p4_V0track2.Pz() );
	  trk_pxDeco(*cascadeVertices[0]) = trk_px;
	  trk_pyDeco(*cascadeVertices[0]) = trk_py;
	  trk_pzDeco(*cascadeVertices[0]) = trk_pz;
	  trk_px_deco(*cascadeVertices[1]) = disVtx.p4_disVtrack.Px();
	  trk_py_deco(*cascadeVertices[1]) = disVtx.p4_disVtrack.Py();
	  trk_pz_deco(*cascadeVertices[1]) = disVtx.p4_disVtrack.Pz();

	  result.push_back( std::make_pair(fit_result.release(),nullptr) );
	}
      }
    } // m_extraTrk1MassHypo<=0
    else { // m_extraTrk1MassHypo>0
      std::vector<double> massesExtra{m_extraTrk1MassHypo};
      std::vector<double> massesJXExtra = massesJX; massesJXExtra.push_back(m_extraTrk1MassHypo);

      for(const xAOD::TrackParticle* tpExtra : *trackContainer) {
	if ( tpExtra->pt()<m_extraTrk1MinPt ) continue;
	if ( !m_trkSelector->decision(*tpExtra, nullptr) ) continue;
	// Check identical tracks in input
	if(std::find(tracksJX.cbegin(),tracksJX.cend(),tpExtra) != tracksJX.cend()) continue;
	if(std::find(tracksV0.cbegin(),tracksV0.cend(),tpExtra) != tracksV0.cend()) continue;
	if(tpExtra == disVtx.track) continue;

	TLorentzVector tmp;
	tmp.SetPtEtaPhiM(tpExtra->pt(),tpExtra->eta(),tpExtra->phi(),m_extraTrk1MassHypo);
	double main_mass = (p4_moth+tmp).M();
	if(m_useImprovedMass) {
	  if(m_jxDaug_num==2 && m_massJpsi>0) main_mass += - (p4_moth - p4_disV).M() + m_massJpsi;
	  else if(m_jxDaug_num>=3 && m_massJX>0) main_mass += - (p4_moth - p4_disV).M() + m_massJX;
	  if(m_massDisV>0) main_mass += - p4_disV.M() + m_massDisV;
	}
	if (main_mass < m_MassLower || main_mass > m_MassUpper) continue;

	std::vector<const xAOD::TrackParticle*> tracksExtra{tpExtra};
	std::vector<const xAOD::TrackParticle*> tracksJXExtra = tracksJX; tracksJXExtra.push_back(tpExtra);

	// Apply the user's settings to the fitter
	std::unique_ptr<Trk::IVKalState> state = m_iVertexFitter->makeState();
	// Robustness: http://cdsweb.cern.ch/record/685551
	int robustness = 0;
	m_iVertexFitter->setRobustness(robustness, *state);
	// Build up the topology
	// Vertex list
	std::vector<Trk::VertexID> vrtList;
	std::vector<Trk::VertexID> vrtList2;
	// https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkVertexFitter/TrkVKalVrtFitter/TrkVKalVrtFitter/IVertexCascadeFitter.h
	// V0 vertex
	Trk::VertexID vID1;
	if (m_constrV0) {
	  vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state,disVtx.V0type==KS?m_massKs:m_massLd);
	} else {
	  vID1 = m_iVertexFitter->startVertex(tracksV0,massesV0,*state);
	}
	vrtList.push_back(vID1);
	// Displaced vertex
	Trk::VertexID vID2;
	if (m_constrDisV) {
	  vID2 = m_iVertexFitter->nextVertex(tracks3,massesDis3,vrtList,*state,m_massDisV);
	} else {
	  vID2 = m_iVertexFitter->nextVertex(tracks3,massesDis3,vrtList,*state);
	}
	vrtList2.push_back(vID2);
	Trk::VertexID vID3;
	if(m_JXSubVtx) {
	  // JXExtra vertex
	  vID3 = m_iVertexFitter->nextVertex(tracksJX,massesJX,*state);
	  vrtList2.push_back(vID3);
	  // Mother vertex
	  if(m_constrMainV) {
	    m_iVertexFitter->nextVertex(tracksExtra,massesExtra,vrtList2,*state,m_massMainV);
	  } else {
	    m_iVertexFitter->nextVertex(tracksExtra,massesExtra,vrtList2,*state);
	  }
	}
	else { // m_JXSubVtx=false
	  // Mother vertex includes just one subvertex (DisV) and JX tracks + extra track
	  if(m_constrMainV) {
	    vID3 = m_iVertexFitter->nextVertex(tracksJXExtra,massesJXExtra,vrtList2,*state,m_massMainV);
	  } else {
	    vID3 = m_iVertexFitter->nextVertex(tracksJXExtra,massesJXExtra,vrtList2,*state);
	  }
	}
	if (m_constrJX && m_jxDaug_num>2) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJX,cnstV,*state,m_massJX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for JX failed");
	  }
	}
	if (m_constrJpsi) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksJpsi,cnstV,*state,m_massJpsi).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for Jpsi failed");
	  }
	}
	if (m_constrX && m_jxDaug_num==4) {
	  std::vector<Trk::VertexID> cnstV;
	  if ( !m_iVertexFitter->addMassConstraint(vID3,tracksX,cnstV,*state,m_massX).isSuccess() ) {
	    ATH_MSG_WARNING("addMassConstraint for X failed");
	  }
	}
	// Do the work
	std::unique_ptr<Trk::VxCascadeInfo> fit_result = std::unique_ptr<Trk::VxCascadeInfo>( m_iVertexFitter->fitCascade(*state) );

	if (fit_result) {
	  for(auto& v : fit_result->vertices()) {
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
	  double lxy_SV1_sub = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
	  double lxy_SV1 = m_CascadeTools->lxy(moms[1],cascadeVertices[1],cascadeVertices[iMoth]);

	  if(chi2CutPassed && lxy_SV1>m_lxyDisV_cut && lxy_SV1_sub>m_lxyV0_cut) {
	    chi2_V0_decor(*cascadeVertices[0]) = disVtx.V0vtx->chiSquared();
	    ndof_V0_decor(*cascadeVertices[0]) = disVtx.V0vtx->numberDoF();
	    if(disVtx.V0type==LAMBDA) {
	      type_V0_decor(*cascadeVertices[0]) = "Lambda";
	    }
	    else if(disVtx.V0type==LAMBDABAR) {
	      type_V0_decor(*cascadeVertices[0]) = "Lambdabar";
	    }
	    else if(disVtx.V0type==KS) {
	      type_V0_decor(*cascadeVertices[0]) = "Ks";
	    }
	    mDec_gfit(*cascadeVertices[0])     = mAcc_gfit.isAvailable(*disVtx.V0vtx) ? mAcc_gfit(*disVtx.V0vtx) : 0;
	    mDec_gmass(*cascadeVertices[0])    = mAcc_gmass.isAvailable(*disVtx.V0vtx) ? mAcc_gmass(*disVtx.V0vtx) : -1;
	    mDec_gmasserr(*cascadeVertices[0]) = mAcc_gmasserr.isAvailable(*disVtx.V0vtx) ? mAcc_gmasserr(*disVtx.V0vtx) : -1;
	    mDec_gchisq(*cascadeVertices[0])   = mAcc_gchisq.isAvailable(*disVtx.V0vtx) ? mAcc_gchisq(*disVtx.V0vtx) : 999999;
	    mDec_gndof(*cascadeVertices[0])    = mAcc_gndof.isAvailable(*disVtx.V0vtx) ? mAcc_gndof(*disVtx.V0vtx) : 0;
	    mDec_gprob(*cascadeVertices[0])    = mAcc_gprob.isAvailable(*disVtx.V0vtx) ? mAcc_gprob(*disVtx.V0vtx) : -1;
	    trk_px.clear(); trk_py.clear(); trk_pz.clear();
	    trk_px.push_back( disVtx.p4_V0track1.Px() ); trk_px.push_back( disVtx.p4_V0track2.Px() );
	    trk_py.push_back( disVtx.p4_V0track1.Py() ); trk_py.push_back( disVtx.p4_V0track2.Py() );
	    trk_pz.push_back( disVtx.p4_V0track1.Pz() ); trk_pz.push_back( disVtx.p4_V0track2.Pz() );
	    trk_pxDeco(*cascadeVertices[0]) = trk_px;
	    trk_pyDeco(*cascadeVertices[0]) = trk_py;
	    trk_pzDeco(*cascadeVertices[0]) = trk_pz;
	    trk_px_deco(*cascadeVertices[1]) = disVtx.p4_disVtrack.Px();
	    trk_py_deco(*cascadeVertices[1]) = disVtx.p4_disVtrack.Py();
	    trk_pz_deco(*cascadeVertices[1]) = disVtx.p4_disVtrack.Pz();

	    result.push_back( std::make_pair(fit_result.release(),nullptr) );
	  }
	}
      } // loop over trackContainer
    } // m_extraTrk1MassHypo>0

    return result;
  }

  void JpsiXPlusDisplaced::fitV0Container(xAOD::VertexContainer* V0ContainerNew, const std::vector<const xAOD::TrackParticle*>& selectedTracks, const std::vector<const xAOD::TrackParticleContainer*>& trackCols) const {
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
  const xAOD::Vertex* JpsiXPlusDisplaced::FindVertex(const xAOD::VertexContainer* cont, const xAOD::Vertex* v) const {
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
