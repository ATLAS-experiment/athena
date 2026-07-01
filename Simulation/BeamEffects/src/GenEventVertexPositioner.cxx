/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "GenEventVertexPositioner.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"

// Framework includes
#include "HepMC_Interfaces/ILorentzVectorGenerator.h"

// CLHEP includes
#include "CLHEP/Vector/LorentzVector.h"

namespace Simulation
{
  /** Constructor **/
  GenEventVertexPositioner::GenEventVertexPositioner( const std::string& t,
                                                      const std::string& n,
                                                      const IInterface* p )
    : base_class(t,n,p)
  {
  }

  /** Athena algtool's Hooks */
  StatusCode  GenEventVertexPositioner::initialize()
  {
    ATH_MSG_VERBOSE("Initializing ...");

    // retrieve Vertex Shifters
    if ( !m_vertexShifters.empty() ) {
      ATH_CHECK(m_vertexShifters.retrieve());
    }

    return StatusCode::SUCCESS;
  }

  /** Athena algtool's Hooks */
  StatusCode  GenEventVertexPositioner::finalize()
  {
    ATH_MSG_VERBOSE("Finalizing ...");
    return StatusCode::SUCCESS;
  }

  /** modifies (displaces) the given GenEvent */
  StatusCode GenEventVertexPositioner::manipulate(HepMC::GenEvent& ge, const EventContext& ctx) const
  {
    // Grab signal_process_vertex pointer
    auto signalProcVtx=HepMC::signal_process_vertex(&ge);
    if(!signalProcVtx) {
      ATH_MSG_ERROR("Expected GenEvent::signal_process_vertex() to already have been set at this point!");
      return StatusCode::FAILURE;
    }

    // loop over all given ILorentzVectorGenerator AlgTools
    for (const auto& vertexShifter : m_vertexShifters) {

      // call VertexShifter and let it compute the current shift
      CLHEP::HepLorentzVector *curShift = vertexShifter->generate(ctx);
      if (!curShift) {
        ATH_MSG_ERROR("Vertex Shifter AthenaTool returned zero-pointer! Ignore.");
        continue;
      }

      ATH_MSG_VERBOSE("Retrieved Vertex shift of: " << *curShift << " from " << vertexShifter->name());

      // As signal process vertex is a pointer, there is some risk
      // that the pointer points to a vertex somewhere else in the
      // event, rather than a unique / new vertex, in which case we
      // will modify its position in the loop below and will not need
      // to treat it separately.
      bool modifySigVtx(true);

      // loop over the vertices in the event, they are in respect with another
      //   (code from Simulation/Fatras/FatrasAlgs/McEventPreProcessing.cxx)
      for(auto& curVtx: ge.vertices()) {
        // NB Doing this check to explicitly avoid the fallback mechanism in
        // HepMC3::GenVertex::position() to return the position of
        // another GenVertex in the event if the position isn't set (or is set to zero)!
        const HepMC::FourVector &curPos = (curVtx->has_set_position()) ? curVtx->position() : HepMC::FourVector::ZERO_VECTOR();

        // get a copy of the current vertex position
        CLHEP::HepLorentzVector newPos( curPos.x(), curPos.y(), curPos.z(), curPos.t() );
        // and update it with the given smearing
        newPos += (*curShift);

        ATH_MSG_VERBOSE( "Original vtx  position = " << curPos.x() << ", " << curPos.y() << ", " << curPos.z() );
        ATH_MSG_VERBOSE( "Updated  vtx  position = " << newPos );

        // store the updated position in the vertex
        curVtx->set_position( HepMC::FourVector(newPos.x(),newPos.y(),newPos.z(),newPos.t()));
        if(modifySigVtx && signalProcVtx==curVtx) {
          modifySigVtx=false;
        }
      }

      // Do the same for the signal process vertex if still required.
      if (modifySigVtx) {
        const HepMC::FourVector &curPos = HepMC::signal_process_vertex(&ge)->position();
        CLHEP::HepLorentzVector newPos( curPos.x(), curPos.y(), curPos.z(), curPos.t() );
        newPos += (*curShift);
        (HepMC::signal_process_vertex(&ge))->set_position( HepMC::FourVector(newPos.x(),newPos.y(),newPos.z(),newPos.t()) );
      }

      // memory cleanup
      delete curShift;
    }

    return StatusCode::SUCCESS;
  }

}
