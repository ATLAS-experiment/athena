/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/ScaleFactors.h"

namespace BJT{

  ScaleFactors::ScaleFactors(const std::string& name) :
    asg::AsgTool(name)
  {}

  StatusCode ScaleFactors::initialize() {

    ATH_MSG_INFO("Initializing ScaleFactors tool");

    // retrive helper tools
    ATH_CHECK(m_histTool2D_quark_eff.retrieve());
    ATH_CHECK(m_histTool2D_gluon_eff.retrieve());
    ATH_CHECK(m_histTool2D_quark_ineff.retrieve());
    ATH_CHECK(m_histTool2D_gluon_ineff.retrieve());

    // print decoration name and validity range info
    ATH_MSG_INFO("Scale Factors tool initialized");
    ATH_MSG_INFO("  jet container    : " << m_jetsKey.key());
    ATH_MSG_INFO("  pT cut low        : " << m_jetPtMin);
    ATH_MSG_INFO("  pT cut high       : " << m_jetPtMax);
    ATH_MSG_INFO("  eta cut high      : " << m_jetEtaMax);

    /// Initialize additional decorators
    ATH_MSG_INFO("Additional decorators that will be attached to jet :");

    ATH_CHECK(m_jetsKey.initialize());

    ATH_CHECK(m_accTaggedKey.initialize());
    ATH_CHECK(m_decEfficiencyKey.initialize());
    ATH_CHECK(m_decInefficiencyKey.initialize());

    ATH_MSG_INFO("  " << m_accTaggedKey.key() << " : pass tagging criteria");
    ATH_MSG_INFO("  " << m_decEfficiencyKey.key() << " : Efficiency correction");
    ATH_MSG_INFO("  " << m_decInefficiencyKey.key() << " : Inefficiency correction");
    
    return StatusCode::SUCCESS;

  }

  StatusCode ScaleFactors::decorate(const xAOD::JetContainer& jets) const {

    ATH_MSG_DEBUG("Obtaining tagger scale factors result");

    /// Create WriteDecorHandles
    SG::WriteDecorHandle<xAOD::JetContainer, float> decEfficiency(m_decEfficiencyKey);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decInefficiency(m_decInefficiencyKey);

    /// Create ReadDecorHandles
    SG::ReadDecorHandle<xAOD::JetContainer, char> accTagged(m_accTaggedKey);

    // loop over jets
    for(const xAOD::Jet* jet : jets){

      /// Get truth label
      static const SG::ConstAccessor<int> truth_label_acc(m_truthLabelName);
      int truth_label = truth_label_acc(*jet);

      /// get tagger decision
      bool pass_taggerWP = accTagged(*jet);

      /// Retrive the value of the scale factor
      JetHelper::JetContext jc;
      ATH_MSG_DEBUG("jet pT: " << jet -> pt());
      ATH_MSG_DEBUG("jet eta: " << jet -> eta());
      ATH_MSG_DEBUG("jet truth label: " << truth_label);
      ATH_MSG_DEBUG("jet pass tagger WP: " << pass_taggerWP);

      float efficiency (-1.), inefficiency (-1.);

      if(pass_taggerWP){
        if(truth_label >= 1 && truth_label <= 5)
          efficiency = m_histTool2D_quark_eff -> getValue(*jet, jc);
        else if(truth_label == 21)
          inefficiency = m_histTool2D_gluon_ineff -> getValue(*jet, jc);
      }
      else{
        if(truth_label >= 1 && truth_label <= 5)
          inefficiency = m_histTool2D_quark_ineff -> getValue(*jet, jc);
        else if(truth_label == 21)
          efficiency = m_histTool2D_gluon_eff -> getValue(*jet, jc);
      }

      ATH_MSG_DEBUG("efficiency: " << efficiency);
      ATH_MSG_DEBUG("inefficiency: " << inefficiency);

      /// Decorate jet with tagging summary
      decEfficiency(*jet) = efficiency;
      decInefficiency(*jet) = inefficiency;
    
    } // end loop over jets

    return StatusCode::SUCCESS;

  }

}