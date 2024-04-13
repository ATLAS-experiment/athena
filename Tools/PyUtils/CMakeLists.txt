# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PyUtils )

find_package( ROOT COMPONENTS Core PyROOT Tree MathCore Hist RIO pthread )

# Define an interface library that can be used by other packages to declare a
# depdendency. Only required in case code from this package is executed during the build.
atlas_add_library( PyUtilsInterface
   INTERFACE
   NO_PUBLIC_HEADERS )
add_dependencies( PyUtilsInterface PyUtilsPythonInstall )

if( XAOD_STANDALONE )
    atlas_install_python_modules( python/MetaReader.py
                                  python/PoolFile.py
                                  python/moduleExists.py
                                  python/RootUtils.py
                                  python/Helpers.py
                                  python/Decorators.py
                                  POST_BUILD_CMD ${ATLAS_FLAKE8} )
    atlas_install_scripts( bin/meta-reader.py bin/meta-diff.py
                           POST_BUILD_CMD ${ATLAS_FLAKE8} )

    atlas_add_alias( meta-reader "meta-reader.py" )
    atlas_add_alias( meta-diff   "meta-diff.py" )
else()
    # External dependencies:
    find_package( graphviz )
    find_package( pygraphviz )
    find_package( requests )
    find_package( six )
    
    # Install files from the package:
    atlas_install_python_modules( python/*.py python/scripts python/AthFile POST_BUILD_CMD ${ATLAS_FLAKE8} )
    atlas_install_scripts( bin/acmd.py bin/checkFile.py bin/checkPlugins.py
       bin/checkSG.py bin/checkMetaSG.py bin/checkTP.py bin/checkxAOD.py
       bin/diff-jobo-cfg.py bin/diffConfigs.py
       bin/diffPoolFiles.py bin/dlldep.py bin/dso-stats.py
       bin/dumpAthfilelite.py bin/filter-and-merge-d3pd.py bin/getMetadata.py
       bin/gprof2dot bin/issues bin/magnifyPoolFile.py
       bin/apydep.py bin/pool_extractFileIdentifier.py
       bin/pool_insertFileToCatalog.py bin/print_auditor_callgraph.py bin/pyroot.py
       bin/vmem-sz.py bin/meta-reader.py bin/meta-diff.py bin/dump-event-from-file.py
       POST_BUILD_CMD ${ATLAS_FLAKE8} )
    
    # Aliases:
    atlas_add_alias( checkFile "checkFile.py" )
    atlas_add_alias( checkMetaSG "checkMetaSG.py" )
    atlas_add_alias( dso-stats "dso-stats.py" )
    atlas_add_alias( gen_klass "acmd.py" "gen-klass" )
    atlas_add_alias( diffConfigs "diffConfigs.py" )
    atlas_add_alias( filter-and-merge-d3pd "filter-and-merge-d3pd.py" )
    atlas_add_alias( diffPoolFiles "diffPoolFiles.py" )
    atlas_add_alias( print_auditor_callgraph "print_auditor_callgraph.py" )
    atlas_add_alias( pyroot "pyroot.py" )
    atlas_add_alias( checkxAOD "checkxAOD.py" )
    atlas_add_alias( checkSG "checkSG.py" )
    atlas_add_alias( diff-jobo-cfg "diff-jobo-cfg.py" )
    atlas_add_alias( acmd "acmd.py" )
    atlas_add_alias( vmem-sz "vmem-sz.py" )
    atlas_add_alias( getMetadata "getMetadata.py" )
    atlas_add_alias( meta-reader "meta-reader.py" )
    atlas_add_alias( meta-diff   "meta-diff.py" )
    
    # Tests:
    atlas_add_test( RootUtils
       SCRIPT test/test_RootUtils.py
       PROPERTIES TIMEOUT 450
       LOG_IGNORE_PATTERN "Ran 1 test in |CheckABICompatibility|standard library|import imp|imp module" )

   atlas_add_test( Decorators
       SCRIPT test/test_Decorators.py
       POST_EXEC_SCRIPT nopost.sh
   )
    
    atlas_add_test( fprint_test
                    SCRIPT python -m PyUtils.fprint )

    # Create python package dependencies in release building mode.
    # Used as input for `acmd.py cmake depends`:
    if( NOT "${CMAKE_PROJECT_NAME}" STREQUAL "WorkDir" )
    
       set( _pydot ${CMAKE_CURRENT_BINARY_DIR}/packages.py.dot )
       add_custom_command( OUTPUT ${_pydot}
          COMMAND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/atlas_build_run.sh
          ${CMAKE_CURRENT_SOURCE_DIR}/bin/apydep.py -o ${_pydot} -p ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/packages.txt ${CMAKE_SOURCE_DIR}/../../ )
    
       add_custom_target( build_pydeps ALL DEPENDS ${_pydot} )
    
       # Install output if available:
       install( FILES ${_pydot} DESTINATION . OPTIONAL )
    endif()
endif()
