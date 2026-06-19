# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsPrdAssociation)

find_package( Acts COMPONENTS Core )

atlas_add_component( ActsPrdAssociation
   src/*.h src/*.cxx
   src/components/*.cxx
   LINK_LIBRARIES
   AthenaBaseComps
   ActsCalibratorsLib
   GaudiKernel
   Acts::Core
   ActsEventLib
   ActsGeometryLib
   )
