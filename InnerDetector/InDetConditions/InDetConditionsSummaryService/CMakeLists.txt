# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetConditionsSummaryService )

set( extra_lib )
if( NOT SIMULATIONBASE )
   set( extra_lib InDetByteStreamErrors )
endif()

atlas_add_library( InDetConditionsSummaryService
                   INTERFACE
                   PUBLIC_HEADERS InDetConditionsSummaryService
                   LINK_LIBRARIES GaudiKernel Identifier ${extra_lib})
