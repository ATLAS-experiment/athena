/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTEERINGEVENT_TRIGROIDESCRIPTORCOLLECTION_H
#define TRIGSTEERINGEVENT_TRIGROIDESCRIPTORCOLLECTION_H

#include "TrigRoiDescriptor.h"

#include "AthContainers/DataVector.h"
#include "xAODCore/CLASS_DEF.h"
#include "xAODCore/BaseInfo.h"

class TrigRoiDescriptorCollection : public DataVector<TrigRoiDescriptor> {
 public:
  TrigRoiDescriptorCollection() {};

  TrigRoiDescriptorCollection(SG::OwnershipPolicy pl) 
    : DataVector<TrigRoiDescriptor>(pl){};
};

CLASS_DEF( TrigRoiDescriptorCollection , 1097199488 , 1 )

SG_BASE(TrigRoiDescriptorCollection, DataVector<TrigRoiDescriptor>);

#endif
