# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigMissingEtEvent )

# Component(s) in the package:
atlas_add_library( TrigMissingEtEvent
                   src/*.cxx
                   src/components/*.cxx
                   PUBLIC_HEADERS TrigMissingEtEvent
                   PRIVATE_LINK_LIBRARIES TrigNavigationLib
                   LINK_LIBRARIES AthContainers AthenaKernel GaudiKernel TrigSerializeCnvSvcLib CxxUtils )

atlas_generate_cliddb( TrigMissingEtEvent )

atlas_add_sercnv_library( TrigMissingEtEventSerCnv
                          FILES TrigMissingEtEvent/TrigMissingET.h TrigMissingEtEvent/TrigMissingETContainer.h TrigMissingEtEvent/TrigMissingEtComponent.h
                          LINK_LIBRARIES TrigMissingEtEvent )

atlas_add_dictionary( TrigMissingEtEventDict
                      TrigMissingEtEvent/TrigMissingEtEventDict.h
                      TrigMissingEtEvent/selection.xml
                      LINK_LIBRARIES TrigMissingEtEvent
                      DATA_LINKS TrigMissingET )
