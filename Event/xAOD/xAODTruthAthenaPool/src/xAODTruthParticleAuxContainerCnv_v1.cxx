//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "xAODTruthParticleAuxContainerCnv_v1.h"

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

void xAODTruthParticleAuxContainerCnv_v1::persToTrans(
    const xAOD::TruthParticleAuxContainer_v1* oldObj,
    xAOD::TruthParticleAuxContainer* newObj, MsgStream&) const {

  // Some security checks.
  assert(oldObj != nullptr);
  assert(newObj != nullptr);

  // Clear the transient object:
  newObj->resize(0);

  // Copy the payload of the v1 object into the latest one by misusing
  // the thinning code a bit.
  SG::copyAuxStoreThinned(*oldObj, *newObj, nullptr);
  //
  // And now fix up the status and barcode variables in the new object...
  //
  // Set up interface containers on top of them:
  xAOD::TruthParticleContainer oldInt;
  for( size_t i = 0; i < oldObj->size(); ++i ) {
    oldInt.push_back( new xAOD::TruthParticle() );
  }
  oldInt.setStore( oldObj );

  xAOD::TruthParticleContainer newInt;
  for( size_t i = 0; i < newObj->size(); ++i ) {
    newInt.push_back( new xAOD::TruthParticle() );
  }
  newInt.setStore( newObj );

  unsigned int index{0};
  static const SG::AuxElement::Accessor<int> barcodeAcc ("barcode");
  // Loop over the interface objects, and do the conversion with their help:
  for( const xAOD::TruthParticle_v1* oldPart : oldInt ) {
    const int oldBarcode = (barcodeAcc.isAvailable (*oldPart)) ? barcodeAcc(*oldPart) : HepMC::INVALID_PARTICLE_ID;
    xAOD::TruthParticle * newPart = newInt.at(index);
    // convert "old" status + barcode to "new" status values
    newPart->setStatus(HepMC::new_particle_status_from_old(oldPart->status(), oldBarcode));
    // The old barcode is still a unique identifier
    newPart->setUid(oldBarcode);
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
void xAODTruthParticleAuxContainerCnv_v1::transToPers(
    const xAOD::TruthParticleAuxContainer*, xAOD::TruthParticleAuxContainer_v1*,
    MsgStream& log) const {

  static const char* const ERRORMSG =
      "Somebody called xAODTruthParticleAuxContainerCnv_v1::transToPers";
  log << MSG::ERROR << ERRORMSG << endmsg;
  throw std::runtime_error(ERRORMSG);
  return;
}
