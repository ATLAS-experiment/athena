# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigAnomalyDetectionHypo )

# External reqs
find_package( ROOT COMPONENTS Core GenVector )
find_package( onnxruntime )

# Component(s) in the package:
atlas_add_component( TrigAnomalyDetectionHypo
                     src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
		     LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} ${ROOT_LIBRARIES} DecisionHandlingLib PathResolver TrigCompositeUtilsLib xAODEgamma xAODJet xAODMuon xAODTau xAODTrigMissingET AthOnnxInterfaces AsgServicesLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )
