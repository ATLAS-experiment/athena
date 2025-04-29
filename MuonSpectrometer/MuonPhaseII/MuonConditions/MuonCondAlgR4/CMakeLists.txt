################################################################################
# Package: MuonCondAlgR4
################################################################################

# Declare the package name:
atlas_subdir( MuonCondAlgR4 )

# External dependencies:
find_package( nlohmann_json )
find_package( CORAL )

find_package(ROOT COMPONENTS Gpad)
# Component(s) in the package:
atlas_add_library( MuonCondAlgR4Lib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondAlgR4
                   INCLUDE_DIRS ${CORAL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS 
                   LINK_LIBRARIES  ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel Identifier GaudiKernel  
                                   StoreGateLib MuonIdHelpersLib MuonAlignmentDataR4 PathResolver CoralUtilitiesLib MuonVisualizationHelpersR4
                                   nlohmann_json::nlohmann_json MuonReadoutGeometryR4 MdtCalibData MuonCalibITools AthenaPoolUtilities
                   PRIVATE_LINK_LIBRARIES EventInfo )

atlas_add_component( MuonCondAlgR4
                     src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel MuonCondAlgR4Lib MdtCalibData)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
