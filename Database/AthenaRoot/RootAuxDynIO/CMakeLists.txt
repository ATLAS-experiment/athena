# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootAuxDynIO )

find_package( ROOT COMPONENTS Core RIO Tree ROOTNTuple ROOTVecOps )

# Component(s) in the package:
atlas_add_component( RootAuxDynIO
                   src/*.cxx
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ROOT::ROOTNTuple
                                  AthenaBaseComps
                                  AthContainers AthContainersInterfaces
                                  AthContainersRoot RootUtils GaudiKernel
                   )

atlas_add_library( RootAuxDynIOHeaders
                   RootAuxDynIO/IRootAuxDynIO.h
                   INTERFACE
                   PUBLIC_HEADERS RootAuxDynIO
                   LINK_LIBRARIES GaudiKernel )
