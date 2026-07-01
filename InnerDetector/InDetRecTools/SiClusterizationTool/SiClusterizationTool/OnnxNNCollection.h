/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SICLUSTERIZATIONTOOL_ONNXNNCOLLECTION_H
#define SICLUSTERIZATIONTOOL_ONNXNNCOLLECTION_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include <memory>

// Forward declaration to avoid propagating onnxruntime headers to downstream packages.
// The destructor is defined in OnnxNNCollection.cxx where the full type is available.
namespace Ort { struct Session; }

struct OnnxNNCollection {
  ~OnnxNNCollection();
  std::unique_ptr<Ort::Session> numberNetwork;
  std::unique_ptr<Ort::Session> positionNetwork1;
  std::unique_ptr<Ort::Session> positionNetwork2;
  std::unique_ptr<Ort::Session> positionNetwork3;
};

CLASS_DEF(OnnxNNCollection, 1196174450, 1)
CONDCONT_DEF(OnnxNNCollection, 1226994230);

#endif
