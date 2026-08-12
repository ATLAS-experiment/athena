# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonAlignErrorTool )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Hist RIO )
# Component(s) in the package:
atlas_add_component( MuonAlignErrorTool
                     src/*.cxx
                     src/tests/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps CxxUtils GeoPrimitives GaudiKernel MuonAlignErrorBase 
                                    MuonCalibITools MuonAlignmentData MuonIdHelpersLib TrkToolInterfaces StoreGateLib PathResolver 
                                    TrkCompetingRIOsOnTrack TrkPrepRawData TrkRIO_OnTrack TrkTrack )

# Install files from the package:
atlas_install_runtime( data/alignment_error_blob_2015.txt data/alignment_error_blob_2016.txt data/alignment_error_blob_2017_start.txt )

# Install files from the package:
atlas_install_python_modules( python/*.py )
