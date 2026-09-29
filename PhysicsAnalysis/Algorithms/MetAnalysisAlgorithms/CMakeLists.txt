# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( MetAnalysisAlgorithms )

atlas_add_library( MetAnalysisAlgorithmsLib
   MetAnalysisAlgorithms/*.h MetAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS MetAnalysisAlgorithms
   LINK_LIBRARIES AnaAlgorithmLib METInterface SystematicsHandlesLib SelectionHelpersLib xAODBase xAODEventInfo xAODMissingET
   PRIVATE_LINK_LIBRARIES METUtilitiesLib )

atlas_add_component( MetAnalysisAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AnaAlgorithmLib METInterface SystematicsHandlesLib xAODBase
   xAODMissingET MetAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_data( data/* )
