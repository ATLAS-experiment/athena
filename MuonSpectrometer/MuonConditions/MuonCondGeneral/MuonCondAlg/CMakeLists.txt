# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondAlg )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( nlohmann_json )
                   
atlas_add_component( MuonCondAlg
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel AthenaPoolUtilities 
                                  Identifier GaudiKernel MuonCondData  MuonCondSvcLib 
                                  CoralUtilitiesLib StoreGateLib MuonIdHelpersLib MuonReadoutGeometry 
                                  MdtCalibInterfacesLib MdtCalibData MuonCalibITools MdtCalibUtils MuonCalibToolsLib 
                                  PathResolver MuonNSWCommonDecode MdtCalibInterfacesLib 
                                  nlohmann_json::nlohmann_json MuonReadoutGeometryR4
                                 ${ROOT_LIBRARIES} EventInfo )


# Install files from the package:
atlas_install_data( data/* )
