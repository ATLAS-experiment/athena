/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STORAGEDEF_TRIGCOMBINEDEVENT
#define STORAGEDEF_TRIGCOMBINEDEVENT
#include "TrigStorageDefinitions/EDM_TypeInformation.h"

#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"

HLT_BEGIN_TYPE_REGISTRATION
  HLT_REGISTER_TYPE(xAOD::TrigComposite, xAOD::TrigCompositeContainer, xAOD::TrigCompositeContainer, xAOD::TrigCompositeAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigComposite, xAOD::TrigComposite, xAOD::TrigCompositeContainer, xAOD::TrigCompositeAuxContainer)

HLT_END_TYPE_REGISTRATION(TrigCombinedEvent) 

#endif
