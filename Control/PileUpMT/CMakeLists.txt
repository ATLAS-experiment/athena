# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(PileUpMT)

# External dependencies:
find_package(CLHEP)
find_package(Boost)
find_package(fmt)
find_package(Rangev3)

# Component(s) in the package:
atlas_add_library(PileUpMTLib
  INTERFACE
  PUBLIC_HEADERS PileUpMT
  LINK_LIBRARIES
  GaudiKernel
  StoreGateLib)

atlas_add_component(
  PileUpMT
  src/*.h
  src/*.cxx
  src/components/*.cxx
  INCLUDE_DIRS
  ${Boost_INCLUDE_DIRS}
  ${CLHEP_INCLUDE_DIRS}
  LINK_LIBRARIES
  PileUpMTLib
  ${Boost_LIBRARIES}
  ${CLHEP_LIBRARIES}
  fmt::fmt
  Rangev3::rangev3
  AthenaBaseComps
  EventInfo
  xAODCnvInterfaces
  xAODEventInfo
  PileUpToolsLib
  AthenaKernel
  SGTools
  StoreGateLib
  GaudiKernel
  BeamSpotConditionsData
  CxxUtils)
