/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



#ifndef F_TAG_GHOST_ELECTRON_ASSOCIATION_ALG_H
#define F_TAG_GHOST_ELECTRON_ASSOCIATION_ALG_H

#include "xAODBase/IParticleContainer.h"
#include "xAODEgamma/ElectronContainerFwd.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>

namespace FlavorTagDiscriminants
{
  /// \brief an algorithm for adding ghost electrons to jets

  class FTagGhostElectronAssociationAlg final : public AthReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    FTagGhostElectronAssociationAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

    
  private:

    // Input containers
    SG::ReadHandleKey<xAOD::JetContainer> m_JetContainerKey {
      this, "jetContainer", "", "the jet collection to run on"
    };
    SG::ReadHandleKey<xAOD::ElectronContainer> m_ElectronContainerKey {
      this, "electronContainer", "Electrons", "the electron collection to run on"
    };

    // Output containers
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ElectronsOutKey {
      this, "outElectrons", "Something.GhostElectrons",
      "Link to be added to the Jet"
    };
  };
}

#endif // F_TAG_GHOST_ELECTRON_ASSOCIATION_ALG_H
