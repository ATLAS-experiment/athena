# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonInference
################################################################################

# Declare the package name:
atlas_subdir( MuonInference )


find_package( onnxruntime REQUIRED)

atlas_add_component(    MuonInference
                        src/components/*.cxx src/*.cxx 
                        LINK_LIBRARIES  AthenaKernel StoreGateLib  PathResolver
                                        AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                        MuonSimEvent xAODMuonSimHit  MdtCalibInterfacesLib
                                        xAODMuonPrepData MuonSpacePoint xAODMuon MuonTesterTreeLib
                                        MuonPatternEvent MuonPatternHelpers MuonPatternHelpers 
                                        MuonInferenceInterfaces
                                        AthOnnxInterfaces  ${ONNXRUNTIME_LIBRARIES}
                    )

atlas_install_python_modules( python/*.py)

file(GLOB _MUONINF_MODELS_ONNX "${CMAKE_CURRENT_SOURCE_DIR}/models/*.onnx")
if(_MUONINF_MODELS_ONNX)
  atlas_install_data(models/*.onnx)
endif()
