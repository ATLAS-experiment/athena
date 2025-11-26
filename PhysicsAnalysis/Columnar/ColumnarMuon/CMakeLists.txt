# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarMuon )

# Create the interface library target
atlas_add_library (ColumnarMuonLib
ColumnarMuon/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarMuon
  LINK_LIBRARIES ColumnarCoreLib xAODMuon ColumnarTrackingLib ColumnarVariantLib TruthUtils)
