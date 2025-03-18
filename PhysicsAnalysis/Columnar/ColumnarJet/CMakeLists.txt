# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarJet )

# Create the interface library target
atlas_add_library (ColumnarJetLib
ColumnarJet/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarJet
  LINK_LIBRARIES ColumnarCoreLib xAODJet)
