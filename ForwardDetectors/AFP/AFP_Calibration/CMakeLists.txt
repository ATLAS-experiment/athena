# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AFP_Calibration )
find_package( Boost COMPONENTS program_options)

find_package( ROOT COMPONENTS MathCore Core Tree Hist RIO pthread )

atlas_add_library( AFP_CalibrationLib
                   src/*.cxx
                   PUBLIC_HEADERS AFP_Calibration
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AFP_Geometry AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib xAODEventInfo xAODForward )

atlas_add_component( AFP_Calibration
                     src/components/*.cxx
                     LINK_LIBRARIES AFP_CalibrationLib )

# Install files from the package:
atlas_install_joboptions( scripts/*.py )

atlas_add_executable( run_AFP_PixelIdentifier
   scripts/run_AFP_PixelIdentifier.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES AFP_CalibrationLib ${Boost_LIBRARIES})
