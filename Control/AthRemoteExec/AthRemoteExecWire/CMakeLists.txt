# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( AthRemoteExecWire )

# Header-only C++ mirror of what code in AthRemoteExecCore expects.
# RPC protocol (e.g. gRPC, Cap'n Proto) must implement it, then
# AthRemoteExecCoreLib must be linked against one of the implementation.
atlas_add_library( AthRemoteExecWireLib
   src/*.cxx
   PUBLIC_HEADERS AthRemoteExecWire
   LINK_LIBRARIES CxxUtils GaudiKernel )
