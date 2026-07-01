# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloCalibHitRec )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( CaloCalibHitRecLib
                   CaloCalibHitRec/*.h
		   INTERFACE
	           PUBLIC_HEADERS CaloCalibHitRec
		   LINK_LIBRARIES GaudiKernel AthenaKernel Identifier CaloSimEvent xAODTruth xAODCaloEvent)

# Component(s) in the package:
atlas_add_component( CaloCalibHitRec
                     src/*.cxx src/components/*.cxx
		     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
		     PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps CaloDmDetDescr CaloGeoHelpers CaloIdentifier StoreGateLib
                     CaloUtilsLib AtlasDetDescr CaloDetDescrLib CaloEvent LArRecEvent TileEvent TruthUtils CaloCalibHitRecLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
