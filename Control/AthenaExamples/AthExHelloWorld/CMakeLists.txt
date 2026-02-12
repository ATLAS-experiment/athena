# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExHelloWorld )

# Component(s) in the package:
atlas_add_component( AthExHelloWorld
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel AthenaBaseComps )

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_python_modules( python/HelloWorldConfig.py 
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package:
atlas_add_test( AthExHelloWorldLegacy
                SCRIPT athena.py AthExHelloWorld/HelloWorldOptions.py )

atlas_add_test( AthExHelloWorld
                SCRIPT athena.py AthExHelloWorld/HelloWorldConfig.py
                LOG_SELECT_PATTERN ".*EventLoopMgr|HelloWorld" )

atlas_add_test( AthExHelloWorldMT_1
                SCRIPT athena.py --threads=1 AthExHelloWorld/HelloWorldConfig.py
                LOG_SELECT_PATTERN ".*EventLoopMgr|HelloWorld" )

atlas_add_test( AthExHelloWorldMT_2
                SCRIPT athena.py --threads=2 AthExHelloWorld/HelloWorldConfig.py
                PROPERTIES PASS_REGULAR_EXPRESSION "AvalancheSchedulerSvc.*events in flight: 2"
                POST_EXEC_SCRIPT nopost.sh ) # no ref comparison due to MT ordering of output
