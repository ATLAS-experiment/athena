# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarEventInfo )

# Create the interface library target
atlas_add_library (ColumnarEventInfoLib
  ColumnarEventInfo/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarEventInfo
  LINK_LIBRARIES ColumnarCoreLib xAODEventInfo)
