# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaCommon )

if (XAOD_STANDALONE)
    # Install files from the package:
    atlas_install_python_modules( python/SystemOfUnits.py
                                  python/Logging.py
                                  python/Constants.py
                                  python/AthOptionsParser.py
                                  python/Utils )
else()
    # Set up specific Athena runtime environment:
    set( AthenaCommonEnvironment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
       CACHE PATH "Location of AthenaCommonEnvironmentConfig.cmake" )
    find_package( AthenaCommonEnvironment )
    
    # Install files from the package:
    atlas_install_python_modules( python/*.py python/Utils )

    atlas_install_joboptions( share/Preparation.py
                              share/Execution.py
                              share/runbatch.py )
    
    atlas_install_scripts( share/athena_preload.sh )

    # Python scripts checked by flake8:
    atlas_install_scripts( share/athena.py
                           POST_BUILD_CMD ${ATLAS_FLAKE8} )
    
    # Aliases:
    atlas_add_alias( athena "athena.py" )
    
    # Tests:
    atlas_add_test( AthAppMgr SCRIPT test/AthAppMgrUnitTests.py
       POST_EXEC_SCRIPT noerror.sh )

    atlas_add_test( Configurable SCRIPT test/ConfigurableUnitTests.py
       POST_EXEC_SCRIPT noerror.sh )

    atlas_add_test( JobProperties SCRIPT test/JobPropertiesUnitTests.py
       POST_EXEC_SCRIPT noerror.sh )

    if (NOT XAOD_ANALYSIS)
        atlas_add_test( ConsistencyOfConstants SCRIPT test/ConstantConsistencyUnitTests.py
           POST_EXEC_SCRIPT noerror.sh )
    endif()

    atlas_add_test( test_interactive_athena
       SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/test_interactive_athena.py )

    atlas_add_test( CFElements SCRIPT python -m unittest -v AthenaCommon.CFElements
       POST_EXEC_SCRIPT noerror.sh )

    if (NOT XAOD_ANALYSIS)
       atlas_add_test( JobOptionsUnitTests SCRIPT test/JobOptionsUnitTests.py
          POST_EXEC_SCRIPT noerror.sh )
    endif()

    # Command line testing:
    atlas_add_test( athenaCA_config
       SCRIPT athena.py --config-only=ca.pkl AthExHelloWorld/HelloWorldConfig.py Exec.MaxEvents=1
       POST_EXEC_SCRIPT noerror.sh )

    atlas_add_test( athenaCA_run
       SCRIPT athena.py --stdcmalloc ca.pkl
       LOG_SELECT_PATTERN "Preloading|processing event #"
       DEPENDS athenaCA_config )

    atlas_add_test( athenaLegacy_config
       SCRIPT athena.py --config-only=legacy.pkl --evtMax=1 AthExHelloWorld/HelloWorldOptions.py
       POST_EXEC_SCRIPT noerror.sh )

    atlas_add_test( athenaLegacy_run
       SCRIPT athena.py --stdcmalloc legacy.pkl
       LOG_SELECT_PATTERN "Preloading|processing event #"
       DEPENDS athenaLegacy_config )

endif()
