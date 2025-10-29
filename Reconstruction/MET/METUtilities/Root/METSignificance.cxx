///////////////////////// -*- C++ -*- /////////////////////////////
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// METSignificance.cxx
// Implementation file for class METSignificance
// Author: P.Francavilla<francav@cern.ch>
// Author: D.Schaefer<schae@cern.ch>
///////////////////////////////////////////////////////////////////

// METUtilities includes
#include "METUtilities/METSignificance.h"

// MET EDM
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"

// Jet EDM
#include "xAODJet/JetAttributes.h"

// Other xAOD EDM
#include "xAODTruth/TruthParticle.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "tauRecTools/TauCombinedTES.h"
#include "AsgTools/AsgToolConfig.h"
#include "PathResolver/PathResolver.h"

// Needed for xAOD::get_eta_calo() function

#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"

#ifdef XAOD_STANDALONE
#include "JetCalibTools/JetCalibrationTool.h"
#endif

namespace met {

  using iplink_t = ElementLink<xAOD::IParticleContainer>;

  static const SG::AuxElement::ConstAccessor<float> acc_varX("varX");
  static const SG::AuxElement::ConstAccessor<float> acc_varY("varY");
  static const SG::AuxElement::ConstAccessor<float> acc_covXY("covXY");
  static const SG::AuxElement::ConstAccessor<float> acc_jvt("Jvt");
  static const SG::AuxElement::ConstAccessor<float> acc_fjvt("fJvt");
  static const SG::AuxElement::ConstAccessor<float> acc_fjvt_der("DFCommonJets_fJvt");
  static const SG::AuxElement::ConstAccessor< std::vector<iplink_t > > acc_constitObjLinks("ConstitObjectLinks");
  const static MissingETBase::Types::bitmask_t invisSource = 0x100000; // doesn't overlap with any other

  METSignificance::METSignificance(const std::string& name) :
    AsgTool(name),
    m_GeV(1.0e3),
    m_jetOK(true),
    m_muonOK(true),
    m_egammaOK(true),
    m_tauOK(true),
    m_softTermParam(met::Random),
    m_jerForEMu(false),
    m_jetPtThr(-1.0),
    m_jetEtaThr(-1.0),
    m_significance(0.0),
    m_rho(0.0),
    m_VarL(0.0),
    m_VarT(0.0),
    m_CvLT(0.0),
    m_met_VarL(0.0),
    m_met_VarT(0.0),
    m_met_CvLT(0.0),
    m_met(0.0),
    m_metx(0.0),
    m_mety(0.0),
    m_metphi(0.0),
    m_metsoft(0.0),
    m_metsoftphi(0.0),
    m_ht(0.0),
    m_sumet(0.0),
    m_file(nullptr),
    m_phi_reso_pt20(nullptr),
    m_phi_reso_pt50(nullptr),
    m_phi_reso_pt100(nullptr)
  {
    declareProperty("SoftTermParam",        m_softTermParam = met::Random );
    declareProperty("SoftTermReso",         m_softTermReso  = 8.5        );
    declareProperty("TreatPUJets",          m_treatPUJets   = true        );
    declareProperty("DoPhiReso",            m_doPhiReso     = false       );
    declareProperty("ApplyBias",            m_applyBias     = false       );
    declareProperty("DoJerForEMu",          m_jerForEMu     = false       ); // run jet resolution for all electrons and muons
    declareProperty("ScalarBias",           m_scalarBias    = 0.0         );
    declareProperty("JetPtThr",             m_jetPtThr      = -1.0        );
    declareProperty("JetEtaThr",            m_jetEtaThr     = -1.0        );
    declareProperty("ConfigPrefix",         m_configPrefix  = "METUtilities/data17_13TeV/metsig_Aug15/");
    declareProperty("ConfigJetPhiResoFile", m_configJetPhiResoFile  = "jet_unc.root" );
    declareProperty("JetResoAux",           m_JetResoAux            = ""  ); // relative pT resolution in addition to normal JES
    declareProperty("EMuResoAux",           m_EMuResoAux            = ""  ); // aux string sets a bool for the leptons to run the jet resolation
    declareProperty("JetCollection",        m_JetCollection         = "AntiKt4EMPFlow" );

    // properties to delete eventually
    declareProperty("IsDataJet",   m_isDataJet     = false   );
    declareProperty("IsDataMuon",  m_isDataMuon    = false   );

    m_file = nullptr;
  }

  METSignificance::~METSignificance()= default;

  StatusCode METSignificance::initialize(){

    ATH_MSG_INFO ("Initializing " << name() << "...");
    ATH_MSG_INFO("Set up JER tools");
    if(m_JetCollection == "AntiKt4EMTopoJets"){
      ATH_MSG_WARNING(" tool wasn't updated for EMTopo jets so far and is not supported.");
    }
    // Phi resolution
    std::string configpath  = PathResolverFindCalibFile(m_configPrefix+m_configJetPhiResoFile);
    m_file = TFile::Open(configpath.c_str());
    if(m_file){
      m_phi_reso_pt20 = static_cast<TH2F *>(m_file->Get("phi_reso_pt20"));
      if(!m_phi_reso_pt20) ATH_MSG_ERROR("PU Jet Uncertainty Histogram not valid");
      m_phi_reso_pt50 = static_cast<TH2F *>(m_file->Get("phi_reso_pt50"));
      if(!m_phi_reso_pt50) ATH_MSG_ERROR("PU Jet Uncertainty Histogram not valid");
      m_phi_reso_pt100 = static_cast<TH2F *>(m_file->Get("phi_reso_pt100"));
      if(!m_phi_reso_pt100) ATH_MSG_ERROR("PU Jet Uncertainty Histogram not valid");
    }
    else{
      ATH_MSG_ERROR("PU Jet Uncertainty TFile is not valid: " << configpath);
      return StatusCode::FAILURE;
    }

    if (m_jetCalibTool.empty()){
      if(m_jetCalibConfig.empty() || m_jetCalibSeq.empty())
        m_jetOK = false;
      else{
        asg::AsgToolConfig toolConfig ("JetCalibrationTool/jetCalibTool_"+m_JetCollection);
        ATH_CHECK( toolConfig.setProperty("JetCollection",m_JetCollection) );
        ATH_CHECK( toolConfig.setProperty("ConfigFile",m_jetCalibConfig) );
        ATH_CHECK( toolConfig.setProperty("CalibSequence",m_jetCalibSeq) );
        if(!m_jetCalibArea.empty()) ATH_CHECK( toolConfig.setProperty("CalibArea",m_jetCalibArea) );
        ATH_CHECK( toolConfig.setProperty("IsData",false) ); // configure for MC due to technical reasons. Both data and MC smearing are available with this setting.
        ATH_CHECK( toolConfig.makePrivateTool (m_jetCalibTool) );
      }
    }
    if(m_jetOK) ATH_CHECK( m_jetCalibTool.retrieve() );
    else ATH_MSG_WARNING("No jet calibration tool or config provided for MET Significance");

    if (m_muonCalibrationAndSmearingTool.empty()) {
      if(m_muonCalibMode == -1)
        m_muonOK = false;
      else{
        asg::AsgToolConfig toolConfig ("CP::MuonCalibTool/METSigAutoConf_MuonCalibrationAndSmearingTool");
        ATH_CHECK(toolConfig.setProperty("calibMode", m_muonCalibMode));
        ATH_CHECK(toolConfig.makePrivateTool(m_muonCalibrationAndSmearingTool));
      }
    }
    if(m_muonOK) ATH_CHECK(m_muonCalibrationAndSmearingTool.retrieve());
    else ATH_MSG_WARNING("No muon calibration tool or config provided for MET Significance");

    if (m_egammaCalibTool.empty()){
      if(m_egESModel.empty() || m_egDecorrModel.empty())
        m_egammaOK = false;
      else{
        asg::AsgToolConfig toolConfig ("CP::EgammaCalibrationAndSmearingTool/METSigAutoConf_EgammaCalibrationAndSmearingTool");
        ATH_CHECK(toolConfig.setProperty("ESModel", m_egESModel));
        ATH_CHECK(toolConfig.setProperty("decorrelationModel", m_egDecorrModel));
        ATH_CHECK(toolConfig.setProperty("useFastSim", m_egUseFastsim ? 1 : 0));
        ATH_CHECK(toolConfig.makePrivateTool (m_egammaCalibTool));
      }
    }
    if(m_egammaOK) ATH_CHECK( m_egammaCalibTool.retrieve() );
    else ATH_MSG_WARNING("No egamma calibration tool or config provided for MET Significance");

    if (m_tauCombinedTES.empty()){
      if(m_tauTESConfig.empty())
        m_tauOK = false;
      else{
        asg::AsgToolConfig toolConfig ("TauCombinedTES/METSigAutoConf_TauPerfTool");
        ATH_CHECK( toolConfig.setProperty("WeightFileName", m_tauTESConfig) );
        ATH_CHECK( toolConfig.setProperty("useMvaResolution", m_tauUseMVARes) );
        ATH_CHECK( toolConfig.makePrivateTool(m_tauCombinedTES) );
      }
    }
    if(m_tauOK) ATH_CHECK( m_tauCombinedTES.retrieve() );
    else ATH_MSG_WARNING("No tau calibration tool or config provided for MET Significance");


    return StatusCode::SUCCESS;
  }

