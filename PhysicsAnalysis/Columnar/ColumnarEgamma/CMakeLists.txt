# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarEgamma )

# Create the interface library target
atlas_add_library (ColumnarEgammaLib
  ColumnarEgamma/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarEgamma
  LINK_LIBRARIES ColumnarCoreLib xAODEgamma ColumnarTrackingLib)
