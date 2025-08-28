//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "xAODTruthVertexAuxContainerCnv_v1.h"

// Gaudi/Athena include(s).
#include "AthContainers/tools/copyAuxStoreThinned.h"
#include "GaudiKernel/MsgStream.h"
#include "TruthUtils/MagicNumbers.h"

// EDM include(s):
#include "xAODTruth/versions/TruthVertexContainer_v1.h"
#include "xAODTruth/TruthVertexContainer.h"

// System include(s):
#include <cassert>
#include <stdexcept>

void xAODTruthVertexAuxContainerCnv_v1::persToTrans(
    const xAOD::TruthVertexAuxContainer_v1* oldObj,
    xAOD::TruthVertexAuxContainer* newObj, MsgStream&) const {

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
  xAOD::TruthVertexContainer_v1 oldInt;
  for( size_t i = 0; i < oldObj->size(); ++i ) {
    oldInt.push_back( new xAOD::TruthVertex_v1() );
  }
  oldInt.setStore( oldObj );

  xAOD::TruthVertexContainer newInt;
  for( size_t i = 0; i < newObj->size(); ++i ) {
    newInt.push_back( new xAOD::TruthVertex() );
  }
  newInt.setStore( newObj );

  unsigned int index{0};
  static const SG::AuxElement::Accessor<int> idAcc ("id");
  static const SG::AuxElement::Accessor<int> barcodeAcc ("barcode");
  // Loop over the interface objects, and do the conversion with their help:
  for( const xAOD::TruthVertex_v1* oldVtx : oldInt ) {
    const int oldID = (idAcc.isAvailable (*oldVtx)) ? idAcc(*oldVtx) : 1000;
    const int oldBarcode = (barcodeAcc.isAvailable (*oldVtx)) ? barcodeAcc(*oldVtx) : HepMC::INVALID_VERTEX_ID;
    xAOD::TruthVertex * newVtx = newInt.at(index);
    // convert "old" id + barcode to "new" status values
    newVtx->setStatus(HepMC::new_vertex_status_from_old(oldID, oldBarcode));
    // The old barcode is still a unique identifier
    newVtx->setUid(oldBarcode);
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
void xAODTruthVertexAuxContainerCnv_v1::transToPers(
    const xAOD::TruthVertexAuxContainer*, xAOD::TruthVertexAuxContainer_v1*,
    MsgStream& log) const {

  static const char* const ERRORMSG =
      "Somebody called xAODTruthVertexAuxContainerCnv_v1::transToPers";
  log << MSG::ERROR << ERRORMSG << endmsg;
  throw std::runtime_error(ERRORMSG);
  return;
}
