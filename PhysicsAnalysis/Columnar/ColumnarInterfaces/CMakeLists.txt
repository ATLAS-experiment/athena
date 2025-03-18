# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarInterfaces )

# Create the interface library target
atlas_add_library (ColumnarInterfacesLib
  ColumnarInterfaces/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarInterfaces)
