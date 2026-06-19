################################################################################
# Package: ISF_Geant4Event
################################################################################

# Declare the package name:
atlas_subdir( ISF_Geant4Event )

# External dependencies:
find_package( CLHEP )
find_package( Eigen )
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( ISF_Geant4Event
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS ISF_Geant4Event
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   ${CLHEP_INCLUDE_DIRS}
                    ${GEANT4_INCLUDE_DIRS}
                   ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES}
                   ${CLHEP_LIBRARIES} AtlasHepMCLib
                   ${GEANT4_LIBRARIES} ${EIGEN_LIBRARIES} ISF_Event
                   ISF_InterfacesLib SimHelpers GeoPrimitives MCTruth TruthUtils )
set_target_properties( ISF_Geant4Event PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

#atlas_add_test(SOURCES
#               INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
#               ${EIGEN_INCLUDE_DIRS}
#               LINK_LIBRARIES
#              ${CLHEP_LIBRARIES} AtlasHepMCLib
#               ${EIGEN_LIBRARIES} ISF_Event SimHelpers GeoPrimitives
#               BarcodeEventLib MCTruth)
