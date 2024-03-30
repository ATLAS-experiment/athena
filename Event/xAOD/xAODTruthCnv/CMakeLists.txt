################################################################################
# Package: xAODTruthCnv
################################################################################

# Declare the package name:
atlas_subdir( xAODTruthCnv )

# Component(s) in the package:
atlas_add_library( xAODTruthCnvLib
   xAODTruthCnv/*.h Root/*.cxx
   PUBLIC_HEADERS xAODTruthCnv
   LINK_LIBRARIES AsgTools xAODTruth TruthUtils)

if( NOT XAOD_STANDALONE )
   atlas_add_component( xAODTruthCnv
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AthLinks AthenaBaseComps StoreGateLib EventInfo
      xAODEventInfo xAODTruth GaudiKernel GeneratorObjects AthenaPoolUtilities IOVDbDataModel
      xAODTruthCnvLib TruthUtils)
endif()

atlas_add_dictionary( xAODTruthCnvDict
   xAODTruthCnv/xAODTruthCnvDict.h
   xAODTruthCnv/selection.xml
   LINK_LIBRARIES xAODTruthCnvLib )

# Install files from the package:
atlas_install_scripts( scripts/dumpTruth.py  )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
