# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( xAODRootAccess )

# Extra dependencies, based on the build environment:
set( extra_libs )
if( NOT XAOD_STANDALONE )
   set( extra_libs SGTools AthenaKernel )
endif()

# External dependencies:
find_package( ROOT COMPONENTS Core Tree RIO Net MathCore ROOTNTuple ROOTNTupleUtil VDT ROOTTPython )
find_package( Python COMPONENTS Development )

# Generate a CINT dictionary source file:
atlas_add_root_dictionary( xAODRootAccess xAODRootAccessDictSource
   ROOT_HEADERS xAODRootAccess/tools/TTransTrees.h
   xAODRootAccess/tools/xAODTEventBranch.h xAODRootAccess/tools/xAODTEventTree.h
   xAODRootAccess/tools/xAODTMetaBranch.h xAODRootAccess/tools/xAODTMetaTree.h
   xAODRootAccess/tools/TFileChecker.h xAODRootAccess/tools/TFileMerger.h
   xAODRootAccess/MakeTransientTree.h xAODRootAccess/Init.h Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

# Build a library that other components can link against:
atlas_add_library( xAODRootAccess
   xAODRootAccess/*.h xAODRootAccess/*.icc
   xAODRootAccess/tools/*.h xAODRootAccess/tools/*.icc
   Root/*.h Root/*.cxx ${xAODRootAccessDictSource}
   PUBLIC_HEADERS xAODRootAccess
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS} ${Python_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${VDT_LIBRARIES} ${Python_LIBRARIES} AthContainers xAODCore xAODEventFormat
   xAODRootAccessInterfaces AsgMessagingLib CxxUtils RootUtils SGCoreLib ${extra_libs}
   )



# Build a (Reflex) dictionary library:
atlas_add_dictionary( xAODRootAccessDict
   xAODRootAccess/xAODRootAccessDict.h
   xAODRootAccess/selection.xml
   LINK_LIBRARIES xAODRootAccess )

# Install the Python modules of the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Install the checkxAOD.py script, only for the standalone analysis release.
# Athena based releases have a different implementation available to them.
if( XAOD_STANDALONE )
   atlas_install_scripts( scripts/checkxAOD.py )
endif()

# Build the executables of the package:
foreach( exec xAODChecker xAODMerge xAODFileReadTest xAODRNChecker xAODRNFileReadTest)
   atlas_add_executable( ${exec} util/${exec}.cxx
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess )
endforeach()



# Helper function setting up the compiled tests of the package:
function( _add_test name )
   cmake_parse_arguments( ARG "" "EXTRA_IGNORE" "" ${ARGN} )
   atlas_add_test( ${name}
      SOURCES test/${name}.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils AthContainers xAODCore
      xAODRootAccess
      PROPERTIES TIMEOUT 600
      LOG_IGNORE_PATTERN "zzzdummy${ARG_EXTRA_IGNORE}"
      POST_EXEC_SCRIPT noerror.sh )
endfunction( _add_test )

# Build/run unit tests for the package:
if( NOT SIMULATIONBASE AND NOT GENERATIONBASE )
   # This one has expected errors.
   _add_test( ut_xaodrootaccess_metadata_test EXTRA_IGNORE "|Entry to read is not set for branch=EventInfo|Failed to load current entry for object xAOD::EventInfo_v1/EventInfo|Didn.t find dictionary for type: ClassA|ERROR.* xAOD::Event::recordMeta.*T = ClassA" )
   _add_test( ut_xaodrootaccess_athenaMode_test )
   _add_test( ut_xaodrootaccess_remap_test )
   _add_test( ut_xaodrootaccess_slimming_test )
   _add_test( ut_xaodrootaccess_stats_test )
   _add_test( ut_xaodrootaccess_tchain_test )
   _add_test( ut_xaodrootaccess_transtree_test )
   _add_test( ut_xaodrootaccess_revent_test )
   atlas_add_test( ut_xaodrootaccess_transtree2_test
      SCRIPT python
      ${CMAKE_CURRENT_SOURCE_DIR}/test/ut_xaodrootaccess_transtree2_test.py
      POST_EXEC_SCRIPT noerror.sh
      PROPERTIES TIMEOUT 600 )
endif()
if( NOT XAOD_STANDALONE AND NOT GENERATIONBASE )
   _add_test( ut_xaodrootaccess_proxydict_test )
endif()
if( NOT SIMULATIONBASE )
   _add_test( ut_xaodrootaccess_tevent_copy_test )
   if( NOT GENERATIONBASE )
      atlas_add_test( ut_xaodrootaccess_tpyevent_test
         SCRIPT python
         ${CMAKE_CURRENT_SOURCE_DIR}/test/ut_xaodrootaccess_tpyevent_test.py
         POST_EXEC_SCRIPT noerror.sh
         PROPERTIES TIMEOUT 600 )
   endif()
endif()
_add_test( ut_xaodrootaccess_tauxvector_test )
_add_test( ut_xaodrootaccess_tauxvectorfactory_test )
_add_test( ut_xaodrootaccess_tauxstore_test )
_add_test( ut_xaodrootaccess_tauxstore_insertmove_test )
_add_test( ut_xaodrootaccess_tfileaccesstracer_test )
_add_test( ut_xaodrootaccess_tfilemerger_test )
_add_test( ut_xaodrootaccess_rauxstore_test )
atlas_add_test( ut_xaodrootaccess_tpystore_test
   SCRIPT python
   ${CMAKE_CURRENT_SOURCE_DIR}/test/ut_xaodrootaccess_tpystore_test.py
   POST_EXEC_SCRIPT noerror.sh )
# These have expected errors.
_add_test( ut_xaodrootaccess_tstore_test EXTRA_IGNORE "|of type ClassA as non-const|of type ConstDataVector<DataVector<ClassA> > as non-const|of type DataVector<ClassA> as non-const" )
if( NOT GENERATIONBASE AND NOT SIMULATIONBASE )
   _add_test( ut_xaodrootaccess_tevent_test EXTRA_IGNORE "|The following objects were not set in the current event: .FloatVec., .Container2Aux..|Didn.t find dictionary for type: ClassA|ERROR.* xAOD::Event::record.*T = ClassA|Destructor not known for type DataVector<ClassA>|Couldn.t delete managed object" )
endif()
