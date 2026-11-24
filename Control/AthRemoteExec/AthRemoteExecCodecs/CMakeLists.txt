# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( AthRemoteExecCodecs )

atlas_add_library( AthRemoteExecCodecsLib
   src/Bytestream.cxx
   src/BytestreamCodec.cxx
   PUBLIC_HEADERS AthRemoteExecCodecs
   LINK_LIBRARIES AthRemoteExecCoreLib ByteStreamData ByteStreamCnvSvcBaseLib
                  GaudiKernel StoreGateLib )

atlas_add_component( AthRemoteExecCodecs
   src/components/*.cxx
   LINK_LIBRARIES AthRemoteExecCodecsLib
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_component( AthRemoteExecCodecsTest
   test/test_bytestream_utils.cxx
   LINK_LIBRARIES AthRemoteExecCodecsLib AthenaBaseComps CxxUtils GaudiKernel
                  SGTools StoreGateLib ByteStreamCnvSvcBaseLib )

atlas_install_scripts(
   test/test_remote_exec_bytestream_roundtrip.sh
   test/test_remote_exec_run_codecs_server.py
   test/test_remote_exec_run_codecs_client.py )

atlas_add_test( Bytestream
   SOURCES test/remote_exec_bytestream.cxx
   LINK_LIBRARIES AthRemoteExecCodecsLib CxxUtils
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( BytestreamRoundtrip
   SCRIPT test_remote_exec_bytestream_roundtrip.sh
   POST_EXEC_SCRIPT noerror.sh
   PRIVATE_WORKING_DIRECTORY )
