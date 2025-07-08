//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "xAODTruthParticleAuxContainerCnv_v2.h"

// Gaudi/Athena include(s).
#include "AthContainers/tools/copyAuxStoreThinned.h"
#include "GaudiKernel/MsgStream.h"
#include "TruthUtils/MagicNumbers.h"

// EDM include(s):
#include "xAODTruth/versions/TruthParticleContainer_v1.h"
#include "xAODTruth/TruthParticleContainer.h"

// System include(s):
#include <cassert>
#include <stdexcept>

namespace HepMC {
  constexpr int INVALID_PARTICLE_ID = -1;
}

void xAODTruthParticleAuxContainerCnv_v2::persToTrans(
    const xAOD::TruthParticleAuxContainer_v2* futureObj,
    xAOD::TruthParticleAuxContainer* newObj, MsgStream& log) const {

  // Some security checks.
  assert(futureObj != nullptr);
  assert(newObj != nullptr);

  // Clear the transient object:
  newObj->resize(0);

  // Copy the payload of the v1 object into the latest one by misusing
  // the thinning code a bit.
  SG::copyAuxStoreThinned(*futureObj, *newObj, nullptr);
  //
  // And now fix up the status and barcode variables in the new object...
  //
  // Set up interface containers on top of them:
  xAOD::TruthParticleContainer futureInt;
  for( size_t i = 0; i < futureObj->size(); ++i ) {
    futureInt.push_back( new xAOD::TruthParticle() );
  }
  futureInt.setStore( futureObj );

  xAOD::TruthParticleContainer newInt;
  for( size_t i = 0; i < newObj->size(); ++i ) {
    newInt.push_back( new xAOD::TruthParticle() );
  }
  newInt.setStore( newObj );

  static const SG::AuxElement::Accessor<int> uniqueIdAcc ("uid");
  unsigned int index{0};
  // Loop over the interface objects, and do the conversion with their help:
  for( const xAOD::TruthParticle_v1* futurePart : futureInt ) {
    const int futureUniqueID = (uniqueIdAcc.isAvailable (*futurePart)) ? uniqueIdAcc(*futurePart) : HepMC::INVALID_PARTICLE_ID;
    xAOD::TruthParticle * newPart = newInt.at(index);
    // convert "future" status to the "old" status value
    newPart->setStatus(HepMC::old_particle_status_from_new(futurePart->status()));
    // The future unique identifiers and barcodes basically correspond one-to-one for generator particles
    int newBarcode = futureUniqueID;
    if (futurePart->status() > HepMC::SIM_STATUS_THRESHOLD ) {
      newBarcode += HepMC::SIM_BARCODE_THRESHOLD;
      if (futurePart->status() > HepMC::SIM_STATUS_INCREMENT) {
        log << MSG::WARNING << "xAODTruthParticleAuxContainerCnv_v2::persToTrans :  Particle which survived interactions during simulation detected!- Calculated barcode values do not encode this information." << endmsg;
        // TODO It would be technically possible to reverse engineer this information via a local implementation of the simulation_history method from MagicNumbers.h, but it would slowdown the TP conversion a lot.
      }
    }
    newPart->setBarcode(newBarcode); // NB This breaks down for simulated particles
    // This should be fine in the case that we are not mixing use of
    // xAOD::Truth with containers linking to HepMC Truth in the same
    // classes.
    ++index;
  }
  return;
}

/// This function should never be called, as we are not supposed to convert
/// objects before writing.
///
void xAODTruthParticleAuxContainerCnv_v2::transToPers(
    const xAOD::TruthParticleAuxContainer*, xAOD::TruthParticleAuxContainer_v2*,
    MsgStream& log) const {

  static const char* const ERRORMSG =
      "Somebody called xAODTruthParticleAuxContainerCnv_v2::transToPers";
  log << MSG::ERROR << ERRORMSG << endmsg;
  throw std::runtime_error(ERRORMSG);
  return;
}
