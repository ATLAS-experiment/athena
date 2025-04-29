################################################################################
# Package: MdtCalibData
################################################################################

# Declare the package name:
atlas_subdir( MdtCalibData )

# External dependencies:
find_package( ROOT COMPONENTS Hist Core Graf Gpad )

# Component(s) in the package:
atlas_add_library( MdtCalibData
                   src/*.cxx
                   PUBLIC_HEADERS MdtCalibData
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthContainers EventPrimitives MuonCalibMath GeoModelUtilities MuonIdHelpersLib
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaKernel GaudiKernel )


file(GLOB_RECURSE tests "test/*.cxx")

foreach(_theTestSource ${tests})
    get_filename_component(_theTest ${_theTestSource} NAME_WE)
    atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                   LINK_LIBRARIES  ${ROOT_LIBRARIES} MuonCalibMath MdtCalibData
                   PROPERTIES TIMEOUT 900
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT nopost.sh)
endforeach()
