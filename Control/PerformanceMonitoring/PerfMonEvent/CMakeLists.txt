# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonEvent )

# Component(s) in the package:
atlas_add_library( PerfMonEvent
                   src/DataModel.cxx
                   PUBLIC_HEADERS PerfMonEvent
                   LINK_LIBRARIES CxxUtils )

atlas_add_dictionary( PerfMonEventDict
                      PerfMonEvent/PerfMonEventDict.h
                      PerfMonEvent/selection.xml
                      LINK_LIBRARIES PerfMonEvent )
