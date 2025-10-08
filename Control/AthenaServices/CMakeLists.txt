# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaServices )

# External dependencies:
find_package( Boost COMPONENTS thread serialization )
find_package( bmpi3 REQUIRED )
find_package( CLHEP )
find_package( Python COMPONENTS Development )
find_package( ROOT COMPONENTS Core )
find_package( TBB )
find_package( UUID )
find_package( yampl )

# Component(s) in the package:
atlas_add_component( AthenaServices
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${Python_INCLUDE_DIRS}
   ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${YAMPL_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} ${Python_LIBRARIES} ${ROOT_LIBRARIES} ${TBB_LIBRARIES} ${UUID_LIBRARIES}
   ${YAMPL_LIBRARIES} bmpi3::bmpi3 ${CLHEP_LIBRARIES}
   AthenaBaseComps AthenaKernel RootUtilsPyROOT CxxUtils SQLiteDBSvcLib
   AthContainers AthContainersInterfaces DataModelRoot SGTools
   StoreGateLib PersistentDataModel EventInfo xAODCore
   xAODEventInfo EventInfoUtils GaudiKernel RootAuxDynIOHeaders )

# Test library checking the ability to build T/P converters:
atlas_add_tpcnv_library( AthenaServicesTest src/test/*.cxx
   NO_PUBLIC_HEADERS
   PRIVATE_LINK_LIBRARIES AthenaKernel)

# The test(s) of the package:
atlas_add_test( AthenaOutputStream_test
   SOURCES test/AthenaOutputStream_test.cxx src/AthenaOutputStream.cxx
   src/OutputStreamSequencerSvc.cxx src/MetaDataSvc.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} TestTools AthenaKernel AthContainers
                  AthContainersInterfaces CxxUtils SGTools StoreGateLib GaudiKernel AthenaBaseComps
                  PersistentDataModel RootAuxDynIOHeaders xAODCore
   LOG_SELECT_PATTERN "AthenaOutputStream_test"
   LOG_IGNORE_PATTERN "ERROR add: can not find clid 13 in clid db" )
# Avoid spurious ubsan warnings.
set_target_properties( AthenaServices_AthenaOutputStream_test PROPERTIES ENABLE_EXPORTS True )

atlas_add_test( FPEControlSvc_test
   SOURCES test/FPEControlSvc_test.cxx
   LINK_LIBRARIES TestTools AthenaKernel CxxUtils GaudiKernel )

atlas_add_test( AthenaEventLoopMgr_test
   SOURCES test/AthenaEventLoopMgr_test.cxx
   LINK_LIBRARIES TestTools AthenaKernel GaudiKernel EventInfo AthenaBaseComps )

atlas_add_test( ConditionsCleanerSvc_test
  SOURCES test/ConditionsCleanerSvc_test.cxx
  LINK_LIBRARIES TestTools AthenaKernel GaudiKernel AthenaBaseComps )

atlas_add_test( RCUSvc_test
   SOURCES test/RCUSvc_test.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaKernel CxxUtils GaudiKernel AthenaBaseComps )

atlas_add_test( DelayedConditionsCleanerSvc_test
   SOURCES test/DelayedConditionsCleanerSvc_test.cxx
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${TBB_LIBRARIES} TestTools AthenaKernel GaudiKernel AthenaBaseComps )

atlas_add_test( ThinningCacheTool_test
   SOURCES test/ThinningCacheTool_test.cxx
   LINK_LIBRARIES TestTools AthContainers AthenaKernel GaudiKernel AthenaBaseComps StoreGateLib )
   
atlas_add_test( MetaDataSvc_test
   SOURCES test/MetaDataSvc_test.cxx src/MetaDataSvc.cxx src/OutputStreamSequencerSvc.cxx 
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} TestTools AthenaKernel CxxUtils GaudiKernel
                  StoreGateLib PersistencySvc AthenaBaseComps RootAuxDynIOHeaders PersistentDataModel SGTools Boost::unit_test_framework
   POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( AthTPCnvSvc
   SCRIPT test/AthTPCnvSvc_test.py
   LOG_IGNORE_PATTERN "^ApplicationMgr +INFO|ERROR Cannot set CLID.*AthenaServicesTestConverters"
   PROPERTIES TIMEOUT 300 )

atlas_add_test( AthDictLoaderSvc
   SCRIPT athena.py --evtMax 1 AthenaServices/AthDictLoaderSvc_test.py
   PROPERTIES TIMEOUT 300 )

atlas_add_test( ItemListSemanticsTest
   SCRIPT test/OutputStreamItemListTest.py
   POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TimeoutAlgTest
   SCRIPT test/TimeoutAlg_test.py
   PROPERTIES PROCESSORS 2
              PASS_REGULAR_EXPRESSION "ERROR Event timeout"
   POST_EXEC_SCRIPT nopost.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py
   POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_joboptions(
   share/AthDictLoaderSvc_test.py
   share/ReadAthenaPoolSeek_jobOptions.py
   share/AthenaOutputStream_test.txt
   share/FPEControlSvc_test.txt
   share/AthenaEventLoopMgr_test.txt
   share/ConditionsCleanerSvc_test.txt
   share/RCUSvc_test.txt
   share/DelayedConditionsCleanerSvc_test.txt
   share/ThinningCacheTool_test.txt
   share/MetaDataSvc_test.txt )
