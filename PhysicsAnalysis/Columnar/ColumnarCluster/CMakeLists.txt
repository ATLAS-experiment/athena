# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarCluster )

# Create the interface library target
atlas_add_library (ColumnarClusterLib
ColumnarCluster/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarCluster
  LINK_LIBRARIES ColumnarCoreLib xAODCaloEvent)
