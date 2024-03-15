# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT2CaloEgamma )

# Component(s) in the package:
atlas_add_library  ( TrigT2CaloEgammaLib
		     src/*.cxx
                     src/helpers/*.cxx
		     PUBLIC_HEADERS TrigT2CaloEgamma
		     LINK_LIBRARIES CaloEvent CaloGeoHelpers SGTools xAODTrigCalo xAODTrigRinger GaudiKernel TrigSteeringEvent TrigT2CaloCommonLib CaloDetDescrLib TrigT2CaloCalibrationLib TrigTimeAlgsLib egammaMVACalibLib 
		     PRIVATE_LINK_LIBRARIES CaloIdentifier AthLinks IRegionSelector TrigT1Interfaces 
			 )
atlas_add_component( TrigT2CaloEgamma
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT2CaloEgammaLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
