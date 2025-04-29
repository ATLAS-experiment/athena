# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CSCcabling )

# Component(s) in the package:
atlas_add_library( CSCcablingLib
                   src/*.cxx
                   PUBLIC_HEADERS CSCcabling
                   LINK_LIBRARIES AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES GaudiKernel GeoModelInterfaces GeometryDBSvcLib RDBAccessSvcLib )

atlas_add_component( CSCcabling
                     src/components/*.cxx
                     LINK_LIBRARIES CSCcablingLib )
