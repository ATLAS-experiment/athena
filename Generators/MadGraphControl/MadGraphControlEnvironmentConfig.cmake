# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for MadGraphControl
# 
#

# Set the environment variable(s):
find_package( FastJet )
find_package( Gosam_contrib )

if( FASTJET_FOUND AND MADGRAPH_FOUND )
  get_filename_component( _fastjetPath "${FASTJET_INCLUDE_DIR}" DIRECTORY )
  set( MADGRAPHCONTROLENVIRONMENT_ENVIRONMENT 
     FORCESET FASTJETPATH ${_fastjetPath}
     FORCESET MADGRAPHVER ${MADGRAPH5AMC_LCGVERSION})
  unset( _fastjetPath )
endif()

# Silently declare the module found:
set( MADGRAPHCONTROLENVIRONMENT_FOUND TRUE )

