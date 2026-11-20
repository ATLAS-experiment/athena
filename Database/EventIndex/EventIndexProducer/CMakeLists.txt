# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventIndexProducer )

# External dependencies:
find_package( Protobuf )
find_package( boto3 )
find_package( botocore )
find_package( jmespath )
find_package( s3transfer )
find_package( stomppy )

# Generate and install the EventIndex proto file(s).
if( Protobuf_FOUND )
   protobuf_generate(
     LANGUAGE python
     OUT_VAR PROTO_PYS
     PROTOS "protocolB/EventIndex.proto" )
   set( _eiPythonFile
      "${CMAKE_PYTHON_OUTPUT_DIRECTORY}/EventIndexProducer/EventIndex_pb2.py" )
   add_custom_command(
      OUTPUT "${_eiPythonFile}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory
              "${CMAKE_PYTHON_OUTPUT_DIRECTORY}/EventIndexProducer"
      COMMAND "${CMAKE_COMMAND}" -E copy "${PROTO_PYS}" "${_eiPythonFile}"
      DEPENDS ${PROTO_PYS} )
   add_custom_target( EventIndexProducerProtobufGen ALL
      SOURCES "${_eiPythonFile}" )
   add_dependencies( Package_EventIndexProducer EventIndexProducerProtobufGen )
   install( FILES "${_eiPythonFile}"
      DESTINATION "${CMAKE_INSTALL_PYTHONDIR}/EventIndexProducer" OPTIONAL )
endif()


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Aliases:
atlas_add_alias( EventPicking "EventPicking.py" )
