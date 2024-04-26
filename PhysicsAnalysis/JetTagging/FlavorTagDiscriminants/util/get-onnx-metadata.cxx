/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <cstdint>

int main(int narg, char* argv[]) {
  if (narg != 3 && narg != 2) {
    std::cout << "usage: " << argv[0] << " <onnx_file> [key]" << std::endl;
    return 1;
  }

  //load the onnx model to memory using the path
  auto env = std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_ERROR, "");

  // initialize session options if needed
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(
    GraphOptimizationLevel::ORT_DISABLE_ALL);

  // create session and load model into memory
  auto session = std::make_unique< Ort::Session >(*env, argv[1],
                                                session_options);

  // get metadata
  Ort::AllocatorWithDefaultOptions allocator;
  Ort::ModelMetadata metadata = session->GetModelMetadata();
  if (narg == 2) {
    std::cout << "keys: ";
    auto keys = metadata.GetCustomMetadataMapKeysAllocated(allocator);
    for (uint64_t i = 0; i < keys.size(); i++) {
      std::cout << keys[i].get();
      if (i+1 < keys.size()) std::cout << ", ";
    }
    std::cout << std::endl;
    return 2;
  }
  auto val = metadata.LookupCustomMetadataMapAllocated(argv[2], allocator);
  std::cout << val.get() << std::endl;

  return 0;
}
