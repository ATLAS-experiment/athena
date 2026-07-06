################################################################################
# Package: LArG4SD
################################################################################

# Declare the package name:
atlas_subdir( LArG4SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( LArG4SD
                   src/*.cc
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} CaloG4SimLib GaudiKernel LArG4Code )
set_target_properties( LArG4SD PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_add_test( LArG4SDToolConfig_test
                SCRIPT test/LArG4SDToolConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py )
