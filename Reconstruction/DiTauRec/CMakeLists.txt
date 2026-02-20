# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DiTauRec )

# External dependencies:
find_package( FastJet )

# Component(s) in the package:
atlas_add_component( DiTauRec
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS}
                     LINK_LIBRARIES ${FASTJET_LIBRARIES} AthenaBaseComps xAODJet xAODTau GaudiKernel JetEDM TrkToolInterfaces CaloEvent xAODTracking StoreGateLib tauRecToolsLib PathResolver)

atlas_add_library( DiTauRecLib
	DiTauRec/*.h src/*.cxx
	PUBLIC_HEADERS DiTauRec
	LINK_LIBRARIES ${FASTJET_LIBRARIES} AthenaBaseComps xAODJet xAODTau GaudiKernel JetEDM TrkToolInterfaces CaloEvent xAODTracking StoreGateLib tauRecToolsLib PathResolver
)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

#Make test scripts available
atlas_install_scripts( test/test*.sh )

