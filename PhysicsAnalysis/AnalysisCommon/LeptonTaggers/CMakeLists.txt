################################################################################
# Package: LeptonTaggers
################################################################################

# Declare the package name:
atlas_subdir( LeptonTaggers )

# External dependencies:
find_package( lwtnn )
find_package( ROOT COMPONENTS TMVA Core )
find_package( CORAL COMPONENTS CoralBase )
find_package(onnxruntime)

# Component(s) in the package:
atlas_add_library( LeptonTaggersLib
   LeptonTaggers/*.h src/*.cxx
   PUBLIC_HEADERS LeptonTaggers
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${LWTNN_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} AthContainers
   AthenaBaseComps
   FourMomUtils
   GaudiKernel
   PathResolver
   FlavorTagInference
   TrkVertexFitterInterfaces
   xAODBase xAODBTagging xAODEgamma
   xAODEventInfo xAODJet xAODMuon xAODTracking )

atlas_add_component( LeptonTaggers
   src/components/*.cxx
   LINK_LIBRARIES GaudiKernel LeptonTaggersLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
