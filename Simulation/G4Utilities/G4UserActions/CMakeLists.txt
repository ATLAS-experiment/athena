################################################################################
# Package: G4UserActions
################################################################################

# Declare the package name:
atlas_subdir( G4UserActions )

# External dependencies:
find_package( AdePT QUIET )
find_package( CLHEP )
find_package( Celeritas QUIET )
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( TBB )

# If Celeritas is found, add its code
if( Celeritas_FOUND )
  message( STATUS "Building G4UserActions with Celeritas support" )
  set( Celeritas_SOURCES src/Celeritas/*.cxx )
endif()

# Component(s) in the package:
atlas_add_library( G4UserActions
                   src/*.cxx ${Celeritas_SOURCES}
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} ${TBB_LIBRARIES} AthenaBaseComps AthenaKernel StoreGateLib GaudiKernel G4AtlasInterfaces G4AtlasToolsLib EventInfo InDetSimEvent MuonSimEvent MCTruth MCTruthBaseLib SimHelpers TrackWriteFastSimLib PathResolver G4DebuggingHelperLib TruthUtils)
set_target_properties( G4UserActions PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Celeritas on CPU or GPU
if( Celeritas_FOUND )
  target_compile_definitions( G4UserActions PRIVATE ATHSIMULATION_USE_CELER )
  cuda_rdc_target_link_libraries( G4UserActions PRIVATE Celeritas::accel )
endif()

# only needed to have ATHSIMULATION_USE_ADEPT available
if( AdePT_FOUND )
  target_compile_definitions( G4UserActions PRIVATE ATHSIMULATION_USE_ADEPT )
endif()


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( share/*.dat )
