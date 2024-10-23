# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSurveyConstraintTool )

# Component(s) in the package:
atlas_add_component( InDetSurveyConstraintTool
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GeoPrimitives InDetAlignGenToolsLib Identifier EventPrimitives GaudiKernel AthenaKernel AtlasDetDescr CxxUtils DetDescrConditions InDetIdentifier InDetReadoutGeometry RegistrationServicesLib StoreGateLib )
