/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthDecayCollectionMaker.cxx
// Removes all truth particles/vertices which do not pass a user-defined cut
// Based on TruthCollectionMaker (but simpler)

#include "DerivationFrameworkMCTruth/TruthDecayCollectionMaker.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstAccessor.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODTruth/TruthVertexAuxContainer.h"

#include "TruthUtils/HepMCHelpers.h"

// STL includes
#include <vector>
#include <string>
// For a find in the vector
#include <algorithm>

// Constructor
DerivationFramework::TruthDecayCollectionMaker::TruthDecayCollectionMaker(const std::string& t,
                                                                          const std::string& n,
                                                                          const IInterface* p)
  : base_class(t,n,p)
{
}

// Destructor
DerivationFramework::TruthDecayCollectionMaker::~TruthDecayCollectionMaker() {
}

// Athena initialize
StatusCode DerivationFramework::TruthDecayCollectionMaker::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");

    // Input truth particles
    ATH_CHECK( m_particlesKey.initialize() );
    ATH_MSG_INFO("Using " << m_particlesKey.key() << " as the input truth container key");

    // Output particle/vertex containers
    if (m_collectionName.empty()) {
        ATH_MSG_FATAL("No base name provided for the new truth particle/vertex containers");
        return StatusCode::FAILURE;
    } else {ATH_MSG_INFO("Base name for new truth particle/vertex containers: " << m_collectionName );}
    m_outputParticlesKey = m_collectionName + "Particles";
    ATH_CHECK(m_outputParticlesKey.initialize());
    ATH_MSG_INFO("New truth particles container key: " << m_outputParticlesKey.key() );
    m_outputVerticesKey = m_collectionName + "Vertices"; 
    ATH_CHECK(m_outputVerticesKey.initialize());
    ATH_MSG_INFO("New truth vertices container key: " << m_outputVerticesKey.key() );

    if (m_pdgIdsToKeep.empty() && !m_keepBHadrons && !m_keepCHadrons && !m_keepBSM) {
        ATH_MSG_FATAL("No PDG IDs provided, not keeping b- or c-hadrons or BSM particles -- what do you want?");
        return StatusCode::FAILURE;
    }

    // Decorators
    m_originDecoratorKey = m_outputParticlesKey.key()+".classifierParticleOrigin";
    ATH_CHECK(m_originDecoratorKey.initialize());
    m_typeDecoratorKey = m_outputParticlesKey.key()+".classifierParticleType";
    ATH_CHECK(m_typeDecoratorKey.initialize());
    m_outcomeDecoratorKey = m_outputParticlesKey.key()+".classifierParticleOutCome";
    ATH_CHECK(m_outcomeDecoratorKey.initialize());
    m_classificationDecoratorKey = m_outputParticlesKey.key()+".Classification";
    ATH_CHECK(m_classificationDecoratorKey.initialize());
    m_motherIDDecoratorKey = m_outputParticlesKey.key()+".motherID";
    ATH_CHECK(m_motherIDDecoratorKey.initialize());
    m_daughterIDDecoratorKey = m_outputParticlesKey.key()+".daughterID";
    ATH_CHECK(m_daughterIDDecoratorKey.initialize());

    return StatusCode::SUCCESS;
}


