/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthBornLeptonCollectionMaker.cxx
// Makes a special collection of Born leptons
#include "GeneratorObjects/McEventCollection.h"
// R/W/D handles
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
// My own header file
#include "TruthBornLeptonCollectionMaker.h"
// EDM includes for the particles we need
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
// To look up which generator is being used
#include "StoreGate/StoreGateSvc.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "AthContainers/ConstAccessor.h"
// STL includes
#include <string>
#include "TruthUtils/HepMCHelpers.h"

// Constructor
DerivationFramework::TruthBornLeptonCollectionMaker::TruthBornLeptonCollectionMaker(const std::string& t,
                                const std::string& n,
                                const IInterface* p)
  : base_class(t,n,p)
  , m_metaStore( "MetaDataStore", n )
{
  declareProperty( "MetaDataStore", m_metaStore );
}

// Destructor
DerivationFramework::TruthBornLeptonCollectionMaker::~TruthBornLeptonCollectionMaker() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::TruthBornLeptonCollectionMaker::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

   // Input truth particles
   ATH_CHECK( m_particlesKey.initialize() );
   ATH_MSG_INFO("Using " << m_particlesKey.key() << " as the input truth container key");
   ATH_CHECK( m_mcEventsName.initialize() );

   // ReadDecorHandleKeys
   ATH_CHECK(m_originAccessorKey.initialize());
   ATH_CHECK(m_typeAccessorKey.initialize());
   ATH_CHECK(m_outcomeAccessorKey.initialize());
   ATH_CHECK(m_classificationAccessorKey.initialize());

  // Output truth particles
  if (m_collectionName.empty()) {
    ATH_MSG_FATAL("No key provided for the new truth particle collection");
    return StatusCode::FAILURE;
  } else {ATH_MSG_INFO("New truth particle collection key: " << m_collectionName.key() );}
  ATH_CHECK( m_collectionName.initialize());

  // Decoration keys - FIXME we should not need to use WriteDecorHandleKeys here.
  ATH_CHECK(m_originDecoratorKey.initialize());
  ATH_CHECK(m_typeDecoratorKey.initialize());
  ATH_CHECK(m_outcomeDecoratorKey.initialize());
  ATH_CHECK(m_classificationDecoratorKey.initialize());

  // TODO: needs to be made MT-friendly
  ATH_CHECK( m_metaStore.retrieve() );

  return StatusCode::SUCCESS;
}

// Selection and collection creation
StatusCode DerivationFramework::TruthBornLeptonCollectionMaker::addBranches(const EventContext& ctx) const
{
  // Retrieve truth collections
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_particlesKey,ctx);    
  if (!truthParticles.isValid()) {        
    ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_particlesKey);        
    return StatusCode::FAILURE;    
  }

  // Create the new particle containers and WriteHandles
  SG::WriteHandle<xAOD::TruthParticleContainer> newParticlesWriteHandle(m_collectionName, ctx);
  ATH_CHECK(newParticlesWriteHandle.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                           std::make_unique<xAOD::TruthParticleAuxContainer>()));
  ATH_MSG_DEBUG( "Recorded new TruthParticleContainer with key: " << (m_collectionName.key()));

  // Set up decorators
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > originDecorator(m_originDecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > typeDecorator(m_typeDecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > outcomeDecorator(m_outcomeDecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > classificationDecorator(m_classificationDecoratorKey, ctx);

  SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > originAccessor(m_originAccessorKey, ctx);
  SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > typeAccessor(m_typeAccessorKey, ctx);
  SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > outcomeAccessor(m_outcomeAccessorKey, ctx);
  SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > classificationAccessor(m_classificationAccessorKey, ctx);

  // Retrieve input data
  SG::ReadHandle<McEventCollection> mcEvts(m_mcEventsName,ctx);    
  if (!mcEvts.isValid()) {        
    ATH_MSG_ERROR("Couldn't retrieve McEventCollection collection with name " << m_mcEventsName);        
    return StatusCode::FAILURE;    
  }

  const HepMC::GenEvent* evt = mcEvts->front();
  const auto& attrs = evt->attributes();
  std::map<int, std::shared_ptr<HepMC3::Attribute> > attrsparticle;
  if (attrs.find("original_momentum") == attrs.end()) {
    // Here one should implement treatment of e.g. SHERPA, anything that does not use PHOTOS.
    // The implementation should be just a normal loop over event record with some conditions.
    for (unsigned int i=0; i<truthParticles->size(); ++i) {
      const xAOD::TruthParticle* theParticle = (*truthParticles)[i];
      // For Sherpa, skip is not status 11
      if (MC::isPhysical(theParticle)) continue;
      // Sherpa may have two sets of status == 11 leptons. Here we take the first set.
      // To do so, we check the first status == 11 leptons, 
      //   check that it has a child a parent barecode corresponding the the lepton's uniqueID
      // If so, then save this parent uniqueID and skip all status 11 leptons with this parent uniqueID.
      bool has_parent_of_same_flavour = false;
      if (theParticle->prodVtx())
      for (size_t p = 0; p < theParticle->prodVtx()->nIncomingParticles(); ++p){
          if (theParticle->parent(p)->pdg_id() == theParticle->pdg_id()) has_parent_of_same_flavour = true;
      }
      if (has_parent_of_same_flavour) continue;
      // Add this particle to the new collection
      xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
      newParticlesWriteHandle->push_back( xTruthParticle );
      // Fill with numerical content
      *xTruthParticle=*theParticle;
      // Copy over the decorations if they are available
      typeDecorator(*xTruthParticle) = typeAccessor(*theParticle);
      originDecorator(*xTruthParticle) = originAccessor(*theParticle);
      outcomeDecorator(*xTruthParticle) = outcomeAccessor(*theParticle);
      classificationDecorator(*xTruthParticle) = classificationAccessor(*theParticle);
    }
  } else {
    // We have info from PHOTOS
    attrsparticle = attrs.at("original_momentum");
    // Loop over particles, add relevant particles to new collection
    for (unsigned int i=0; i<truthParticles->size(); ++i) {
      const xAOD::TruthParticle* theParticle = (*truthParticles)[i];
      if (!theParticle) continue;
      if (!theParticle->isLepton()) continue;
      int id  = HepMC::uniqueID(theParticle); // Get particle id
      if (!attrsparticle.count(id)) continue; // Check that the particle has atribute "original_momentum", i.e. was a subject to radiation by PHOTOS.
      auto vecAttr = std::dynamic_pointer_cast<HepMC3::VectorDoubleAttribute>(attrsparticle.at(id)); // Cast the attribute to VectorDoubleAttribute
      if (!vecAttr) continue;
      // Add this particle to the new collection
      xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
      newParticlesWriteHandle->push_back( xTruthParticle );
      // Fill with numerical content
      *xTruthParticle=*theParticle;
      // Use original momenta from attribute
      xTruthParticle->setPx(vecAttr->value().at(0));
      xTruthParticle->setPy(vecAttr->value().at(1));
      xTruthParticle->setPz(vecAttr->value().at(2));
      xTruthParticle->setE(vecAttr->value().at(3));
      // Copy over the decorations if they are available
      typeDecorator(*xTruthParticle) = typeAccessor(*theParticle);
      originDecorator(*xTruthParticle) = originAccessor(*theParticle);
      outcomeDecorator(*xTruthParticle) = outcomeAccessor(*theParticle);
      classificationDecorator(*xTruthParticle) = classificationAccessor(*theParticle);
    } // Loop over all particles
  }
  return StatusCode::SUCCESS;
}
