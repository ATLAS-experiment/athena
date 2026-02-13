# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigServices )

# External dependencies:
find_package( Boost COMPONENTS regex python )
find_package( CORAL COMPONENTS CoralBase CoralKernel )
find_package( ROOT COMPONENTS Core Tree Hist RIO )
find_package( tdaq COMPONENTS owl )
find_package( tdaq-common COMPONENTS CTPfragment eformat hltinterface ers webdaq )
find_package( TBB )
find_package( Python COMPONENTS Development )
find_package( requests )

# Component(s) in the package:
atlas_add_component( TrigServices
                     src/*.h src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS
                     ${TDAQ-COMMON_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
                     LINK_LIBRARIES
                     ${TDAQ-COMMON_LIBRARIES} ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${CORAL_LIBRARIES} ${TBB_LIBRARIES}
                     AthenaBaseComps AthenaInterprocess AthenaKernel AthenaMonitoringKernelLib AthenaPoolUtilities
                     ByteStreamCnvSvcBaseLib ByteStreamData CxxUtils EventInfoUtils GaudiKernel RDBAccessSvcLib StoreGateLib TrigCostMonitorLib TrigKernel
                     TrigDataAccessMonitoringLib TrigOutputHandlingLib TrigSteeringEvent TrigSteerMonitorLib
                     TrigTimeAlgsLib xAODEventInfo xAODTrigger TrigT1Result)

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Python boost bindings for calling ITrigEventLoopMgr methods directly
atlas_add_library( TrigServicesHelper
                   python/TrigServicesHelper.cxx
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${Python_INCLUDE_DIRS}
                   LINK_LIBRARIES ${Boost_LIBRARIES} ${Python_LIBRARIES} GaudiKernel TrigKernel )

atlas_add_test( TrigMessageSvc
                SOURCES test/test_TrigMessageSvc.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils GaudiKernel
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TrigServicesConfig
                SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/python/TrigServicesConfig.py
                POST_EXEC_SCRIPT noerror.sh )
