################################################################################
# Package: Geneva_i
################################################################################

# Declare the package name:
atlas_subdir( Geneva_i )

# External dependencies:
find_package( CLHEP )
find_package( Lhapdf )

find_package( Geneva )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py share/common/*.py )

atlas_add_component( Geneva_i    
   src/components/*.cxx
)

# Set Geneva specific environment variable(s).
set( GenevaEnvironment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   CACHE PATH "Location of GenevaEnvironment.cmake" )
find_package( GenevaEnvironment )
