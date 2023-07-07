# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GNNVertexConstructor )

# External dependencies:
find_package( Boost COMPONENTS filesystem )
find_package( lwtnn )
find_package( onnxruntime )
find_package(ROOT COMPONENTS Core MathCore Tree Hist RIO pthread REQUIRED)
find_package(nlohmann_json REQUIRED)


atlas_add_component( GNNVertexConstructor 
                     src/components/*.cxx
                     LINK_LIBRARIES GNNVertexConstructorLib 
                                    AthenaBaseComps 
                                    StoreGateLib 
                                    xAODTracking 
                                    xAODEventInfo 
                                    xAODJet 
                                    xAODBTagging 
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
 )


atlas_add_library(   GNNVertexConstructorLib
                     src/*.cxx
                     PUBLIC_HEADERS GNNVertexConstructor
                     PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
                     PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver CxxUtils 
                     LINK_LIBRARIES AthenaBaseComps 
                                    StoreGateLib 
                                    xAODTracking 
                                    xAODEventInfo 
                                    xAODJet 
                                    xAODBTagging 
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
 )
                     
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )




