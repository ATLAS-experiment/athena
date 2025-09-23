# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkTau )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkTau
                     src/*.cxx src/components/*.cxx
		     LINK_LIBRARIES AsgTools AthenaBaseComps DerivationFrameworkInterfaces ExpressionEvaluationLib GaudiKernel DiTauRecLib TauAnalysisToolsLib tauRecToolsLib xAODPFlow xAODTau xAODTracking )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
