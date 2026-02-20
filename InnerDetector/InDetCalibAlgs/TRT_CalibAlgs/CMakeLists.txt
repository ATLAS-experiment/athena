# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TRT_CalibAlgs )

# External dependencies:
find_package( ROOT COMPONENTS Graf Core Tree MathCore Hist HistPainter RIO MathMore Physics
   Matrix Gpad )

# Libraries in the package:
atlas_add_library( TRT_CalibAlgsLib
    src/*.h src/*.cxx
    PUBLIC_HEADERS TRT_CalibAlgs
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel CxxUtils StoreGateLib xAODEventInfo GaudiKernel TrkToolInterfaces 
        CommissionEvent AthContainers Identifier xAODTracking TRT_ConditionsData TRT_ConditionsServicesLib InDetIdentifier 
        InDetRawData InDetPrepRawData InDetRIO_OnTrack TrkParameters TrkTrack VxVertex TrkFitterInterfaces TRT_TrackHoleSearchLib 
        TRT_ElectronPidToolsLib TRT_CalibData TRT_CalibToolsLib
    PRIVATE_LINK_LIBRARIES AthenaPoolUtilities)

# Component(s) in the package:
atlas_add_component( TRT_CalibAlgs
    src/components/*.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils TRT_CalibAlgsLib)

atlas_add_executable( TRTCalib_bhadd 
    scripts/TRTCalib_bhadd.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils)

atlas_add_executable( TRTCalib_makeplots 
    scripts/TRTCalib_makeplots.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils)

atlas_add_executable( TRTCalib_StrawStatus_merge 
    scripts/TRTCalib_StrawStatus_merge.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils)

atlas_add_executable( TRTCalib_StrawStatusReport
    scripts/TRTCalib_StrawStatusReport.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver CxxUtils)

atlas_add_executable( TRTCalib_StrawStatusPlots
    scripts/TRTCalib_StrawStatusPlots.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver CxxUtils)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.sh )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/*.txt scripts/*.C)
