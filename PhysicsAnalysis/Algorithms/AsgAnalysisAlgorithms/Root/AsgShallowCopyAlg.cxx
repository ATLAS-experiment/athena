/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#include <AsgAnalysisAlgorithms/AsgShallowCopyAlg.h>

#include <SystematicsHandles/CopyHelpers.h>
#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODCore/AuxContainerBase.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/DiTauJetContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTruth/TruthParticleContainer.h>


namespace CP
{
  template<typename Type> StatusCode AsgShallowCopyAlg ::
  executeTemplate (const EventContext& ctx, const CP::SystematicSet& sys)
  {
    const Type *input = nullptr;
    ANA_CHECK (evtStore()->retrieve (input, m_inputHandle.getName (sys)));

    const auto& name = m_outputHandle.getName(sys);
    [[maybe_unused]] Type *output = nullptr;
    ANA_CHECK (detail::ShallowCopy<Type>::getCopy
               (msg(), ctx, output, input, name));

    return StatusCode::SUCCESS;
  }



  StatusCode AsgShallowCopyAlg ::
  executeFindType (const EventContext& ctx, const CP::SystematicSet& sys)
  {
    const xAOD::IParticleContainer *input = nullptr;
    if (evtStore()->contains<xAOD::IParticleContainer>(m_inputHandle.getName(sys)))
      {
        ANA_CHECK (m_inputHandle.retrieve (input, sys, ctx));
      }

    if (dynamic_cast<const xAOD::ElectronContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::ElectronContainer>;
    }
    else if (dynamic_cast<const xAOD::PhotonContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::PhotonContainer>;
    }
    else if (dynamic_cast<const xAOD::JetContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::JetContainer>;
    }
    else if (dynamic_cast<const xAOD::MuonContainer*> (input)) {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::MuonContainer>;
    }
    else if (dynamic_cast<const xAOD::TauJetContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::TauJetContainer>;
    }
    else if (dynamic_cast<const xAOD::DiTauJetContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::DiTauJetContainer>;
    }
    else if (dynamic_cast<const xAOD::TrackParticleContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::TrackParticleContainer>;
    }
    else if (dynamic_cast<const xAOD::TruthParticleContainer*> (input))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::TruthParticleContainer>;
    }
    else if (evtStore()->contains<xAOD::MissingETContainer>(m_inputHandle.getName(sys)))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::MissingETContainer>;
    }
    else if (evtStore()->contains<xAOD::CaloClusterContainer>(m_inputHandle.getName(sys)))
    {
      m_function =
        &AsgShallowCopyAlg::executeTemplate<xAOD::CaloClusterContainer>;
    }
    else
    {
      ANA_MSG_ERROR ("unknown type contained in AsgShallowCopyAlg, please extend it");
      return StatusCode::FAILURE;
    }

    return (this->*m_function) (ctx, sys);
  }



  StatusCode AsgShallowCopyAlg ::
  initialize ()
  {
    for (const auto& deco : m_declareDecorations)
      ANA_CHECK (m_systematicsList.service().setDecorSystematics (m_inputHandle.getNamePattern(), deco, {}));
    ANA_CHECK (m_systematicsList.service().registerCopy (m_inputHandle.getNamePattern(), m_outputHandle.getNamePattern()));
    ANA_CHECK (m_inputHandle.initialize (m_systematicsList));
    ANA_CHECK (m_outputHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());

#ifndef XAOD_STANDALONE
    const CLID clidRead = detail::getClidForDependency<xAOD::IParticleContainer>("", "deco", false);
    const CLID clidWrite = detail::getClidForDependency<xAOD::IParticleContainer>("", "deco", true);
    std::function<void(const DataObjID&, Gaudi::DataHandle::Mode)> addAlgDependency = [this] (const DataObjID& id, Gaudi::DataHandle::Mode mode) {
      this->addDependency(id, mode);
    };
    for (const auto& decoName : m_systematicsList.service().getObjectDecorations(m_inputHandle.getNamePattern()))
    {
      ANA_CHECK (detail::addSysDependency(msg(), m_systematicsList.service(), addAlgDependency, clidRead, m_inputHandle.getNamePattern(), Gaudi::DataHandle::Reader, decoName, false));
      ANA_CHECK (detail::addSysDependency(msg(), m_systematicsList.service(), addAlgDependency, clidWrite, m_outputHandle.getNamePattern(), Gaudi::DataHandle::Writer, decoName, true));
    }
#endif

    return StatusCode::SUCCESS;
  }



  StatusCode AsgShallowCopyAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK ((this->*m_function) (ctx, sys));
    }
    return StatusCode::SUCCESS;
  }
}
