# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for Pythia8
# 
#

# Set the environment variable(s):
find_package( Pythia8 )
find_package( Lhapdf )

if( PYTHIA8_FOUND AND LHAPDF_FOUND )
  set( PYTHIA8ENVIRONMENT_ENVIRONMENT 
        FORCESET PYTHIA8VER ${PYTHIA8_LCGVERSION}
        FORCESET PYTHIA8BVER ${PYTHIA8_LCGVERSION}
        FORCESET PY8PATH ${PYTHIA8_LCGROOT}
        FORCESET PYTHIA8CMND $PY8PATH/share/Pythia8
        FORCESET LHAPDFVER ${LHAPDF_LCGVERSION} 
        FORCESET LHAPDF_INSTAL_PATH ${LHAPDF_LCGROOT}
        PREPEND LHAPDF_DATA_PATH
      /cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current
        PREPEND LHAPATH
      /cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current

 )
endif()

# Silently declare the module found:
set( PYTHIA8ENVIRONMENT_FOUND TRUE )



