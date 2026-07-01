/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Include the full OnnxRuntime header here so that Ort::Session is a complete
// type when unique_ptr's destructor is instantiated. This keeps the heavy
// onnxruntime headers out of the public OnnxNNCollection.h.
#include <onnxruntime_cxx_api.h>
#include "SiClusterizationTool/OnnxNNCollection.h"

OnnxNNCollection::~OnnxNNCollection() = default;
