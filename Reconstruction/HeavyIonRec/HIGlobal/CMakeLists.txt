# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HIGlobal )

# Component(s) in the package:
atlas_add_library( HIGlobalLib
                   HIGlobal/*.h
                   INTERFACE
                   PUBLIC_HEADERS HIGlobal
                   LINK_LIBRARIES AsgTools AthenaBaseComps CaloEvent GaudiKernel HIEventUtilsLib NavFourMom StoreGateLib xAODCaloEvent xAODHIEvent )

atlas_add_component( HIGlobal
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES CaloEvent GaudiKernel HIEventUtilsLib HIGlobalLib Navigation StoreGateLib xAODHIEvent TrkSpacePoint xAODCore xAODTracking  xAODTrigMinBias InDetPrepRawData TileEvent)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( HIGlobal
   SCRIPT python -m HIGlobal.HIGlobalConfig
   PROPERTIES TIMEOUT 1800
   POST_EXEC_SCRIPT nopost.sh )
