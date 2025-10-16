/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHDECAYCOLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTHDECAYCOLLECTIONMAKER_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
// EDM -- typedefs so these are includes
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
// Handles and keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
// Standard library includes
#include <vector>
#include <string>

namespace DerivationFramework {


  class TruthDecayCollectionMaker : public extends<AthAlgTool, IAugmentationTool> {
  public:
    TruthDecayCollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
    ~TruthDecayCollectionMaker();
    StatusCode initialize();
    virtual StatusCode addBranches(const EventContext& ctx) const;

  private:
    Gaudi::Property<std::vector<int> > m_pdgIdsToKeep //!< List of PDG IDs to build this collection from
    {this, "PDGIDsToKeep", {}, "PDG IDs of particles to build the collection from"};
    Gaudi::Property<bool> m_keepBHadrons //!< Option to keep all b-hadrons (better than giving PDG IDs)
      {this, "KeepBHadrons", false, "Keep b-hadrons (easier than by PDG ID)"};
    Gaudi::Property<bool> m_keepCHadrons //!< Option to keep all c-hadrons (better than giving PDG IDs)
      {this, "KeepCHadrons", false, "Keep c-hadrons (easier than by PDG ID)"};
    Gaudi::Property<bool> m_keepBSM //!< Option to keep all BSM particles (better than giving PDG IDs)
      {this, "KeepBSM", false, "Keep BSM particles (easier than by PDG ID)"};
    Gaudi::Property<bool> m_rejectHadronChildren //!< Option to reject hadron descendants
      {this, "RejectHadronChildren", false, "Drop hadron descendants"};

    // Read(Decor)HandleKeys
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
      {this, "ParticlesKey", "TruthParticles", "ReadHandleKey for input TruthParticleContainer"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_originAccessorKey
      {this, "Input_classifierParticleOrigin", m_particlesKey, "classifierParticleOrigin","Name of the decoration which records the particle origin as determined by the MCTruthClassifier"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_typeAccessorKey
      {this, "Input_classifierParticleType", m_particlesKey, "classifierParticleType","Name of the decoration which records the particle type as determined by the MCTruthClassifier"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeAccessorKey
      {this, "Input_classifierParticleOutCome", m_particlesKey, "classifierParticleOutCome","Name of the decoration which records the particle outcome as determined by the MCTruthClassifier"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_classificationAccessorKey
      {this, "Input_Classification", m_particlesKey, "Classification","Name of the decoration which records the particle outcome as determined by the MCTruthClassifier"};

    // Write(Decor)HandleKeys
    SG::WriteHandleKey<xAOD::TruthVertexContainer> m_outputVerticesKey
      {this, "NewVertexKey", "", "WriteHandleKey for new TruthVertexContainer"};
    SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outputParticlesKey
      {this, "NewParticleKey", "", "WriteHandleKey for new TruthParticleContainer"};
    /// FIXME Using WriteDecorHandles for decorations on a Container created in the current algorithm is unnecessary.
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey
      {this, "classifierParticleOrigin", m_outputParticlesKey, "classifierParticleOrigin","Name of the decoration which records the particle origin as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_typeDecoratorKey
      {this, "classifierParticleType", m_outputParticlesKey, "classifierParticleType","Name of the decoration which records the particle type as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeDecoratorKey
      {this, "classifierParticleOutCome", m_outputParticlesKey, "classifierParticleOutCome","Name of the decoration which records the particle outcome as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_classificationDecoratorKey
      {this, "Classification", m_outputParticlesKey, "Classification","Name of the decoration which records the particle outcome as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_motherIDDecoratorKey
      {this, "motherID", m_outputParticlesKey, "motherID","Name of the decoration which records the ID of the particle's mother"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_daughterIDDecoratorKey
      {this, "daughterID", m_outputParticlesKey, "daughterID","Name of the decoration which records the ID of the particle's daughter"};
    /// ^^^^ These should be replaced by SG::Accessor.

    Gaudi::Property<int> m_generations //!< Number of generations after the particle in question to keep
      {this, "Generations", -1, "Number of generations after the particle in question to keep (-1 for all)"};

    // Helper functions for building up the decay product collections
    int addTruthParticle( const EventContext& ctx,
                          const xAOD::TruthParticle& old_part, xAOD::TruthParticleContainer* part_cont,
                          xAOD::TruthVertexContainer* vert_cont, std::vector<int>& seen_particles,
                          const int generations=-1) const;
    int addTruthVertex( const EventContext&,
                        const xAOD::TruthVertex& old_vert, xAOD::TruthParticleContainer* part_cont,
                        xAOD::TruthVertexContainer* vert_cont, std::vector<int>& seen_particles,
                        const int generations=-1) const;
    bool id_ok( const xAOD::TruthParticle& part ) const;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHDECAYCOLLECTIONMAKER_H
