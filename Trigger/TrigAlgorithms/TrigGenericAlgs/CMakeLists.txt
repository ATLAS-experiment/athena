# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#External dependencies:
find_package( tdaq-common COMPONENTS CTPfragment)
find_package( CLHEP )

# Declare the package name:
atlas_subdir( TrigGenericAlgs )

# Component(s) in the package:
atlas_add_component( TrigGenericAlgs
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib ByteStreamCnvSvcBaseLib DecisionHandlingLib GaudiKernel LumiBlockCompsLib StoreGateLib TrigCompositeUtilsLib TrigConfData TrigSteeringEvent xAODEventShape xAODTracking xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )
