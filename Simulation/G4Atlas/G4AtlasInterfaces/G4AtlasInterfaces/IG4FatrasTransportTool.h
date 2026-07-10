/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IG4FatrasTransportTool_H
#define IG4FatrasTransportTool_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
/* Input to particle transport will be a G4Track*/
#include "G4Track.hh"
/* Transport steps will be returned as G4FieldTracks*/
#include "G4FieldTrack.hh"
/* Volume look-up uses a position and returns a physical volume*/
#include "G4ThreeVector.hh"

class G4VPhysicalVolume;

static const InterfaceID IID_IG4FatrasTransportTool("IG4FatrasTransportTool", 1, 0);

class IG4FatrasTransportTool : virtual public IAlgTool
{
 public:
    /** AlgTool interface methods */
    static const InterfaceID& interfaceID() { return IID_IG4FatrasTransportTool; }

    virtual std::vector<G4FieldTrack> transport(const G4Track& G4InputTrack) = 0;
    virtual StatusCode initializePropagator() = 0;
};

#endif // IG4FatrasTransportTool_H