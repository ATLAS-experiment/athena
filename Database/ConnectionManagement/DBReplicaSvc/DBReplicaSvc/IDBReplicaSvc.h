/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DBREPLICASVC_IDBREPLICASVC_H
#define DBREPLICASVC_IDBREPLICASVC_H
// IDBReplicaSvc.h - abstract interface to service inplementing 
// CORAL IReplicaSortingAlgorithm
// Richard Hawkimgs, started 24/4/07

#include "GaudiKernel/IInterface.h"
#include "RelationalAccess/IReplicaSortingAlgorithm.h"

class IDBReplicaSvc : virtual public IInterface, 
                      virtual public coral::IReplicaSortingAlgorithm {
 public:
  DeclareInterfaceID(IDBReplicaSvc, 1, 0);
};

#endif // DBREPLICASVC_IDBREPLICASVC_H
