///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IxAODClusterCompressor.h 
// Header file for class IxAODClusterCompressor
// Author: Walter Lampl
/////////////////////////////////////////////////////////////////// 
#ifndef CALOINTERFACES_IXAODCLUSTERCOMPRESSOR_H
#define CALOINTERFACES_IXAODCLUSTERCOMPRESSOR_H 1

// FrameWork includes
#include "GaudiKernel/IService.h"

#include "xAODCaloEvent/CaloClusterContainer.h"


class IxAODClusterCompressor
  : virtual public ::IService
{
 public: 
  DeclareInterfaceID(IxAODClusterCompressor, 1, 0);

  /** Destructor: 
   */
  virtual ~IxAODClusterCompressor() {};

  virtual void compress(xAOD::CaloClusterContainer* clustercontainer) const=0;
}; 

#endif //> !CALOINTERFACES_IXAODCLUSTERCOMPRESSOR_H
