# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTrigger )

# Create the interface library target
atlas_add_library (ColumnarTriggerLib
  ColumnarTrigger/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarTrigger
  LINK_LIBRARIES ColumnarCoreLib xAODTrigger)
