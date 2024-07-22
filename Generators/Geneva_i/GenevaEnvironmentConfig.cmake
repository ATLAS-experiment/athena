# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for Geneva
#

# Set the environment variable(s):
find_package( Geneva )
find_package( OpenLoops )

if ( GENEVA_FOUND )
  set( GENEVAENVIRONMENT_ENVIRONMENT
        FORCESET GENEVAVER ${GENEVA_LCGVERSION}
        FORCESET GENEVA_DATA_DIR ${GENEVA_LCGROOT}/share/Geneva/ )
endif()

# Silently declare the module found:
set( GENEVAENVIRONMENT_FOUND TRUE )
