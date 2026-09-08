################################################################################
# Package: G4DebuggingTools
################################################################################

# Declare the package name:
atlas_subdir( G4DebuggingTools )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( G4DebuggingHelperLib
                   src/helper/G4DebuggingHelper.cxx
                   OBJECT
                   PUBLIC_HEADERS G4DebuggingTools
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} )
set_target_properties( G4DebuggingHelperLib PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_add_library( G4DebuggingTools
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES}
                                          GaudiKernel AthenaBaseComps StoreGateLib xAODEventInfo G4AtlasInterfaces GeoPrimitives
                                          G4AtlasToolsLib MCTruth MCTruthBaseLib SimHelpers G4DebuggingHelperLib )
set_target_properties( G4DebuggingTools PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
