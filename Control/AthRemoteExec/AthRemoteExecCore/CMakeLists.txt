# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( AthRemoteExecCore )

atlas_add_library( AthRemoteExecCoreLib
   src/RemoteExecHiveEventLoopMgr.cxx
   src/InputCnvAlg.cxx
   src/OutputCnvAlg.cxx
   src/RequestAlg.cxx
   src/PayloadCodecBase.cxx
   PUBLIC_HEADERS AthRemoteExecCore
   LINK_LIBRARIES AthRemoteExecGrpcLib AthRemoteExecWireLib
                  AthenaHiveEventLoopMgrLib AthenaBaseComps AthenaKernel CxxUtils GaudiKernel SGTools StoreGateLib )

atlas_add_component( AthRemoteExecCore
   src/components/*.cxx
   LINK_LIBRARIES AthRemoteExecCoreLib
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# --- Tests --- #

atlas_add_component( AthRemoteExecCoreTest
   test/test_utils.cxx
   LINK_LIBRARIES AthRemoteExecCoreLib AthenaBaseComps CxxUtils GaudiKernel SGTools StoreGateLib )

atlas_install_scripts(
   test/test_remote_exec_fragment.py
   test/test_remote_exec_run_*.py
   test/test_remote_exec_athena_client.sh
   test/test_remote_exec_backpressure.sh
   test/test_remote_exec_menu_client.sh )

atlas_add_executable( test_remote_exec_load_client
   test/remote_exec_load_client.cxx
   LINK_LIBRARIES AthRemoteExecGrpcObj )
atlas_add_executable( test_remote_exec_menu_client
   test/remote_exec_menu_client.cxx
   LINK_LIBRARIES AthRemoteExecGrpcObj CxxUtils )
# FIXME: Intentional to hide torrent of warnings from generated protobuf code which does not follow ATLAS standard
target_compile_options( test_remote_exec_load_client PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )
target_compile_options( test_remote_exec_menu_client PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )

atlas_add_test( ProtocolRoundTrip
   SOURCES test/remote_exec_protocol_roundtrip.cxx
   LINK_LIBRARIES AthRemoteExecCoreLib CxxUtils
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( RemoteExecFragment
   SCRIPT test_remote_exec_fragment.py
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( RemoteExecAthenaClient
   SCRIPT test_remote_exec_athena_client.sh
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( RemoteExecBackpressure
   SCRIPT test_remote_exec_backpressure.sh
   POST_EXEC_SCRIPT noerror.sh
   PRIVATE_WORKING_DIRECTORY )

atlas_add_test( RemoteExecHiveLoopMenuClient
   SCRIPT test_remote_exec_menu_client.sh
   POST_EXEC_SCRIPT noerror.sh
   PRIVATE_WORKING_DIRECTORY )
