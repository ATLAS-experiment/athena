################################################################################
# Package: LArG4Barrel
################################################################################

# Declare the package name:
atlas_subdir( LArG4Barrel )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( LArG4Barrel
                   src/*.cxx src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} LArG4Code CaloG4SimLib StoreGateLib
                   PRIVATE_LINK_LIBRARIES AthenaKernel GaudiKernel LArG4RunControl LArHV PathResolver CxxUtils )
set_target_properties( LArG4Barrel PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_install_python_modules( python/*.py )