// Selection and collection creation
StatusCode DerivationFramework::TruthDecayCollectionMaker::addBranches() const
{
    // Event context for AthenaMT
    const EventContext& ctx = Gaudi::Hive::currentContext();
     
    // Retrieve truth collections
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_particlesKey,ctx);
    if (!truthParticles.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_particlesKey);
        return StatusCode::FAILURE;
    }

    // Create the new particle containers and WriteHandles
    SG::WriteHandle<xAOD::TruthParticleContainer> newParticlesWriteHandle(m_outputParticlesKey, ctx);
    ATH_CHECK(newParticlesWriteHandle.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                             std::make_unique<xAOD::TruthParticleAuxContainer>()));
    ATH_MSG_DEBUG( "Recorded new TruthParticleContainer with key: " << (m_outputParticlesKey.key()));
    // Create the new vertex containers and WriteHandles
    SG::WriteHandle<xAOD::TruthVertexContainer> newVerticesWriteHandle(m_outputVerticesKey, ctx);
    ATH_CHECK(newVerticesWriteHandle.record(std::make_unique<xAOD::TruthVertexContainer>(),
                                            std::make_unique<xAOD::TruthVertexAuxContainer>()));
    ATH_MSG_DEBUG( "Recorded new TruthVertexContainer with key: " << (m_outputVerticesKey.key()));

    // List of unique IDs for particles in our collection already.  Because of the way we recurse,
    // adding more particles as we go, there should be no need to add (or help from adding) the
    // unique IDs of particles that we are *not* going to keep
    std::vector<int> seen_particles;
    // Go through that list of particles!
    for (const auto * part : *truthParticles){
        // If this passes my cuts, keep it
        if (id_ok(*part)){
            addTruthParticle( ctx, *part, newParticlesWriteHandle.ptr(), newVerticesWriteHandle.ptr(), seen_particles , m_generations );
        }
    } // Loop over the initial truth particle collection
    return StatusCode::SUCCESS;
}

