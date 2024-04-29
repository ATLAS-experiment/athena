# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FastCaloSimServices )

# External dependencies:
find_package( CLHEP )
find_package(lwtnn)

# Option(s) for switching on GPU/CUDA support.
set( _linkLibraries )
set( _definitions )
if( USE_GPU AND CMAKE_CUDA_COMPILER )
  message( STATUS "Compiling ISF_FastCaloSimServices with CUDA support" )
  list( APPEND _linkLibraries ISF_FastCaloGpuLib AthCUDACoreLib )
  list( APPEND _definitions USE_GPU )
endif()

# Component(s) in the package:
atlas_add_library( ISF_FastCaloSimServices
  src/*.cxx
  src/components/*.cxx
  OBJECT
  NO_PUBLIC_HEADERS
  PRIVATE_DEFINITIONS ${_definitions}
  INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}  ${LWTNN_INCLUDE_DIRS}
  LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib ${LWTNN_LIBRARIES}
  AthenaBaseComps AthenaKernel GaudiKernel IdDictParser ISF_InterfacesLib
  TrkEventPrimitives TrkExInterfaces CaloEvent CaloDetDescrUtils StoreGateLib NavFourMom
  GeneratorObjects ISF_Event ISF_FastCaloSimEvent
  ISF_FastCaloSimInterfaces ISF_FastCaloSimParametrizationLib PathResolver
  ${_linkLibraries} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( ISF_FastCaloSimServices_Config_test
                SCRIPT test/FastCaloSimServicesTest.py -n 3
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 4000)
