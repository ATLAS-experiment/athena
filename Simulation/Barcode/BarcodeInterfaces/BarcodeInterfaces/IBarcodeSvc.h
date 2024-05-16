/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BARCODEINTERFACES_IBARCODESVC_H
#define BARCODEINTERFACES_IBARCODESVC_H 1

// Barcode includes
#include "TruthUtils/MagicNumbers.h"

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

namespace Barcode {

  /** @class IBarcodeSvc

      @author Andreas.Salzburger -at- cern.ch , Elmar.Ritsch -at- cern.ch
  */
  class IBarcodeSvc : virtual public IInterface {

  public:

    //TODO not fully implemented in Generic, Global, and Validation BarcodeSvcs
    //only fully implemented in LegacyBarcodeSvc (only barcode service used in production)
    virtual StatusCode initializeBarcodes(int, int) { return StatusCode::SUCCESS; };
    virtual StatusCode resetBarcodes(int, int) { return StatusCode::SUCCESS; };

    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IBarcodeSvc, 1 , 0);


    /** Generate a new unique barcode for a secondary particle above the simulation offset */
    virtual int newSecondaryParticle(int parentBC) = 0;

    /** Generate a new unique particle barcode below the simulation offset (for particles from pre-defined decays) */
    virtual int newGeneratedParticle(int parentBC) = 0;

    /** Generate a new unique vertex barcode above the simulation offset */
    virtual int newSimulationVertex() = 0;

    /** Generate a new unique vertex barcode below the simulation offset */
    virtual int newGeneratedVertex() = 0;

    /** Inform the BarcodeSvc about the largest particle and vertex Barcodes
        in the event input */
    virtual void registerLargestGeneratedParticleBC( int bc) = 0;
    virtual void registerLargestGeneratedVtxBC( int bc) = 0;
    virtual void registerLargestSecondaryParticleBC( int bc) = 0;
    virtual void registerLargestSimulationVtxBC( int bc) = 0;

    /** Return the secondary particle and vertex offsets */
    virtual int secondaryParticleBcOffset() const = 0;
    virtual int   secondaryVertexBcOffset()  const = 0;
  };

}

#endif //> !BARCODEINTERFACES_IBARCODESVC_H
