# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for SFGen
# 
#

# Set the environment variable(s):

find_package( SFGen )
find_package( Lhapdf )

if( SFGEN_FOUND )

  set( SFGENENVIRONMENT_ENVIRONMENT 
       FORCESET SFGENVER ${SFGEN_LCGVERSION}
       FORCESET SFGENPATH ${SFGEN_LCGROOT} 
       APPEND LHAPATH ${LHAPDF_DATA_PATH}
       APPEND LHAPDF_DATA_PATH ${LHAPDF_DATA_PATH} )

endif()


# Silently declare the module found:
set( SFGENENVIRONMENT_FOUND TRUE )
