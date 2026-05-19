# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTruth )

# Create the interface library target
atlas_add_library (ColumnarTruthLib
ColumnarTruth/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarTruth
  LINK_LIBRARIES ColumnarCoreLib xAODTruth)
