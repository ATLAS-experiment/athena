# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DataModelRunTests )

# Install files from the package:
atlas_install_joboptions( share/*.py )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
function (datamodel_run_test testName)
  cmake_parse_arguments( ARG "MT;RNTUPLE" "DEPENDS;COMMAND;ARG;EXTRA_IGNORE" "" ${ARGN} )

  if( ARG_COMMAND )
    set( _command ${ARG_COMMAND} )
  else()
     set( _command athena.py )
  endif()

  if( ARG_ARG )
    set( _arg ${CMAKE_CURRENT_SOURCE_DIR}/test/${ARG_ARG} )
  else()
    set( _arg ${CMAKE_CURRENT_SOURCE_DIR}/test/${testName}.py )
  endif()

  set ( _options )
  if( ARG_MT )
    set( testName "${testName}MT" )
    set( _command "${_command} --threads=1 " )
  endif()

  if( ARG_RNTUPLE )
    set( testName "${testName}RNTup" )
    set( _options " Output.StorageTechnology.EventData='ROOTRNTUPLE' " )
  endif()

  atlas_add_test( ${testName}
                  SCRIPT ${_command} ${_arg} ${_options}
                  PROPERTIES TIMEOUT 600
                  LOG_IGNORE_PATTERN "has different type than the branch|no dictionary for class|^StorageSvc +INFO|Cache alignment|Unable to locate catalog|IOVDbSvc[0-9 ]+INFO|Invalid ./TagInfo${ARG_EXTRA_IGNORE}|The database does not exist|IDatabaseSvc::openDatabase.*=>|New file created"
                  ENVIRONMENT "ATLAS_REFERENCE_TAG=DataModelRunTests/DataModelRunTestsReference-01-00-03"
                  DEPENDS ${ARG_DEPENDS} )
endfunction (datamodel_run_test)

datamodel_run_test (DataModelTestWrite)
datamodel_run_test (DataModelTestRead DEPENDS DataModelTestWrite)
datamodel_run_test (DataModelTestRead2 DEPENDS DataModelTestRead)
datamodel_run_test (xAODTestWrite) # o:xaoddata.root
datamodel_run_test (xAODTestRead DEPENDS xAODTestWrite) #i:xaoddata.root o:xaoddata2.root
datamodel_run_test (xAODTestRead2 DEPENDS xAODTestRead) #i:xaoddata2.root o:xaoddata2x.root
datamodel_run_test (xAODTestTypelessRead DEPENDS xAODTestWrite)
datamodel_run_test (xAODTestRead3 DEPENDS xAODTestTypelessRead)
datamodel_run_test (xAODTestRead3 MT
                    EXTRA_IGNORE "|^xAODTe"
                    DEPENDS xAODTestTypelessRead)
datamodel_run_test (xAODTestReadRename DEPENDS xAODTestWrite)
datamodel_run_test (xAODTestReadRename MT
                    DEPENDS xAODTestWrite)
datamodel_run_test (xAODTestReadFilter DEPENDS xAODTestWrite)
datamodel_run_test (xAODTestReadFilterRead DEPENDS xAODTestReadFilter)
datamodel_run_test (xAODRootTestRead
                    COMMAND python
                    ARG xAODRootTestRead_t.py
                    DEPENDS xAODTestWrite)
datamodel_run_test (xAODRootTestRead2
                    COMMAND python
                    ARG xAODRootTestRead2_t.py
                    DEPENDS xAODRootTestRead)

datamodel_run_test (ByteStreamTestWrite)
datamodel_run_test (ByteStreamTestRead DEPENDS ByteStreamTestWrite)

datamodel_run_test (xAODTestSymlinks1)
datamodel_run_test (xAODTestSymlinks1 MT
                    EXTRA_IGNORE "|^xAODTe")

datamodel_run_test (xAODTestSymlinks2
                    DEPENDS xAODTestWrite)
datamodel_run_test (xAODTestSymlinks2 MT
                    DEPENDS xAODTestWrite)

datamodel_run_test (xAODTestDecorHandle1)
datamodel_run_test (xAODTestDecorHandle1 MT)

datamodel_run_test (xAODTestDecorHandle2
                    DEPENDS xAODTestWrite)
datamodel_run_test (xAODTestDecorHandle2 MT
                    DEPENDS xAODTestWrite)

datamodel_run_test (CondWriter)
datamodel_run_test (CondReader
                    DEPENDS CondWriter)
datamodel_run_test (CondReader MT
                    DEPENDS CondWriter)

datamodel_run_test (CondReadWrite)

datamodel_run_test (xAODTestWriteThinned)
datamodel_run_test (xAODTestReadThinned DEPENDS xAODTestWriteThinned)


##############################################################################
# AllocTest
#

datamodel_run_test (AllocTestWriteWithoutAlloc)
datamodel_run_test (AllocTestWriteWithAlloc)

datamodel_run_test (AllocTestReadNoAllocWithoutAlloc
                    COMMAND "athena.py --filesInput=alloctestWithoutAlloc.root"
                    ARG AllocTestReadWithoutAlloc.py
                    DEPENDS AllocTestWriteWithoutAlloc)
datamodel_run_test (AllocTestReadAllocWithAlloc
                    COMMAND "athena.py --filesInput=alloctestWithAlloc.root"
                    ARG AllocTestReadWithAlloc.py
                    DEPENDS AllocTestWriteWithAlloc)

datamodel_run_test (AllocTestReadNoAllocWithAlloc
                    COMMAND "athena.py --filesInput=alloctestWithoutAlloc.root"
                    ARG AllocTestReadWithAlloc.py
                    DEPENDS AllocTestWriteWithoutAlloc)
datamodel_run_test (AllocTestReadAllocWithoutAlloc
                    COMMAND "athena.py --filesInput=alloctestWithAlloc.root"
                    ARG AllocTestReadWithoutAlloc.py
                    DEPENDS AllocTestWriteWithAlloc)

datamodel_run_test (RootAllocTestReadNoAllocWithoutAlloc
                    COMMAND python
                    ARG RootAllocTestReadNoAllocWithoutAlloc_t.py
                    DEPENDS AllocTestWriteWithoutAlloc)
datamodel_run_test (RootAllocTestReadAllocWithAlloc
                    COMMAND python
                    ARG RootAllocTestReadAllocWithAlloc_t.py
                    DEPENDS AllocTestWriteWithAlloc)

datamodel_run_test (RootAllocTestReadNoAllocWithAlloc
                    COMMAND python
                    ARG RootAllocTestReadNoAllocWithAlloc_t.py
                    DEPENDS AllocTestWriteWithoutAlloc)
datamodel_run_test (RootAllocTestReadAllocWithoutAlloc
                    COMMAND python
                    ARG RootAllocTestReadAllocWithoutAlloc_t.py
                    DEPENDS AllocTestWriteWithAlloc)


##############################################################################
# RNtuple
#

datamodel_run_test (xAODTestWrite RNTUPLE) # o:xaoddata.rntuple.root
datamodel_run_test (xAODTestRead RNTUPLE DEPENDS xAODTestWriteRNTup) #i:xaoddata.rntuple.root o:xaoddata2.rntuple.root
datamodel_run_test (xAODTestRead2 RNTUPLE DEPENDS xAODTestReadRNTup) #i:xaoddata2.rntuple.root o:xaoddata2x.rntuple.root
datamodel_run_test (xAODTestTypelessRead RNTUPLE DEPENDS xAODTestWriteRNTup)
datamodel_run_test (xAODTestRead3 RNTUPLE DEPENDS xAODTestTypelessReadRNTup)
datamodel_run_test (xAODTestReadRename RNTUPLE DEPENDS xAODTestWriteRNTup)
datamodel_run_test (xAODTestReadFilter RNTUPLE DEPENDS xAODTestWriteRNTup)
datamodel_run_test (xAODTestWriteThinned RNTUPLE)
datamodel_run_test (xAODTestReadThinned RNTUPLE DEPENDS xAODTestWriteThinnedRNTup)


##############################################################################


# xAODTestWrite:
# writes xaoddata.root (cvec, cvecWD, cview, gvec, ctrig, cinfo, hvec, hview, S2)
#        
# xAODTestRead:
# reads xaoddata.root
# writes xaoddata2.root (cvec, cvecWD, cview, gvec, ctrig, cinfo, hvec, hview,
#                        copy_cvec, copy_cvecWD, copy_cview, copy_gvec, copy_ctrig,
#                        copy_cinfo, copy_hvec, copy_hview)
# writes xaoddata2b.root (same, with different decor)
#
# xAODTestRead2
# reads xaoddata2.root
# writes xaoddata2x.root (cvec)
#
# xAODTestRead2b
# reads xaoddata2b.root
#
# xAODTestTypelessRead
# reads xaoddata.root
# writes xaoddata3.root (cvec, cvecWD, cview, ctrig, cinfo, hvec, hview,
#                        copy_cvec, copy_cvecWD, copy_cview, copy_ctrig,
#                        copy_cinfo, copy_hvec, copy_hview)
#
# xAODTestRead3
# reads xaoddata3.root
#
# xAODTestReadRename
# reads xaoddata.root
#
# xAODTestReadFilter
# reads xaoddata.root
# writes xaoddata_filt.root (cvec)
#
# xAODTestReadFilterRead
# reads xaoddata_filt.root
#
# xAODRootTestRead
# reads xaoddata.root
# writes xaodroot.root
#
# xAODRootTestRead2
# reads xaoddata.root

