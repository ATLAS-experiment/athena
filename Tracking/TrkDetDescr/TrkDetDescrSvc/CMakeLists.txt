# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkDetDescrSvc )

# Component(s) in the package:
atlas_add_library( TrkDetDescrSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS TrkDetDescrSvc
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel TrkDetDescrInterfaces TrkDetDescrUtils TrkGeometry TrkVolumes EventInfoMgtLib
                   PRIVATE_LINK_LIBRARIES StoreGateLib )

atlas_add_component( TrkDetDescrSvc
                     src/components/*.cxx
                     LINK_LIBRARIES TrkDetDescrSvcLib )
