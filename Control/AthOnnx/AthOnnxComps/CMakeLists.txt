# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthOnnxComps )

# External dependencies.
find_package( onnxruntime )

# AthenaBaseComps if in Athena
if (XAOD_STANDALONE)
  set(COND_ATHENA_BASE_COMPS "")
else()
  set(COND_ATHENA_BASE_COMPS AthenaBaseComps)
endif()

# Library in the package.
atlas_add_library( AthOnnxCompsLib
   src/*.cxx
   PUBLIC_HEADERS AthOnnxComps
   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AsgTools AsgServicesLib AthOnnxInterfaces AthOnnxUtilsLib ${COND_ATHENA_BASE_COMPS} PathResolver
)

atlas_add_dictionary( AthOnnxCompsDict
    AthOnnxComps/AthOnnxCompsDict.h
    AthOnnxComps/selection.xml
    LINK_LIBRARIES AthOnnxCompsLib 
)

if (NOT XAOD_STANDALONE)
   atlas_add_component( AthOnnxComps
   src/components/*.cxx
   LINK_LIBRARIES AthOnnxCompsLib
)
endif()


# install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
