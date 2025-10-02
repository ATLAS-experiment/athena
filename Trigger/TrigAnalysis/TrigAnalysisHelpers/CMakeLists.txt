# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigAnalysisHelpers )

atlas_add_library( TrigAnalysisHelpersLib
   TrigAnalysisHelpers/*.h Root/*.cxx
   PUBLIC_HEADERS TrigAnalysisHelpers
   LINK_LIBRARIES AsgMessagingLib TrigDecisionInterface TrigCompositeUtilsLib )
