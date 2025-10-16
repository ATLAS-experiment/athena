/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthBornLeptonCollectionMaker.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TRUTHBORNLEPTONCOLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTHBORNLEPTONCOLLECTIONMAKER_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
// EDM includes for the particles we need
#include "xAODTruth/TruthParticle.h"
// R/W/D key handles
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
// For the Metadata store
#include "GaudiKernel/ServiceHandle.h"
// STL includes
#include <string>

// Forward declarations
class StoreGateSvc;

namespace DerivationFramework {

  class TruthBornLeptonCollectionMaker : public extends<AthAlgTool, IAugmentationTool> {
  public:
    TruthBornLeptonCollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
    ~TruthBornLeptonCollectionMaker();
    StatusCode initialize();
    virtual StatusCode addBranches(const EventContext& ctx) const;

  private:
    //!< Input particle collection key
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
    {this, "ParticlesKey", "TruthParticles", "Name of TruthParticle key for input"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_originAccessorKey
      {this, "Input_classifierParticleOrigin", m_particlesKey, "classifierParticleOrigin", "Particle origin decoration"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_typeAccessorKey
      {this, "Input_classifierParticleType",m_particlesKey, "classifierParticleType", "Particle type decoration"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeAccessorKey
      {this, "Input_classifierParticleOutCome", m_particlesKey, "classifierParticleOutCome", "Particle outcome decoration"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_classificationAccessorKey
      {this, "Input_Classification", m_particlesKey, "Classification", "Classification code decoration"};
    //!< Output particle collection key
    SG::WriteHandleKey<xAOD::TruthParticleContainer> m_collectionName
      {this, "NewCollectionName", "", "Name of TruthParticle key for output"};
    // Decorators - FIXME we should not need to use WriteDecorHandleKeys here.
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey
      {this, "classifierParticleOrigin", m_collectionName, "classifierParticleOrigin", "Particle origin decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_typeDecoratorKey
      {this, "classifierParticleType",m_collectionName, "classifierParticleType", "Particle type decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeDecoratorKey
      {this, "classifierParticleOutCome", m_collectionName, "classifierParticleOutCome", "Particle outcome decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_classificationDecoratorKey
      {this, "Classification", m_collectionName, "Classification", "Classification code decoration"};

    ServiceHandle<StoreGateSvc> m_metaStore; //!< Handle on the metadata store for init
    /// Helper function for finding bare descendents of born leptons
    bool hasBareDescendent( const xAOD::TruthParticle* p ) const;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHBORNLEPTONCOLLECTIONMAKER_H
