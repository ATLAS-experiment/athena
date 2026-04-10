/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEF_MERGEPACKS
#define TRIGSTORAGEDEF_MERGEPACKS

#include "TrigStorageDefinitions/EDM_TypeInformation.h"
#include "TrigStorageDefinitions/TrigInDetEvent.h"
#include "TrigStorageDefinitions/TrigSteeringEvent.h"
#include "TrigStorageDefinitions/TrigMuonEvent.h"
#include "TrigStorageDefinitions/TrigMissingEtEvent.h"
#include "TrigStorageDefinitions/TrigParticle.h"
#include "TrigStorageDefinitions/TrigTopoEvent.h"
#include "TrigStorageDefinitions/TrigCaloEvent.h"
#include "TrigStorageDefinitions/TrigCombinedEvent.h"
#include "TrigStorageDefinitions/TrigMonitoringEvent.h"
#include "TrigStorageDefinitions/TrigBphysicsEvent.h"


TYPEMAPCLASS(TrigBphysicsEvent)
TYPEMAPCLASS(TrigMonitoringEvent)
TYPEMAPCLASS(TrigTopoEvent)
TYPEMAPCLASS(TrigCombinedEvent)
TYPEMAPCLASS(TrigCaloEvent)
TYPEMAPCLASS(TrigParticle)
TYPEMAPCLASS(TrigMissingEtEvent)
TYPEMAPCLASS(TrigMuonEvent) 
TYPEMAPCLASS(TrigInDetEvent)
TYPEMAPCLASS(TrigSteeringEvent)

   
struct TypeInfo_EDM {
  using map = HLT::TypeInformation::List<>
    ::join<class_TrigBphysicsEvent::map>
    ::join<class_TrigMonitoringEvent::map>
    ::join<class_TrigCombinedEvent::map>
    ::join<class_TrigCaloEvent::map>
    ::join<class_TrigTopoEvent::map>
    ::join<class_TrigParticle::map>
    ::join<class_TrigMissingEtEvent::map>
    ::join<class_TrigMuonEvent::map>
    ::join<class_TrigInDetEvent::map>
    ::join<class_TrigSteeringEvent::map>;
};

#endif
