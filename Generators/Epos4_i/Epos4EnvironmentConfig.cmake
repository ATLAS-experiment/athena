# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for Epos4
# 
#

# Set the environment variable(s):
find_package( EPOS4 )

if( EPOS4_FOUND  )
  set( EPOS4ENVIRONMENT_ENVIRONMENT SET EPOS4VER ${EPOS4_LCGVERSION} )
endif()

# Silently declare the module found:
set( EPOS4ENVIRONMENT_FOUND TRUE )