  StatusCode METSignificance::finalize(){

    ATH_MSG_INFO ("Finalizing " << name() << "...");
    delete m_phi_reso_pt20;
    delete m_phi_reso_pt50;
    delete m_phi_reso_pt100;

    return StatusCode::SUCCESS;
  }

  // **** Rebuild generic MET term ****
  StatusCode METSignificance::varianceMET(xAOD::MissingETContainer* metCont, float avgmu, const std::string& jetTermName, const std::string& softTermName, const std::string& totalMETName){

    // reset variables
    m_VarL = 0.0;
    m_VarT = 0.0;
    m_CvLT = 0.0;

    int metTerm = 0;
    double particle_sum[2][2] = {{0.0,0.0}, {0.0,0.0}};
    m_metphi = 0.0; //Angle for rotation of the cov matrix
    m_met = -1.0; // Numerator
    m_metsoft = 0.0;
    m_metsoftphi = 0.0;
    m_sumet=-1.0;
    m_ht=0.0;
    m_term_VarL.clear();
    m_term_VarT.clear();
    m_term_CvLT.clear();

    unsigned nIterSoft=0;
    double softSumET=0.0;

    // first fill the total MET
    if(metCont->find(totalMETName)!=metCont->end()){
      const auto &tot_met = static_cast<xAOD::MissingET*>(*(metCont->find(totalMETName)));
      if(!MissingETBase::Source::isTotalTerm(tot_met->source())){
        ATH_MSG_ERROR("NOT the total MET with name:" <<totalMETName);
        return StatusCode::SUCCESS;
      }
      m_met    = tot_met->met()/m_GeV;
      m_metx   = tot_met->mpx()/m_GeV;
      m_mety   = tot_met->mpy()/m_GeV;
      m_metphi = tot_met->phi();
      m_sumet  = tot_met->sumet()/m_GeV;
      m_ht     = m_sumet;
      ATH_MSG_VERBOSE("total MET: " << m_met << " phi: " << m_metphi << " name: " << tot_met->name());
    }
    else{
      ATH_MSG_ERROR("Could not find the total MET with name:" <<totalMETName);
      return StatusCode::SUCCESS;
    }
    m_met_vect.SetXYZ(m_metx, m_mety, 0);

    // Fill the remaining terms
    for(const auto met : *metCont) {

      // skip the invisible and total MET
      if(MissingETBase::Source::isTotalTerm(met->source())){
        ATH_MSG_VERBOSE("Total: " << met->name() << " val: " << met->met());
        continue;
      }
      if(met->source()==invisSource) continue;

      // Soft term collection
      if(MissingETBase::Source::isSoftTerm(met->source())){

        if(!MissingETBase::Source::hasPattern(met->source(),MissingETBase::Source::Signal::Track)) continue;
        ATH_MSG_VERBOSE("Soft Name: " << met->name());
        // make sure the container name matches
        if(met->name()!=softTermName || nIterSoft>0){
          if(nIterSoft>0) ATH_MSG_ERROR("Found multiple soft terms with the name:" <<softTermName << ". Your MET configuration is wrong!!!");
          continue;
        }
        ++nIterSoft;
        softSumET=(met->sumet()/m_GeV);

        AddSoftTerm(met, m_met_vect, particle_sum);
        m_metsoft = met->met()/m_GeV;
        m_metsoftphi = met->phi();
        metTerm = 2; // this is actually filled in AddSoftTerm
        // done with the soft term. go to the next term.
        continue;
      }
      ATH_MSG_VERBOSE("Add MET term " << met->name() );
      for(const auto& el : acc_constitObjLinks(*met)) {
        const xAOD::IParticle* obj(*el);
        float pt_reso=0.0, phi_reso=0.0;
        if(!obj){
          ATH_MSG_ERROR("Particle pointer is not valid. This will likely result in a crash " << obj);
          return StatusCode::FAILURE;
        }
        ATH_MSG_VERBOSE("pT: " << obj->pt() << " type: " << obj->type() << " truth: " << (obj->type()==xAOD::Type::TruthParticle));
        if(obj->type()==xAOD::Type::Muon || (obj->type()==xAOD::Type::TruthParticle && std::abs(static_cast<const xAOD::TruthParticle*>(obj)->pdgId())==13)){
          ATH_CHECK(AddMuon(obj, pt_reso, phi_reso, avgmu));
          metTerm=4;
        }
        else if(obj->type()==xAOD::Type::Jet){
          // make sure the container name matches
          if(met->name()!=jetTermName) continue;
          ATH_CHECK(AddJet(obj, pt_reso, phi_reso, avgmu));
          metTerm=1;
        }
        else if(obj->type()==xAOD::Type::Electron || (obj->type()==xAOD::Type::TruthParticle && std::abs(static_cast<const xAOD::TruthParticle*>(obj)->pdgId())==11)){
          ATH_CHECK(AddElectron(obj, pt_reso, phi_reso, avgmu));
          metTerm=3;
        }
        else if(obj->type()==xAOD::Type::Photon || (obj->type()==xAOD::Type::TruthParticle && std::abs(static_cast<const xAOD::TruthParticle*>(obj)->pdgId())==22)){
          ATH_CHECK(AddPhoton(obj, pt_reso, phi_reso));
          metTerm=5;
        }
        else if(obj->type()==xAOD::Type::Tau || (obj->type()==xAOD::Type::TruthParticle && std::abs(static_cast<const xAOD::TruthParticle*>(obj)->pdgId())==15)){
          ATH_CHECK(AddTau(obj, pt_reso, phi_reso));
          metTerm=6;
        }

        // compute NEW
        double particle_u[2][2]     = {{pt_reso*pt_reso*obj->pt()*obj->pt()/m_GeV/m_GeV,0.0},
                                        {0.0,phi_reso*phi_reso/m_GeV/m_GeV}};
        double particle_u_rot[2][2] = {{pt_reso*pt_reso*obj->pt()*obj->pt()/m_GeV/m_GeV,0.0},
                                        {0.0,phi_reso*phi_reso/m_GeV/m_GeV}};
        RotateXY(particle_u, particle_u_rot,m_met_vect.DeltaPhi(obj->p4().Vect()));
        m_VarL+=particle_u_rot[0][0];
        m_VarT+=particle_u_rot[1][1];
        m_CvLT+=particle_u_rot[0][1];

        // Save the resolutions separated for each object type
        AddResoMap(particle_u_rot[0][0],
                    particle_u_rot[1][1],
                    particle_u_rot[0][1],
                    metTerm);

        RotateXY (particle_u,   particle_u_rot, obj->p4().Phi()); // positive phi rotation
        AddMatrix(particle_sum, particle_u_rot, particle_sum);
        // END compute NEW

        ATH_MSG_VERBOSE("Resolution: " << pt_reso << " phi reso: " << phi_reso );
      }
    }

    // setting the MET directed variables for later phi rotations if requested
    m_met_VarL=m_VarL;
    m_met_VarT=m_VarT;
    m_met_CvLT=m_CvLT;

    if( m_VarL != 0 ){

      if(m_applyBias){
        TVector3 met_vect = m_met_vect;
        TVector3 soft_vect = m_soft_vect;

        // should be done to reset the phi as well...
        if(m_softTermParam==met::TSTParam){
          Double_t Bias_TST = BiasPtSoftdir(m_metsoft);
          Double_t MEx = m_met * std::cos(m_metphi) - Bias_TST * std::cos(m_metsoftphi);
          Double_t MEy = m_met * std::sin(m_metphi) - Bias_TST * std::sin(m_metsoftphi);
          met_vect.SetXYZ(MEx,MEy,0.0);
        }
        else if(m_softTermParam==met::PthardParam){
          m_soft_vect.SetPtEtaPhi(m_metsoft, 0.0, m_metsoftphi);
          m_pthard_vect = m_soft_vect - m_met_vect;
          Double_t PtSoftparaPH = m_pthard_vect.Mag()>0.0 ? (m_soft_vect.Dot(m_pthard_vect))/m_pthard_vect.Mag() : 0.0;
          Double_t Bias_pthard = Bias_PtSoftParall(PtSoftparaPH);
          Double_t MEx = m_met * std::cos(m_metphi) - Bias_pthard * std::cos(m_metsoftphi);
          Double_t MEy = m_met * std::sin(m_metphi) - Bias_pthard * std::sin(m_metsoftphi);
          met_vect.SetXYZ(MEx,MEy,0.0);
        }
        // Rotate  & compute
        ATH_CHECK(RotateToPhi(met_vect.Phi()));
        m_significance = Significance_LT(met_vect.Pt(), m_VarL, m_VarT, m_CvLT);
        m_rho = m_CvLT / std::sqrt( m_VarL * m_VarT ) ;
      }
      else{
        // standard calculation
        m_significance = Significance_LT(m_met, m_VarL, m_VarT, m_CvLT);
        m_rho = m_CvLT / std::sqrt( m_VarL * m_VarT ) ;
      }
      m_ht-=softSumET;
      ATH_MSG_VERBOSE("     Significance (squared): " << m_significance << " rho: " << GetRho()
                    << " MET: " << m_met << " phi: " << m_metphi << " SUMET: " << m_sumet << " HT: " << m_ht << " sigmaL: " << GetVarL()
                    << " sigmaT: " << GetVarT() << " MET/sqrt(SumEt): " << GetMETOverSqrtSumET()
                    << " MET/sqrt(HT): " << GetMETOverSqrtHT()
                    << " sqrt(signif): " << GetSignificance()
                    << " sqrt(signifDirectional): " << GetSigDirectional());
    }
    else
      ATH_MSG_DEBUG("Var_L is 0");

    return StatusCode::SUCCESS;
  }

