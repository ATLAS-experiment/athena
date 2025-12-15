/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_MAXCELLDECORATOR_H
#define DERIVATIONFRAMEWORK_MAXCELLDECORATOR_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "GaudiKernel/EventContext.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/Jet.h"
#include "xAODPFlow/PFO.h"
#include "xAODPFlow/FlowElement.h"

namespace DerivationFramework {

  class MaxCellDecorator : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

    struct calculation
    {
      float maxEcell_time = -9999.9;
      float maxEcell_energy = -9999.9;
      int maxEcell_gain = -1;
      uint64_t maxEcell_onlId = 0;
      float maxEcell_x = -9999.9;
      float maxEcell_y = -9999.9;
      float maxEcell_z = -9999.9;
    };

  private:
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{
      this,
        "CablingKey",
        "LArOnOffIdMap",
        "SG Key of LArOnOffIdMapping object"
        };

    SG::ReadHandleKey<xAOD::EgammaContainer>
    m_SGKey_photons{ this, "SGKey_photons", "", "SG key of photon container" };

    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons{
      this,
      "SGKey_electrons",
      "",
      "SG key of electron container"
    };

    /** This should be only for using run 2 reprocessing, which misses the
        cell link from LRT electron clusters :
        try to get info from the best matched "regular" egamma cluster */
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_SGKey_egammaClusters{
      this,
      "SGKey_egammaClusters",
      "",
      "SG key of cluster container associated to standard egammas"
    };

    SG::ReadHandleKey<xAOD::TauJetContainer>
    m_SGKey_taus{ this, "SGKey_taus", "", "SG key of tau container" };

    SG::ReadHandleKey<xAOD::JetContainer>
    m_SGKey_jets{ this, "SGKey_jets", "", "SG key of jet container" };

    /** @brief matching cone size*/
    Gaudi::Property<double> m_dRLRTegClusegClusMax{
      this, "dRLRTegClusegClusMax",
      0.05,
      "Maximum delta R to match LRT egammaCluster to std egammaCluster"
    };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_photons_decorations{
      this,
      "SGKey_photons_decorations",
      m_SGKey_photons, {"maxEcell_time", "maxEcell_energy", "maxEcell_gain",
        "maxEcell_onlId", "maxEcell_x", "maxEcell_y", "maxEcell_z"},
      "SG keys for photon decorations not really configurable"
    };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_electrons_decorations{
      this,
      "SGKey_electrons_decorations",
      m_SGKey_electrons, {"maxEcell_time", "maxEcell_energy", "maxEcell_gain",
        "maxEcell_onlId", "maxEcell_x", "maxEcell_y", "maxEcell_z"},
      "SG keys for electrons decorations not really configurable"
    };

    SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer>
    m_SGKey_taus_decorations{
      this,
      "SGKey_taus_decorations",
      m_SGKey_taus, {"maxEcell_time", "maxEcell_energy", "maxEcell_gain",
        "maxEcell_onlId", "maxEcell_x", "maxEcell_y", "maxEcell_z"},
      "SG keys for tau decorations not really configurable"
    };

    SG::WriteDecorHandleKeyArray<xAOD::JetContainer>
    m_SGKey_jets_decorations{
      this,
      "SGKey_jets_decorations",
      m_SGKey_jets, {"maxEcell_time", "maxEcell_energy", "maxEcell_gain",
        "maxEcell_onlId", "maxEcell_x", "maxEcell_y", "maxEcell_z"},
      "SG keys for jet decorations not really configurable"
    };

    calculation decorateObject(const xAOD::CaloCluster* cluster,
                               const EventContext& ctx) const;
  };
}

#endif // DERIVATIONFRAMEWORK_MAXCELLDECORATOR_H
