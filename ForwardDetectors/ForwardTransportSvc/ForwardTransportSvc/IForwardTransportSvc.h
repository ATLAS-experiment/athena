/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FORWARDTRANSPORTSVC_IFORWARDTRANSPORTSVC_H
#define FORWARDTRANSPORTSVC_IFORWARDTRANSPORTSVC_H

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"

#include "ForwardTracker/ConfigData.h"

#include <string>

#include "G4ThreeVector.hh"

class IForwardTransportSvc: virtual public IInterface {

 public:

    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IForwardTransportSvc, 6 , 0);

    virtual ForwardTracker::ConfigData getConfigData() const = 0;
    virtual bool getTransportFlag() const = 0;
    virtual double getEtaCut() const = 0;
    virtual double getXiCut() const = 0;
    virtual bool selectedParticle(G4ThreeVector mom, int pid) = 0;
};

#endif // FORWARDTRANSPORTSVC_IFORWARDTRANSPORTSVC_H
