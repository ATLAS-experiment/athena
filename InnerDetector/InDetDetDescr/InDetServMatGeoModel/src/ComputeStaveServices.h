/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSERVMATGEOMODEL_COMPUTESTAVESERVICES_H
#define INDETSERVMATGEOMODEL_COMPUTESTAVESERVICES_H

#include "StaveServices.h"
#include "GaudiKernel/MsgStream.h"

class ComputeStaveServices {
public:

  ComputeStaveServices() {}

  StaveServices compute( DetType::Type, DetType::Part, int layerNumber, int nModulesPerStave, int nChipsPerModule,
                         MsgStream& msg) const;
  int computeLVGaugeSerial( DetType::Type, DetType::Part, int layerNumber, 
			    int nModules, double moduleCurrent, double moduleVoltage,
			    double poweringLoss, double lossInCable, double cableLen,
                            MsgStream& msg) const;
};

#endif
