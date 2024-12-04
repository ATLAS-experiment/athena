// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/ViewVectorBase.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/IConstAuxStore.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainersInterfaces/IAuxSetOption.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/PackedLink.h"
#include "AthContainers/OwnershipPolicy.h"
#include "AthContainers/PackedParameters.h"
#include "AthContainers/ThinningDecision.h"
#include "AthContainers/debug.h"
#include "AthLinks/DataLink.h"


namespace {
  struct DUMMY_INSTANTIATION_ATHCONTAINERS {
    DataLink<SG::IConstAuxStore> dummy1;
    std::pair<SG::auxid_set_t::const_iterator, bool> p2;
  };
}

// Work around cling error.
template class std::vector<std::pair<unsigned int, unsigned int> >;