int DerivationFramework::TruthDecayCollectionMaker::addTruthParticle( const EventContext& ctx,
                                                                      const xAOD::TruthParticle& old_part, 
                                                                      xAOD::TruthParticleContainer* part_cont, 
                                                                      xAOD::TruthVertexContainer* vert_cont, 
                                                                      std::vector<int>& seen_particles,
                                                                      const int generations) const {
    // See if we've seen it - note, could also do this with a unary function on the container itself
    if (std::find(seen_particles.begin(),seen_particles.end(),HepMC::uniqueID(&old_part))!=seen_particles.end()){
      for (size_t p=0;p<part_cont->size();++p){
        // Was it a hit?
        const xAOD::TruthParticle *theParticle = (*part_cont)[p];
        if (HepMC::is_same_particle(theParticle,&old_part)) return p;
      } // Look through the old container
    } // Found it in the old container
    // Now we have seen it
    seen_particles.push_back(HepMC::uniqueID(&old_part));
    // Set up decorators
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > originDecorator(m_originDecoratorKey, ctx);  
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > typeDecorator(m_typeDecoratorKey, ctx);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > outcomeDecorator(m_outcomeDecoratorKey, ctx);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > classificationDecorator(m_classificationDecoratorKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer, int > motherIDDecorator(m_motherIDDecoratorKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer, int > daughterIDDecorator(m_daughterIDDecoratorKey, ctx);
    // Make a truth particle and add it to the container
    xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
    part_cont->push_back( xTruthParticle );
    // Fill with numerical content
    xTruthParticle->setPdgId(old_part.pdgId());
    xTruthParticle->setBarcode(HepMC::barcode(&old_part)); // FIXME barcode-based
    xTruthParticle->setStatus(old_part.status());
    xTruthParticle->setM(old_part.m());
    xTruthParticle->setPx(old_part.px());
    xTruthParticle->setPy(old_part.py());
    xTruthParticle->setPz(old_part.pz());
    xTruthParticle->setE(old_part.e());
    // Copy over the polarization information if it's there
    if (old_part.polarization().valid()){
        xTruthParticle->setPolarizationParameter( old_part.polarizationParameter( xAOD::TruthParticle::polarizationPhi ) , xAOD::TruthParticle::polarizationPhi );
        xTruthParticle->setPolarizationParameter( old_part.polarizationParameter( xAOD::TruthParticle::polarizationTheta ) , xAOD::TruthParticle::polarizationTheta );
    }
    // Make a link to this particle
    int my_index = part_cont->size()-1;
    ElementLink<xAOD::TruthParticleContainer> eltp(*part_cont, my_index);
    // Decay vertex information
    if (old_part.hasDecayVtx()) {
        int vert_index = addTruthVertex( ctx, *old_part.decayVtx(), part_cont, vert_cont, seen_particles, generations);
        ElementLink<xAOD::TruthVertexContainer> eltv( *vert_cont, vert_index );
        xTruthParticle->setDecayVtxLink( eltv );
        (*vert_cont)[vert_index]->addIncomingParticleLink( eltp );
    }
    // Copy over the decorations if they are available
    static const SG::ConstAccessor<unsigned int> classifierParticleTypeAcc("classifierParticleType");
    static const SG::ConstAccessor<unsigned int> classifierParticleOriginAcc("classifierParticleOrigin");
    static const SG::ConstAccessor<unsigned int> classifierParticleOutComeAcc("classifierParticleOutCome");
    static const SG::ConstAccessor<unsigned int> ClassificationAcc("Classification");

    typeDecorator(*xTruthParticle) = classifierParticleTypeAcc.withDefault (old_part, 0);
    originDecorator(*xTruthParticle) = classifierParticleOriginAcc.withDefault (old_part, 0);
    outcomeDecorator(*xTruthParticle) = classifierParticleOutComeAcc.withDefault (old_part, 0);
    classificationDecorator(*xTruthParticle) = ClassificationAcc.withDefault (old_part, 0);

    // Return a link to this particle
    return my_index;
}

int DerivationFramework::TruthDecayCollectionMaker::addTruthVertex( const EventContext& ctx, 
                                                                    const xAOD::TruthVertex& old_vert, 
                                                                    xAOD::TruthParticleContainer* part_cont, 
                                                                    xAOD::TruthVertexContainer* vert_cont, 
                                                                    std::vector<int>& seen_particles,
                                                                    const int generations) const {
    // Make a new vertex and add it to the container
    xAOD::TruthVertex* xTruthVertex = new xAOD::TruthVertex();
    vert_cont->push_back( xTruthVertex );
    // Get a link to this vertex -- will be used to set production vertices on all the next particles
    int my_index = vert_cont->size()-1;
    ElementLink<xAOD::TruthVertexContainer> eltv(*vert_cont, my_index);
    // Set properties
    xTruthVertex->setId(HepMC::status(old_vert));
    xTruthVertex->setBarcode(HepMC::barcode(&old_vert)); // FIXME barcode-based
    xTruthVertex->setX(old_vert.x());
    xTruthVertex->setY(old_vert.y());
    xTruthVertex->setZ(old_vert.z());
    xTruthVertex->setT(old_vert.t());
    // If we are done, then stop here
    if (generations==0) return my_index;
    // Add all the outgoing particles
    for (size_t n=0;n<old_vert.nOutgoingParticles();++n){
        if (!old_vert.outgoingParticle(n)) continue; // Just in case we removed some truth particles, e.g. G4 decays
        if (m_rejectHadronChildren && old_vert.outgoingParticle(n)->isHadron()) { // Option to skip hadrons outright{
          continue;
        }
        // Continue on the next generation; note that we only decrement the generation if this particle doesn't also pass our cuts
        int part_index = addTruthParticle( ctx, *old_vert.outgoingParticle(n), part_cont, vert_cont, seen_particles,
                                           generations-1+(id_ok(*old_vert.outgoingParticle(n))?1:0) );
        ElementLink<xAOD::TruthParticleContainer> eltp( *part_cont, part_index);
        xTruthVertex->addOutgoingParticleLink( eltp );
        (*part_cont)[part_index]->setProdVtxLink( eltv );
    }
    // Return a link to this vertex
    return my_index;
}

bool DerivationFramework::TruthDecayCollectionMaker::id_ok( const xAOD::TruthParticle& part ) const
{
    // Check list of PDG IDs to keep
    for (int id : m_pdgIdsToKeep){
        if (part.absPdgId()==id){
            return true;
        } // Found a particle of interest!
    } // Loop over the PDG IDs we want to keep
    // Also check functions for B/C/BSM
    return (m_keepBHadrons && part.isBottomHadron()) ||

        (m_keepCHadrons && part.isCharmHadron()) ||

        (m_keepBSM && part.isBSM());
}
