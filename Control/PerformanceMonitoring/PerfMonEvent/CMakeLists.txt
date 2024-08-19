# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonEvent )

# External dependencies:
find_package( Python COMPONENTS Development )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( PerfMonEvent
                   src/PyStore.cxx
                   src/PyChrono.cxx
                   src/DataModel.cxx
                   src/MemStatsHooks.cxx
                   src/MallocStats.cxx
                   PUBLIC_HEADERS PerfMonEvent
                   INCLUDE_DIRS ${Python_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${Python_LIBRARIES} CxxUtils GaudiKernel rt nlohmann_json::nlohmann_json CxxUtils
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_dictionary( PerfMonEventDict
                      PerfMonEvent/PerfMonEventDict.h
                      PerfMonEvent/selection.xml
                      INCLUDE_DIRS ${Python_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${Python_LIBRARIES} ${ROOT_LIBRARIES} rt GaudiKernel PerfMonEvent )

