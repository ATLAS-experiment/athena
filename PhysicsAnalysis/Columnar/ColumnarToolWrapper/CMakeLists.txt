# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarToolWrapper )

# Add the shared library:
atlas_add_library (ColumnarToolWrapperLib
ColumnarToolWrapper/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarToolWrapper
  LINK_LIBRARIES ColumnarInterfacesLib
  PRIVATE_LINK_LIBRARIES CxxUtils )
