# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


atlas_subdir( SharedDataHelpers )

find_package( Boost )
find_package( ROOT COMPONENTS Core Hist RIO )

atlas_add_library( SharedDataHelpersLib
   SharedDataHelpers/*.h SharedDataHelpers/*.icc Root/*.cxx
   PUBLIC_HEADERS SharedDataHelpers
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES AsgMessagingLib CxxUtils ${ROOT_LIBRARIES} )

# Note that this uses `nopost.sh`, as it checks failure modes, and
# `noerror.sh` would falsely indicate failure based on errors in the
# output mode.
atlas_add_test( gt_SharedDataHelpers
   SOURCES test/gt_SharedDataHelpers.cxx
   LINK_LIBRARIES AsgTestingLib SharedDataHelpersLib
   POST_EXEC_SCRIPT nopost.sh )
