# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetGNNTracking )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( HepPDT )
find_package(Boost REQUIRED COMPONENTS graph)
find_package( Acts COMPONENTS Core OPTIONAL_COMPONENTS PluginGnn )

# Component(s) in the package:
set(_sources
    src/DumpObjects.cxx
    src/ExaTrkXUtils.cxx
    src/GNNSeedingTrackMaker.cxx
    src/GNNTrackFinderTritonTool.cxx
    src/GNNTrackReaderTool.cxx
    src/SeedFitterTool.cxx
    src/SiGNNTrackFinderTool.cxx
    src/SiSPGNNTrackMaker.cxx
    src/SpacepointFeatureTool.cxx
    src/components/*.cxx)
set(_extraLibs)

if(TARGET Acts::PluginGnn)
    list(APPEND _sources src/ActsGnnModuleMapFinderTool.cxx)
    list(APPEND _extraLibs Acts::PluginGnn)
endif()

atlas_add_component( InDetGNNTracking
    ${_sources}
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS}
    LINK_LIBRARIES ${HEPPDT_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel InDetRecToolInterfaces
    PathResolver AthOnnxInterfaces AthOnnxUtilsLib
    TrkTrack StoreGateLib TrkExInterfaces TrkSpacePoint TrkFitterInterfaces
    PixelReadoutGeometryLib SCT_ReadoutGeometry InDetSimData
    InDetPrepRawData TrkTrack TrkRIO_OnTrack InDetSimEvent
    AtlasHepMCLib InDetRIO_OnTrack InDetRawData TrkTruthData
    SiSPSeededTrackFinderData Boost::graph CxxUtils
    Acts::Core ActsInteropLib InDetIdentifier
    ${_extraLibs}
)

atlas_install_python_modules( python/*.py )