  StatusCode METSignificance::RotateToPhi(float phi){

    // Rotation (components)
    std::tie(m_VarL, m_VarT, m_CvLT) = CovMatrixRotation(m_met_VarL , m_met_VarT, m_met_CvLT, (phi-m_metphi));

    if( m_VarL != 0 ){
      m_significance = Significance_LT(m_met,m_VarL,m_VarT,m_CvLT );
      m_rho = m_CvLT  / std::sqrt( m_VarL * m_VarT ) ;
    }
    ATH_MSG_DEBUG("     Significance (squared) at new phi: " << m_significance
                << " rho: " << GetRho()
                << " MET: " << m_met
                << " sigmaL: " << GetVarL()
                << " sigmaT: " << GetVarT() );

    return StatusCode::SUCCESS;
  }

  StatusCode METSignificance::SetLambda(const float px, const float py, const bool GeV){

    // compute the new direction
    double GeVConv = GeV ? 1.0 : m_GeV;
    m_lamda_vect.SetXYZ(px/GeVConv, py/GeVConv, 0.0);
    m_lamda_vect = (m_met_vect - m_lamda_vect);
    const double met_m_lamda = m_lamda_vect.Pt();

    // Rotation (components)
    std::tie(m_VarL, m_VarT, m_CvLT) = CovMatrixRotation(m_met_VarL , m_met_VarT, m_met_CvLT, (m_lamda_vect.Phi()-m_metphi));

    if( m_VarL != 0 ){
      m_significance = Significance_LT(met_m_lamda,m_VarL,m_VarT,m_CvLT );
      m_rho = m_CvLT  / std::sqrt( m_VarL * m_VarT ) ;
    }
    ATH_MSG_DEBUG("     Significance (squared) at new phi: " << m_significance
                << " rho: " << GetRho()
                << " MET: " << m_met
                << " sigmaL: " << GetVarL()
                << " sigmaT: " << GetVarT() );

    return StatusCode::SUCCESS;
  }

  // Muon propagation of resolution
  StatusCode METSignificance::AddMuon(const xAOD::IParticle* obj, float &pt_reso, float &phi_reso, float avgmu){

    int dettype = 0;
    bool DoEMuReso = false;
    ATH_MSG_VERBOSE("Particle type: " << obj->type());

    if(obj->type()==xAOD::Type::TruthParticle){
      pt_reso =0.01;
      if(obj->pt()>0.5e6) pt_reso=0.03;
      if(obj->pt()>1.0e6) pt_reso=0.1;// this is just a rough estimate for the time being until the interface can handle truth muons
    }
    else{
      if(!m_muonOK){
        ATH_MSG_ERROR("MET Significance received a muon but was not configured for muons!");
        return StatusCode::FAILURE;
      }
      const xAOD::Muon* muon(static_cast<const xAOD::Muon*>(obj));
      if(muon->muonType()==0){//Combined
        dettype=3;//CB
      }
      else if(muon->muonType()==1){//MuonStandAlone
        dettype=1;//MS
      }
      else if(muon->muonType()>1){//Segment, Calo, Silicon
        dettype=2;//ID
      }
      else{
        ATH_MSG_VERBOSE("This muon had none of the normal muon types (ID,MS,CB) - check this in detail");
        return StatusCode::FAILURE;
      }

      pt_reso=m_muonCalibrationAndSmearingTool->expectedResolution(dettype,*muon,!m_isDataMuon);
      if(m_doPhiReso) phi_reso = muon->pt()*0.001;
      // run the jet resolution for muons. for validation region extrapolation
      if(!m_EMuResoAux.empty()){
        SG::AuxElement::ConstAccessor<bool>  acc_EMReso(m_EMuResoAux);
        DoEMuReso = acc_EMReso.isAvailable(*muon) ? acc_EMReso(*muon) : false;
      }
      ATH_MSG_VERBOSE("muon: " << pt_reso << " dettype: " << dettype << " " << muon->pt() << " " << muon->p4().Eta() << " " << muon->p4().Phi());
    }// end reco setup

    // Common setup
    if(m_doPhiReso) phi_reso = obj->pt()*0.001;
    ATH_MSG_VERBOSE("muon: " << pt_reso << " dettype: " << dettype << " " << obj->pt() << " " << obj->p4().Eta() << " " << obj->p4().Phi());

    if(m_jerForEMu || DoEMuReso){
      bool treatPUJets = m_treatPUJets;
      m_treatPUJets=false; //turn off pileup jet treatement for this electron
      ATH_CHECK(AddJet(obj, pt_reso, phi_reso, avgmu));
      m_treatPUJets = treatPUJets; // reset value
    }

    return StatusCode::SUCCESS;
  }

