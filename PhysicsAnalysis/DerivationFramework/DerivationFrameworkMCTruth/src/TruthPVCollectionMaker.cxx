/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthPVCollectionMaker.cxx
// Makes a small collection of 'primary' vertices, one per event
// A bit like a collection of 'reconstructable' vertices
// Future option: try to add b-decay vertices, or LLP vertices?

#include "DerivationFrameworkMCTruth/TruthPVCollectionMaker.h"
#include "xAODTruth/TruthVertexAuxContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "TruthUtils/HepMCHelpers.h"

// Constructor
DerivationFramework::TruthPVCollectionMaker::TruthPVCollectionMaker(const std::string& t,
                                                                    const std::string& n,
                                                                    const IInterface* p)
  : base_class(t,n,p)
 {
}

// Destructor
DerivationFramework::TruthPVCollectionMaker::~TruthPVCollectionMaker() = default;
// Athena initialize
StatusCode DerivationFramework::TruthPVCollectionMaker::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");

    // Check configuration, print errors, warning, and information for the user
    ATH_CHECK(m_eventsKey.initialize());
    ATH_MSG_INFO("Using " << m_eventsKey.fullKey() << " as the source collections for new truth collections");
    ATH_CHECK(m_outVtxKey.initialize());
    ATH_MSG_INFO("New truth vertex collection key: " << m_outVtxKey.fullKey() );

    return StatusCode::SUCCESS;
}


// Selection and collection creation
StatusCode DerivationFramework::TruthPVCollectionMaker::addBranches() const
{
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    // Retrieve truth collections
    SG::ReadHandle<xAOD::TruthEventContainer> importedTruthEvents{m_eventsKey, ctx};
    if (!importedTruthEvents.isValid()) {
        ATH_MSG_ERROR("No TruthEvent collection with name " << m_eventsKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }

    // Create the new vertex containers
    SG::WriteHandle<xAOD::TruthVertexContainer> writeHandleVtx{m_outVtxKey, ctx};
    ATH_CHECK(writeHandleVtx.record(std::make_unique<xAOD::TruthVertexContainer>(), 
                                    std::make_unique<xAOD::TruthVertexAuxContainer>()));
    xAOD::TruthVertexContainer* newVertexCollection = writeHandleVtx.ptr();
    ATH_MSG_DEBUG( "Recorded new TruthVertexContainer with key: " << m_outVtxKey.fullKey());

    // Go through the events, add one vertex for each event
    for (const auto * event : *importedTruthEvents){
        // Just in case there is a place-holder
        if (!event) continue;
        // Try with the signal process vertex
        const xAOD::TruthVertex* old_vert(event->signalProcessVertex());
        // If that's not there, grab the first vertex
        if (!old_vert && event->nTruthVertices()>0){
            old_vert = event->truthVertex(0);
        }
        if (old_vert){
            // Hit -- copy it!
            // Make a new vertex and add it to the container
            xAOD::TruthVertex* xTruthVertex = new xAOD::TruthVertex();
            newVertexCollection->push_back( xTruthVertex );
            // Set properties
            xTruthVertex->setStatus(HepMC::status(old_vert));
            xTruthVertex->setUid(HepMC::uniqueID(old_vert));
            xTruthVertex->setX(old_vert->x());
            xTruthVertex->setY(old_vert->y());
            xTruthVertex->setZ(old_vert->z());
            xTruthVertex->setT(old_vert->t());
        } else {
            ATH_MSG_WARNING("No signal vertex or vertices associated to an input event!");
        }
    }
    return StatusCode::SUCCESS;
}
