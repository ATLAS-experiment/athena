/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SALT_MODEL_OPTIONS_H
#define SALT_MODEL_OPTIONS_H

#include <string>

namespace FlavorTagInference {
  // Options for the onnx session. The defaults give the CPU session that
  // has always been built here.
  struct SaltModelOptions {
    std::string execution_provider = "CPU";
    int device_id = 0;
    bool use_tf32 = false;
  };
}

#endif
