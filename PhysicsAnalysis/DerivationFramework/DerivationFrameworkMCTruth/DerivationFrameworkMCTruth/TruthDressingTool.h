/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
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
      TruthDressingTool(const std::string& t, const std::string& n, const IInterface* p);
      ~TruthDressingTool();
      StatusCode initialize();
      virtual StatusCode addBranches() const;

    private:
      /// ReadHandleKey input collection key
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
         {this, "particlesKey", "TruthParticles", "ReadHandleKey for TruthParticles for photon list input"};
      /// ReadHandleKey for particles to be dressed
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_dressParticlesKey
         {this, "dressParticlesKey", "TruthParticles", "ReadHandleKey for input particles to be dressed.  If taus are selected, everything in this input key will be used"};
      /// Ensure that the algorithm is scheduled after the truth classifier
      SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthClassKey{this, "truthClassifierKey", "", "Will be over written during initialize"};
      /// WriteDecorHandleKeys for decorations
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_eKey
         {this, "e_dressed", "TruthParticles.e_dressed", "e_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_ptKey
         {this, "pt_dressed", "TruthParticles.pt_dressed", "pt_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_etaKey
         {this, "eta_dressed", "TruthParticles.eta_dressed", "eta_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_phiKey 
         {this, "phi_dressed", "TruthParticles.phi_dressed", "phi_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_pt_visKey
         {this, "pt_vis_dressed", "TruthParticles.pt_vis_dressed", "pt_vis_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_eta_visKey
         {this, "eta_vis_dressed", "TruthParticles.eta_vis_dressed", "eta_vis_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_phi_visKey
         {this, "phi_vis_dressed", "TruthParticles.phi_vis_dressed", "phi_vis_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_m_visKey
         {this, "m_vis_dressed", "TruthParticles.m_vis_dressed", "m_vis_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorator_nphotonKey
         {this, "nPhotons_dressed", "TruthParticles.nPhotons_dressed", "nPhotons_dressed decoration"};
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decorationKey
         {this, "DressingKey", "TruthParticles.dressedPhoton", "Dressed photon decoration"};

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
      /// Parameter: Name of the decoration to apply
      Gaudi::Property<std::string> m_decorationName
         {this, "decorationName", "", "Name of the decoration for photons that were used in dressing"};
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTHDRESSINGTool_H
