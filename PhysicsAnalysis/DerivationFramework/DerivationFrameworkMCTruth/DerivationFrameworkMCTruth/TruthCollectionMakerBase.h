/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBASE_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBASE_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTruth/TruthParticleContainer.h"
// Handles and keys
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"

// Forward declarations
class StoreGateSvc;

namespace DerivationFramework {

  class TruthCollectionMakerBase : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;
  protected:
    virtual std::vector<int> updateMask(const xAOD::TruthParticleContainer*) const;
  private:
    mutable std::atomic<unsigned int> m_ntotpart{};
    mutable std::atomic<unsigned int> m_npasspart{};
    // R/W handles
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
      {this, "ParticlesKey", "TruthParticles", "ReadHandleKey for input TruthParticleContainer"};
    SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outputParticlesKey
      {this, "OutputCollectionName", "", "WriteHandleKey for new TruthParticleContainer"};
    // Non-handle properties
    Gaudi::Property<bool> m_buildingW{this, "BuildingW", false, "Are we building W bosons?"};
    Gaudi::Property<bool> m_buildingZ{this, "BuildingZ", false, "Are we building Z bosons?"};
    Gaudi::Property<bool> m_do_compress
      {this, "Do_Compress", false, "Removes particles with the same pdgId in a decay chain (but keeps first and last)"};
    Gaudi::Property<bool> m_do_sherpa
      {this, "Do_Sherpa", false, "Checks if there are truth W bosons in the current record.  If not, tries to combine W daughters to create one"};
    Gaudi::Property<bool> m_keep_navigation_info
      {this, "KeepNavigationInfo", true, "m_do_sherpa currently only works for W+jets"};
    // Decor handles
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_linkDecoratorKey
      {this, "originalTruthParticle", m_outputParticlesKey, "originalTruthParticle", "Name of the decoration linking to the original truth particle"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey
      {this, "classifierParticleOrigin", m_outputParticlesKey, "classifierParticleOrigin", "Name of the decoration which records the particle origin as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_typeDecoratorKey
      {this, "classifierParticleType", m_outputParticlesKey, "classifierParticleType", "Name of the decoration which records the particle type as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeDecoratorKey
      {this, "classifierParticleOutCome", m_outputParticlesKey, "classifierParticleOutCome", "Name of the decoration which records the particle outcome as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_classificationDecoratorKey
      {this, "Classification", m_outputParticlesKey, "Classification", "Name of the decoration which records the particle classification as determined by the MCTruthClassifier"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_motherIDDecoratorKey
      {this, "motherID", m_outputParticlesKey, "motherID", "Name of the decoration which records the ID of the particle's mother"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_daughterIDDecoratorKey
      {this, "daughterID", m_outputParticlesKey, "daughterID", "Name of the decoration which records the ID of the particle's daughter"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_hadronOriginDecoratorKey
      {this, "TopHadronOriginFlag", m_outputParticlesKey, "TopHadronOriginFlag", "Name of the decoration which records the origin of hadrons from top decays"};

    // Decorations to be read from elements of the "TruthParticles" container
    // to be copied onto elements of the output container.
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_originReadDecorKey
      {this, "inputClassifierParticleOrigin", m_particlesKey, "classifierParticleOrigin", "Particle origin"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_typeReadDecorKey
      {this, "inputClassifierParticleType", m_particlesKey, "classifierParticleType", "Particle type"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeReadDecorKey
      {this, "inputClassifierParticleOutCome", m_particlesKey, "classifierParticleOutCome", "Particle outcome"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_classificationReadDecorKey
      {this, "inputClassification", m_particlesKey, "Classification", "Classification code"};

    ServiceHandle<StoreGateSvc> m_metaStore{this, "MetaDataStore", "MetaDataStore"}; //!< Handle on the metadata store for init
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBASE_H
