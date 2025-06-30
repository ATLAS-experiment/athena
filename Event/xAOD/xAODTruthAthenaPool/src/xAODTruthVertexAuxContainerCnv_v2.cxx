//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "xAODTruthVertexAuxContainerCnv_v2.h"

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

namespace HepMC {
  constexpr int INVALID_VERTEX_ID = 1;
}

void xAODTruthVertexAuxContainerCnv_v2::persToTrans(
    const xAOD::TruthVertexAuxContainer_v2* futureObj,
    xAOD::TruthVertexAuxContainer* newObj, MsgStream&) const {

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
  xAOD::TruthVertexContainer_v1 futureInt;
  for( size_t i = 0; i < futureObj->size(); ++i ) {
    futureInt.push_back( new xAOD::TruthVertex_v1() );
  }
  futureInt.setStore( futureObj );

  xAOD::TruthVertexContainer newInt;
  for( size_t i = 0; i < newObj->size(); ++i ) {
    newInt.push_back( new xAOD::TruthVertex() );
  }
  newInt.setStore( newObj );

  unsigned int index{0};
  static const SG::AuxElement::Accessor<int> statusAcc ("status"); // was idAcc
  static const SG::AuxElement::Accessor<int> uidAcc ("uid"); // was barcodeAcc
  int minGenUID{0};
  // Loop to find the last Generator particle to try to fix the
  // barcodes for simulated vertices (will not be perfect in the case
  // of truth-thinning)
  for( const xAOD::TruthVertex_v1* futureVtx : futureInt ) {
    const int futureStatus = (statusAcc.isAvailable (*futureVtx)) ? statusAcc(*futureVtx) : 1000; // FIXME need to set an appropriate default
    if (futureStatus > HepMC::SIM_STATUS_THRESHOLD) {continue; }
    const int futureUniqueID = (uidAcc.isAvailable (*futureVtx)) ? uidAcc(*futureVtx) : HepMC::INVALID_VERTEX_ID;
    minGenUID = std::min(minGenUID, futureUniqueID);
  }

  // Loop over the interface objects, and do the conversion with their help:
  for( const xAOD::TruthVertex_v1* futureVtx : futureInt ) {
    const int futureStatus = (statusAcc.isAvailable (*futureVtx)) ? statusAcc(*futureVtx) : 1000; // FIXME need to set an appropriate default
    const int futureUniqueID = (uidAcc.isAvailable (*futureVtx)) ? uidAcc(*futureVtx) : HepMC::INVALID_VERTEX_ID;
    xAOD::TruthVertex * newVtx = newInt.at(index);
    // convert "future" id + barcode to "new" status values
    newVtx->setId(HepMC::old_vertex_status_from_new(futureStatus));
    const int newBarcode = ((futureStatus > HepMC::SIM_STATUS_THRESHOLD) ? -HepMC::SIM_BARCODE_THRESHOLD-minGenUID : 0) + futureUniqueID;
    newVtx->setBarcode(newBarcode);
    ++index;
  }
  return;
}

/// This function should never be called, as we are not supposed to convert
/// objects before writing.
///
void xAODTruthVertexAuxContainerCnv_v2::transToPers(
    const xAOD::TruthVertexAuxContainer*, xAOD::TruthVertexAuxContainer_v2*,
    MsgStream& log) const {

  static const char* const ERRORMSG =
      "Somebody called xAODTruthVertexAuxContainerCnv_v2::transToPers";
  log << MSG::ERROR << ERRORMSG << endmsg;
  throw std::runtime_error(ERRORMSG);
  return;
}
