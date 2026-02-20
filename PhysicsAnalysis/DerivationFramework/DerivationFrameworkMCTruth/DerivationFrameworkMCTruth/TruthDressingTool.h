/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHDRESSINGTOOL_H
#define DERIVATIONFRAMEWORK_TRUTHDRESSINGTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "DerivationFrameworkMCTruth/DecayGraphHelper.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

namespace DerivationFramework {

  class TruthDressingTool : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /// ReadHandleKey input collection key
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
    {this, "particlesKey", "TruthParticles", "ReadHandleKey for TruthParticles for photon list input"};
    /// WriteDecorHandleKeys for decorations for particlesKey
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorationKey
      {this, "decorationName", m_particlesKey, "unusedPhotonDecoration", "Name of the decoration for photons that were used in dressing"};
    /// ReadHandleKey for particles to be dressed
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_dressParticlesKey
      {this, "dressParticlesKey", "", "ReadHandleKey for input particles to be dressed.  If taus are selected, everything in this input key will be used"};
    //  WriteDecorHandleKeys for decorations for container of particles to be dressed
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_eKey
      {this, "e_dressed", m_dressParticlesKey, "e_dressed", "e_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_ptKey
      {this, "pt_dressed", m_dressParticlesKey, "pt_dressed", "pt_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_etaKey
      {this, "eta_dressed", m_dressParticlesKey, "eta_dressed", "eta_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_phiKey
      {this, "phi_dressed", m_dressParticlesKey, "phi_dressed", "phi_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_pt_visKey
      {this, "pt_vis_dressed", m_dressParticlesKey, "pt_vis_dressed", "pt_vis_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_eta_visKey
      {this, "eta_vis_dressed", m_dressParticlesKey, "eta_vis_dressed", "eta_vis_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_phi_visKey
      {this, "phi_vis_dressed", m_dressParticlesKey, "phi_vis_dressed", "phi_vis_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_m_visKey
      {this, "m_vis_dressed", m_dressParticlesKey, "m_vis_dressed", "m_vis_dressed decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_nphotonKey
      {this, "nPhotons_dressed", m_dressParticlesKey, "nPhotons_dressed", "nPhotons_dressed decoration"};
    /// To ensure that the algorithm is scheduled after the truth classifier
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthClassKey{this, "truthClassifierKey", m_dressParticlesKey, "Classification"};

    /// Parameter: Use photons from hadron decays?
    Gaudi::Property<bool> m_usePhotonsFromHadrons
      {this, "usePhotonsFromHadrons", false,  "Add photons coming from hadron decays while dressing"};
    /// Parameter: Use leptons from hadron decays?
    Gaudi::Property<bool> m_useLeptonsFromHadrons
      {this, "useLeptonsFromHadrons", false, "Consider leptons coming from hadron decays?"};
    /// Parameter: Cone size for dressing
    Gaudi::Property<float> m_coneSize
      {this, "dressingConeSize", 0.1, "Size of dR cone in which to include FSR photons in dressing"};
    /// Parameter: List of pdgIDs of particles to dress
    Gaudi::Property< std::vector<int> > m_listOfPIDs
      {this, "particleIDsToDress", {11,13}, "List of the pdgID's of particles to be dressed (usually 11,13).  Special treatment for taus (15)"};
    /// Parameter: Use antikT algorithm for dressing?
    Gaudi::Property<bool> m_useAntiKt
      {this, "useAntiKt", false, "use anti-k_T instead of fixed-cone dressing"};
    /// Parameter: Do we want to decorate the photons used for dressing?
    Gaudi::Property<bool> m_decoratePhotons
      {this, "decoratePhotons", true, "Do we want to decorate the photons used for dressing?"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHDRESSINGTool_H
