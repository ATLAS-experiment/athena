# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonInference )


find_package( onnxruntime )
find_package( fmt )

atlas_add_component(    MuonInference
                        src/components/*.cxx src/*.cxx
                        INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS}
                        LINK_LIBRARIES  AthenaKernel StoreGateLib  PathResolver
                                        AthenaBaseComps GeoPrimitives MuonIdHelpersLib
                                        MuonSimEvent xAODMuonSimHit  MdtCalibInterfacesLib
                                        xAODMuonPrepData MuonSpacePoint xAODMuon MuonTesterTreeLib
                                        MuonPatternEvent MuonTrackEvent MuonPatternHelpers MuonPatternHelpers
                                        CaloEvent FourMomUtils xAODEventInfo
                                        MuonInferenceInterfaces 
                                        AthOnnxInterfaces AthOnnxCompsLib  ${ONNXRUNTIME_LIBRARIES} ${FMT_LIBRARIES}
                    )

atlas_install_python_modules( python/*.py)

file(GLOB _MUONINF_MODELS_ONNX "${CMAKE_CURRENT_SOURCE_DIR}/models/*.onnx")
if(_MUONINF_MODELS_ONNX)
  atlas_install_data(models/*.onnx)
endif()

atlas_add_test( testMuonBucketInference
                SCRIPT python -m MuonInference.muonBucketInference --nEvents 5 --use-cpu
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                LOG_IGNORE_PATTERN "PathResolver.*ERROR Locating dev file dev/MuonRecRTT/edgecnn_mu200\\.onnx\\. Do not let this propagate to a release!"
                POST_EXEC_SCRIPT noerror.sh)