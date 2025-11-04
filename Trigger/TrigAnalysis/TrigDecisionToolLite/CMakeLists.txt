# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigDecisionToolLite )

atlas_add_library( TrigDecisionToolLiteLib
   TrigDecisionToolLite/*.h Root/*.cxx
   PUBLIC_HEADERS TrigDecisionToolLite
   LINK_LIBRARIES AsgTools AsgDataHandlesLib AsgMessagingLib xAODBase xAODTrigger
   TrigCompositeUtilsLib TrigDecisionInterface TrigAnalysisHelpersLib )

atlas_add_dictionary( TrigDecisionToolLiteDict
   TrigDecisionToolLite/TrigDecisionToolLiteDict.h
   TrigDecisionToolLite/selection.xml
   LINK_LIBRARIES TrigDecisionToolLiteLib ${ROOT_LIBRARIES} )

if( NOT XAOD_STANDALONE )
   atlas_add_component( TrigDecisionToolLite
      src/components/*.cxx
      LINK_LIBRARIES TrigDecisionToolLiteLib )
endif()

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
