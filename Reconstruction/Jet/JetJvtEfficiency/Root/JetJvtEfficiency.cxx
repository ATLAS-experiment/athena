/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetJvtEfficiency/JetJvtEfficiency.h"
#include "AsgMessaging/StatusCode.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include <AsgTools/AsgToolConfig.h>

#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include "xAODJet/JetContainer.h"


namespace CP {

static const SG::AuxElement::Decorator<char>  isPUDec("isJvtPU");
static const SG::AuxElement::ConstAccessor<float> acc_jetTiming("Timing");

JetJvtEfficiency::JetJvtEfficiency( const std::string& name): asg::AsgTool( name ),
  m_appliedSystEnum(NONE),
  m_NNJvtTool_handle("", this),
  m_jvtSelTool("", this),
  m_jvtEffTool("", this),
  m_h_JvtHist(nullptr),
  m_h_EffHist(nullptr),
  m_passJvtDecName(""),
  m_useMuBinsSF(false),
  m_useDummySFs(false),
  m_jvtCut(0),
  m_jvtCutBorder(0),
  m_jetEtaAcc(nullptr),
  m_passORAcc(nullptr),
  m_sfDec(nullptr),
  m_isHSDec(nullptr),
  m_isHSAcc(nullptr)
  {
    declareProperty( "TaggingAlg",                m_tagger = CP::JvtTagger::NNJvt                                 );
    declareProperty( "NNJvtTool",                 m_NNJvtTool_handle,                "NN Jvt tool"                );
    declareProperty( "WorkingPoint",              m_wp = "Default"                                                );
    declareProperty( "SFFile",                    m_file = ""                                                     );
    declareProperty( "JetContainerName",          m_jetContainerName = "AntiKt4EMPFlowJets"                       );
    declareProperty( "ScaleFactorDecorationName", m_sf_decoration_name = ""                                       );
    declareProperty( "OverlapDecorator",          m_ORdec = ""                                                    );
    declareProperty( "JetEtaName",                m_jetEtaName   = "DetectorEta"                                  );
    declareProperty( "MaxPtForJvt",               m_maxPtForJvt   = 60e3                                          );
    declareProperty( "DoTruthReq",                m_doTruthRequirement   = true                                   );
    declareProperty( "TruthLabel",                m_isHS_decoration_name  = "isJvtHS"                             );
    declareProperty( "TruthJetContainerName",     m_truthJetContName = "AntiKt4TruthDressedWZJets"                );
    // Allow to configure NNJvt Tool directly instead via WP property
    declareProperty( "NNJvtParamFile",            m_NNJvtParamFile= ""                                            );
    declareProperty( "NNJvtCutFile",              m_NNJvtCutFile= ""                                              );
    declareProperty( "SuppressOutputDependence",  m_suppressOutputDependence = true                               );
    // Legacy properties, kept for backwards compatibility
    declareProperty( "JetJvtMomentName",          m_jetJvtMomentName   = "Jvt"                                    );

}

StatusCode JetJvtEfficiency::initialize(){

  // Check if specified tagger is defined (Athena/Gaudi doesn't allow enums as properties so we make the conversion manually)
  m_taggingAlg = static_cast<CP::JvtTagger>(m_tagger);
  if (m_taggingAlg != CP::JvtTagger::NNJvt && m_taggingAlg != CP::JvtTagger::fJvt && m_taggingAlg != CP::JvtTagger::Jvt) {
    ATH_MSG_ERROR("Invalid Jvt tagger selected: " << m_tagger << "! Choose between CP::JvtTagger::NNJvt, CP::JvtTagger::fJvt and CP::JvtTagger::Jvt (deprecated).");
    return StatusCode::FAILURE;
  }

  // NN Jvt has been developed and tested only on EMPflow jets so far and R22 calibrations are only provided for that jet collection
  if (m_jetContainerName != "AntiKt4EMPFlowJets") {
        ATH_MSG_WARNING("Only AntiKt4EMPFlowJets are supported, use other jet collections at your own risk.");
  }

  ATH_CHECK(m_passJvtKey.initialize());
  #ifndef XAOD_STANDALONE
    if (m_suppressOutputDependence) {
      renounce (m_passJvtKey);
    }
  #endif

  asg::AsgToolConfig selToolCfg;
  ATH_CHECK(selToolCfg.setProperty("MaxPtForJvt", m_maxPtForJvt));
  if (m_wp != "Default")
    ATH_CHECK(selToolCfg.setProperty("WorkingPoint", m_wp));
  ATH_CHECK(selToolCfg.setProperty("OutputLevel", msg().level()));
  
  asg::AsgToolConfig effToolCfg;
  ATH_CHECK(effToolCfg.setProperty("MaxPtForJvt", m_maxPtForJvt));
  if (m_wp != "Default")
    ATH_CHECK(effToolCfg.setProperty("WorkingPoint", m_wp));
  // Setting DummySFs.root should be replaced by an empty string now
  if (!m_file.empty() && m_file != "DummySFs.root") {
    // This is annoying but the tool used to allow you to set a nonsensical SF file and it would
    // just ignore it
    if (m_taggingAlg == JvtTagger::NNJvt && m_file.find("NNJvtSFFile") == std::string::npos)
      ATH_MSG_WARNING("Supplied SF file " << m_file << " doesn't seem to contain SFs for NNJvt, falling back to dummy SFs ...");
    else
      ATH_CHECK(effToolCfg.setProperty("SFFile", m_file));
  }
  ATH_CHECK(effToolCfg.setProperty("DoTruthReq", m_doTruthRequirement));
  ATH_CHECK(effToolCfg.setProperty("TruthHSLabel", m_isHS_decoration_name));
  ATH_CHECK(effToolCfg.setProperty("OutputLevel", msg().level()));

  // Configure for NNJvt mode
  if (m_taggingAlg == JvtTagger::NNJvt){

    ATH_MSG_INFO("Configuring JetJvtEfficiency tool for NNJvt algorithm.");
    selToolCfg.setTypeAndName("CP::NNJvtSelectionTool/JvtSelTool");
    ATH_CHECK(selToolCfg.setProperty("JvtMomentName", "NNJvt"));
    effToolCfg.setTypeAndName("CP::NNJvtEfficiencyTool/JvtEffTool");
    

    // select cut file according to WP
    if (m_wp == "Default") { m_wp = "FixedEffPt"; }
    if (m_NNJvtCutFile.empty()) {
        if (m_wp == "FixedEffPt") {
                m_NNJvtCutFile = "NNJVT.Cuts.FixedEffPt.Offline.Nonprompt_All_MaxW.json";
        }
        else if (m_wp == "TightFwd") {
                m_NNJvtCutFile = "NNJVT.Cuts.TightFwd.Offline.Nonprompt_All_MaxWeight.json";
        }
        else {
          ATH_MSG_ERROR("Unkown NNJvt WP " << m_wp << ", choose between FixedEffPt (Default) and TightFwd.");
          return StatusCode::FAILURE;
        }
    }

    // set a default SF decoration name if not explicitly specified
    if (m_sf_decoration_name.empty()){
      m_sf_decoration_name = "NNJvtSF";
    }

    if (m_NNJvtParamFile.empty()) {
      m_NNJvtParamFile = "NNJVT.Network.graph.Offline.Nonprompt_All_MaxWeight.json";
    }

    // setup the NNJvt tool for recalculating NNJvt scores
    if (m_NNJvtTool_handle.empty()) {
      ATH_MSG_INFO( "NNJvtTool is empty! Initializing default tool ");
      asg::AsgToolConfig config_NNjvt ("JetPileupTag::JetVertexNNTagger/NNJvt");
      ATH_CHECK(config_NNjvt.setProperty("JetContainer", m_jetContainerName+"_NNJvtCopy"));
      ATH_CHECK(config_NNjvt.setProperty("NNParamFile", m_NNJvtParamFile));
      ATH_CHECK(config_NNjvt.setProperty("NNCutFile", m_NNJvtCutFile));
      ATH_CHECK(config_NNjvt.setProperty("SuppressInputDependence", true)); // otherwise decorations can't be accessed properly
      ATH_CHECK(config_NNjvt.setProperty("SuppressOutputDependence", m_suppressOutputDependence));
      ATH_CHECK(config_NNjvt.makePrivateTool(m_NNJvtTool_handle));
    }

    ATH_CHECK(m_NNJvtTool_handle.retrieve());

    // NNJvt tool will decorate decision on jets that we can retrieve
    m_passJvtDecName = "NNJvtPass";
  }
  // configure for fJvt mode
  else if (m_taggingAlg == JvtTagger::fJvt){

    ATH_MSG_INFO("Configuring JetJvtEfficiency tool for fJvt algorithm.");
    selToolCfg.setTypeAndName("CP::FJvtSelectionTool/JvtSelTool");
    ATH_CHECK(selToolCfg.setProperty("JvtMomentName", "DFCommonJets_fJvt"));
    effToolCfg.setTypeAndName("CP::FJvtEfficiencyTool/JvtEffTool");
    // set a default SF decoration name if not explicitly specified
    if (m_sf_decoration_name.empty()){
      m_sf_decoration_name = "fJvtSF";
    }

    // fJvt uses mu vs pT binning
    m_useMuBinsSF = true;
  }
  // configure for Jvt mode (deprecated)
  else {

    bool ispflow = (m_jetContainerName.find("EMPFlow") != std::string::npos);
    ATH_MSG_INFO("Configuring JetJvtEfficiency tool for Jvt algorithm.");
    ATH_MSG_WARNING("Jvt is deprecated in R22 and no calibrations will be provided, please move to NNJvt.");
    selToolCfg.setTypeAndName("CP::JvtSelectionTool/JvtSelTool");
    ATH_CHECK(selToolCfg.setProperty("IsPFlow", ispflow));
    ATH_CHECK(selToolCfg.setProperty("JvtMomentName", m_jetJvtMomentName));
    ATH_CHECK(selToolCfg.setProperty("JetContainer", m_jetContainerName));
    effToolCfg.setTypeAndName("CP::JvtEfficiencyTool/JvtEffTool");
    ATH_CHECK(effToolCfg.setProperty("JetContainer", m_jetContainerName));
    ATH_CHECK(effToolCfg.setProperty("IsPFlow", ispflow));
    // set a default SF decoration name if not explicitly specified
    if (m_sf_decoration_name.empty()){
      m_sf_decoration_name = "JvtSF";
    }
  }

  ATH_CHECK(selToolCfg.makePrivateTool(m_jvtSelTool));
  ATH_CHECK(effToolCfg.makePrivateTool(m_jvtEffTool));
  addAffectingSystematics(m_jvtEffTool->affectingSystematics());
  ATH_CHECK(addRecommendedSystematics(m_jvtEffTool->recommendedSystematics()));

  if (m_file.find("DummySF") != std::string::npos) {
    m_useDummySFs = true;
    ATH_MSG_INFO("Using dummy SFs of 1 +/ 10% for NNJvt.");
  }

  // Configure for nominal systematics
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Could not configure for nominal settings");
    return StatusCode::FAILURE;
  }

