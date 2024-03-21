# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GNNVertexConstructor )

# External dependencies:
#find_package( Boost COMPONENTS filesystem )
find_package( Boost )
find_package( lwtnn )
find_package( onnxruntime )
find_package(ROOT COMPONENTS Core MathCore Tree Hist RIO pthread REQUIRED)
find_package(nlohmann_json REQUIRED)


atlas_add_component( GNNVertexConstructor 
                     src/*.h src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} 
                                    ${ROOT_LIBRARIES}
                                    GNNVertexConstructorLib 
                                    AthenaBaseComps 
                                    StoreGateLib 
                                    xAODTracking 
                                    xAODEventInfo 
                                    xAODJet 
                                    xAODBTagging
                                    xAODCore 
                                    FlavorTagDiscriminants  
                                    GaudiKernel
                                    AsgTools
                                    SystematicsHandlesLib
                                    FTagAnalysisInterfacesLib
                                    FourMomUtils
                                    TruthUtils
                                    AthContainers
                                    AthLinks
                                    AnaAlgorithmLib
                                    TrkVKalVrtFitterLib 
                                    AnalysisUtilsLib  
                                    GeoPrimitives 
                                    BeamSpotConditionsData 
                                    MVAUtils 
                                    PathResolver 
                                    TrkExInterfaces 
                                    TrkGeometry 
                                    TrkNeutralParameters 
                                    TrkTrackSummary  
                                    VxSecVertex 
                                    VxVertex
                                    xAODTruth
                                    TrkToolInterfaces
                                    InDetTrackSystematicsToolsLib                                    
 )

atlas_add_library(   GNNVertexConstructorLib
                     GNNVertexConstructor/*.h Root/*.cxx
                     PUBLIC_HEADERS GNNVertexConstructor
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
                     PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver CxxUtils 
                     LINK_LIBRARIES ${Boost_LIBRARIES} 
                                    ${ROOT_LIBRARIES}
                                    AthenaBaseComps 
                                    StoreGateLib 
                                    xAODTracking 
                                    xAODEventInfo 
                                    xAODJet 
                                    xAODBTagging 
                                    xAODCore
                                    FlavorTagDiscriminants  
                                    GaudiKernel
                                    AsgTools
                                    SystematicsHandlesLib
                                    FTagAnalysisInterfacesLib
                                    FourMomUtils
                                    TruthUtils
                                    AthContainers
                                    AthLinks
                                    AnaAlgorithmLib
                                    TrkVKalVrtFitterLib 
                                    AnalysisUtilsLib  
                                    GeoPrimitives 
                                    BeamSpotConditionsData 
                                    MVAUtils  
                                    TrkExInterfaces 
                                    TrkGeometry 
                                    TrkNeutralParameters 
                                    TrkTrackSummary  
                                    VxSecVertex
                                    VxVertex 
                                    xAODTruth
                                    TrkToolInterfaces
                                    InDetTrackSystematicsToolsLib                                    
 )
                     
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

#atlas_install_data( data/* )



