# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkPixelByteStreamCnv )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

# Component(s) in the package:
atlas_add_library( ITkPixelByteStreamCnvLib
   ITkPixelByteStreamCnv/*.h
   INTERFACE
   PUBLIC_HEADERS ITkPixelByteStreamCnv
   LINK_LIBRARIES GaudiKernel ByteStreamData InDetByteStreamErrors InDetRawData )

atlas_add_component( ITkPixelByteStreamCnv
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} ByteStreamData AthenaKernel EventContainers
   GaudiKernel InDetRawData AthenaBaseComps AthContainers CxxUtils StoreGateLib
   ByteStreamCnvSvcBaseLib InDetIdentifier PixelReadoutGeometryLib IRegionSelector
   xAODEventInfo TrigSteeringEvent InDetByteStreamErrors  ITkPixelByteStreamCnvLib ByteStreamCnvSvcLib )
   
atlas_add_test(ITkPixelRodDecoder_test
  SOURCES test/ITkPixelRodDecoder_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES}   AthenaBaseComps GaudiKernel IdDictParser StoreGateLib  Identifier ITkPixelByteStreamCnvLib
  POST_EXEC_SCRIPT "nopost.sh" )
  
atlas_add_test(ITkPixelRawDataProviderTool_test
  SOURCES test/ITkPixelRawDataProviderTool_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES}   AthenaBaseComps GaudiKernel IdDictParser StoreGateLib  Identifier ITkPixelByteStreamCnvLib
  POST_EXEC_SCRIPT "nopost.sh" )

# Install files from the package:
atlas_install_joboptions( share/*.txt )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
