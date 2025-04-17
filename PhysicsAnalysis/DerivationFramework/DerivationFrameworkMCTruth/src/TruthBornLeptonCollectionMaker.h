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
      virtual StatusCode addBranches() const;

    private:
       //!< Input particle collection key 
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
        {this, "ParticlesKey", "TruthParticles", "Name of TruthParticle key for input"};
      //!< Output particle collection key 
      SG::WriteHandleKey<xAOD::TruthParticleContainer> m_collectionName
        {this, "NewCollectionName", "", "Name of TruthParticle key for output"};   
      // Decorators
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey         
        {this, "classifierParticleOrigin", "TruthParticles.classifierParticleOrigin", "Particle origin decoration, set at initialisation"};      
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_typeDecoratorKey         
        {this, "classifierParticleType","TruthParticles.classifierParticleType", "Particle type decoration, set at initialisation"};      
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeDecoratorKey         
        {this, "classifierParticleOutCome", "TruthParticles.classifierParticleOutCome", "Particle outcome decoration, set at initialisation"};      
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_classificationDecoratorKey         
        {this, "Classification", "TruthParticles.Classification", "Classification code decorator, set at initialisation"};

      ServiceHandle<StoreGateSvc> m_metaStore; //!< Handle on the metadata store for init
      /// Helper function for finding bare descendents of born leptons
      bool hasBareDescendent( const xAOD::TruthParticle* p ) const;
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTHBORNLEPTONCOLLECTIONMAKER_H
