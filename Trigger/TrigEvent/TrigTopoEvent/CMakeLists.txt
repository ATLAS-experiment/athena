# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTopoEvent )

# Component(s) in the package:
atlas_add_library( TrigTopoEvent
                   src/*.cxx
                   PUBLIC_HEADERS TrigTopoEvent
                   LINK_LIBRARIES AthContainers AthenaKernel GaudiKernel
                   PRIVATE_LINK_LIBRARIES TrigNavigationLib )

atlas_add_sercnv_library( TrigTopoEventSerCnv
                          FILES TrigTopoEvent/ElectronMuonTopoInfo.h TrigTopoEvent/ElectronMuonTopoInfoContainer.h
                          LINK_LIBRARIES TrigTopoEvent TrigSerializeCnvSvcLib )

atlas_add_dictionary( TrigTopoEventDict
                      TrigTopoEvent/TrigTopoEventDict.h
                      TrigTopoEvent/selection.xml
                      LINK_LIBRARIES TrigTopoEvent
                      DATA_LINKS ElectronMuonTopoInfo
                      ELEMENT_LINKS ElectronMuonTopoInfoContainer )

atlas_generate_cliddb( TrigTopoEvent )
