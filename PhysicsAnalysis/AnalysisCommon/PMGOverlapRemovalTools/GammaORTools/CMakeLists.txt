# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GammaORTools )

# Libraries in the package:
atlas_add_library( GammaORToolsLib
   GammaORTools/*.h Root/*.cxx
   PUBLIC_HEADERS GammaORTools
   LINK_LIBRARIES AsgTools xAODTruth MCTruthClassifierLib TruthUtils)

atlas_add_component( GammaORTools
  src/components/*.cxx
  LINK_LIBRARIES GammaORToolsLib)

atlas_add_dictionary( GammaORToolsDict
  GammaORTools/GammaORToolsDict.h
  GammaORTools/selection.xml
  LINK_LIBRARIES GammaORToolsLib )
