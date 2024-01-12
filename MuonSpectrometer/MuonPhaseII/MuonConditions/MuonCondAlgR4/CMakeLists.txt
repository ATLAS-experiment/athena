################################################################################
# Package: MuonCondAlgR4
################################################################################

# Declare the package name:
atlas_subdir( MuonCondAlgR4 )

# External dependencies:
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( MuonCondAlgR4Lib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondAlgR4
                   INCLUDE_DIRS 
                   PRIVATE_INCLUDE_DIRS 
                   LINK_LIBRARIES  AthenaBaseComps AthenaKernel Identifier GaudiKernel  
                                   StoreGateLib MuonIdHelpersLib MuonAlignmentDataR4 PathResolver 
                                   MuonStationGeoHelpersLib nlohmann_json::nlohmann_json MuonReadoutGeometryR4
                   PRIVATE_LINK_LIBRARIES EventInfo )

atlas_add_component( MuonCondAlgR4
                     src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel MuonCondAlgR4Lib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