  // Electron propagation of resolution
  StatusCode METSignificance::AddElectron(const xAOD::IParticle* obj, float &pt_reso, float &phi_reso, float avgmu){

    if(!m_egammaOK){
      ATH_MSG_ERROR("MET Significance received an electron but was not configured for egamma!");
      return StatusCode::FAILURE;
    }

    bool DoEMuReso = false;
    if(obj->type()==xAOD::Type::TruthParticle){
      pt_reso=m_egammaCalibTool->resolution(obj->e(),obj->eta(),obj->eta(),PATCore::ParticleType::Electron);
      if(m_doPhiReso) phi_reso = obj->pt()*0.004;
    }
    else{
      const xAOD::Electron* ele(static_cast<const xAOD::Electron*>(obj));
      const auto cl_etaCalo = xAOD::get_eta_calo(*(ele->caloCluster()), ele->author());
      pt_reso=m_egammaCalibTool->resolution(ele->e(),ele->caloCluster()->eta(),cl_etaCalo,PATCore::ParticleType::Electron);
      if(m_doPhiReso) phi_reso = ele->pt()*0.004;
      ATH_MSG_VERBOSE("el: " << pt_reso << " " << ele->pt() << " " << ele->p4().Eta() << " " << ele->p4().Phi());

      // run the jet resolution for muons. for validation region extrapolation
      if(!m_EMuResoAux.empty()){
        SG::AuxElement::ConstAccessor<bool>  acc_EMReso(m_EMuResoAux);
        DoEMuReso = acc_EMReso.isAvailable(*ele) ? acc_EMReso(*ele) : false;
      }
    }

    if(m_jerForEMu || DoEMuReso){
      bool treatPUJets = m_treatPUJets;
      m_treatPUJets=false; //turn off pileup jet treatement for this electron
      ATH_CHECK(AddJet(obj, pt_reso, phi_reso, avgmu));
      m_treatPUJets = treatPUJets; // reset value
    }
    return StatusCode::SUCCESS;
  }

  // Photon propagation of resolution
  StatusCode METSignificance::AddPhoton(const xAOD::IParticle* obj, float &pt_reso, float &phi_reso){

    if(!m_egammaOK){
      ATH_MSG_ERROR("MET Significance received a photon but was not configured for egamma!");
      return StatusCode::FAILURE;
    }

    if(obj->type()==xAOD::Type::TruthParticle){
      pt_reso=m_egammaCalibTool->resolution(obj->e(),obj->eta(),obj->eta(),PATCore::ParticleType::Electron); // leaving as an electron for the truth implementation rather than declaring a reco photon
      if(m_doPhiReso) phi_reso = obj->pt()*0.004;
    }
    else{
      const xAOD::Egamma* pho(static_cast<const xAOD::Egamma*>(obj));
      pt_reso=m_egammaCalibTool->getResolution(*pho);
      if(m_doPhiReso) phi_reso = pho->pt()*0.004;
      ATH_MSG_VERBOSE("pho: " << pt_reso << " " << pho->pt() << " " << pho->p4().Eta() << " " << pho->p4().Phi());
    }
    return StatusCode::SUCCESS;
  }

