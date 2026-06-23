/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "ZeroLifetimePositioner.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"

// CLHEP includes
#include "CLHEP/Vector/LorentzVector.h"

#include <limits>
#include <algorithm>

/** Constructor **/
Simulation::ZeroLifetimePositioner::ZeroLifetimePositioner( const std::string& name,
                                                            ISvcLocator* pSvcLocator )
  : base_class(name, pSvcLocator)
{
}


/** Athena algtool's Hooks */
StatusCode  Simulation::ZeroLifetimePositioner::initialize()
{
  ATH_MSG_VERBOSE("Initializing ...");
  for(auto& pdgcode : m_pdgCodesToCheck) {
    pdgcode = std::abs(pdgcode);
    ATH_MSG_DEBUG("Will look for verices where " << pdgcode <<
                  " oscillates into -" << pdgcode << " (and vice versa).");
  }
  return StatusCode::SUCCESS;
}


/** Athena algtool's Hooks */
StatusCode  Simulation::ZeroLifetimePositioner::finalize()
{
  ATH_MSG_VERBOSE("Finalizing ...");
  return StatusCode::SUCCESS;
}


StatusCode Simulation::ZeroLifetimePositioner::applyWorkaround(HepMC::GenEvent& ge) const
{
  ATH_MSG_DEBUG("applyWorkaround");
  return this->manipulate(ge, m_applyPatch, false);
}


StatusCode Simulation::ZeroLifetimePositioner::removeWorkaround(HepMC::GenEvent& ge) const
{
  ATH_MSG_DEBUG("removeWorkaround");
  return this->manipulate(ge, false, m_removePatch);
}


/** modifies (displaces) the given GenEvent */
StatusCode Simulation::ZeroLifetimePositioner::manipulate(HepMC::GenEvent& ge, bool applyPatch, bool removePatch) const
{
  // loop over the vertices in the event

  const auto pdgCodesBegin = m_pdgCodesToCheck.begin();
  const auto pdgCodesEnd = m_pdgCodesToCheck.end();
  for (auto& curVtx: ge.vertices()) {
    if (curVtx->particles_in().size()!=1 || curVtx->particles_out().size()!=1) { continue; }
    const int pdgIn=curVtx->particles_in().front()->pdg_id();
    const int pdgOut=curVtx->particles_out().front()->pdg_id();
    if (pdgIn!=-pdgOut ||
        std::find(pdgCodesBegin, pdgCodesEnd, std::abs(pdgIn))== pdgCodesEnd) {
      continue;
    }
    HepMC::GenVertexPtr nextVtx = curVtx->particles_out().front()->end_vertex();
    if(!nextVtx) { continue; }
    ATH_MSG_DEBUG("Found a vertex to correct with incoming PDG code = " << pdgIn);
    ATH_MSG_VERBOSE("Next Vertex:");
    if (ATH_UNLIKELY(this->msgLvl (MSG::VERBOSE))) {
      HepMC::Print::line(nextVtx);
    }
    // NB Doing this check to explicitly avoid the fallback mechanism in
    // HepMC3::GenVertex::position() to return the position of
    // another GenVertex in the event if the position isn't set (or is set to zero)!
    const HepMC::FourVector &nextVec = (nextVtx->has_set_position()) ? nextVtx->position() : HepMC::FourVector::ZERO_VECTOR();
    const CLHEP::HepLorentzVector nextPos( nextVec.x(), nextVec.y(), nextVec.z(), nextVec.t() );
    ATH_MSG_VERBOSE("Current Vertex:");
    if (ATH_UNLIKELY(this->msgLvl (MSG::VERBOSE))) {
       HepMC::Print::line(curVtx);
    }
    if (applyPatch) {
      HepMC::GenVertexPtr prevVtx = curVtx->particles_in().front()->production_vertex();
      ATH_MSG_VERBOSE("Previous Vertex:");
      if (ATH_UNLIKELY(this->msgLvl (MSG::VERBOSE))) {
        HepMC::Print::line(prevVtx);
      }
      // NB Doing this check to explicitly avoid the fallback mechanism in
      // HepMC3::GenVertex::position() to return the position of
      // another GenVertex in the event if the position isn't set (or is set to zero)!
      const HepMC::FourVector &prevVec = (prevVtx->has_set_position()) ? prevVtx->position() : HepMC::FourVector::ZERO_VECTOR();
      const CLHEP::HepLorentzVector prevPos( prevVec.x(), prevVec.y(), prevVec.z(), prevVec.t() );
      CLHEP::HepLorentzVector newPos = 0.5*(prevPos+nextPos);
      curVtx->set_position(HepMC::FourVector(newPos.x(),newPos.y(),newPos.z(),newPos.t()));
      ATH_MSG_DEBUG("Revised current Vertex");
      if (ATH_UNLIKELY(this->msgLvl (MSG::VERBOSE))) {
        HepMC::Print::line(curVtx);
      }
    }
    if (removePatch) {
      CLHEP::HepLorentzVector newPos = nextPos;
      curVtx->set_position(HepMC::FourVector(newPos.x(),newPos.y(),newPos.z(),newPos.t()));
      ATH_MSG_DEBUG("Revised current Vertex");
      if (ATH_UNLIKELY(this->msgLvl (MSG::VERBOSE))) {
        HepMC::Print::line(curVtx);
      }
    }
  }

  return StatusCode::SUCCESS;
}
