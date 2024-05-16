/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IG4CaloTransportTool_H
#define IG4CaloTransportTool_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
/* Input to particle transport will be a G4Track*/
#include "G4Track.hh"
/* Transport steps will be returned as G4FieldTracks*/
#include "G4FieldTrack.hh"

static const InterfaceID IID_IG4CaloTransportTool("IG4CaloTransportTool", 1, 0);

class IG4CaloTransportTool : virtual public IAlgTool
{
 public:
    /** AlgTool interface methods */
    static const InterfaceID& interfaceID() { return IID_IG4CaloTransportTool; }

    virtual std::vector<G4FieldTrack> transport(const G4Track& G4InputTrack) = 0;
    virtual StatusCode initializePropagator() = 0;
};

#endif // IG4CaloTransportTool_H