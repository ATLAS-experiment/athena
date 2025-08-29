# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarVariant )

# Create the interface library target
atlas_add_library (ColumnarVariantLib
ColumnarVariant/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarVariant
  LINK_LIBRARIES ColumnarCoreLib )
