# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTau )

# Create the interface library target
atlas_add_library (ColumnarTauLib
ColumnarTau/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarTau
  LINK_LIBRARIES ColumnarCoreLib xAODTau)
