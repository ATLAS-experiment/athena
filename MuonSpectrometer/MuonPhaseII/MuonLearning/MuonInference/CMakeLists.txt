# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonInference
################################################################################

# Declare the package name:
atlas_subdir( MuonInference )


find_package( onnxruntime REQUIRED)

atlas_add_component( MuonInference
                     src/components/*.cxx src/*.cxx 
                     LINK_LIBRARIES AthenaKernel StoreGateLib  MuonInferenceInterfaces PathResolver
                                    AthOnnxInterfaces  ${ONNXRUNTIME_LIBRARIES})

atlas_install_python_modules( python/*.py)

