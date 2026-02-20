# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTracking )

# Create the interface library target
atlas_add_library (ColumnarTrackingLib
ColumnarTracking/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarTracking
  LINK_LIBRARIES ColumnarCoreLib xAODTracking TruthUtils)
