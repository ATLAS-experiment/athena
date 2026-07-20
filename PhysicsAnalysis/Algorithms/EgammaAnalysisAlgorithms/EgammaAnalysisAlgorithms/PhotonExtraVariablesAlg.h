/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Abicht

#ifndef PHOTON_EXTRAVARIABLES_ALG__H
#define PHOTON_EXTRAVARIABLES_ALG__H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEgamma/PhotonContainer.h>

namespace CP {

  class PhotonExtraVariablesAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;

  private:
    CP::SysListHandle m_systematicsList {this};
    CP::SysReadHandle<xAOD::PhotonContainer> m_photonsHandle { this, "photons", "", "the input photon container" };
    CP::SysWriteDecorHandle<int> m_conversionTypeHandle { this, "conversionType", "conversionType_%SYS%", "decoration name for photon conversionType" };
    CP::SysWriteDecorHandle<float> m_caloEta2Handle { this, "caloEta2", "caloEta2_%SYS%", "decoration name for photon caloEtaBE(2)" };
  };

} // namespace

#endif
