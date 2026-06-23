# Copyright (C) 2002-2026 by CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCnvToolInterfaces )

# Component(s) in the package:
atlas_add_library( MuonCnvToolInterfacesLib
                   MuonCnvToolInterfaces/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonCnvToolInterfaces
                   LINK_LIBRARIES GaudiKernel ByteStreamCnvSvcBaseLib Identifier AthContainers AthenaKernel ByteStreamData CxxUtils )

atlas_add_dictionary( MuonCnvToolInterfacesDict
                      MuonCnvToolInterfaces/MuonCnvToolInterfacesDict.h
                      MuonCnvToolInterfaces/selection.xml
                      LINK_LIBRARIES MuonCnvToolInterfacesLib )
