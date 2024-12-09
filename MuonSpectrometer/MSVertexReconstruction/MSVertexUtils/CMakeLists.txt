################################################################################
# Package: MSVertexUtils
################################################################################

# Declare the package name:
atlas_subdir( MSVertexUtils )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore )

# tag ROOTBasicLibs was not recognized in automatic conversion in cmt2cmake

# tag ROOTSTLDictLibs was not recognized in automatic conversion in cmt2cmake

# Component(s) in the package:
atlas_add_library( MSVertexUtils
                   src/*.cxx
                   PUBLIC_HEADERS MSVertexUtils
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} GeoPrimitives EventPrimitives xAODTracking GaudiKernel MuonIdHelpersLib MuonPrepRawData
                   PRIVATE_LINK_LIBRARIES )

