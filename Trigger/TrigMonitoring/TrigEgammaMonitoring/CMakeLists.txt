# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEgammaMonitoring )

# External dependencies:
find_package( Boost )

# Component(s) in the package:
atlas_add_component( TrigEgammaMonitoring
                     src/*.h
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} AthenaBaseComps AthenaMonitoringKernelLib AthenaMonitoringLib
                     EgammaAnalysisInterfacesLib GaudiKernel PATCoreLib StoreGateLib 
                     TrigDecisionToolLib TrigAnalysisHelpersLib TrigEgammaMatchingToolLib
                     TrigEgammaEmulationToolLib xAODBase xAODEgamma xAODEventInfo xAODJet
                     xAODMissingET xAODTracking xAODTrigCalo xAODTrigEgamma xAODTrigger 
                     xAODTruth egammaRecEvent LArRecEvent)

# Suppress spurious warnings seen in the LTO build originating 
# in the boost parser code.
set_target_properties( TrigEgammaMonitoring
                       PROPERTIES LINK_FLAGS " -Wno-maybe-uninitialized "  )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