  m_jetEtaAcc = std::make_unique<SG::AuxElement::ConstAccessor<float>>(m_jetEtaName);
  m_sfDec = std::make_unique<SG::AuxElement::Decorator< float>>(m_sf_decoration_name);
  m_isHSDec = std::make_unique<SG::AuxElement::Decorator<char>>(m_isHS_decoration_name);
  m_isHSAcc = std::make_unique<SG::AuxElement::ConstAccessor<char>>(m_isHS_decoration_name);
  if (!m_ORdec.empty()) {
    m_passORAcc = std::make_unique<SG::AuxElement::ConstAccessor<char>>(m_ORdec);
  }

  return StatusCode::SUCCESS;
}

CorrectionCode JetJvtEfficiency::getEfficiencyScaleFactor( const xAOD::Jet& jet,float& sf ){
  return m_jvtEffTool->getEfficiencyScaleFactor(jet, sf);
}

CorrectionCode JetJvtEfficiency::getInefficiencyScaleFactor( const xAOD::Jet& jet,float& sf ){
  return m_jvtEffTool->getInefficiencyScaleFactor(jet, sf);
}

CorrectionCode JetJvtEfficiency::applyEfficiencyScaleFactor(const xAOD::Jet& jet) {
    float sf = 0;
    CorrectionCode result = this->getEfficiencyScaleFactor(jet,sf);
    (*m_sfDec)(jet) = sf;
    return result;
}

CorrectionCode JetJvtEfficiency::applyInefficiencyScaleFactor(const xAOD::Jet& jet) {
    float sf = 0;
    CorrectionCode result = this->getInefficiencyScaleFactor(jet,sf);
    (*m_sfDec)(jet) = sf;
    return result;
}

CorrectionCode JetJvtEfficiency::applyAllEfficiencyScaleFactor(const xAOD::IParticleContainer *jets,float& sf) {
  sf = 1;
  const xAOD::JetContainer *truthJets = nullptr;
  if( evtStore()->retrieve(truthJets, m_truthJetContName).isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve truth jet container with name " << m_truthJetContName);
    return CP::CorrectionCode::Error;

  }
  if(!truthJets || tagTruth(jets,truthJets).isFailure()) {
    ATH_MSG_ERROR("Unable to match truthJets to jets in tagTruth() method");
    return CP::CorrectionCode::Error;
  }
  for(const auto *const ipart : *jets) {
    if (ipart->type()!=xAOD::Type::Jet) {
      ATH_MSG_ERROR("Input is not a jet");
      return CP::CorrectionCode::Error;
    }
    const xAOD::Jet *jet = static_cast<const xAOD::Jet*>(ipart);
    float current_sf = 0;

    CorrectionCode result;
    if (passesJvtCut(*jet)) {
      result = this->getEfficiencyScaleFactor(*jet,current_sf);
    }
    else {
      result = this->getInefficiencyScaleFactor(*jet,current_sf);
    }

    // set the SF to 1 in case the corrections don't apply for this jet
    if (result == CP::CorrectionCode::OutOfValidityRange) {
      current_sf = 1.0;
    }
    else if (result == CP::CorrectionCode::Error) {
      ATH_MSG_ERROR("Inexplicably failed JVT calibration" );
      return result;
    }
    (*m_sfDec)(*jet) = current_sf;
    sf *= current_sf;
  }
  return CorrectionCode::Ok;
}

StatusCode JetJvtEfficiency::decorate(const xAOD::JetContainer& jets) const{
  SG::WriteDecorHandle<xAOD::JetContainer, char> passJvtHandle(m_passJvtKey);
  for(const xAOD::Jet* jet : jets)
    passJvtHandle(*jet) = passesJvtCut(*jet);
  return StatusCode::SUCCESS;
}

StatusCode JetJvtEfficiency::recalculateScores(const xAOD::JetContainer& jets) const{
  // recalculate NNJvt scores and passNNJvt decorations
  if (m_taggingAlg == JvtTagger::NNJvt) {
    ATH_CHECK(m_NNJvtTool_handle->decorate(jets));
    return StatusCode::SUCCESS;
  }
  // not required for other taggers, so return gently
  else {
    return StatusCode::SUCCESS;
  }
}

bool JetJvtEfficiency::passesJvtCut(const xAOD::Jet& jet) const {
  ATH_MSG_DEBUG("In JetJvtEfficiency::passesJvtCut ()");
  return bool(m_jvtSelTool->accept(&jet));
}

bool JetJvtEfficiency::isInRange(const xAOD::Jet& jet) const {
  // if defined, require jet to pass OR flag
  if (m_passORAcc && !(*m_passORAcc)(jet)) return false;
  if ( m_useMuBinsSF ){
    if (std::abs((*m_jetEtaAcc)(jet))<2.5) return false;
    if (std::abs((*m_jetEtaAcc)(jet))>4.5) return false;
  } else {
    // in case a SF file has been specified look up if eta of jet is covered by the file
    if (!m_useDummySFs) {
      if (std::abs((*m_jetEtaAcc)(jet))<m_h_JvtHist->GetYaxis()->GetBinLowEdge(1)) return false;
      if (std::abs((*m_jetEtaAcc)(jet))>m_h_JvtHist->GetYaxis()->GetBinUpEdge(m_h_JvtHist->GetNbinsY())) return false;
    }
    // in case dummy SFs are used (NNJvt only), manually restrict pile-up jets to central region
    else {
      if (std::abs((*m_jetEtaAcc)(jet))>2.5) return false;
    }
  }
  // skip check of histograms when using dummy SFs as no histograms have been loaded
  if (!m_useDummySFs){
    if (jet.pt()<m_h_JvtHist->GetXaxis()->GetBinLowEdge(1)) return false;
    if (jet.pt()>m_h_JvtHist->GetXaxis()->GetBinUpEdge(m_h_JvtHist->GetNbinsX())) return false;
  }
  else {
    if (jet.pt() < 20e3) return false;
  }
  if (jet.pt()>m_maxPtForJvt) return false;
  return true;
}

StatusCode JetJvtEfficiency::sysApplySystematicVariation(const CP::SystematicSet& systSet){
  return m_jvtEffTool->applySystematicVariation(systSet);
}

StatusCode JetJvtEfficiency::tagTruth(const xAOD::IParticleContainer *jets,const xAOD::IParticleContainer *truthJets) {
    for(const auto *const jet : *jets) {
      bool ishs = false;
      bool ispu = true;
      for(const auto *const tjet : *truthJets) {
        if (tjet->p4().DeltaR(jet->p4())<0.3 && tjet->pt()>10e3) ishs = true;
        if (tjet->p4().DeltaR(jet->p4())<0.6) ispu = false;
      }
      (*m_isHSDec)(*jet)=ishs;
      isPUDec(*jet)=ispu;
    }
    return StatusCode::SUCCESS;
  }

} /* namespace CP */
