/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak



//
// includes
//

#include <MuonAnalysisAlgorithms/MuonTriggerEfficiencyScaleFactorAlg.h>

#include <AthContainers/ConstDataVector.h>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace CP
{

  MuonTriggerEfficiencyScaleFactorAlg::MuonTriggerEfficiencyScaleFactorAlg(const std::string& name,
                                         ISvcLocator* svcLoc)
    : AnaAlgorithm(name, svcLoc)
  {
    declareProperty("efficiencyScaleFactorTool", m_efficiencyScaleFactorTool, "trigger efficiency scale factor tool");
  }

  StatusCode MuonTriggerEfficiencyScaleFactorAlg ::
  initialize ()
  {
    if (m_trigger.empty())
    {
      ANA_MSG_ERROR ("trigger name needs to be set");
      return StatusCode::FAILURE;
    }

    if (m_efficiencyScaleFactorTool.empty())
    {
      ANA_MSG_ERROR ("efficiency scale factor tool needs to be set");
      return StatusCode::FAILURE;
    }

    if (m_scaleFactorDecoration.empty() && m_mcEfficiencyDecoration.empty() && m_dataEfficiencyDecoration.empty())
    {
      ANA_MSG_ERROR ("no scale factor or efficiency decoration name set");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_efficiencyScaleFactorTool.retrieve());
    ANA_CHECK (m_eventInfoHandle.initialize (m_systematicsList));
    ANA_CHECK (m_muonHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_muonHandle, SG::AllowEmpty));
    ANA_CHECK (m_scaleFactorDecoration.initialize (m_systematicsList, m_muonHandle, SG::AllowEmpty));
    ANA_CHECK (m_mcEfficiencyDecoration.initialize (m_systematicsList, m_muonHandle, SG::AllowEmpty));
    ANA_CHECK (m_dataEfficiencyDecoration.initialize (m_systematicsList, m_muonHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.addSystematics (*m_efficiencyScaleFactorTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode MuonTriggerEfficiencyScaleFactorAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK (m_efficiencyScaleFactorTool->applySystematicVariation (sys));
      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventInfoHandle.retrieve (eventInfo, sys));

      static const SG::ConstAccessor<unsigned int> acc_rnd("RandomRunNumber");
      unsigned int randomRunNumber = acc_rnd(*eventInfo);
      bool validEvent = m_minRunNumber <= randomRunNumber && m_maxRunNumber >= randomRunNumber;

      for (const xAOD::Muon *muon : *muons)
      {
        if (validEvent && m_preselection.getBool (*muon, sys))
        {
          if (m_scaleFactorDecoration) {
            double sf = 0;
            ConstDataVector<xAOD::MuonContainer> singleMuonContainer(SG::VIEW_ELEMENTS);
            singleMuonContainer.push_back(muon);
            ANA_CHECK_CORRECTION (m_outOfValidity, *muon, m_efficiencyScaleFactorTool->getTriggerScaleFactor (*singleMuonContainer.asDataVector(), sf, m_trigger));
            m_scaleFactorDecoration.set (*muon, sf, sys);
          }

          if (m_mcEfficiencyDecoration) {
            double eff = 0;
            ANA_CHECK_CORRECTION (m_outOfValidity, *muon, m_efficiencyScaleFactorTool->getTriggerEfficiency (*muon, eff, m_trigger, false));
            m_mcEfficiencyDecoration.set (*muon, eff, sys);
          }

          if (m_dataEfficiencyDecoration) {
            double eff = 0;
            ANA_CHECK_CORRECTION (m_outOfValidity, *muon, m_efficiencyScaleFactorTool->getTriggerEfficiency (*muon, eff, m_trigger, true));
            m_dataEfficiencyDecoration.set (*muon, eff, sys);
          }
        } else {
          if (m_scaleFactorDecoration) {
            m_scaleFactorDecoration.set (*muon, invalidScaleFactor(), sys);
          }

          if (m_mcEfficiencyDecoration) {
            m_mcEfficiencyDecoration.set (*muon, invalidEfficiency(), sys);
          }

          if (m_dataEfficiencyDecoration) {
            m_dataEfficiencyDecoration.set (*muon, invalidEfficiency(), sys);
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
