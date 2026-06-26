# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigInDetAccelerationService )

# Component(s) in the package:
atlas_add_library(TrigInDetAccelerationServiceLib
   TrigInDetAccelerationService/*.h
   INTERFACE
   PUBLIC_HEADERS TrigInDetAccelerationService
   LINK_LIBRARIES GaudiKernel TrigAccelEvent)

atlas_add_component(TrigInDetAccelerationService
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES TrigInDetAccelerationServiceLib CxxUtils GaudiKernel
                  AthCUDAInterfacesLib AthenaInterprocess AthenaBaseComps
                  StoreGateLib TrigAccelEvent InDetIdentifier
                  InDetReadoutGeometry PixelReadoutGeometryLib
                  SCT_ReadoutGeometry ${CMAKE_DL_LIBS})
