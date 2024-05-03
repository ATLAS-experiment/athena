# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODCaloEventCnv )

# Component(s) in the package:
atlas_add_library( xAODCaloEventCnvLib
                   xAODCaloEventCnv/*.h
                   INTERFACE
                   PUBLIC_HEADERS xAODCaloEventCnv 
                   LINK_LIBRARIES GaudiKernel xAODCaloEvent )

# Component(s) in the package:
atlas_add_component( xAODCaloEventCnv
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES CaloInterfaceLib xAODCaloEventCnvLib xAODCaloEvent CaloEvent AthenaBaseComps AthenaKernel GaudiKernel )

