################################################################################
# Package: AmdcOracle
################################################################################

# Declare the package name:
atlas_subdir( AmdcOracle )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )

# Component(s) in the package:
atlas_add_library( AmdcOracleLib
                   src/*.cxx
                   PUBLIC_HEADERS AmdcOracle
                   PRIVATE_INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel AmdcDbLib
                   PRIVATE_LINK_LIBRARIES ${CORAL_LIBRARIES} )

atlas_add_component( AmdcOracle
                     src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps GaudiKernel AmdcDbLib AmdcOracleLib )


