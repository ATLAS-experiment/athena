################################################################################
# Package: G4RunManagement
################################################################################

# Declare the package name:
atlas_subdir( G4RunManagement )

# External dependencies:
find_package( Geant4 )
find_package( Threads )

# Component(s) in the package:
atlas_add_library( G4RunManagement
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   PUBLIC_HEADERS G4RunManagement
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} AthenaBaseComps AthenaKernel StoreGateLib GaudiKernel G4AtlasInterfaces G4AtlasToolsLib MCTruth MCTruthBaseLib SimHelpers TruthUtils)
set_target_properties( G4RunManagement PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Tests:
atlas_add_test( G4RunToolStateSynchronization_test
                SOURCES test/G4RunToolStateSynchronization_test.cxx
                        src/G4RunToolStateSynchronization.cxx
                INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
                LINK_LIBRARIES Threads::Threads
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 10 )

atlas_add_test( G4RunToolEventQueue_test
                SOURCES test/G4RunToolEventQueue_test.cxx
                        src/G4RunToolEventQueue.cxx
                        src/AtlasG4SyncEventUserInfo.cxx
                INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                             ${CMAKE_CURRENT_SOURCE_DIR}/src
                LINK_LIBRARIES ${GEANT4_LIBRARIES} Threads::Threads
                               AthenaKernel GaudiKernel HitManagement MCTruth
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 10 )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
