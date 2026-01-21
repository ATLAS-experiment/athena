################################################################################
# Package: G4RunManagement
################################################################################

# Declare the package name:
atlas_subdir( G4RunManagement )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( G4RunManagement
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   PUBLIC_HEADERS G4RunManagement
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} AthenaBaseComps AthenaKernel StoreGateLib GaudiKernel G4AtlasInterfaces G4AtlasToolsLib MCTruth MCTruthBaseLib SimHelpers TruthUtils)
set_target_properties( G4RunManagement PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
