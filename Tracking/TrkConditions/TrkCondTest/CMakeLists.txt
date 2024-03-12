# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkCondTest )

# Component(s) in the package:
atlas_add_component( TrackingGeometryCondAlgTest
   TrkCondTest/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES
   AthenaBaseComps
   AthenaKernel
   CxxUtils
   GaudiKernel
   PersistentDataModel
   StoreGateLib
   TrkDetDescrInterfaces
   TrkGeometry
   InDetReadoutGeometry
    )
