# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( egammaMVACalib )

# Extra dependencies for Athena capable builds:
set( extra_libs )
set(extra_plibs)
if( NOT XAOD_ANALYSIS )
   set( extra_libs PRIVATE_LINK_LIBRARIES GaudiKernel )
   set ( extra_plibs Identifier egammaInterfacesLib egammaLayerRecalibTool FlavorTagInference egammaCaloUtils )
endif()

# External dependencies:
find_package( ROOT COMPONENTS Tree Core Hist)

# Component(s) in the package:
atlas_add_library( egammaMVACalibLib
   egammaMVACalib/*.h Root/*.cxx
   PUBLIC_HEADERS egammaMVACalib
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AsgServicesLib AsgMessagingLib 
   xAODCaloEvent xAODEgamma  xAODTracking MVAUtils PathResolver EgammaAnalysisInterfacesLib CxxUtils
   ${extra_plibs}
   ${extra_libs} )

if (XAOD_STANDALONE)
atlas_add_dictionary( egammaMVACalibDict
	egammaMVACalib/egammaMVACalibDict.h
	egammaMVACalib/selection.xml
	LINK_LIBRARIES egammaMVACalibLib )
endif()

if (NOT XAOD_STANDALONE)
atlas_add_component( egammaMVACalib
	src/components/*.cxx
	LINK_LIBRARIES egammaMVACalibLib)
endif()

if( NOT XAOD_ANALYSIS)
atlas_add_test(egammaMVASvcConfigTest
   SCRIPT python -m egammaMVACalib.egammaMVACalibConfig
   POST_EXEC_SCRIPT noerror.sh)
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
