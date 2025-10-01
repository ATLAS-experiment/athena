# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1ZDC )

# External packages:
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_component( TrigT1ZDC
                      src/*.cxx
                      src/components/*.cxx
                      LINK_LIBRARIES nlohmann_json::nlohmann_json AthenaBaseComps GaudiKernel AthContainers StoreGateLib xAODForward TrigT1Interfaces PathResolver ZdcByteStreamLib ZdcConditions ZdcIdentifier ZdcUtilsLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
