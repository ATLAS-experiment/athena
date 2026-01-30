/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/qgTagger.h"

namespace BJT{

  qgTagger::qgTagger(const std::string& name) :
    JSSTaggerBase(name)
  {}

  StatusCode qgTagger::initialize() {

    ATH_MSG_INFO("Initializing qgTagger tool");

    /// pT values are defined in GeV
    m_ptGeV = true;

    if (! m_configFile.empty()) {

      /// Get configReader
      ATH_CHECK(getConfigReader());

      if (m_wkpt.empty()) {
        m_InputFileName = m_configReader.GetValue("InputFileName", "");
        m_HistoName = m_configReader.GetValue("HistoName", "");
      }
      else {
        m_InputFileName = m_configReader.GetValue((m_wkpt+".InputFileName").c_str(), "");
        m_HistoName = m_configReader.GetValue((m_wkpt+".HistoName").c_str(), "");
      }

      // retrive helper tools
      ATH_CHECK(m_histTool2D.retrieve());

      /// Get min and max jet pt
      m_jetPtMin = m_configReader.GetValue("pTCutLow", 200.0);
      m_jetPtMax = m_configReader.GetValue("pTCutHigh", 2500.0);
      m_jetEtaMax = m_configReader.GetValue("EtaCutHigh", 2.0);

      /// Get the decoration name
      m_decorationName = m_configReader.GetValue("DecorationName", "");

      /// Get the scale factor configuration
      m_calcSF = m_configReader.GetValue("CalcSF", false);
      if (m_calcSF) {
        m_weightDecorationName = m_configReader.GetValue("WeightDecorationName", "");
        m_weightFileName = m_configReader.GetValue("WeightFile", "");

        /// Get truth label name information
        m_truthLabelName = m_configReader.GetValue("TruthLabelName", "PartonTruthLabelID");

        // ToDo: add here the loading of the SF file

      }

    }
    else { /// No config file
      ATH_MSG_ERROR("No config file provided AND no parameters specified.") ;
    }

    ///
    ATH_MSG_INFO("qg Tagger tool initialized");
    ATH_MSG_INFO("  DecorationName    : " << m_decorationName);
    if (m_calcSF) {
      ATH_MSG_INFO("weightDecorationName    : " << m_weightDecorationName);
      ATH_MSG_INFO("weightFile              : " << m_weightFileName);
    }
    ATH_MSG_INFO("  pT cut low        : " << m_jetPtMin);
    ATH_MSG_INFO("  pT cut high       : " << m_jetPtMax);
    ATH_MSG_INFO("  eta cut high      : " << m_jetEtaMax);

    /// Set the possible states that the tagger can be left in after the JSSTaggerBase::tag() function is called
    m_acceptInfo.addCut("PassScore", "ScoreJet > ScoreCut");

    /// Loop over and print out the cuts that have been configured
    printCuts();

    /// Call base class initialize
    ATH_CHECK(JSSTaggerBase::initialize());

    /// Initialize additional decorators
    ATH_MSG_INFO("Additional decorators that will be attached to jet :");

    m_decValidKinRangeKey = m_containerName + "." + m_decorationName + "_" + m_decValidKinRangeKey.key();
    m_decPassScoreKey = m_containerName + "." + m_decorationName + "_" + m_decPassScoreKey.key();
    m_decCutScoreKey = m_containerName + "." + m_decorationName + "_" + m_decCutScoreKey.key();

    ATH_CHECK(m_decValidKinRangeKey.initialize());
    ATH_CHECK(m_decPassScoreKey.initialize());
    ATH_CHECK(m_decCutScoreKey.initialize());

    ATH_MSG_INFO("  " << m_decValidKinRangeKey.key() << " : pass kinematic range");
    ATH_MSG_INFO("  " << m_decPassScoreKey.key() << " : pass Score cut");
    ATH_MSG_INFO("  " << m_decCutScoreKey.key() << " : Score cut");

    m_decAcceptKey = m_containerName + "." + m_decorationName + "_" + m_decAcceptKey.key();
    ATH_CHECK(m_decAcceptKey.initialize());

  #ifndef XAOD_STANDALONE
    if (m_suppressOutputDependence) {
      renounce(m_decTaggedKey);
      renounce(m_decValidJetContentKey);
      renounce(m_decValidEventContentKey);
    }
  #endif
    
    return StatusCode::SUCCESS;

  }

  StatusCode qgTagger::tag(const xAOD::Jet& jet) const {

    ATH_MSG_DEBUG("Obtaining tag result   " << jet.pt() << "   " << jet.m());

    return StatusCode::SUCCESS;

  }

  StatusCode qgTagger::decorate(const xAOD::JetContainer& jets) const {

    ATH_MSG_DEBUG("Obtaining qg result");

    /// Create WriteDecorHandles
    SG::WriteDecorHandle<xAOD::JetContainer, char> decValidKinRange(m_decValidKinRangeKey);
    SG::WriteDecorHandle<xAOD::JetContainer, char> decPassScore(m_decPassScoreKey);
    SG::WriteDecorHandle<xAOD::JetContainer, char> decTagged(m_decTaggedKey);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decCutScore(m_decCutScoreKey);

    /// Create asg::AcceptData object
    asg::AcceptData acceptData(&m_acceptInfo);

    /// Reset the AcceptData cut results
    ATH_CHECK(resetCuts(acceptData));

    // loop over jets
    for(const xAOD::Jet* jet : jets){

      /// Check basic kinematic selection
      bool pass_kin_range = passKinRange(*jet);
      decValidKinRange(*jet) = pass_kin_range;

      /// Get Score value
      static const SG::AuxElement::ConstAccessor<float> Score(m_decorationName + "_ConstScore");
      // ToDo: add a protection here
      float jet_score = Score(*jet);

      ATH_MSG_DEBUG("Score: " << jet_score);

      /// Evaluate the values of the lower score cut
      JetHelper::JetContext jc;
      ATH_MSG_DEBUG("jet pT: " << jet -> pt());
      ATH_MSG_DEBUG("jet eta: " << jet -> eta());

      float cut_score = m_histTool2D -> getValue(*jet, jc);

      ATH_MSG_DEBUG("HistoInput2D value: " << cut_score);

      /// Evaluate the cut criteria on the tagger score
      bool pass_score = jet_score > cut_score ? true : false;
      if(pass_score) acceptData.setCutResult("PassScore", true);

      ATH_MSG_DEBUG("tagger decision: " << pass_score);

      decPassScore(*jet) = acceptData.getCutResult("PassScore");

      bool passCuts = acceptData.getCutResult("PassScore");

      /// Decorate jet with tagging summary
      decTagged(*jet) = passCuts;

      /// Get SF weight
      // ToDo: add the decoration of the SF here
    
    } // end loop over jets

    return StatusCode::SUCCESS;

  }

}