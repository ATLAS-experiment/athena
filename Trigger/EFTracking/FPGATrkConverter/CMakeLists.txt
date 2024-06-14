################################################################################
# Package: FPGATrk_DataProviderSvc
################################################################################

# Declare the package name:
atlas_subdir( FPGATrkConverter )

# External dependencies:
find_package( CLHEP )
find_package( Eigen )
#find_package( AtlasHepMC )
find_package( Acts COMPONENTS Core PluginJson )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FPGATrkConverterLib
                   src/*.cxx
                   PUBLIC_HEADERS FPGATrkConverter
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel StoreGateLib SGtests FPGATrkConverterInterface FPGATrackSimObjectsLib FPGATrackSimConfToolsLib InDetIdentifier InDetReadoutGeometry SCT_ReadoutGeometry PixelReadoutGeometryLib InDetPrepRawData xAODInDetMeasurement ActsEventLib ActsGeometryLib
                   
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} ${EIGEN_LIBRARIES} AtlasDetDescr GeoPrimitives FPGATrackSimInputLib PixelGeoModelLib)

atlas_add_component( FPGATrkConverter
                     FPGATrkConverter/*.h src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} ${EIGEN_LIBRARIES} FPGATrkConverterLib)

# Install files from the package:
#atlas_install_python_modules( python/*.py )
#atlas_install_joboptions( share/*.py )



