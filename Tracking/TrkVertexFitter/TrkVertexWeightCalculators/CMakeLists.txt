# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkVertexWeightCalculators )

# External dependencies:
find_package( ROOT COMPONENTS Tree Core MathCore Hist Matrix )

# Component(s) in the package:
atlas_add_component( TrkVertexWeightCalculators
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GeoPrimitives xAODTracking GaudiKernel
                     TrkVertexFitterInterfaces GeneratorObjects VxVertex TrkNeuralNetworkUtilsLib
                     AsgTools MVAUtils AnaAlgorithmLib PhotonVertexSelectionLib PATCoreLib
                     PRIVATE_LINK_LIBRARIES PathResolver PhotonVertexSelectionLib TrkParameters)

# Install files from the package
atlas_install_runtime( share/*.root )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Add test
atlas_add_test( TrkVertexWeightCalculatorsBDT_test
   SCRIPT python -m TrkVertexWeightCalculators.TrkVertexWeightCalculatorsConfig
   POST_EXEC_SCRIPT noerror.sh )

