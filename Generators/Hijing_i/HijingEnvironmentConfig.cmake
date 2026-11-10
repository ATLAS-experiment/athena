# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for Hijing
# 
#

# Set the environment variable(s):
find_package( Hijing )

if( HIJING_FOUND )
  set( HIJINGENVIRONMENT_ENVIRONMENT 
        FORCESET HIJINGVER ${HIJING_LCGVERSION} )
endif()

# Silently declare the module found:
set( HIJINGENVIRONMENT_FOUND TRUE )


