/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler



//
// includes
//

#include <FTagAnalysisAlgorithms/BTaggingTriggerEfficiencyAlg.h>

//
// method implementations
//

namespace CP
{
  BTaggingTriggerEfficiencyAlg::BTaggingTriggerEfficiencyAlg
  (const std::string &name, ISvcLocator *svcLoc) :
    EL::AnaAlgorithm(name, svcLoc) {}

  StatusCode BTaggingTriggerEfficiencyAlg ::
  initialize ()
  {
    if (m_scaleFactorDecoration.empty())
    {
      ANA_MSG_ERROR ("no scale factor decoration name set");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_offlineEfficiencyTool.retrieve());
    ANA_CHECK (m_triggerEfficiencyTool.retrieve());
    ANA_CHECK (m_conditionalEfficiencyTool.retrieve());
    
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_jetHandle));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));

    ANA_CHECK (m_scaleFactorDecoration.initialize (m_systematicsList, m_jetHandle));
    ANA_CHECK (m_matchingDecoration.initialize (m_systematicsList, m_jetHandle));
    ANA_CHECK (m_bTagMatchingDecoration.initialize (m_systematicsList, m_jetHandle));

    ANA_CHECK (m_systematicsList.addSystematics (*m_offlineEfficiencyTool));
    ANA_CHECK (m_systematicsList.addSystematics (*m_triggerEfficiencyTool));
    ANA_CHECK (m_systematicsList.addSystematics (*m_conditionalEfficiencyTool));

    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode BTaggingTriggerEfficiencyAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK (m_offlineEfficiencyTool->applySystematicVariation (sys));
      ANA_CHECK (m_triggerEfficiencyTool->applySystematicVariation (sys));
      ANA_CHECK (m_conditionalEfficiencyTool->applySystematicVariation (sys));
      
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      for (const xAOD::Jet *jet : *jets)
      {
        if (m_preselection.getBool (*jet, sys))
        {
          float sf = 1.;
          CP::CorrectionCode valid;

          // For non-bjet, just get offline scale factor
          if(m_truthFlav.get(*jet, sys)!=5){
            valid = m_offlineEfficiencyTool->getScaleFactor(*jet, sf);
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
          }

          else {
            float trigEff_MC = 0;
            float trigEff_data = 0;
            valid = m_triggerEfficiencyTool->getMCEfficiency(*jet, trigEff_MC);
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
            valid = m_triggerEfficiencyTool->getScaleFactor(*jet, trigEff_data);
            trigEff_data *= trigEff_MC;
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);

            float condEff_MC = 0;
            float condEff_data = 0;
            valid = m_conditionalEfficiencyTool->getMCEfficiency(*jet, condEff_MC);
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
            valid = m_conditionalEfficiencyTool->getScaleFactor(*jet, condEff_data);
            condEff_data *= condEff_MC;
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);

            if(static_cast<bool>(m_matchingDecoration.get(*jet, sys))){
              if(static_cast<bool>(m_bTagMatchingDecoration.get(*jet,sys))){
                sf = condEff_data * trigEff_data;
                sf /= condEff_MC * trigEff_MC;
              }
              else{
                float offlEff_MC = 0;
                float offlEff_data = 0;
                valid = m_offlineEfficiencyTool->getMCEfficiency(*jet, offlEff_MC);
                ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
                valid = m_offlineEfficiencyTool->getScaleFactor(*jet, offlEff_data);
                offlEff_data *= offlEff_MC;
                ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);

                float num = offlEff_data - condEff_data * trigEff_data;
                float denom = offlEff_MC - condEff_MC * trigEff_MC;
                if(num>0 && denom>0) sf = num / denom;
                else sf = invalidScaleFactor();
              }
            } else {
              valid = m_offlineEfficiencyTool->getScaleFactor(*jet, sf);
              ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
            }
          }

          if (m_outOfValidity.get(*jet))
            m_scaleFactorDecoration.set (*jet, sf, sys);
          else
            m_scaleFactorDecoration.set (*jet, invalidScaleFactor(), sys);
        } else {
          m_scaleFactorDecoration.set (*jet, invalidScaleFactor(), sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }

}
