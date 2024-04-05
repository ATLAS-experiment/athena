/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Christian Grefe



//
// includes
//

#include <TauAnalysisAlgorithms/TauTruthDecorationsAlg.h>
#include <AthContainers/AuxElement.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <TauAnalysisTools/HelperFunctions.h>

//
// method implementations
//

namespace CP
{

  StatusCode TauTruthDecorationsAlg ::
  initialize ()
  {
    ANA_CHECK (m_tauHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_tauHandle, SG::AllowEmpty));
    for (const auto& decorationName : m_doubleDecorations) {
      m_doubleWriteHandles.push_back(std::make_pair(std::make_unique<SG::AuxElement::ConstAccessor<double>>(decorationName), std::make_unique<CP::SysWriteDecorHandle<float>>(m_prefix+decorationName, this)));
      ANA_CHECK( m_doubleWriteHandles.back().second->initialize(m_systematicsList, m_tauHandle));
    }
    for (const auto& decorationName : m_floatDecorations) {
      m_floatWriteHandles.push_back(std::make_pair(std::make_unique<SG::AuxElement::ConstAccessor<float>>(decorationName), std::make_unique<CP::SysWriteDecorHandle<float>>(m_prefix+decorationName, this)));
      ANA_CHECK( m_floatWriteHandles.back().second->initialize(m_systematicsList, m_tauHandle));
    }
    for (const auto& decorationName : m_intDecorations) {
      m_intWriteHandles.push_back(std::make_pair(std::make_unique<SG::AuxElement::ConstAccessor<int>>(decorationName), std::make_unique<CP::SysWriteDecorHandle<int>>(m_prefix+decorationName, this)));
      ANA_CHECK( m_intWriteHandles.back().second->initialize(m_systematicsList, m_tauHandle));
    }
    for (const auto& decorationName : m_charDecorations) {
      m_charWriteHandles.push_back(std::make_pair(std::make_unique<SG::AuxElement::ConstAccessor<char>>(decorationName), std::make_unique<CP::SysWriteDecorHandle<char>>(m_prefix+decorationName, this)));
      ANA_CHECK( m_charWriteHandles.back().second->initialize(m_systematicsList, m_tauHandle));
    }
    ANA_CHECK (m_truthDecayModeDecoration.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode TauTruthDecorationsAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));
      for (const xAOD::TauJet *tau : *taus)
      {
        const xAOD::TruthParticle* truthParticle = xAOD::TauHelpers::getTruthParticle(tau);
        if (truthParticle == nullptr) continue;
        
        for (auto& [acc, writeHandle] : m_doubleWriteHandles) {
          if (truthParticle == nullptr or !acc->isAvailable(*truthParticle)) {
            writeHandle->set(*tau, 0., sys);
          } else {
            writeHandle->set(*tau, acc->operator()(*truthParticle), sys);
          }
        }

        for (auto& [acc, writeHandle] : m_floatWriteHandles) {
          if (truthParticle == nullptr or !acc->isAvailable(*truthParticle)) {
            writeHandle->set(*tau, 0., sys);
          } else {
            writeHandle->set(*tau, acc->operator()(*truthParticle), sys);
          }
        }

        for (auto& [acc, writeHandle] : m_intWriteHandles) {
          if (truthParticle == nullptr or !acc->isAvailable(*truthParticle)) {
            writeHandle->set(*tau, 0, sys);
          } else {
            writeHandle->set(*tau, acc->operator()(*truthParticle), sys);
          }
        }

        for (auto& [acc, writeHandle] : m_charWriteHandles) {
          if (truthParticle == nullptr or !acc->isAvailable(*truthParticle)) {
            writeHandle->set(*tau, 0, sys);
          } else {
            writeHandle->set(*tau, acc->operator()(*truthParticle), sys);
          }
        }

        m_truthDecayModeDecoration.set(*tau, TauAnalysisTools::getTruthDecayMode(*truthParticle), sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
