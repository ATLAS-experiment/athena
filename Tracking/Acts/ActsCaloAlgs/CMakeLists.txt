# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsCaloAlgs
################################################################################

# Declare the package name:
atlas_subdir( ActsCaloAlgs )

find_package( Acts COMPONENTS Core )

atlas_add_component( ActsCaloAlgs
		     src/*.cxx
             src/components/*.cxx
             INCLUDE_DIRS
		     LINK_LIBRARIES Acts::Core egammaInterfacesLib ActsToolInterfacesLib ActsEventLib egammaUtils MagFieldConditions
                            AthenaBaseComps StoreGateLib InDetTrackSelectionToolLib 
                            FourMomUtils )
