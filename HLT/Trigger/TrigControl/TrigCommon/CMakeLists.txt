# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigCommon )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( bin/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( bin/asetup_wrapper )

# Aliases:
atlas_add_alias( athenaHLT "athenaHLT.py" )
atlas_add_alias( athenaEF "athenaEF.py" )

# Tests:
atlas_add_test( test_AthHLT
   SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/python/AthHLT.py
   POST_EXEC_SCRIPT noerror.sh
   PROPERTIES TIMEOUT 300 )
