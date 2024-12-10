# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigHypoCommonTools )

# External reqs
find_package( ROOT COMPONENTS Core GenVector )

# Component(s) in the package:
atlas_add_component( TrigHypoCommonTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthLinks AthenaBaseComps AthenaMonitoringKernelLib DecisionHandlingLib FourMomUtils GaudiKernel HLTSeedingLib StoreGateLib TrigCompositeUtilsLib TrigSteeringEvent TrigT1Result xAODBase xAODTrigMissingET )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )
