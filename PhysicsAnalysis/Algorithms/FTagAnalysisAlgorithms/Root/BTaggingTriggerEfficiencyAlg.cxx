/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler



//
// includes
//

#include <FTagAnalysisAlgorithms/BTaggingTriggerEfficiencyAlg.h>

#include <algorithm>

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
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK (m_offlineEfficiencyTool->applySystematicVariation (sys));
      ANA_CHECK (m_triggerEfficiencyTool->applySystematicVariation (sys));
      ANA_CHECK (m_conditionalEfficiencyTool->applySystematicVariation (sys));
      
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys, ctx));

      for (const xAOD::Jet *jet : *jets)
      {
        if (m_preselection.getBool (*jet, sys))
        {
          float sf = 1.;
          ANA_CHECK_CORRECTION (m_outOfValidity, *jet, triggerScaleFactor (*jet, sys, sf));
          m_scaleFactorDecoration.set (*jet, m_outOfValidity.get (*jet) ? sf : invalidScaleFactor(), sys);
        } else {
          m_scaleFactorDecoration.set (*jet, invalidScaleFactor(), sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }



  CP::CorrectionCode BTaggingTriggerEfficiencyAlg ::
  triggerScaleFactor (const xAOD::Jet& jet, const CP::SystematicSet& sys, float& sf)
  {
    // the trigger did not look at this jet: offline scale factor only
    if (m_truthFlav.get (jet, sys) != 5 || !static_cast<bool> (m_matchingDecoration.get (jet, sys)))
      return m_offlineEfficiencyTool->getScaleFactor (jet, sf);

    // inputs shared by both trigger-matched cases
    float trigSF = 0, condSF = 0;
    CP::CorrectionCode code = std::min (m_triggerEfficiencyTool->getScaleFactor (jet, trigSF),
                                        m_conditionalEfficiencyTool->getScaleFactor (jet, condSF));
    if (code == CP::CorrectionCode::Error) return code;
    // the trigger calibration does not cover this jet: fall back to the offline scale factor
    if (code == CP::CorrectionCode::OutOfValidityRange)
      return m_offlineEfficiencyTool->getScaleFactor (jet, sf);

    // online b-tag passed: p(trig) * p(off | trig)
    if (static_cast<bool> (m_bTagMatchingDecoration.get (jet, sys)))
    {
      sf = condSF * trigSF;
      return CP::CorrectionCode::Ok;
    }

    // online b-tag failed, offline passed: p(off) - p(trig) * p(off | trig)
    float trigEff_data = 0, condEff_data = 0, offlEff_data = 0, offlSF = 0;
    code = std::min ({m_triggerEfficiencyTool->getEfficiency (jet, trigEff_data),
                      m_conditionalEfficiencyTool->getEfficiency (jet, condEff_data),
                      m_offlineEfficiencyTool->getEfficiency (jet, offlEff_data),
                      m_offlineEfficiencyTool->getScaleFactor (jet, offlSF)});
    if (code == CP::CorrectionCode::Error) return code;
    if (code == CP::CorrectionCode::OutOfValidityRange)
      return m_offlineEfficiencyTool->getScaleFactor (jet, sf);

    const float trigEff_MC = trigEff_data / trigSF;
    const float condEff_MC = condEff_data / condSF;
    const float offlEff_MC = offlEff_data / offlSF;
    const float num   = offlEff_data - condEff_data * trigEff_data;
    const float denom = offlEff_MC   - condEff_MC   * trigEff_MC;
    if (!(num > 0 && denom > 0))
    {
      ANA_MSG_WARNING ("SF computed with negative efficiency num=" << num << " denom=" << denom);
      // no scale factor can be given for this jet; the caller decorates it as invalid
      return CP::CorrectionCode::OutOfValidityRange;
    }
    sf = num / denom;
    return CP::CorrectionCode::Ok;
  }

}
