// Copyright (C) 2002-2026 by CERN for the benefit of the ATLAS collaboration

// For legacy EDM classes the registration with the Navigation was done
// directly in the legacy Trig[ABC]Event package. But once we remove one of
// those packages we need to still register the type map with the Navigation.
//
// Note that the "package" name is really just a name for the type map and we
// used the same name also for the Run-3 EDM registration.
//
// See TrigStorageDefinitions/EDM_MergedPackages.h for the names.

#include "TrigStorageDefinitions/EDM_TypeInformation.h"
#include "TrigNavigation/TypeRegistration.h"

REGISTER_PACKAGE_WITH_NAVI(TrigBphysicsEvent)
REGISTER_PACKAGE_WITH_NAVI(TrigCombinedEvent)
REGISTER_PACKAGE_WITH_NAVI(TrigMissingEtEvent)
