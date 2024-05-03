# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetRecConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*py)
atlas_install_joboptions( share/*.py )


atlas_add_test( JetDefUTest
   SCRIPT test_JetDefinitions_properties.py
   POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( JetDefCopyTest
   SCRIPT test_JetDefinitions_copy.py
   POST_EXEC_SCRIPT nopost.sh )

if( NOT GENERATIONBASE  ) 
  # argument '-n 0' prevent a full run and performs only the jet config
  atlas_add_test( JetStandarSmallRConfig
    SCRIPT test_StandardJets.py "-j smallR -n 0"
    POST_EXEC_SCRIPT nopost.sh )

  atlas_add_test( JetStandarLargeRConfig
    SCRIPT test_StandardJets.py "-j largeR -n 0"
    POST_EXEC_SCRIPT nopost.sh )

  atlas_add_test( JetStandarCSSKConfig
    SCRIPT test_StandardJets.py "-j cssk -n 0"
    POST_EXEC_SCRIPT nopost.sh )
 
  atlas_add_test( JetStandarDerivConfig
    SCRIPT test_StandardJets.py "-j deriv -n 0"
    POST_EXEC_SCRIPT nopost.sh )
endif()
