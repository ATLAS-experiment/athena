# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# The name of the package:
atlas_subdir( FTagAnalysisAlgorithms )

atlas_add_library( FTagAnalysisAlgorithmsLib
   FTagAnalysisAlgorithms/*.h FTagAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS FTagAnalysisAlgorithms
   LINK_LIBRARIES xAODJet xAODBTagging SelectionHelpersLib SystematicsHandlesLib
   TrigCompositeUtilsLib TrigDecisionToolLib
   AnaAlgorithmLib FTagAnalysisInterfacesLib )

atlas_add_component( FTagAnalysisAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES FTagAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
