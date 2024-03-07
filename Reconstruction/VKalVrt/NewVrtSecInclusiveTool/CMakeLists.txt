# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( NewVrtSecInclusiveTool )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

atlas_add_library(   NewVrtSecInclusiveToolLib
                     src/*.cxx
                     PUBLIC_HEADERS NewVrtSecInclusiveTool
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
		     LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} TruthUtils AnalysisUtilsLib AthenaBaseComps GaudiKernel GeoPrimitives BeamSpotConditionsData MVAUtils PathResolver TrkExInterfaces TrkGeometry TrkNeutralParameters TrkTrackSummary TrkVKalVrtFitterLib VxSecVertex xAODTracking xAODTruth CxxUtils xAODJet )

atlas_add_component( NewVrtSecInclusiveTool
                     src/components/*.cxx
		     LINK_LIBRARIES NewVrtSecInclusiveToolLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
