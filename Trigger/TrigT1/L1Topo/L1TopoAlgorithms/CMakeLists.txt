# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1TopoAlgorithms )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist )
find_package( nlohmann_json )

# Build ADVAE2A as part of the package.
include( "cmake/advae2a-build.cmake" )
# Build ARTEMIS2A as part of the package.
include( "cmake/artemis2a-build.cmake" )

# Make sure that the executable of the package gets linked against all of its
# dependencies:
atlas_disable_as_needed()

# Special treatment for xAOD libraries
if( NOT TRIGCONF_STANDALONE )
 set( extra_libs xAODTrigger PathResolver)
endif()

# The component(s) in the package:
atlas_add_library( L1TopoAlgorithms
   L1TopoAlgorithms/*.h Root/*.cxx
   PUBLIC_HEADERS L1TopoAlgorithms
   PRIVATE_INCLUDE_DIRS ${ADVAE2A_INCLUDE_DIRS} ${ARTEMIS2A_INCLUDE_DIRS}
   LINK_LIBRARIES L1TopoEvent L1TopoInterfaces TrigConfBase TrigConfIO TrigConfData nlohmann_json::nlohmann_json ${extra_libs}
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${ADVAE2A_LIBRARIES} ${ARTEMIS2A_LIBRARIES} L1TopoCommon L1TopoSimulationUtils)
add_dependencies( L1TopoAlgorithms ADVAE2A ARTEMIS2A )

atlas_add_executable( TrigConfL1TopoGenPyAlg
   src/test/L1TopoGenPyAlg.cxx
   LINK_LIBRARIES CxxUtils L1TopoAlgorithms )

# Install files from the package:
atlas_install_python_modules( python/*.py )

# In release recompilation mode only parse the following if the package is
# being rebuilt:
if( ATLAS_RELEASE_RECOMPILE_DRYRUN )
   return()
endif()
if( ATLAS_RELEASE_MODE AND NOT ATLAS_PACKAGE_RECOMPILE )
   return()
endif()

# Create an auto-generated python description of the available algorithms:
add_custom_command( OUTPUT
   ${CMAKE_PYTHON_OUTPUT_DIRECTORY}/L1TopoAlgorithms/L1TopoAlgConfig.py
   COMMAND ${CMAKE_COMMAND} -E make_directory
   ${CMAKE_PYTHON_OUTPUT_DIRECTORY}/L1TopoAlgorithms
   COMMAND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/atlas_build_run.sh
   TrigConfL1TopoGenPyAlg
   ${CMAKE_PYTHON_OUTPUT_DIRECTORY}/L1TopoAlgorithms/L1TopoAlgConfig.py
   DEPENDS TrigConfL1TopoGenPyAlg )
add_custom_target( L1TopoPyGenAlg ALL SOURCES
   ${CMAKE_PYTHON_OUTPUT_DIRECTORY}/L1TopoAlgorithms/L1TopoAlgConfig.py )
add_dependencies( Package_L1TopoAlgorithms L1TopoPyGenAlg )

# Install the auto-generated file:
install( FILES
   ${CMAKE_PYTHON_OUTPUT_DIRECTORY}/L1TopoAlgorithms/L1TopoAlgConfig.py
   DESTINATION ${CMAKE_INSTALL_PYTHONDIR}/L1TopoAlgorithms OPTIONAL )
