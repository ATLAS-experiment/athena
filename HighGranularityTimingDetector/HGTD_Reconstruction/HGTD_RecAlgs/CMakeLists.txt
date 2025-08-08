# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_RecAlgs )

find_package( ROOT COMPONENTS TMVA )
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( HGTD_RecAlgs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES Acts::Core ActsEventLib AthenaBaseComps GaudiKernel GeneratorObjects
                     HGTD_RecToolInterfaces HGTD_RawData HGTD_RIO_OnTrack HGTD_Identifier
                     xAODTracking xAODTruth InDetSimData AthenaMonitoringKernelLib
                     PathResolver ${ROOT_LIBRARIES})

atlas_install_runtime( share/*.xml )
