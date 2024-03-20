# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondAlg )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( nlohmann_json )
find_package( Boost COMPONENTS regex )

# Component(s) in the package:
atlas_add_library( MuonCondAlgLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondAlg
                   INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel AthenaPoolUtilities 
                                  Identifier GaudiKernel MuonCondData MuonCondInterface MuonCondSvcLib 
                                  CoralUtilitiesLib StoreGateLib MuonIdHelpersLib MuonReadoutGeometry 
                                  MdtCalibSvcLib MdtCalibData MuonCalibITools MdtCalibUtils MuonCalibToolsLib 
                                  PathResolver MuonNSWCommonDecode MdtCalibInterfacesLib 
                                  nlohmann_json::nlohmann_json MuonReadoutGeometryR4
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} EventInfo )

atlas_add_component( MuonCondAlg
                     src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel MuonCondAlgLib nlohmann_json::nlohmann_json )

# Install files from the package:
atlas_install_data( data/* )
