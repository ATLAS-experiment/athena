# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetBeamSpotFinder )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist Matrix Minuit RooFitCore RooFit )

# Component(s) in the package:
atlas_add_component( InDetBeamSpotFinder
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     DEFINITIONS ${CLHEP_DEFINITIONS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AtlasDetDescr BeamSpotConditionsData CxxUtils EventInfo GaudiKernel Identifier LumiBlockData StoreGateLib TrkEventPrimitives TrkFitterInterfaces TrkFitterUtils TrkParameters TrkPseudoMeasurementOnTrack TrkRIO_OnTrack TrkTrack TrkVertexFitterInterfaces VxVertex xAODEventInfo xAODTracking )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
