# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( egammaTransformerCalib )

# Extra dependencies for Athena capable builds:
set( extra_libs )
if( NOT XAOD_STANDALONE )
   set( extra_libs PRIVATE_LINK_LIBRARIES GaudiKernel )
endif()

# External dependencies:
find_package( ROOT COMPONENTS Tree Core Hist)

# Component(s) in the package:
atlas_add_library( egammaTransformerCalibLib
   egammaTransformerCalib/*.h Root/*.cxx
   PUBLIC_HEADERS egammaTransformerCalib
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AsgServicesLib AsgMessagingLib 
   xAODCaloEvent xAODEgamma xAODTracking FlavorTagInference PathResolver EgammaAnalysisInterfacesLib egammaMVACalibLib egammaLayerRecalibTool egammaInterfacesLib CxxUtils
   ${extra_libs} )

atlas_add_dictionary( egammaTransformerCalibDict
	egammaTransformerCalib/egammaTransformerCalibDict.h
	egammaTransformerCalib/selection.xml
	LINK_LIBRARIES egammaTransformerCalibLib )

atlas_add_component( egammaTransformerCalib
	src/*.cxx src/components/*.cxx
	LINK_LIBRARIES egammaTransformerCalibLib)

if( NOT XAOD_ANALYSIS)
atlas_add_test(egammaTransformerSvcConfigTest
   SCRIPT python -m egammaTransformerCalib.egammaTransformerCalibConfig
   POST_EXEC_SCRIPT nopost.sh)
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )