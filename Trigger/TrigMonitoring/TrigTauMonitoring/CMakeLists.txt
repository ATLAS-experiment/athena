# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTauMonitoring )

atlas_add_library( TrigTauMonitoringLib
                   TrigTauMonitoring/*.h Root/*.cxx
                   PUBLIC_HEADERS TrigTauMonitoring
)

atlas_add_dictionary( TrigTauMonitoringDict
                      TrigTauMonitoring/TrigTauMonitoringDict.h TrigTauMonitoring/selection.xml
                      LINK_LIBRARIES TrigTauMonitoringLib )

# Component(s) in the package:
atlas_add_component( TrigTauMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthContainers AthenaMonitoringKernelLib AthenaMonitoringLib CxxUtils GaudiKernel LArRecEvent StoreGateLib TrigCompositeUtilsLib TrigDecisionToolLib TrigTauMonitoringLib TruthUtils xAODEgamma xAODMuon xAODTau xAODTrigger xAODTruth )

# Suppress spurious warnings seen in the LTO build originating 
# in the boost parser code.
set_target_properties( TrigTauMonitoring
                       PROPERTIES LINK_FLAGS " -Wno-maybe-uninitialized -Wno-uninitialized "  )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