  // Jet propagation of resolution. returns the relative pT and phi resolution.
  StatusCode METSignificance::AddJet(const xAOD::IParticle* obj, float &pt_reso, float &phi_reso, float &avgmu){

    const xAOD::Jet* jet(static_cast<const xAOD::Jet*>(obj));
    double pt_reso_dbl_data=0.0, pt_reso_dbl_mc=0.0, pt_reso_dbl_max=0.0;

    // setting limits on jets if requested
    if(m_jetPtThr>0.0 && m_jetPtThr>jet->pt())          return StatusCode::SUCCESS;
    if(m_jetEtaThr>0.0 && m_jetEtaThr<std::abs(jet->eta())) return StatusCode::SUCCESS;

    if(!m_jetOK){
      ATH_MSG_ERROR("MET Significance received a jet but was not configured for jets!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_jetCalibTool->getNominalResolutionData(*jet, pt_reso_dbl_data));
    ATH_CHECK(m_jetCalibTool->getNominalResolutionMC(*jet, pt_reso_dbl_mc));
    pt_reso_dbl_max = std::max(pt_reso_dbl_data,pt_reso_dbl_mc);
    pt_reso = pt_reso_dbl_max;

    ATH_MSG_VERBOSE("jet: " << pt_reso  << " jetpT: " << jet->pt() << " " << jet->p4().Eta() << " " << jet->p4().Phi());

    // Add extra uncertainty for PU jets based on JVT
    if(m_treatPUJets){
      double jet_pu_unc  = 0.;
      if(acc_fjvt.isAvailable(*jet))
        jet_pu_unc = GetPUProb(jet->eta(), jet->phi(),jet->pt()/m_GeV, acc_jvt(*jet), acc_fjvt(*jet), avgmu);
      else if(acc_fjvt_der.isAvailable(*jet))
        jet_pu_unc = GetPUProb(jet->eta(), jet->phi(),jet->pt()/m_GeV, acc_jvt(*jet), acc_fjvt_der(*jet), avgmu);
      else{
        ATH_MSG_ERROR("No fJVT decoration available - must have treat pileup jets set to off or provide fJVT!");
        return StatusCode::FAILURE;
      }
      pt_reso = std::sqrt(jet_pu_unc*jet_pu_unc + pt_reso*pt_reso);
      ATH_MSG_VERBOSE("jet_pu_unc: " << jet_pu_unc);
    }

    // Use the phi resolution of the jets
    // needs to be finished
    if(m_doPhiReso){
      double jet_phi_unc = std::abs(GetPhiUnc(jet->eta(), jet->phi(),jet->pt()/m_GeV));
      phi_reso = jet->pt()*jet_phi_unc;
    }

    // Add user defined additional resolutions. For example, b-tagged jets
    if(!m_JetResoAux.empty()){
      SG::AuxElement::ConstAccessor<float> acc_extra(m_JetResoAux);
      if(acc_extra.isAvailable(*jet)){
        float extra_relative_pt_reso = acc_extra(*jet);
        pt_reso = std::sqrt(pt_reso*pt_reso + extra_relative_pt_reso*extra_relative_pt_reso);
      }
    }

    return StatusCode::SUCCESS;
  }

  // Tau propagation of resolution
  StatusCode METSignificance::AddTau(const xAOD::IParticle* obj, float &pt_reso, float &phi_reso){

    // tau objects
    if(obj->type()==xAOD::Type::TruthParticle){
      pt_reso= 0.1;
      if(m_doPhiReso) phi_reso = obj->pt()*0.01;
    }
    else{
      if(!m_tauOK){
        ATH_MSG_ERROR("MET Significance received a tau but was not configured for taus!");
        return StatusCode::FAILURE;
      }
      const xAOD::TauJet* tau(static_cast<const xAOD::TauJet*>(obj));
      if (auto *combp4 = dynamic_cast<TauCombinedTES*>(&*m_tauCombinedTES)) {
        pt_reso = combp4->getMvaEnergyResolution(*tau);
      }

      if(m_doPhiReso) phi_reso = tau->pt()*0.01;
      ATH_MSG_VERBOSE("tau: " << pt_reso << " " << tau->pt() << " " << tau->p4().Eta() << " " << tau->p4().Phi() << " phi reso: " << phi_reso);
    }
    return StatusCode::SUCCESS;
  }

  //
  // Soft term propagation of resolution
  //
  void METSignificance::AddSoftTerm(const xAOD::MissingET* soft, const TVector3 &met_vect, double (&particle_sum)[2][2]){

    if(m_softTermParam==met::Random){

      ATH_MSG_VERBOSE("Resolution Soft term set to 10GeV");

      m_soft_vect.SetXYZ(soft->mpx()/m_GeV, soft->mpy()/m_GeV, 0);

      double particle_u[2][2] = {{m_softTermReso*m_softTermReso,0.0},
                                 {0.0,m_softTermReso*m_softTermReso}};
      double particle_u_rot[2][2] = {{m_softTermReso*m_softTermReso,0.0},
                                     {0.0,m_softTermReso*m_softTermReso}};

      RotateXY(particle_u, particle_u_rot,met_vect.DeltaPhi(m_soft_vect));
      m_VarL+=particle_u_rot[0][0];
      m_VarT+=particle_u_rot[1][1];
      m_CvLT+=particle_u_rot[0][1];

      // Save the resolutions separated for each object type
      AddResoMap(particle_u_rot[0][0],
                 particle_u_rot[1][1],
                 particle_u_rot[0][1],
                 met::ResoSoft);

      RotateXY (particle_u,   particle_u_rot,-1.0*soft->phi()); // negative phi rotation
      AddMatrix(particle_sum, particle_u_rot,     particle_sum);

      ATH_MSG_VERBOSE("SOFT " << soft->name() <<" - pt_reso: " << m_softTermReso << " soft: " << soft->met() << " phi: " << soft->phi()
                   << " Var_L: " << particle_u_rot[0][0] << " Var_T: " << particle_u_rot[1][1]
                   << " " << particle_u_rot[0][1]);
    }
    else if (m_softTermParam==met::PthardParam){

      ATH_MSG_VERBOSE("Resolution Soft term parameterized in pthard direction");

      m_soft_vect.SetXYZ(soft->mpx()/m_GeV, soft->mpy()/m_GeV, 0);

      m_pthard_vect =  m_soft_vect - met_vect;

      double varTST = Var_Ptsoft(soft->met()/m_GeV);

      double particle_u[2][2] = {{varTST,0.0},
                                 {0.0,varTST}};
      double particle_u_rot[2][2] = {{varTST,0.0},
                                     {0.0,varTST}};

      RotateXY(particle_u, particle_u_rot,met_vect.DeltaPhi(m_pthard_vect));
      m_VarL+=particle_u_rot[0][0];
      m_VarT+=particle_u_rot[1][1];
      m_CvLT+=particle_u_rot[0][1];

      // Save the resolutions separated for each object type
      AddResoMap(particle_u_rot[0][0],
                 particle_u_rot[1][1],
                 particle_u_rot[0][1],
                 met::ResoSoft);

      RotateXY (particle_u,   particle_u_rot,-1.0*m_pthard_vect.Phi()); // negative phi rotation
      AddMatrix(particle_sum, particle_u_rot,     particle_sum);

    }
    else if (m_softTermParam==met::TSTParam){

      ATH_MSG_VERBOSE("Resolution Soft term parameterized in TST");

      m_soft_vect.SetXYZ(soft->mpx()/m_GeV, soft->mpy()/m_GeV, 0);

      double varTST = VarparPtSoftdir(soft->met()/m_GeV, soft->sumet()/m_GeV);

      double particle_u[2][2] = {{varTST,0.0},
                                 {0.0,varTST}};
      double particle_u_rot[2][2] = {{varTST,0.0},
                                     {0.0,varTST}};

      RotateXY(particle_u, particle_u_rot,met_vect.DeltaPhi(m_soft_vect));
      m_VarL+=particle_u_rot[0][0];
      m_VarT+=particle_u_rot[1][1];
      m_CvLT+=particle_u_rot[0][1];

      // Save the resolutions separated for each object type
      AddResoMap(particle_u_rot[0][0],
                 particle_u_rot[1][1],
                 particle_u_rot[0][1],
                 met::ResoSoft);

      RotateXY (particle_u,   particle_u_rot,-1.0*soft->phi()); // negative phi rotation
      AddMatrix(particle_sum, particle_u_rot,     particle_sum);

    }
    else{
      ATH_MSG_ERROR("Soft term parameterization is NOT defined for:" << m_softTermParam);
    }

  }

  double METSignificance::GetPUProb(double jet_eta, double /*jet_phi*/,
                                    double jet_pt,  double jet_jvt,
                                    double jet_fjvt,
                                    float avgmu) {

    double unc=0.0;

    // Coefficients from Doug Schaefer <schae@cern.ch> and the MET subgroup
    if(m_JetCollection == "AntiKt4EMTopoJets"){
      if(std::abs(jet_eta)<2.4){
        if(jet_pt<30){
          if(jet_jvt<0.11)      unc = 1;
          else if(jet_jvt<0.25) unc = 0.0730 + 0.0024 * avgmu + 0.00001 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 0.0995 + 0.0031 * avgmu + 0.00005 * avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 0.0311 + 0.0025 * avgmu + 0.00005 * avgmu * avgmu;
          else                  unc = 0.0308 -0.0010 * avgmu + 0.00006 * avgmu * avgmu ;
        }else if(jet_pt<40){
          if(jet_jvt<0.11)      unc = 1.;
          else if(jet_jvt<0.25) unc = 1.;
          else if(jet_jvt<0.85) unc = -0.0188 + 0.0039 * avgmu + 0.00002 * avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 0.0252 -0.0009 * avgmu + 0.00006 * avgmu * avgmu  ;
          else                  unc = 0.0085 -0.0003 * avgmu + 0.00002 * avgmu * avgmu  ;
        }else if(jet_pt<50){
          if(jet_jvt<0.11)      unc = 1;
          else if(jet_jvt<0.25) unc = 0.0345 -0.0006 * avgmu + 0.00004 * avgmu * avgmu  ;
          else if(jet_jvt<0.85) unc = 0.1078 -0.0051 * avgmu + 0.00011 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = -0.0026 + 0.0005 * avgmu + 0.00002 * avgmu * avgmu;
          else                  unc = 0.0090 -0.0004 * avgmu + 0.00001 * avgmu * avgmu  ;
        }else if(jet_pt<60){
          if(jet_jvt<0.11)      unc = 1;
          else if(jet_jvt<0.25) unc = -0.0321 + 0.0030 * avgmu -0.00002 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 0.0260 -0.0007 * avgmu + 0.00003 * avgmu * avgmu ;
          else                  unc = -0.0040 + 0.0003 * avgmu;
        }else if(jet_pt<100){
          unc = 0.9492 -2.0757 * jet_jvt + 1.13328 * jet_jvt * jet_jvt;
        }else if(jet_pt<150){
          unc = 0.7888 -1.8372 * jet_jvt + 1.05539 * jet_jvt * jet_jvt;
        }
      }else if(std::abs(jet_eta)<2.6){
        if(jet_pt<30){
          if(jet_jvt<0.11)      unc = 0.2633 + 0.0091 * avgmu + -0.00009 * avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 0.1841 + 0.0144 * avgmu + -0.00008 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 0.1401 + 0.0048 * avgmu + 0.00006 * avgmu * avgmu ;
          else if(jet_jvt<0.95) unc = -0.0118 + 0.0076 * avgmu + 0.00003 * avgmu * avgmu;
          else                  unc = 0.0534 + -0.0011 * avgmu + 0.00010 * avgmu * avgmu;
        }else if(jet_pt<40){
          if(jet_jvt<0.11)      unc = 0.1497 + 0.0133 * avgmu + -0.00015 * avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = -0.2260 + 0.0276 * avgmu + -0.00021 * avgmu * avgmu ;
          else if(jet_jvt<0.85) unc = 0.2743 + -0.0093 * avgmu + 0.00022 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 0.0604 + 0.0006 * avgmu + 0.00006 * avgmu * avgmu   ;
          else                  unc = 0.0478 + -0.0009 * avgmu + 0.00004 * avgmu * avgmu  ;
        }else if(jet_pt<50){
          if(jet_jvt<0.11)      unc = -0.2187 + 0.0317 * avgmu + -0.00037 * avgmu * avgmu ;
          else if(jet_jvt<0.25) unc = 0.0964 + 0.0053 * avgmu + 0.00002 * avgmu * avgmu   ;
          else if(jet_jvt<0.85) unc = 1.1730 + -0.0624 * avgmu + 0.00088 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = -0.2011 + 0.0151 * avgmu + -0.00018 * avgmu * avgmu ;
          else                  unc = 0.0145 + -0.0003 * avgmu + 0.00002 * avgmu * avgmu  ;
        }else if(jet_pt<60){
          if(jet_jvt<0.11)      unc = 0.0051 + 0.0113 * avgmu + -0.00008 * avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = -0.1024 + 0.0109 * avgmu + -0.00006 * avgmu * avgmu ;
          else if(jet_jvt<0.85) unc = 1.2491 + -0.0501 * avgmu + 0.00052 * avgmu * avgmu  ;
          else                  unc = 0.0267 + -0.0014 * avgmu + 0.00003 * avgmu * avgmu  ;
        }else if(jet_pt<100){
          unc = 0.8951 -2.4995 * jet_jvt + 1.63229 * jet_jvt * jet_jvt;
        }else if(jet_pt<150){
          unc = 0.9998 -1.7319 * jet_jvt + 0.72680 * jet_jvt * jet_jvt;
        }
      }else if(std::abs(jet_eta)<2.7){
        if(jet_pt<30){
          if(jet_jvt<0.11)      unc = 0.3001 + 0.0054 * avgmu -0.00004 * avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = 0.0663 + 0.0198 * avgmu -0.00013 * avgmu * avgmu  ;
          else if(jet_jvt<0.85) unc = -0.0842 + 0.0163 * avgmu -0.00008 * avgmu * avgmu ;
          else if(jet_jvt<0.95) unc = -0.0219 + 0.0080 * avgmu + 0.00003 * avgmu * avgmu;
          else                  unc = 0.0461 -0.0003 * avgmu + 0.00012 * avgmu * avgmu  ;
        }else if(jet_pt<40){
          if(jet_jvt<0.11)      unc = 0.1885 + 0.0083 * avgmu -0.00006 * avgmu * avgmu ;
          else if(jet_jvt<0.25) unc = -0.0286 + 0.0150 * avgmu -0.00007 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 0.0152 + 0.0028 * avgmu + 0.00005 * avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 0.1815 -0.0076 * avgmu + 0.00018 * avgmu * avgmu ;
          else                  unc = 0.0192 -0.0003 * avgmu + 0.00007 * avgmu * avgmu ;
        }else if(jet_pt<50){
          if(jet_jvt<0.11)      unc = 0.1257 + 0.0074 * avgmu -0.00004 * avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = -0.0276 + 0.0080 * avgmu + 0.00000 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 0.1403 -0.0051 * avgmu + 0.00009 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 0.2078 -0.0101 * avgmu + 0.00017 * avgmu * avgmu  ;
          else                  unc = 0.2597 -0.0132 * avgmu + 0.00020 * avgmu * avgmu  ;
        }else if(jet_pt<60){
          if(jet_jvt<0.11)      unc = 0.1111 + 0.0045 * avgmu -0.00000 * avgmu * avgmu ;
          else if(jet_jvt<0.25) unc = 0.0975 -0.0011 * avgmu + 0.00008 * avgmu * avgmu ;
          else if(jet_jvt<0.85) unc = 0.0920 -0.0053 * avgmu + 0.00013 * avgmu * avgmu ;
          else                  unc = -0.0071 + 0.0016 * avgmu -0.00001 * avgmu * avgmu;
        }else if(jet_pt<100){
          unc = 0.4660 -1.2116 * jet_jvt + 0.78807 * jet_jvt * jet_jvt;
        }else if(jet_pt<150){
          unc = 0.2254 -0.5476 * jet_jvt + 0.32617 * jet_jvt * jet_jvt;
        }
      }// end eta 2.7
      else{//forward jets
        float fjvt = jet_fjvt>0.6 ? 0.6 : jet_fjvt; // the pileup more or less plateaus at 0.6
        if(jet_pt<30)       unc = 0.5106 + 1.2566 * fjvt -1.15060  * fjvt * fjvt;
        else if(jet_pt<40)  unc = 0.2972 + 1.9418 * fjvt -1.82694  * fjvt * fjvt;
        else if(jet_pt<50)  unc = 0.1543 + 1.9864 * fjvt -1.48429  * fjvt * fjvt;
        else if(jet_pt<60)  unc = 0.1050 + 1.3196 * fjvt + 0.03554 * fjvt * fjvt;
        else if(jet_pt<120) unc = 0.0400 + 0.5653 * fjvt + 1.96323 * fjvt * fjvt;
        // max of 0.9 seems reasonable
        if(jet_fjvt>0.6) unc = 0.9;
      }
      // end emtopo
      // Coefficients from Badr-eddine Ngair <badr-eddine.ngair@cern.ch>
    //  Pile-UP (resolution) estimatetd using Z->ee Events using exclusive jet_pt binning using RUN3 samples
    }else{//p-flow inputs
      if(std::abs(jet_eta)<2.4){
        if(jet_pt<30){
          if(jet_jvt<0.11)      unc = 0.524466 + 0.00750057 * avgmu -4.73422e-05 * avgmu * avgmu ;
          else if(jet_jvt<0.25) unc = 4.17584e-01 + 1.00112e-02  * avgmu -7.43546e-05 * avgmu * avgmu ;
          else if(jet_jvt<0.85) unc = 2.12625e-01 + 1.03484e-02 * avgmu  -5.68063e-05  * avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 1.08396e-01 + 1.04273e-02 * avgmu -5.00299e-05 * avgmu * avgmu;
          else                  unc = 1.26304e-03 +2.10385e-04  * avgmu + 1.10086e-06 * avgmu * avgmu ;
        }else if(jet_pt<40){
          if(jet_jvt<0.11)      unc = 3.78090e-01 + 8.83535e-03 * avgmu  -5.38873e-05* avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 2.67244e-01+ 9.81193e-03* avgmu  -5.87765e-05  * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 9.60892e-02 +8.11069e-03 * avgmu  -3.73101e-05 * avgmu * avgmu ;
          else if(jet_jvt<0.95) unc = 5.16235e-02 +6.27371e-03 * avgmu -1.95433e-05 * avgmu * avgmu ;
          else                  unc = -2.74714e-03 +2.45273e-04* avgmu -1.44731e-06* avgmu * avgmu ;
        }else if(jet_pt<50){
          if(jet_jvt<0.11)      unc = 2.44953e-01 + 1.23246e-02 * avgmu  -8.48696e-05 * avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 1.78141e-01 + 1.09451e-02  * avgmu  -6.90796e-05 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 9.60998e-02+6.21945e-03* avgmu -2.76203e-05* avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 5.79210e-02+ 4.49780e-03 * avgmu + -1.15125e-05 * avgmu * avgmu ;
          else                  unc = -2.96644e-03 +2.27707e-04 * avgmu -1.86712e-06 * avgmu * avgmu  ;
        }else if(jet_pt<60){
          if(jet_jvt<0.11)      unc = 1.97017e-01 + 1.34089e-02 * avgmu  -9.18923e-05 * avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = 1.27602e-01 +  1.12287e-02  * avgmu -6.62192e-05  * avgmu * avgmu  ;
          else if(jet_jvt<0.85) unc = 6.94905e-02 + 6.27784e-03 * avgmu -3.07298e-05  * avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 3.58417e-02  +4.62268e-03  * avgmu -2.12417e-05 * avgmu * avgmu ;
          else                  unc = 1.35616e-03  + 5.46723e-06* avgmu + 1.92327e-07 * avgmu * avgmu;
        }else if(jet_pt<100){
          unc = 6.19009e-01 -8.96042e-01 * jet_jvt + 2.89066e-01 * jet_jvt * jet_jvt;
        }else if(jet_pt<150){
          unc = 6.18350e-01 -8.97327e-01 * jet_jvt + 2.90998e-01 * jet_jvt * jet_jvt;
        }
      }else if(std::abs(jet_eta)<2.6){
        if(jet_pt<30){
          if(jet_jvt<0.11)      unc = 5.06496e-01 + 8.21123e-03 * avgmu -5.17501e-05  * avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 4.26616e-01  + 9.25936e-03  * avgmu -5.68847e-05  * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 2.03333e-01 + 1.11951e-02 * avgmu-6.09233e-05 * avgmu * avgmu ;
          else if(jet_jvt<0.95) unc = 1.03167e-01  + 1.13444e-02 * avgmu  -5.43274e-05 * avgmu * avgmu;
          else                  unc = 1.51480e-03 + 2.08394e-04 * avgmu + 1.39579e-06 * avgmu * avgmu;
        }else if(jet_pt<40){
          if(jet_jvt<0.11)      unc = 3.40612e-01 + 9.94199e-03 * avgmu + -5.93760e-05* avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = 2.43360e-01 + 1.05579e-02* avgmu + -6.05403e-05* avgmu * avgmu ;
          else if(jet_jvt<0.85) unc = 8.34364e-02 + 8.76364e-03 * avgmu  -3.64035e-05  * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 4.40362e-02 + 6.92580e-03  * avgmu  -1.79853e-05 * avgmu * avgmu   ;
          else                  unc = -2.68670e-03 + 2.50861e-04 * avgmu + -1.46410e-06* avgmu * avgmu  ;
        }else if(jet_pt<50){
          if(jet_jvt<0.11)      unc = 2.36561e-01 + 1.14078e-02 * avgmu +-7.10025e-05 * avgmu * avgmu ;
          else if(jet_jvt<0.25) unc = 1.86653e-01 +9.61140e-03 * avgmu +-5.15356e-05 * avgmu * avgmu   ;
          else if(jet_jvt<0.85) unc = 9.37026e-02 +5.93028e-03 * avgmu +-2.02571e-05 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 5.79210e-02+ 4.49780e-03 * avgmu + -1.15125e-05 * avgmu * avgmu ;
          else                  unc = -3.02487e-03 + 2.31337e-04* avgmu + -1.85225e-06  * avgmu * avgmu  ;
        }else if(jet_pt<60){
          if(jet_jvt<0.11)      unc =1.75215e-01+ 1.21805e-02 * avgmu + -7.48846e-05 * avgmu * avgmu  ;
          else if(jet_jvt<0.25) unc = 1.26276e-01+ 9.80117e-03  * avgmu + -4.99913e-05  * avgmu * avgmu ;
          else if(jet_jvt<0.85) unc = 7.91422e-02 + 5.26009e-03 * avgmu +-1.87388e-05 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 4.39136e-02+ 4.09435e-03* avgmu +-1.35926e-05  * avgmu * avgmu ;
          else                  unc = 1.21410e-03 + 1.14188e-05 * avgmu + 1.53654e-07 * avgmu * avgmu  ;
        }else if( jet_pt<100){
          unc = 6.44179e-01 -9.20194e-01* jet_jvt + 2.89686e-01 * jet_jvt * jet_jvt;
        }else if(jet_pt<150){
          unc = 6.43423e-01 -9.21407e-01 * jet_jvt + 2.91648e-01 * jet_jvt * jet_jvt;
        }
      }else if(std::abs(jet_eta)<2.7){
        if(jet_pt<30){
          if(jet_jvt<0.11)      unc = 4.76243e-01 + 9.22046e-03 * avgmu -5.88765e-05 * avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 4.07406e-01+ 1.01167e-02  * avgmu -6.30429e-05 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 2.01324e-01+ 1.20631e-02 * avgmu  -6.75582e-05 * avgmu * avgmu;
          else if(jet_jvt<0.95) unc =  1.03815e-01 + 1.24007e-02 * avgmu  -6.26892e-05  * avgmu * avgmu;
          else                  unc = 1.63714e-03 +2.00682e-04  * avgmu +  1.53621e-06 * avgmu * avgmu;
        }else if(jet_pt<40){
          if(jet_jvt<0.11)      unc = 2.89505e-01 + 1.11643e-02 * avgmu  -6.45475e-05* avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 2.15968e-01 + 1.14451e-02* avgmu  -6.32545e-05 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 7.92319e-02 +9.66239e-03* avgmu -3.81872e-05 * avgmu * avgmu  ;
          else if(jet_jvt<0.95) unc = 4.25501e-02  +7.90022e-03  * avgmu  -1.93561e-05   * avgmu * avgmu;
          else                  unc = -2.68089e-03 +2.50117e-04 * avgmu -1.43591e-06 * avgmu * avgmu  ;
        }else if(jet_pt<50){
          if(jet_jvt<0.11)      unc = 1.66062e-01+ 1.21029e-02  * avgmu  -7.00743e-05 * avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 1.36874e-01+ 1.03543e-02  * avgmu  -5.13482e-05 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 7.24015e-02  +6.72611e-03 * avgmu  -1.98316e-05 * avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 4.84508e-02  +5.10548e-03* avgmu -8.65067e-06  * avgmu * avgmu;
          else                  unc = -3.02710e-03  +2.31422e-04* avgmu  -1.84795e-06 * avgmu * avgmu;
        }else if(jet_pt<60){
          if(jet_jvt<0.11)      unc =1.33626e-01 +  1.02813e-02   * avgmu  -4.87698e-05 * avgmu * avgmu;
          else if(jet_jvt<0.25) unc = 1.06724e-01 + 8.45131e-03  * avgmu -3.26833e-05 * avgmu * avgmu;
          else if(jet_jvt<0.85) unc = 7.47468e-02 +4.85387e-03 * avgmu  -1.01430e-05* avgmu * avgmu;
          else if(jet_jvt<0.95) unc = 4.57521e-02   + 3.86782e-03 * avgmu -6.38948e-06  * avgmu * avgmu;
          else                  unc = 1.20495e-03   +1.18941e-05* avgmu  +1.47846e-07  * avgmu * avgmu;
        }else if( jet_pt<100){
          unc = 6.59079e-01 -9.29754e-01 * jet_jvt + 2.83653e-01 * jet_jvt * jet_jvt;
        }else if(jet_pt<150){
          unc = 6.58295e-01 -9.31032e-01 * jet_jvt + 2.85724e-01 * jet_jvt * jet_jvt;
        }
      }// end eta 2.7
      else{//forward jets
        float fjvt = jet_fjvt>0.6 ? 0.6 : jet_fjvt; // the pileup more or less plateaus at 0.6
        if(jet_pt<30)       unc = 0.605329 + 0.625734 * fjvt -0.42484  * fjvt * fjvt;
        else if(jet_pt<40)  unc = 0.409696 + 1.00173 * fjvt -0.609179 * fjvt * fjvt;
        else if(jet_pt<50)  unc = 0.173755 + 1.48847 * fjvt -0.803771    * fjvt * fjvt;
        else if(jet_pt<60)  unc = 0.0140303 + 1.79909 * fjvt -0.889274 * fjvt * fjvt;
        else if(jet_pt<120) unc = -0.0828333 + 1.81167  * fjvt -0.716881 * fjvt * fjvt;
        // max of 0.9 seems reasonable
        if(jet_fjvt>0.6) unc = 0.9;
      }
    }// end pflow

    unc = std::min(unc, 1.0);
    unc = std::max(unc, 0.0);

    return unc;
  }

  double METSignificance::GetPhiUnc(double jet_eta, double jet_phi,double jet_pt){

    unsigned int xbin = getEtaBin(jet_eta);
    unsigned int ybin = jet_phi>0.0 ? int(jet_phi/0.4)+9 : int(jet_phi/0.4)+8;

    // Stored as bin content = Mean, error = RMS, we want to use the RMS.
    if(!m_phi_reso_pt20 || !m_phi_reso_pt50 || !m_phi_reso_pt100){
      ATH_MSG_ERROR("Jet Phi Resolution histograms are invalid.");
      return 0.0;
    }

    // Collect the phi resolution
    if(jet_pt<50.0)
      return m_phi_reso_pt20->GetBinError(xbin, ybin);
    else if(jet_pt<100.0)
      return m_phi_reso_pt50->GetBinError(xbin, ybin);
    return m_phi_reso_pt100->GetBinError(xbin, ybin);
  }

  unsigned int METSignificance::getEtaBin(double jet_eta){
    // For the phi uncertainty lookup
    if(-4.5<jet_eta && -3.8>=jet_eta)      return 1;
    else if(-3.8<jet_eta && -3.5>=jet_eta) return 2;
    else if(-3.5<jet_eta && -3.0>=jet_eta) return 3;
    else if(-3.0<jet_eta && -2.7>=jet_eta) return 4;
    else if(-2.7<jet_eta && -2.4>=jet_eta) return 5;
    else if(-2.4<jet_eta && -1.5>=jet_eta) return 6;
    else if(-1.5<jet_eta && -0.5>=jet_eta) return 7;
    else if(-0.5<jet_eta &&  0.0>=jet_eta) return 8;
    else if(0.0<jet_eta  &&  0.5>=jet_eta) return 9;
    else if(0.5<jet_eta  &&  1.5>=jet_eta) return 10;
    else if(1.5<jet_eta  &&  2.4>=jet_eta) return 11;
    else if(2.4<jet_eta  &&  2.7>=jet_eta) return 12;
    else if(2.7<jet_eta  &&  3.0>=jet_eta) return 13;
    else if(3.0<jet_eta  &&  3.5>=jet_eta) return 14;
    else if(3.5<jet_eta  &&  3.8>=jet_eta) return 15;
    else if(3.8<jet_eta                  ) return 16;
    return 0;
  }

  std::tuple<double,double,double> METSignificance::CovMatrixRotation(double var_x, double var_y, double cv_xy, double Phi){
    // Covariance matrix parallel and transverse to the Phi direction
    Double_t V11 = std::pow(std::cos(Phi),2)*var_x + 2*std::sin(Phi)*std::cos(Phi)*cv_xy + std::pow(std::sin(Phi),2)*var_y;
    Double_t V22 = std::pow(std::sin(Phi),2)*var_x - 2*std::sin(Phi)*std::cos(Phi)*cv_xy + std::pow(std::cos(Phi),2)*var_y;
    Double_t V12 = std::pow(std::cos(Phi),2)*cv_xy -std::sin(Phi)*std::cos(Phi)*var_x + std::sin(Phi)*std::cos(Phi)*var_y - std::pow(std::sin(Phi),2)*cv_xy;   // rho is equal to one for just one jet
    return  std::make_tuple( V11, V22, V12);
  }

  double METSignificance::Significance_LT(double Numerator, double var_parall, double var_perpen, double cov){

    Double_t rho = cov / std::sqrt( var_parall * var_perpen ) ;
    Double_t Significance = 0;
    if (std::abs( rho ) >= 0.9 ){  //Cov Max not invertible -> Significance diverges
      ATH_MSG_VERBOSE("rho is large: " << rho);
      Significance = std::pow( Numerator - m_scalarBias , 2 ) / (  var_parall  ) ;
    }
    else
      Significance = std::pow( Numerator - m_scalarBias , 2 ) / (  var_parall * ( 1 - std::pow(rho,2) ) ) ;

    if( std::abs(Significance) >= 10e+15)
      ATH_MSG_WARNING("warning -->"<< Significance);

    return Significance;
  }

  void METSignificance::InvertMatrix(double (&mat)[2][2], double (&m)[2][2]){

    // determinant
    double det = mat[0][0]*mat[1][1]-mat[0][1]*mat[1][0];

    m[0][0]=0.0;
    m[0][1]=0.0;
    m[1][0]=0.0;
    m[1][1]=0.0;

    if(det==0.0) return;

    m[0][0]= 1.0/det*(mat[1][1]);
    m[1][0]=-1.0/det*(mat[1][0]);
    m[0][1]=-1.0/det*(mat[0][1]);
    m[1][1]= 1.0/det*(mat[0][0]);
  }

  void METSignificance::AddMatrix(double (&X)[2][2],double (&Y)[2][2], double (&mat_new)[2][2]){
    mat_new[0][0]=X[0][0]+Y[0][0];
    mat_new[0][1]=X[0][1]+Y[0][1];
    mat_new[1][0]=X[1][0]+Y[1][0];
    mat_new[1][1]=X[1][1]+Y[1][1];
  }

  void METSignificance::RotateXY(const double (&mat)[2][2], double (&mat_new)[2][2], double phi){

    double c = std::cos(phi);
    double s = std::sin(phi);
    double cc = c*c;
    double ss = s*s;
    double cs = c*s;

    double V11 = mat[0][0]*cc + mat[1][1]*ss - cs*(mat[1][0] + mat[0][1]);
    double V12 = mat[0][1]*cc - mat[1][0]*ss + cs*(mat[0][0] - mat[1][1]);
    double V21 = mat[1][0]*cc - mat[0][1]*ss + cs*(mat[0][0] - mat[1][1]);
    double V22 = mat[0][0]*ss + mat[1][1]*cc + cs*(mat[1][0] + mat[0][1]);

    mat_new[0][0]=V11;
    mat_new[0][1]=V12;
    mat_new[1][0]=V21;
    mat_new[1][1]=V22;
  }

  /// Parameterization with PtSoft Direction //
  // Coefficients from Doug Schaefer <schae@cern.ch> and the MET subgroup
  double METSignificance::BiasPtSoftdir(const double PtSoft){
    if (PtSoft<60.) return (0.145)+(-0.45)*PtSoft;
    else return (0.145)+(-0.45)*(60.);
  }

  // variation in ptsoft direction
  // Coefficients from Doug Schaefer <schae@cern.ch> and the MET subgroup
  double METSignificance::VarparPtSoftdir(const double PtSoft, const double SoftSumet){
    if (SoftSumet<25){
      if (PtSoft<50.) return 41.9+3.8*PtSoft+0.1*std::pow(PtSoft,2)-12.7+ 1.39*SoftSumet-0.03*std::pow(SoftSumet,2);
      else return 41.9+3.8*50.+0.1*std::pow(50.,2)-12.7+ 1.39*SoftSumet-0.03*std::pow(SoftSumet,2);
    }
    else{
      if (PtSoft<50.) return 41.9+3.8*PtSoft+0.1*std::pow(PtSoft,2);
      else return (40.5614)+(4.10965)*50.+(0.0955044)*std::pow(50.,2);
    }
  }

  // Coefficients from Doug Schaefer <schae@cern.ch> and the MET subgroup
  double METSignificance::Var_Ptsoft(const double PtSoft){
    if (PtSoft<45.) return 40. + 2*PtSoft + 0.1*std::pow(PtSoft,2);
    else return 40. + 2*45 + 0.1*std::pow(45,2);
  }

  // Coefficients from Doug Schaefer <schae@cern.ch> and the MET subgroup
  double METSignificance::Bias_PtSoftParall(const double PtSoft_Parall)
  {
    if (-60.<=PtSoft_Parall && PtSoft_Parall<0.) return -8. -0.4*PtSoft_Parall;
    if (-60.>PtSoft_Parall) return -8. -0.4 * (-60.);
    if( PtSoft_Parall>=0. && PtSoft_Parall<60.) return -8. -PtSoft_Parall;
    if(PtSoft_Parall>60.) return -8. -60.;
    return 0.0;
  }

  void METSignificance::AddResoMap(const double varL, const double varT, const double CvLT, const int term){

    m_term_VarL[term] += varL;
    m_term_VarT[term] += varT;
    m_term_CvLT[term] += CvLT;

  }

} //> end namespace met
