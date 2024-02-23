# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HIJetRec )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist Physics )

# Component(s) in the package:
atlas_add_library( HIJetRecLib
                   HIJetRec/*.h
                   INTERFACE
                   PUBLIC_HEADERS HIJetRec
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools FourMomUtils HIEventUtilsLib xAODBase xAODCaloEvent xAODHIEvent xAODJet )

atlas_add_component( HIJetRec
                     Root/*.cxx src/*.h src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthenaBaseComps CaloEvent CaloInterfaceLib CaloUtilsLib CxxUtils FourMomUtils HIEventUtilsLib HIJetRecLib JetInterface JetRecLib NavFourMom Navigation PathResolver StoreGateLib xAODBase xAODCaloEvent xAODCore xAODEventInfo xAODHIEvent xAODJet xAODTracking )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( HIJet
             SCRIPT  python -m HIJetRec.HIJetRecConfigCA
             PROPERTIES TIMEOUT 1800
             POST_EXEC_SCRIPT nopost.sh)

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )  # too slow for dbg build
   set_tests_properties( HIJetRec_HIJet_ctest PROPERTIES DISABLED True )
endif()

atlas_add_test( HIegamma
             SCRIPT  python -m egammaConfig.egammaReconstructionConfig Reco.EnableHI=True
             POST_EXEC_SCRIPT noerror.sh)
