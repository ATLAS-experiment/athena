# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarMet )

# Create the interface library target
atlas_add_library (ColumnarMetLib
  ColumnarMet/*.h
  INTERFACE
  LINK_LIBRARIES ColumnarCoreLib xAODMissingET METUtilitiesLib ColumnarEgammaLib ColumnarJetLib ColumnarMuonLib
  PUBLIC_HEADERS ColumnarMet)
