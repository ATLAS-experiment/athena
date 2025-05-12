################################################################################
# Package: MuonStationIndex
################################################################################

# Declare the package name:
atlas_subdir( MuonStationIndex )

# This package doesn't depend on anything.

# The main library of the package:
atlas_add_library( MuonStationIndexLib
   MuonStationIndex/*.h Root/*.cxx
   PUBLIC_HEADERS MuonStationIndex )

# we also add unit tests. 
# Build them from the files in test/ 
file(GLOB_RECURSE tests "test/*.cxx")

foreach(_theTestSource ${tests})
   get_filename_component(_theTest ${_theTestSource} NAME_WE)
   atlas_add_test( ${_theTest} SOURCES ${_theTestSource}
                    LINK_LIBRARIES MuonStationIndexLib 
                    POST_EXEC_SCRIPT nopost.sh)
endforeach()

