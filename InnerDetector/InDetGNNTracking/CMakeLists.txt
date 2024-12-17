# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetGNNTracking )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( HepPDT )
find_package(Boost REQUIRED COMPONENTS graph)

# Component(s) in the package:
atlas_add_component( InDetGNNTracking
    src/*.cxx
    src/components/*.cxx
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS}
    LINK_LIBRARIES ${HEPPDT_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel InDetRecToolInterfaces
    PathResolver AthOnnxInterfaces AthOnnxUtilsLib
    TrkTrack StoreGateLib TrkExInterfaces TrkSpacePoint TrkFitterInterfaces
    PixelReadoutGeometryLib SCT_ReadoutGeometry InDetSimData
    InDetPrepRawData TrkTrack TrkRIO_OnTrack InDetSimEvent
    AtlasHepMCLib InDetRIO_OnTrack InDetRawData TrkTruthData
    SiSPSeededTrackFinderData Boost::graph
)

atlas_install_python_modules( python/*.py )
