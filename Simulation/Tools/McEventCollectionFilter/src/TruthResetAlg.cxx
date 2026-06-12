/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthResetAlg.h"
//
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorObjects/McEventCollection.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Units/SystemOfUnits.h"
//
#include "CLHEP/Geometry/Point3D.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include <climits>

//----------------------------------------------------
StatusCode TruthResetAlg::initialize() {
//----------------------------------------------------

  ATH_CHECK(m_inputMcEventCollection.initialize());
  ATH_CHECK(m_outputMcEventCollection.initialize());
  return StatusCode::SUCCESS;

}

//-------------------------------------------------
StatusCode TruthResetAlg::execute(const EventContext& ctx) const {
//-------------------------------------------------

  ATH_MSG_DEBUG( " execute..... " );
  SG::ReadHandle<McEventCollection> inputMcEventCollection(m_inputMcEventCollection, ctx);
  if (!inputMcEventCollection.isValid()) {
    ATH_MSG_ERROR("Could not find input McEventCollection called " << inputMcEventCollection.name() << " in store " << inputMcEventCollection.store() << ".");
    return StatusCode::FAILURE;
  }
  const HepMC::GenEvent& inputEvent(**(inputMcEventCollection->begin()));

  //Sanity check
  bool inputProblem(false);
  for (const auto& particle: inputEvent) {
    if (MC::isStable(particle)) {
      if (!particle->production_vertex()) {
        ATH_MSG_ERROR("Stable particle without a production vertex!! " << particle);
        inputProblem = true;
      }
    }
    else if (MC::isDecayed(particle)) {
      if (!particle->production_vertex()) {
        ATH_MSG_ERROR("Decyed particle without a production vertex!! " << particle);
        inputProblem = true;
      }
      if (!particle->end_vertex()) {
        ATH_MSG_ERROR("Decyed particle without an end vertex!! " << particle);
        inputProblem = true;
      }
    }
  }
  if (inputProblem) {
    ATH_MSG_FATAL("Problems in input GenEvent - bailing out.");
    return StatusCode::FAILURE;
  }
   /// The algorithm makes a deep copy of the event and drops the particles and vertices created by the simulation
   /// from the copied event.
   std::unique_ptr<HepMC::GenEvent>  outputEvent = std::make_unique<HepMC::GenEvent>(inputEvent);
   if (inputEvent.run_info()) outputEvent->set_run_info(std::make_shared<HepMC3::GenRunInfo>(*(inputEvent.run_info().get())));
   for (;;) {
     std::vector<HepMC::GenParticlePtr> p_to_remove;
     std::vector<HepMC::GenVertexPtr> v_to_remove;
     for (auto& particle: outputEvent->particles()) {
       if (HepMC::is_simulation_particle(particle)) {
         p_to_remove.push_back(particle);
       }
     }
     for (auto& particle: p_to_remove) outputEvent->remove_particle(particle);
     for (auto& vertex: outputEvent->vertices()) {
       if (HepMC::is_simulation_vertex(vertex) || vertex->particles_out().empty() ) {
         v_to_remove.push_back(vertex);
       }
     }
     for (auto& vertex: v_to_remove) outputEvent->remove_vertex(vertex);
     if (p_to_remove.empty() && v_to_remove.empty()) break;
   }


  //Sanity check
  bool outputProblem(false);
  for (const auto& particle: *(outputEvent.get())) {
    if (MC::isStable(particle)) {
      if (!particle->production_vertex()) {
        ATH_MSG_ERROR("Stable particle without a production vertex!! " << particle);
        outputProblem = true;
      }
      if (particle->end_vertex()) {
        ATH_MSG_ERROR("Stable particle with an end vertex!! " << particle);
        outputProblem = true;
      }
    }
    else if (MC::isDecayed(particle)) {
      if (!particle->production_vertex()) {
        ATH_MSG_ERROR("Decayed particle without a production vertex!! " << particle);
        outputProblem = true;
      }
      if (!particle->end_vertex()) {
        ATH_MSG_ERROR("Decayed  particle without an end vertex!! " << particle);
        outputProblem = true;
      }
    }
  }
  if (outputProblem) {
    ATH_MSG_FATAL("Problems in output GenEvent - bailing out.");
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<McEventCollection> outputMcEventCollection(m_outputMcEventCollection, ctx);
  ATH_CHECK(outputMcEventCollection.record(std::make_unique<McEventCollection>()));
  outputMcEventCollection->push_back(outputEvent.release());
  if (!outputMcEventCollection.isValid()) {
    ATH_MSG_ERROR("Could not record output McEventCollection called " << outputMcEventCollection.name() << " in store " << outputMcEventCollection.store() << ".");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "succeded TruthResetAlg ..... " );

  return StatusCode::SUCCESS;

}
