# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ByteStreamTest )

# Component(s) in the package:
atlas_add_component( ByteStreamTest
                     src/EvenEventsSelectorTool.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaPoolUtilities GaudiKernel )

# Tests:
atlas_add_test( SkipNone
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_skipEvents.py
   LOG_SELECT_PATTERN "^EventSelector |^AthenaEventLoopMgr"
   LOG_IGNORE_PATTERN "/cvmfs/" )

atlas_add_test( SkipOne
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_skipEvents.py Exec.SkipEvents=1
   LOG_SELECT_PATTERN "^EventSelector |^AthenaEventLoopMgr"
   LOG_IGNORE_PATTERN "/cvmfs/" )

atlas_add_test( EvenEvents
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_skipEvents.py --selectorTool EvenEventsSelectorTool
   LOG_SELECT_PATTERN "^EventSelector |^AthenaEventLoopMgr"
   LOG_IGNORE_PATTERN "/cvmfs/" )
