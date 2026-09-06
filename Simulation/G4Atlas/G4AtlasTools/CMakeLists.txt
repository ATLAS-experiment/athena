# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( G4AtlasTools )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( TBB )

# Possible extra dependencies:
# Should only apply in case of Athena builds below (and not e.g in AthSimulation)
set( extra_lib )
set( extra_incl )
# Gather all source files that match the pattern
file(GLOB ALL_SRC_LIB_FILES src/*.cxx src/*.cc)
file(GLOB ALL_SRC_COMPONENT_FILES src/components/*.cxx)

# define extra_incl, extra_lib for Athena builds (and not AthSimulation)
if( NOT SIMULATIONBASE )
  find_package( Acts COMPONENTS PluginJson Fatras ) # for ActsFatrasG4
  find_package( ROOT COMPONENTS Core MathCore Hist RIO pthread )
  find_package( lwtnn )

  # set extra_lib
  set( extra_incl ${ROOT_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} )
  set( extra_lib ${ROOT_LIBRARIES} ${LWTNN_LIBRARIES}
       PathResolver ISF_InterfacesLib
       Acts::Core Acts::Fatras Acts::PluginJson ActsEventLib ActsInteropLib ActsGeometryLib # for ActsFatrasG4 
       GeoModelUtilities TrkTrack InDetSimEvent MCTruth # for ActsFatrasG4
       GeneratorObjects # for HepMcParticleLink
       StoreGateLib     # for ActsFatrasG4 (StoreGate/WriteHandleKeyArray)
  )                     
else()
  # Filter out files that contain "PunchThrough" anywhere in their name
  list(FILTER ALL_SRC_LIB_FILES EXCLUDE REGEX "PunchThrough")
  list(FILTER ALL_SRC_COMPONENT_FILES EXCLUDE REGEX "PunchThrough")
  # Filter out files that contain "ActsFatras" anywhere in their name 
  list(FILTER ALL_SRC_LIB_FILES EXCLUDE REGEX "ActsFatras")
  list(FILTER ALL_SRC_COMPONENT_FILES EXCLUDE REGEX "ActsFatras")
endif()

# Component(s) in the package:
# below is safe including for AthSimulation
atlas_add_library( G4AtlasToolsLib
                   ${ALL_SRC_LIB_FILES}
                   OBJECT
                   PUBLIC_HEADERS G4AtlasTools
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS} ${extra_incl}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} 
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${TBB_LIBRARIES}
                                  AthenaKernel AthenaBaseComps  
                                  G4AtlasInterfaces HitManagement 
                                  SubDetectorEnvelopesLib GeoPrimitives
                                  ${extra_lib}
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} XMLCoreParser CxxUtils GaudiKernel PathResolver )
set_target_properties( G4AtlasToolsLib PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_add_library( G4AtlasTools
                   ${ALL_SRC_COMPONENT_FILES}
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${extra_incl}
                   PRIVATE_LINK_LIBRARIES G4AtlasToolsLib)
set_target_properties( G4AtlasTools PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

#testing just the simulation parts, no forward detectors or region
atlas_add_test( G4GeometryToolConfig_Simtest
                SCRIPT test/G4GeometryToolConfig_Simtest.py
                POST_EXEC_SCRIPT noerror.sh )

#full test
if( NOT SIMULATIONBASE )
  atlas_add_test( G4GeometryToolConfig_test
                SCRIPT test/G4GeometryToolConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )
endif()

atlas_add_test( G4PhysicsRegionConfig_test
                SCRIPT test/G4PhysicsRegionConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( G4FieldConfig_test
                SCRIPT test/G4FieldConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( G4AtlasToolsConfig_test
                SCRIPT test/G4AtlasToolsConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )
